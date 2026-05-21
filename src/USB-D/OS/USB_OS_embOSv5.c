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
----------------------------------------------------------------------
Purpose : Kernel abstraction for embOS version >= 5.0
-------------------------- END-OF-HEADER -----------------------------
*/


#include "USB.h"
#include "RTOS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
//
// Tick rate of the OS system tick counter in Hz
//
#ifndef USB_OS_TICK_RATE_HZ
  #define USB_OS_TICK_RATE_HZ     1000u
#endif

#if !defined(USB_IS_IN_INT) && USBD_OS_USE_USBD_X_INTERRUPT > 0
  #if USBD_OS_USE_ISR_FLAG
    #define USB_IS_IN_INT()   (USBD_IsInInterrupt != 0)
  #else
    #define USB_IS_IN_INT()   (OS_INT_InInterrupt() != 0)
  #endif
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_MAILBOX _aMailBox[USB_NUM_EPS + USB_EXTRA_EVENTS];
static U32        _aMBBuffer[USB_NUM_EPS + USB_EXTRA_EVENTS];
#if USBD_OS_USE_USBD_X_INTERRUPT > 0
  static OS_MUTEX _Sema;
#endif
#if USB_NUM_MUTEXES > 0
  static OS_MUTEX _aMutex[USB_NUM_MUTEXES];
  static I8       _NumUsedMutexes;
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       USB_OS_Init
*
*  Function description
*    This function initializes all OS objects that are necessary.
*/
void USB_OS_Init(void) {
  unsigned i;

#if USB_DEBUG_LEVEL > 0
  if (OS_TIME_Convertms2Ticks(1000) != USB_OS_TICK_RATE_HZ) {
    USB_PANIC("Setting of USB_OS_TICK_RATE_HZ does not match OS configuration");
  }
#endif
  for (i = 0; i < SEGGER_COUNTOF(_aMailBox); i++) {
    OS_MAILBOX_Create(_aMailBox + i, sizeof(U32), 1, _aMBBuffer + i);
  }
#if USBD_OS_USE_USBD_X_INTERRUPT > 0
  OS_MUTEX_Create(&_Sema);
#endif
#if USB_NUM_MUTEXES > 0
  _NumUsedMutexes = 0;
#endif
}

/*********************************************************************
*
*       USB_OS_DeInit
*
*  Function description
*    Frees all resources used by the OS layer.
*
*/
void USB_OS_DeInit(void) {
  unsigned i;

  for (i = 0; i < SEGGER_COUNTOF(_aMailBox); i++) {
    OS_MAILBOX_Delete(&_aMailBox[i]);
  }
#if USBD_OS_USE_USBD_X_INTERRUPT > 0
  OS_MUTEX_Delete(&_Sema);
#endif
}

/*********************************************************************
*
*       USB_OS_Signal
*
*  Function description
*    Wakes the task waiting for signal.
*
*  Parameters
*    EPIndex:     Endpoint index. Signaling must be independent for all endpoints.
*    TransactCnt: Transaction counter. Specifies which transaction has been finished.
*
*  Additional information
*    This routine is typically called from within an interrupt
*    service routine.
*/
void USB_OS_Signal(unsigned EPIndex, unsigned TransactCnt) {
  U32 Tmp = TransactCnt;

  while (OS_MAILBOX_Put(&_aMailBox[EPIndex], &Tmp) != 0) {
    OS_MAILBOX_Clear(&_aMailBox[EPIndex]);
  }
}

/*********************************************************************
*
*        USB_OS_Wait
*
*  Function description
*    Blocks the task until USB_OS_Signal() is called for a given transaction.
*
*  Parameters
*    EPIndex:     Endpoint index. Signaling must be independent for all endpoints.
*    TransactCnt: Transaction counter.  Specifies the transaction to wait for.
*
*  Additional information
*    The function must ignore signaling transactions other than given in  TransactCnt . If
*    this transaction was signaled before this function was called, it must return immediately.
*
*    This routine is called from a task.
*/
void USB_OS_Wait(unsigned EPIndex, unsigned TransactCnt) {
  U32 Tmp;

  do {
    OS_MAILBOX_GetBlocked(&_aMailBox[EPIndex], &Tmp);
  } while (Tmp != TransactCnt);
}

