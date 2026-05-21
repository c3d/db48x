/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2023  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : IP_OS_embOS.c
Purpose : Kernel abstraction for embOS. Do not modify to allow easy updates!
*/

#include "IP.h"
#include "RTOS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// The default tick is expected to be 1ms. For a finer tick
// like 1us a multiplicator has to be configured. The tick
// should match the OS tick.
// Examples:
//   - 1ms   = 1
//   - 100us = 10
//   - 10us  = 100
//
#define TICK_MULTIPLICATOR  1  // Default, 1 = 1ms.

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef struct TCP_WAIT_STRUCT TCP_WAIT;
struct TCP_WAIT_STRUCT {
  TCP_WAIT* pNext;
  TCP_WAIT* pPrev;
  void*     pWaitItem;
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
  OS_TASK*  pTask;
#endif
  OS_EVENT  Event;
};

typedef struct {
  TCP_WAIT*            volatile pTCPWait;          // Head of List. One entry per waiting task
#if (OS_VERSION >= 38606)                          // OS_AddOnTerminateHook() is supported since embOS v3.86f .
  OS_ON_TERMINATE_HOOK          OnTerminateTaskHook;
#endif
#if (IP_ALLOW_DEINIT && (OS_VERSION >= 43000))
  IP_ON_EXIT_HOOK               OnExitHook;
#endif
#if IP_DEBUG
  OS_PRIO                       MaxNonRxTaskPrio;  // The highest task priority seen is saved upon IP_OS_Delay()
                                                   // and wait event calls that are not from the RxTask.
                                                   // The RxTask then checks its own priority to be higher
                                                   // than the highest priority seen from other IP tasks.
#endif
  U8                            IsInited;
} IP_OS_STATIC;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

#define _Static     _IP_OS_Static
static IP_OS_STATIC _IP_OS_Static;

#if (IP_VERSION >= 34300)
static const IP_OS_API _IP_OS_Api = {
  35000u  // IP_VERSION in which this IP_OS layer was last "updated".
          // All IP_OS layers share the same "updated" version.
          // This version is checked by the stack to detect if any
          // important changes/bugfixes that have been published via
          // the official IP_OS layers might be missing due to an
          // outdated IP_OS layer.
};
#endif

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

OS_RSEMA IP_OS_RSema;       // Public only to allow inlining (direct call from IP-Stack).
OS_EVENT IP_OS_EventRx;     // Public only to allow inlining (direct call from IP-Stack).
OS_TASK* IP_OS_pIPTask;     // Public only to allow inlining (direct call from IP-Stack).
                            // Also first task that has called IP_OS_WaitNetEventTimed() .
                            // Is checked to remain the same task as multiple tasks
                            // calling IP_TASK_WaitForEvent() is not supported.

#if (IP_VERSION >= 34300)
OS_EVENT IP_OS_EventDTask;  // Public only to allow inlining (direct call from IP-Stack).
#else
OS_EVENT IP_OS_EventWiFi;   // Public only to allow inlining (direct call from IP-Stack).
#endif

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _DLIST_RemoveDelete()
*
*  Function description
*    Removes a waitable object from the doubly linked list and deletes
*    its wait object from embOS lists.
*
*  Parameters
*    pTCPWait: Item to remove.
*
*  Additional information
*    Function is called from IP_OS_WaitItemTimed() and _IP_OS_cbOnTerminateTask().
*    Calling functions have to make sure that it is not called recursive
*    by disabling task switch before calling this routine.
*/
static void _DLIST_RemoveDelete(TCP_WAIT* pTCPWait) {
  //
  // Remove entry from doubly linked list.
  //
  if (pTCPWait->pPrev) {
    pTCPWait->pPrev->pNext = pTCPWait->pNext;
  } else {
    _Static.pTCPWait = pTCPWait->pNext;
  }
  if (pTCPWait->pNext) {
    pTCPWait->pNext->pPrev = pTCPWait->pPrev;
  }
  //
  // Delete the event object.
  //
  OS_EVENT_Set(&pTCPWait->Event);  // Set event to prevent error on removing an unsignalled event.
  OS_EVENT_Delete(&pTCPWait->Event);
}

