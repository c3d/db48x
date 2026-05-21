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
----------------------------------------------------------------------
File        : FS_OS.h
Purpose     : File system's OS Layer header file
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_OS_H             // Avoid recursive and multiple inclusion
#define FS_OS_H

#include "FS_ConfDefaults.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       FS_OS_TYPE_LOCK
*
*  Function description
*    Request exclusive access to the file system.
*
*  Parameters
*    LockIndex    Index of OS synchronization object (0-based)
*/
typedef void FS_OS_TYPE_LOCK(unsigned LockIndex);

/*********************************************************************
*
*       FS_OS_TYPE_UNLOCK
*
*  Function description
*    Releases the exclusive access to the file system.
*
*  Parameters
*    LockIndex    Index of OS synchronization object (0-based)
*/
typedef void FS_OS_TYPE_UNLOCK(unsigned LockIndex);

/*********************************************************************
*
*       FS_OS_TYPE_INIT
*
*  Function description
*    Allocates the resources required by OS layer.
*
*  Parameters
*    NumLocks     Number of OS synchronization objects to be allocated.
*/
typedef void FS_OS_TYPE_INIT(unsigned NumLocks);

/*********************************************************************
*
*       FS_OS_TYPE_DEINIT
*
*  Function description
*    Frees the resources allocated for the OS layer.
*/
typedef void FS_OS_TYPE_DEINIT(void);

/*********************************************************************
*
*       FS_OS_TYPE_GET_TIME
*
*  Function description
*    Returns the time expired since system start.
*
*  Return value
*    Time value in milliseconds.
*/
typedef U32 FS_OS_TYPE_GET_TIME(void);

/*********************************************************************
*
*       FS_OS_TYPE_DELAY
*
*  Function description
*    Blocks the execution of a task.
*
*  Parameters
*    ms     Time to block in milliseconds.
*/
typedef void FS_OS_TYPE_DELAY(int ms);

/*********************************************************************
*
*       FS_OS_TYPE_WAIT
*
*  Function description
*    Waits for an OS event to be signaled.
*
*  Parameters
*    TimeOut    Maximum time to wait for the OS event to be signaled.
*
*  Return value
*    ==0    OS event signaled.
*    !=0    Timeout or other error occurred.
*/
typedef int FS_OS_TYPE_WAIT(int TimeOut);

/*********************************************************************
*
*       FS_OS_TYPE_SIGNAL
*
*  Function description
*    Signals an OS event.
*/
typedef void FS_OS_TYPE_SIGNAL(void);

/*********************************************************************
*
*       FS_OS_TYPE
*
*  Description
*    OS layer API.
*/
typedef struct {
  FS_OS_TYPE_LOCK     * pfLock;           // Request exclusive access to the file system.
  FS_OS_TYPE_UNLOCK   * pfUnlock;         // Releases the exclusive access to the file system.
  FS_OS_TYPE_INIT     * pfInit;           // Allocates the resources required by OS layer.
  FS_OS_TYPE_DEINIT   * pfDeInit;         // Frees the resources allocated for the OS layer.
  FS_OS_TYPE_GET_TIME * pfGetTime;        // Returns the time expired since system start.
  FS_OS_TYPE_DELAY    * pfDelay;          // Blocks the execution of a task.
  FS_OS_TYPE_WAIT     * pfWait;           // Waits for an OS event to be signaled.
  FS_OS_TYPE_SIGNAL   * pfSignal;         // Signals an OS event.
} FS_OS_TYPE;

/*********************************************************************
*
*       Public data
*/
extern const FS_OS_TYPE FS_OS_Default;

/*********************************************************************
*
*       API functions
*/
void FS_X_OS_Lock   (unsigned LockIndex);
void FS_X_OS_Unlock (unsigned LockIndex);
void FS_X_OS_Init   (unsigned NumLocks);
void FS_X_OS_DeInit (void);
U32  FS_X_OS_GetTime(void);
void FS_X_OS_Delay  (int ms);
int  FS_X_OS_Wait   (int TimeOut);
void FS_X_OS_Signal (void);

void FS_OS_Delay  (int ms);
U32  FS_OS_GetTime(void);
#if (FS_OS_LOCKING != 0) && (FS_OS_SUPPORT_RUNTIME_CONFIG != 0)
void FS_OS_SetType(const FS_OS_TYPE * pOSType);
#endif // FS_OS_LOCKING != 0 && FS_OS_SUPPORT_RUNTIME_CONFIG != 0
void FS_OS_Signal (void);
int  FS_OS_Wait   (int TimeOut);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FS_OS_H

/*************************** End of file ****************************/