/*********************************************************************
*
*        USB_OS_WaitTimed
*
*  Function description
*    Blocks the task until USB_OS_Signal() is called for a given transaction or a timeout
*    occurs.
*
*  Parameters
*    EPIndex:     Endpoint index. Signaling must be independent for all endpoints.
*    ms:          Timeout time given in ms.
*    TransactCnt: Transaction counter.  Specifies the transaction to wait for.
*
*  Return value
*    == 0:        Task was signaled within the given timeout.
*    == 1:        Timeout occurred.
*
*  Additional information
*    The function must ignore signaling transactions other than given in  TransactCnt. If
*    this transaction was signaled before this function was called, it must return immediately.
*
*    USB_OS_WaitTimed() is called from a task. This function is used by all available timed
*    routines.
*/
int USB_OS_WaitTimed(unsigned EPIndex, unsigned ms, unsigned TransactCnt) {
  U32 Tmp;
  char r;

#if USB_OS_TICK_RATE_HZ != 1000u
  ms = (ms * USB_OS_TICK_RATE_HZ + 999u) / 1000u;
#endif
  do {
    r = OS_MAILBOX_GetTimed(&_aMailBox[EPIndex], &Tmp, ms);
  } while (r == '\0' && Tmp != TransactCnt);
  return r;
}

/*********************************************************************
*
*       USB_OS_DecRI
*
*  Function description
*    Leave a critical region for the USB stack: Decrements interrupt disable count and
*    enable interrupts if counter reaches 0.
*
*  Additional information
*    The USB stack will perform nested calls to  USB_OS_IncDI()  and  USB_OS_DecRI().
*    This function may be called from a task context or from within an interrupt. If called
*    from an interrupt, it need not do anything.
*
*    An alternate implementation would be to
*      * enable the USB interrupts,
*      * unlock the mutex or semaphore locked in  USB_OS_IncDI()
*    if the disable count reaches 0.
*
*    This may be more efficient, because interrupts of other peripherals can be serviced
*    while inside a critical section of the USB stack.
*/
void USB_OS_DecRI(void) {
#if USBD_OS_USE_USBD_X_INTERRUPT > 0
  if (!USB_IS_IN_INT()) {
    if (OS_MUTEX_GetValue(&_Sema) == 1) {
      USBD_X_EnableInterrupt();
    }
    OS_MUTEX_Unlock(&_Sema);
  }
#else
  OS_INT_DecRI();
#endif
}

/*********************************************************************
*
*        USB_OS_IncDI
*
*  Function description
*    Enter a critical region for the USB stack: Increments interrupt disable count and
*    disables interrupts.
*
*  Additional information
*    The USB stack will perform nested calls to  USB_OS_IncDI()  and  USB_OS_DecRI().
*    This function may be called from a task context or from within an interrupt. If called
*    from an interrupt, it need not do anything.
*
*    An alternate implementation would be to
*      * perform a lock using a mutex or semaphore and
*      * disable the USB interrupts.
*
*    This may be more efficient, because interrupts of other peripherals can be serviced
*    while inside a critical section of the USB stack.
*/
void USB_OS_IncDI(void) {
#if USBD_OS_USE_USBD_X_INTERRUPT > 0
  if (!USB_IS_IN_INT()) {
    if (OS_MUTEX_LockBlocked(&_Sema) == 1) {
      USBD_X_DisableInterrupt();
    }
  }
#else
  OS_INT_IncDI();
#endif
}

/*********************************************************************
*
*       USB_OS_Delay
*
*  Function description
*    Delays for a given number of ms.
*
*  Parameters
*    ms:     Number of ms.
*/
void USB_OS_Delay(int ms) {
#if USB_OS_TICK_RATE_HZ != 1000u
  ms = (ms * USB_OS_TICK_RATE_HZ + 999u) / 1000u;
#endif
  OS_TASK_Delay(ms);
}