#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
/*********************************************************************
*
*       _IP_OS_cbOnTerminateTask()
*
*  Function description
*    This routine removes the registered wait objects from the doubly
*    linked list of wait objects upon termination of its task. This
*    is necessary due to the fact that the element is publically known
*    due to a doubly linked list but is stored on a task stack. In case
*    this task gets terminated we need to gracefully remove the element
*    from all resources and even remove it from any embOS list.
*
*  Parameters
*    pTask: Task handle of task that will be terminated.
*
*  Additional information
*    Function is called from an application task via OS hook with
*    task switching disabled.
*/
static void _IP_OS_cbOnTerminateTask(OS_CONST_PTR OS_TASK* pTask) {
  TCP_WAIT* pTCPWait;

  for (pTCPWait = _Static.pTCPWait; pTCPWait; pTCPWait = pTCPWait->pNext) {
    if (pTCPWait->pTask == pTask) {
      //
      // Prior to deleting an event object it needs to be set to be unused
      // (no task waiting for it). Setting the EVENT object is safe as in
      // all cases only one the task that created the object on its stack
      // is waiting for the event and task switching is disabled. Therefore
      // we will stay in this routine and finish our work.
      //
      OS_EVENT_Set(&pTCPWait->Event);
      _DLIST_RemoveDelete(pTCPWait);
      break;
    }
  }
}
#endif

#if (IP_ALLOW_DEINIT && (OS_VERSION >= 43000))
/*********************************************************************
*
*       _IP_OS_cbOnExit()
*
*  Function description
*    Called in case of a de-initialization of the stack.
*/
static void _IP_OS_cbOnExit(void) {
  OS_EVENT_Set(&IP_OS_EventRx);
  OS_EVENT_Delete(&IP_OS_EventRx);
#if (IP_VERSION >= 34300)
  OS_EVENT_Set(&IP_OS_EventDTask);
  OS_EVENT_Delete(&IP_OS_EventDTask);
#else
  OS_EVENT_Set(&IP_OS_EventWiFi);
  OS_EVENT_Delete(&IP_OS_EventWiFi);
#endif
  OS_DeleteRSema(&IP_OS_RSema);
  OS_RemoveTerminateHook(&_Static.OnTerminateTaskHook);
  IP_OS_pIPTask    = NULL;
  IP_MEMSET(&_Static, 0, sizeof(_Static));
}
#endif

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

#if (IP_VERSION >= 34300)
/*********************************************************************
*
*       IP_OS_Init()
*
*  Function description
*    Initialize (create) all objects required for task synchronization.
*    These are 3 events (for IP_Task, IP_RxTask and DriverTask)
*    and one semaphore for protection of critical code which may not
*    be executed from multiple tasks at the same time.
*
*  Return value
*    Pointer to the IP_OS API table.
*/
const IP_OS_API* IP_OS_Init(void) {
  if (_Static.IsInited == 0u) {
    OS_CREATERSEMA(&IP_OS_RSema);
    OS_EVENT_Create(&IP_OS_EventRx);
#if (IP_VERSION >= 34300)
    OS_EVENT_Create(&IP_OS_EventDTask);
#else
    OS_EVENT_Create(&IP_OS_EventWiFi);
#endif
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
    OS_AddOnTerminateHook(&_Static.OnTerminateTaskHook, _IP_OS_cbOnTerminateTask);  // Has been renamed to OS_AddTerminateHook() with embOS v4.30 using a compatibility macro.
#endif
#if (IP_ALLOW_DEINIT && (OS_VERSION >= 43000))
    //
    // OS_RemoveTerminateHook() is supported since embOS v4.30 .
    // Only add an OnExit hook if we are able to remove the OS hook
    // as well as otherwise we would use OS_AddTerminateHook()
    // twice due to _Static.IsInited != 0u .
    //
    IP_AddOnExitHandler(&_Static.OnExitHook, _IP_OS_cbOnExit);
#endif
    _Static.IsInited = 1u;
  }
  return &_IP_OS_Api;
}

