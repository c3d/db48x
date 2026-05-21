/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2024  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : FS_OS_embOS.c
Purpose : embOS OS Layer for the file system.
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include "FS.h"
#include "FS_OS.h"
#include "RTOS.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_RSEMA   * _paSema;
static OS_EVENT     _Event;
#if FS_SUPPORT_DEINIT
  static unsigned   _NumLocks;
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_X_OS_Lock
*
*  Function description
*    Acquires the specified OS resource.
*
*  Parameters
*    LockIndex    Identifies the OS resource (0-based).
*
*  Additional information
*    This function has to block until it can acquire the OS resource.
*    The OS resource is later released via a call to FS_X_OS_Unlock().
*/
void FS_X_OS_Lock(unsigned LockIndex) {
  OS_RSEMA * pSema;

  pSema = _paSema + LockIndex;
  (void)OS_Use(pSema);
  FS_DEBUG_LOG((FS_MTYPE_OS, "OS: LOCK Index: %d\n", LockIndex));
}

/*********************************************************************
*
*       FS_X_OS_Unlock
*
*  Function description
*    Releases the specified OS resource.
*
*  Parameters
*    LockIndex    Identifies the OS resource (0-based).
*
*  Additional information
*    The OS resource to be released was acquired via a call to FS_X_OS_Lock()
*/
void FS_X_OS_Unlock(unsigned LockIndex) {
  OS_RSEMA * pSema;

  pSema = _paSema + LockIndex;
  FS_DEBUG_LOG((FS_MTYPE_OS, "OS: UNLOCK Index: %d\n", LockIndex));
  OS_Unuse(pSema);
}

/*********************************************************************
*
*       FS_X_OS_Init
*
*  Function description
*    Initializes the OS resources.
*
*  Parameters
*    NumLocks   Number of locks that should be created.
*
*  Additional information
*    This function is called by FS_Init(). It has to create all resources
*    required by the OS to support multi tasking of the file system.
*/
void FS_X_OS_Init(unsigned NumLocks) {
  unsigned   i;
  OS_RSEMA * pSema;
  unsigned   NumBytes;

  NumBytes = NumLocks * sizeof(OS_RSEMA);
  _paSema = SEGGER_PTR2PTR(OS_RSEMA, FS_ALLOC_ZEROED((I32)NumBytes, "OS_RSEMA"));
  pSema =_paSema;
  for (i = 0; i < NumLocks; i++) {
    OS_CREATERSEMA(pSema++);
  }
  OS_EVENT_Create(&_Event);
#if FS_SUPPORT_DEINIT
  _NumLocks = NumLocks;
#endif
}

#if FS_SUPPORT_DEINIT

/*********************************************************************
*
*       FS_X_OS_DeInit
*
*  Function description
*    This function has to release all the resources that have been
*    allocated by FS_X_OS_Init().
*/
void FS_X_OS_DeInit(void) {
  unsigned   i;
  OS_RSEMA * pSema;
  unsigned   NumLocks;

  NumLocks = _NumLocks;
  pSema   = &_paSema[0];
  for (i = 0; i < NumLocks; i++) {
    OS_DeleteRSema(pSema);
    pSema++;
  }
  OS_EVENT_Delete(&_Event);
  FS_Free(_paSema);
  _paSema  = NULL;
  _NumLocks = 0;
}

#endif // FS_SUPPORT_DEINIT

/*********************************************************************
*
*       FS_X_OS_GetTime
*/
U32 FS_X_OS_GetTime(void) {
  return (U32)OS_GetTime32();
}

/*********************************************************************
*
*       FS_X_OS_Wait
*
*  Function description
*    Waits for an event to be signaled.
*
*  Parameters
*    Timeout  Time to be wait for the event object.
*
*  Return value
*    ==0      Event object was signaled within the timeout value
*    !=0      An error or a timeout occurred.
*/
int FS_X_OS_Wait(int TimeOut) {
  int r;

  r = -1;
  if ((U8)OS_EVENT_WaitTimed(&_Event, TimeOut) == 0u) {
    r = 0;
  }
  return r;
}

/*********************************************************************
*
*       FS_X_OS_Signal
*
*  Function description
*    Signals an event.
*/
void FS_X_OS_Signal(void) {
  OS_EVENT_Set(&_Event);
}

/*********************************************************************
*
*       FS_X_OS_Delay
*
*  Function description
*    Blocks the execution for the specified number of milliseconds.
*/
void FS_X_OS_Delay(int ms) {
  OS_Delay(ms + 1);
}

/*************************** End of file ****************************/