/*********************************************************************
*
*        USB_OS_GetTickCnt
*
*  Function description
*    Returns the current system time in milliseconds.
*
*  Return value
*    Current system time.
*/
U32 USB_OS_GetTickCnt(void) {
#if USB_OS_TICK_RATE_HZ > 1000
  U32        Ticks;
  static U32 HighTickBits = 0;
  static U32 LastTicks    = 0;

  Ticks = OS_TIME_GetTicks32();
  //
  // The value returned must use the full value range of an U32 before wrapping around,
  // in order to allow calculation of correct time differences.
  // When dividing OS_TIME_GetTicks32(), the upper bits are lost,
  // so we have to simulate them.
  //
  if (Ticks < LastTicks) {
    //
    // Wrap around of OS_TIME_GetTicks32() value occurred
    //
    HighTickBits += 0xFFFFFFFF / (USB_OS_TICK_RATE_HZ / 1000) + 1;
  }
  LastTicks = Ticks;
  return HighTickBits + Ticks / (USB_OS_TICK_RATE_HZ / 1000);
#else
  //
  // Case USB_OS_TICK_RATE_HZ <= 1000:
  //
#if 1000 % USB_OS_TICK_RATE_HZ == 0
  //
  // USB_OS_TICK_RATE_HZ is a divider of 1000 (== 1000, 500, 250, 200, 125, 100, ...):
  // Only a single multiplication with a constant, to achieve a correct wrap around.
  // Even tough "(Ticks * 1000) / USB_OS_TICK_RATE_HZ" may be more accurate for small values of Ticks,
  // it would give completely wrong results for values of Ticks near 0xFFFFFFFF.
  //
  return OS_TIME_GetTicks32() * (1000 / USB_OS_TICK_RATE_HZ);
#else
  //
  // Worse case: USB_OS_TICK_RATE_HZ is a crooked number < 1000.
  // Now we need 64-bit calculations for accurate results.
  //
  return (OS_TIME_GetTicks32() * 1000uLL) / USB_OS_TICK_RATE_HZ;
#endif
#endif
}

#if USB_NUM_MUTEXES > 0

/*********************************************************************
*
*        USB_OS_MutexAlloc
*
*  Function description
*    Allocates a new mutex to be used by USB_OS_MutexLock() / USB_OS_MutexUnlock() calls.
*
*  Return value
*    >= 0:        Valid index to be used for USB_OS_MutexLock() / USB_OS_MutexUnlock().
*    <  0:        Error: No mutex available.
*/
int USB_OS_MutexAlloc(void) {
  int i;

  i = _NumUsedMutexes;
  if (i >= USB_NUM_MUTEXES) {
    USB_PANIC("Can't allocate new mutex (USB_NUM_MUTEXES)");
    return -1;
  }
  OS_MUTEX_Create(&_aMutex[i]);
  _NumUsedMutexes = i + 1;
  return i;
}

/*********************************************************************
*
*        USB_OS_MutexFree
*
*  Function description
*    Releases all mutexes allocated by USB_OS_MutexAlloc()
*/
void USB_OS_MutexFree(void) {
  int i;

  for (i = _NumUsedMutexes; --i >= 0; ) {
    OS_MUTEX_Delete(&_aMutex[i]);
  }
  _NumUsedMutexes = 0;
}

/*********************************************************************
*
*        USB_OS_MutexLock
*
*  Function description
*    This function locks a mutex object that was allocated by USB_OS_MutexAlloc().
*
*  Parameters
*    Idx:     Index of the mutex to be locked (from USB_OS_MutexAlloc()).
*/
void USB_OS_MutexLock(int Idx) {
#if USB_SUPPORT_WARN
  if (Idx < 0 || Idx >= _NumUsedMutexes) {
    USB_WARN((USB_MTYPE_CORE, "Bad mutex index"));
    return;
  }
#endif
  OS_MUTEX_LockBlocked(&_aMutex[Idx]);
}

/*********************************************************************
*
*        USB_OS_MutexUnlock
*
*  Function description
*    This function unlocks a mutex object that was allocated by USB_OS_MutexAlloc().
*
*  Parameters
*    Idx:     Index of the mutex to be unlocked (from USB_OS_MutexAlloc()).
*/
void USB_OS_MutexUnlock(int Idx) {
#if USB_SUPPORT_WARN
  if (Idx < 0 || Idx >= _NumUsedMutexes) {
    USB_WARN((USB_MTYPE_CORE, "Bad mutex index"));
    return;
  }
#endif
  OS_MUTEX_Unlock(&_aMutex[Idx]);
}

#endif /* USB_NUM_MUTEXES > 0 */

/*************************** End of file ****************************/