#else

/*********************************************************************
*
*       IP_OS_Init()
*
*  Function description
*    Initialize (create) all objects required for task synchronization.
*    These are 3 events (for IP_Task, IP_RxTask and WiFi task)
*    and one semaphore for protection of critical code which may not
*    be executed from multiple tasks at the same time.
*/
void IP_OS_Init(void) {
  if (_Static.IsInited == 0u) {
    OS_CREATERSEMA(&IP_OS_RSema);
    OS_EVENT_Create(&IP_OS_EventRx);
#if (IP_VERSION >= 34300)
    OS_EVENT_Create(&IP_OS_EventDTask);
#else
    OS_EVENT_Create(&IP_OS_EventWiFi);
#endif
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
    OS_AddOnTerminateHook(&_Static.OnTerminateTaskHook, _IP_OS_cbOnTerminateTask);  // Has been renamed to OS_AddTerminateHook() with embOS v4.30 using a compatibility macro.
#endif
#if (IP_ALLOW_DEINIT && (OS_VERSION >= 43000))
    //
    // OS_RemoveTerminateHook() is supported since embOS v4.30 .
    // Only add an OnExit hook if we are able to remove the OS hook
    // as well as otherwise we would use OS_AddTerminateHook()
    // twice due to _Static.IsInited != 0u .
    //
    IP_AddOnExitHandler(&_Static.OnExitHook, _IP_OS_cbOnExit);
#endif
    _Static.IsInited = 1u;
  }
}
#endif

/*********************************************************************
*
*       IP_OS_DisableInterrupt()
*
*  Function description
*    Disables interrupts to lock against calls from interrupt routines.
*/
void IP_OS_DisableInterrupt(void) {
  OS_IncDI();
}

/*********************************************************************
*
*       IP_OS_EnableInterrupt()
*
*  Function description
*    Enables interrupts that have previously been disabled.
*/
void IP_OS_EnableInterrupt(void) {
  OS_DecRI();
}

/*********************************************************************
*
*       IP_OS_GetTime32()
*
*  Function description
*    Return the current system time in ms;
*    The value will wrap around after approximately 49.7 days; This
*    is taken into account by the stack.
*
*  Return value
*    U32 timestamp in system ticks (typically 1ms).
*/
U32 IP_OS_GetTime32(void) {
  return OS_GetTime32();
}

/*********************************************************************
*
*       IP_OS_Delay()
*
*  Function description
*    Blocks the calling task for a given time.
*
*  Parameters
*    ms: Time to block in system ticks (typically 1ms).
*
*  Notes
*    (1) Do not save the task priority for RxTask prio checking here
*        as this routine might be used in applications in non IP
*        related tasks instead of the plain OS delay call.
*/
void IP_OS_Delay(unsigned ms) {
#if (TICK_MULTIPLICATOR != 1)
  ms *= TICK_MULTIPLICATOR;
#endif
  OS_Delay(ms + 1);
}

#if (IP_VERSION >= 34300)
/*********************************************************************
*
*       IP_OS_WaitNetEventTimed()
*
*  Function description
*    Called from IP_Task() only or alternatively IP_TASK_WaitForEvent() .
*    Blocks until the timeout expires or a NET-event occurs, meaning
*    IP_OS_SignalNetEvent() is called from an other task or ISR.
*
*  Parameters
*    Timeout: Time [ms] to wait for an event to be signaled. 0 means
*             infinite wait.
*
*  Return value
*    == 0: An event was signaled.
*    != 0: Timeout.
*
*  Additional information
*    For the IP_Task() task events are used as they are using less
*    overhead and are slightly faster using event objects. However
*    as the task control block needs to be known the following rare
*    case might occur:
*
*      - IP_Init() is done and Rx interrupts are enabled.
*
*      - IP_Task() which runs a loop of IP_Exec();
*        IP_OS_WaitNetEventTimed() gets interrupted before
*        IP_OS_WaitNetEventTimed() can set IP_OS_pIPTask .
*
*      - The Rx interrupt now calls IP_OS_SignalNetEvent() but
*        actually does not signal anything as IP_OS_pIPTask is
*        still NULL.
*
*      - One interrupt might get lost which is not a problem
*        for the IP_Task() as it will get woken up regularly
*        anyhow or at least with the second Rx event.
*/
unsigned IP_OS_WaitNetEventTimed(unsigned Timeout) {
  unsigned r;
#if IP_DEBUG
  OS_PRIO  Prio;

  //
  // Save the highest task priority to be able to
  // compare it against the RxTask priority.
  //
  Prio = OS_GetPriority(NULL);
  if (Prio > _Static.MaxNonRxTaskPrio) {
    _Static.MaxNonRxTaskPrio = Prio;
  }
  //
  // Check if we are called from different tasks at any time.
  // This might happen if the application uses IP_TASK_Exec()
  // and IP_TASK_WaitForEvent() instead of IP_Task() . Due to
  // using a task event it is not intended to call this routine
  // from different tasks.
  //
  if (IP_OS_pIPTask != NULL) {  // Set before aka not the first call ?
    if (OS_GetTaskID() != IP_OS_pIPTask) {
      IP_PANIC("IP_OS_WaitNetEventTimed() called from different tasks.");
    }
  }
#endif
#if (TICK_MULTIPLICATOR != 1)
  Timeout *= TICK_MULTIPLICATOR;
#endif
  //
  // OS_GetpCurrentTask() is a macro returning a variable.
  // Keep outside of the "IP_OS_pIPTask != NULL" check as
  // it does not really cost performance and we would be
  // able to see the last task that has called this routine
  // in a release build.
  //
  IP_OS_pIPTask = OS_GetpCurrentTask();
  if (Timeout == 0u) {
    r = OS_WaitEvent(1u);
  } else {
    r = OS_WaitEventTimed(1u, Timeout);
  }
  //
  // Negate the result to stick with EVENT object result
  // format for all WaitForEvent API.
  //
  r ^= 1u;
  return r;
}

#else

/*********************************************************************
*
*       IP_OS_WaitNetEvent()
*
*  Function description
*    Called from IP_Task() only.
*    Blocks until the timeout expires or a NET-event occurs, meaning
*    IP_OS_SignalNetEvent() is called from an other task or ISR.
*
*  Parameters
*    Timeout: Timeout for waiting in system ticks (typically 1ms).
*
*  Additional information
*    For the IP_Task() task events are used as they are using less
*    overhead and are slightly faster using event objects. However
*    as the task control block needs to be known the following rare
*    case might occur:
*
*      - IP_Init() is done and Rx interrupts are enabled.
*
*      - IP_Task() which runs a loop of IP_Exec(); IP_OS_WaitNetEvent()
*        gets interrupted before IP_OS_WaitNetEvent() can set
*        IP_OS_pIPTask .
*
*      - The Rx interrupt now calls IP_OS_SignalNetEvent() but
*        actually does not signal anything as IP_OS_pIPTask is
*        still NULL.
*
*      - One interrupt might get lost which is not a problem
*        for the IP_Task() as it will get woken up regularly
*        anyhow or at least with the second Rx event.
*/
void IP_OS_WaitNetEvent(unsigned Timeout) {
#if IP_DEBUG
  OS_PRIO Prio;

  //
  // Save the highest task priority to be able to
  // compare it against the RxTask priority.
  //
  Prio = OS_GetPriority(NULL);
  if (Prio > _Static.MaxNonRxTaskPrio) {
    _Static.MaxNonRxTaskPrio = Prio;
  }
  //
  // Check if we are called from different tasks at any time.
  // This might happen if the application uses IP_TASK_Exec()
  // and IP_TASK_WaitForEvent() instead of IP_Task() . Due to
  // using a task event it is not intended to call this routine
  // from different tasks.
  //
  if (IP_OS_pIPTask != NULL) {  // Set before aka not the first call ?
    if (OS_GetTaskID() != IP_OS_pIPTask) {
      IP_PANIC("IP_OS_WaitNetEvent() called from different tasks");
    }
  }
#endif
#if (TICK_MULTIPLICATOR != 1)
  Timeout *= TICK_MULTIPLICATOR;
#endif
  //
  // OS_GetpCurrentTask() is a macro returning a variable.
  // Keep outside of the "IP_OS_pIPTask != NULL" check as
  // it does not really cost performance and we would be
  // able to see the last task that has called this routine
  // in a release build.
  //
  IP_OS_pIPTask = OS_GetpCurrentTask();
  (void)OS_WaitEventTimed(1u, Timeout);
}
#endif

/*********************************************************************
*
*       IP_OS_SignalNetEvent()
*
*  Function description
*    Wakes the IP_Task if it is waiting for a NET-event or timeout in
*    the function IP_OS_WaitNetEventTimed().
*/
void IP_OS_SignalNetEvent(void) {
  if (IP_OS_pIPTask != NULL) {
    OS_SignalEvent(1u, IP_OS_pIPTask);
  }
}

#if (IP_VERSION >= 34300)
/*********************************************************************
*
*       IP_OS_WaitRxEventTimed()
*
*  Function description
*    Called whenever the RxTask handling is idle (no more packets in
*    the In-FIFO).
*
*  Parameters
*    Timeout: Time [ms] to wait for an event to be signaled. 0 means
*             infinite wait.
*
*  Return value
*    == 0: An event was signaled.
*    != 0: Timeout.
*
*  Additional information
*    See IP_OS_WaitNetEventTimed() regarding problems that might occur
*    when using task events. The same problematic applies to the
*    IP_RxTask() but as this is treated like an interrupt we do not
*    use a fallback activation each x seconds when being called from
*    IP_RxTask() . Therefore we use event objects in all other cases
*    which are always valid after IP_Init() and can be signaled before
*    being waited on. In this case the wait will simply run through
*    this time.
*    Instead of using the IP_RxTask() as an alternative the
*    application can ask us to wait with a timeout to be able to
*    do some other things in-between actual events.
*/
unsigned IP_OS_WaitRxEventTimed(unsigned Timeout) {
  unsigned r;
#if IP_DEBUG
  OS_PRIO  Prio;

  //
  // Check if we have seen any other IP related task with
  // a priority higher than the RxTask. This violates our
  // API locking and might cause hard to identify problems.
  //
  Prio = OS_GetPriority(NULL);
  if (Prio <= _Static.MaxNonRxTaskPrio) {
    IP_PANIC("RxTask has not the highest IP task priority.");
  }
#endif
  if (Timeout == 0u) {
    OS_EVENT_Wait(&IP_OS_EventRx);
    r = 0u;  // Event signaled.
  } else {
    r = OS_EVENT_WaitTimed(&IP_OS_EventRx, Timeout);
  }
  return r;
}

#else

/*********************************************************************
*
*       IP_OS_WaitRxEvent()
*
*  Function description
*    Called whenever the RxTask handling is idle (no more packets in
*    the In-FIFO).
*
*  Additional information
*    See IP_OS_WaitNetEvent() regarding problems that might occur
*    when using task events. The same problematic applies to the
*    IP_RxTask() but as this is treated like an interrupt we do not
*    use a fallback activation each x seconds. Therefore we use event
*    objects in all other cases which are always valid after
*    IP_Init() and can be signaled before being waited on. In this
*    case the wait will simply run through this time.
*/
void IP_OS_WaitRxEvent(void) {
#if IP_DEBUG
  OS_PRIO Prio;

  //
  // Check if we have seen any other IP related task with
  // a priority higher than the RxTask. This violates our
  // API locking and might cause hard to identify problems.
  //
  Prio = OS_GetPriority(NULL);
  if (Prio <= _Static.MaxNonRxTaskPrio) {
    IP_PANIC("RxTask has not the highest IP prio");
  }
#endif
  OS_EVENT_Wait(&IP_OS_EventRx);
}
#endif

/*********************************************************************
*
*       IP_OS_SignalRxEvent()
*
*  Function description
*    Called by an interrupt to signal that the RxTask handling needs
*    to check for new received packets.
*/
void IP_OS_SignalRxEvent(void) {
  OS_EVENT_Set(&IP_OS_EventRx);
}

#if (IP_VERSION >= 34300)
/*********************************************************************
*
*       IP_OS_WaitDTaskEventTimed()
*
*  Function description
*    Called whenever the DTask handling is idle (no more events to
*    handle).
*
*  Parameters
*    Timeout: Time [ms] to wait for an event to be signaled. 0 means
*             infinite wait.
*
*  Return value
*    == 0: An event was signaled.
*    != 0: Timeout.
*
*  Additional information
*    See IP_OS_WaitRxEventTimed() regarding task event vs. event
*    object usage.
*/
unsigned IP_OS_WaitDTaskEventTimed(unsigned Timeout) {
  unsigned r;
#if IP_DEBUG
  OS_PRIO  Prio;

  //
  // Save the highest task priority to be able to
  // compare it against the RxTask priority.
  //
  Prio = OS_GetPriority(NULL);
  if (Prio > _Static.MaxNonRxTaskPrio) {
    _Static.MaxNonRxTaskPrio = Prio;
  }
#endif

  if (Timeout == 0u) {
    OS_EVENT_Wait(&IP_OS_EventDTask);
    r = 0u;  // Event signaled.
  } else {
    r = OS_EVENT_WaitTimed(&IP_OS_EventDTask, Timeout);
  }
  return r;
}

/*********************************************************************
*
*       IP_OS_SignalDTaskEvent()
*
*  Function description
*    Called by an interrupt from an external module to signal that an
*    events needs to be handled by the DTask.
*/
void IP_OS_SignalDTaskEvent(void) {
  OS_EVENT_Set(&IP_OS_EventDTask);
}

#else

/*********************************************************************
*
*       IP_OS_WaitWiFiEventTimed()
*
*  Function description
*    Called whenever the WiFi task handling is idle (no more events to
*    handle).
*
*  Parameters
*    Timeout: Time [ms] to wait for an event to be signaled. 0 means
*             infinite wait.
*
*  Additional information
*    See IP_OS_WaitRxEvent() regarding task event vs. event object
*    usage.
*/
void IP_OS_WaitWiFiEventTimed(unsigned Timeout) {
#if IP_DEBUG
  OS_PRIO Prio;

  //
  // Save the highest task priority to be able to
  // compare it against the RxTask priority.
  //
  Prio = OS_GetPriority(NULL);
  if (Prio > _Static.MaxNonRxTaskPrio) {
    _Static.MaxNonRxTaskPrio = Prio;
  }
#endif
  if (Timeout == 0) {
    OS_EVENT_Wait(&IP_OS_EventWiFi);
  } else {
    (void)OS_EVENT_WaitTimed(&IP_OS_EventWiFi, Timeout);
  }
}

/*********************************************************************
*
*       IP_OS_SignalWiFiEvent()
*
*  Function description
*    Called by an interrupt from an external WiFi module to signal
*    that the IP_WIFI_IsrTask() needs to handle a WiFi event.
*/
void IP_OS_SignalWiFiEvent(void) {
  OS_EVENT_Set(&IP_OS_EventWiFi);
}
#endif

/*********************************************************************
*
*       IP_OS_Lock()
*
*  Function description
*    The stack requires a single lock, typically a resource semaphore
*    or mutex; This function locks this object, guarding sections of
*    the stack code against other threads;
*/
void IP_OS_Lock(void) {
  (void)OS_Use(&IP_OS_RSema);
}

/*********************************************************************
*
*       IP_OS_Unlock()
*
*  Function description
*    Unlocks the single lock, locked by a previous call to IP_OS_Lock()
*    and signals the IP_Task() if a packet has been freed.
*/
void IP_OS_Unlock(void) {
  int Status;

  //
  // Read the current lock count before unlocking to prevent
  // directly being locked again by a higher priority task.
  //
  Status = OS_GetSemaValue(&IP_OS_RSema);
  OS_Unuse(&IP_OS_RSema);
  //
  // If this was the last unlock, signal the IP_Task().
  //
  if ((Status - 1) == 0) {
    IP_SignalIfPacketFreeUsed();
  }
}

/*********************************************************************
*
*       IP_OS_AssertLock()
*
*  Function description
*    Makes sure that the lock is in use. Called in debug builds only.
*/
void IP_OS_AssertLock(void) {
  if (IP_OS_RSema.UseCnt == 0) {
    for (;;) {
      // Allows setting a breakpoint here.
    }
  } else {
    if (IP_OS_RSema.pTask != OS_GetpCurrentTask()) {
      for (;;) {
        // Allows setting a breakpoint here.
      }
    }
  }
}

#if (IP_VERSION < 34300)
/*********************************************************************
*
*       IP_OS_WaitItem()
*
*  Function description
*    Suspend a task which needs to wait for an object;
*    This object is identified by a pointer to it and can be of any
*    type, for example socket.
*
*  Parameters
*    pWaitItem: Item to wait for.
*
*  Additional information
*    Function is called from an application task.
*/
void IP_OS_WaitItem(void* pWaitItem) {
  IP_OS_WaitItemTimed(pWaitItem, 0u);
}

/*********************************************************************
*
*       IP_OS_WaitItemTimed()
*
*  Function description
*    Suspend a task which needs to wait for a object;
*    This object is identified by a pointer to it and can be of any
*    type, for example a socket.
*
*  Parameters
*    pWaitItem: Item to wait for.
*    Timeout  : Timeout for waiting in system ticks (typically 1ms).
*
*  Additional information
*    Function is called from an application task and is locked in
*    every case.
*/
void IP_OS_WaitItemTimed(void* pWaitItem, unsigned Timeout) {

#else

/*********************************************************************
*
*       IP_OS_WaitItemTimed()
*
*  Function description
*    Suspend a task which needs to wait for a object;
*    This object is identified by a pointer to it and can be of any
*    type, for example a socket.
*
*  Parameters
*    pWaitItem: Item to wait for.
*    Timeout  : Time [ms] to wait for an event to be signaled. 0 means
*               infinite wait.
*
*  Return value
*    == 0: An event was signaled.
*    != 0: Timeout.
*
*  Additional information
*    Function is called from an application task and is locked in
*    every case.
*/
unsigned IP_OS_WaitItemTimed(void* pWaitItem, unsigned Timeout) {
  unsigned r;
#endif
  TCP_WAIT TCPWait;
#if ((IP_DEBUG != 0) && (IP_VERSION >= 34300))
  OS_PRIO  Prio;

  //
  // Save the highest task priority to be able to
  // compare it against the RxTask priority.
  //
  Prio = OS_GetPriority(NULL);
  if (Prio > _Static.MaxNonRxTaskPrio) {
    _Static.MaxNonRxTaskPrio = Prio;
  }
#endif

#if (TICK_MULTIPLICATOR != 1)
  Timeout *= TICK_MULTIPLICATOR;
#endif
  //
  // Create the wait object which contains the OS-Event object.
  //
  TCPWait.pWaitItem = pWaitItem;
  OS_EVENT_Create(&TCPWait.Event);
  //
  // Add to beginning of doubly-linked list.
  //
  TCPWait.pPrev = NULL;
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
  TCPWait.pTask = OS_GetpCurrentTask();
  OS_EnterRegion();        // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
#endif
  TCPWait.pNext          = _Static.pTCPWait;
  _Static.pTCPWait       = &TCPWait;
  if (TCPWait.pNext != NULL) {
    TCPWait.pNext->pPrev = &TCPWait;
  }
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
  //
  // Unlock mutex.
  //
  IP_OS_UNLOCK();
  //
  //  Suspend this task.
  //
#if (IP_VERSION >= 34300)
  if (Timeout == 0) {
    OS_EVENT_Wait(&TCPWait.Event);
    r = 0u;  // Event signaled.
  } else {
    r = OS_EVENT_WaitTimed(&TCPWait.Event, Timeout);
  }
#else
  if (Timeout == 0) {
    OS_EVENT_Wait(&TCPWait.Event);
  } else {
    (void)OS_EVENT_WaitTimed(&TCPWait.Event, Timeout);
  }
#endif
  //
  // Lock the mutex again.
  //
  (void)IP_OS_LOCK();
  //
  // Remove it from doubly linked list and delete event object.
  //
#if (OS_VERSION >= 38606)  // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
  OS_EnterRegion();
#endif
  _DLIST_RemoveDelete(&TCPWait);
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
#if (IP_VERSION >= 34300)
  return r;
#endif
}

/*********************************************************************
*
*       IP_OS_SignalItem()
*
*  Function description
*    Sets an object to signaled state, or resumes tasks which are
*    waiting at the event object.
*
*  Parameters
*    pWaitItem: Item to signal.
*
*  Additional information
*    Function is called from a task, not an ISR and is locked in
*    every case.
*/
void IP_OS_SignalItem(void* pWaitItem) {
  TCP_WAIT* pTCPWait;

#if (OS_VERSION >= 38606)  // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
  OS_EnterRegion();
#endif
  for (pTCPWait = _Static.pTCPWait; pTCPWait; pTCPWait = pTCPWait->pNext) {
    if (pTCPWait->pWaitItem == pWaitItem) {
      OS_EVENT_Set(&pTCPWait->Event);
    }
  }
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
}

/*********************************************************************
*
*       IP_OS_AddTickHook()
*
*  Function description
*    Add tick hook. This is a function which is called from the tick
*    handler, typically because the driver's interrupt handler is not
*    called via it's own hardware ISR. (We poll 1000 times per second)
*
*  Parameters
*    pfHook: Callback to be called on every tick.
*
*  Additional information
*    Function is called from a task, not an ISR.
*/
void IP_OS_AddTickHook(void (*pfHook)(void)) {
#if (OS_VERSION >= 36000)
  static OS_TICK_HOOK _cb;
  OS_AddTickHook(&_cb, pfHook);
#else
  IP_PANIC("IP_OS_AddTickHook() requires an OS version >= 3.60");  // This requires a newer version of the OS.
#endif
}

/*********************************************************************
*
*       IP_OS_GetTaskName()
*
*  Function description
*    Retrieves the task name (if available from the OS and not in
*    interrupt) for the currently active task.
*
*  Parameters
*    pTask: Pointer to a task identifier such as a task control block.
*
*  Return value
*    Terminated string with task name.
*/
const char* IP_OS_GetTaskName(void* pTask) {
  return OS_GetTaskName((OS_TASK*)pTask);
}

/*************************** End of file ****************************/
