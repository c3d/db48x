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

File    : IP_ConfigIO.c
Purpose : I/O Configuration routines for TCP/IP.
*/

#include <stdio.h>
#include "IP.h"

#ifdef __CROSSWORKS_ARM
  #include "__putchar.h"
#endif

/*********************************************************************
*
*       Defines, configurable
*
*       This section is normally the only section which requires
*       changes on most embedded systems
*
**********************************************************************
*/

#ifndef   USE_RTT
  #define USE_RTT         0
#endif
#ifndef   USE_SYSTEMVIEW
  #define USE_SYSTEMVIEW  0
#endif
#ifndef   USE_DCC
  #define USE_DCC         0
#endif
#ifndef   USE_EMBOS_VIEW
  #define USE_EMBOS_VIEW  0
#endif
#ifndef   SHOW_TIME
  #define SHOW_TIME       1
#endif
#ifndef   SHOW_TASK
  #define SHOW_TASK       1
#endif

//#define   IP_ALLOW_DEINIT  (1)


#if (USE_RTT != 0)
#include "SEGGER_RTT.h"
#endif
#if (USE_SYSTEMVIEW != 0)
#include "SEGGER_SYSVIEW.h"
#endif
#if (USE_DCC != 0)
#include "JLINKDCC.h"
#endif
#if (USE_EMBOS_VIEW != 0)
#include "RTOS.h"
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

#if (USE_EMBOS_VIEW != 0)
#if (IP_ALLOW_DEINIT != 0)
static IP_ON_EXIT_HOOK _OnExitHook;
static char            _IsInitialized;
#endif
static char            _EntranceCnt;
#endif

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

#if ((IP_ALLOW_DEINIT != 0) && (USE_EMBOS_VIEW != 0))
/*********************************************************************
*
*       _IP_IO_cbOnExit()
*
*  Function description
*    Called in case of a de-initialization of the stack.
*/
static void _IP_IO_cbOnExit(void) {
  _EntranceCnt   = 0;
  _IsInitialized = 0;
}
#endif

#if (USE_SYSTEMVIEW == 0)
/*********************************************************************
*
*       _puts()
*
*  Function description
*    Local (static) replacement for puts.
*    The reason why puts is not used is that puts always appends a NL
*    character, which screws up our formatting.
*
*  Parameters
*    s: String to output.
*
*  Additional information
*    We expect _puts() always to be called with interrupts disabled.
*    This might not be necesaary for all interfaces but is the default
*    expected by the shipped pre-configured configurations that can
*    be selected via configuration defines at the top of this file.
*/
static void _puts(const char* s) {
#if (USE_EMBOS_VIEW != 0)
#if IP_ALLOW_DEINIT
  if (_IsInitialized == 0) {
    _IsInitialized++;
    IP_AddOnExitHandler(&_OnExitHook, _IP_IO_cbOnExit);
  }
#endif
  //
  // Prevent using OS_SendString() from an interrupt as this is not
  // valid. Might happen in case we are overrun by packets.
  // OS_InInterrupt() might not be supported by ALL embOS ports but
  // should be support by typically any newer embOS port.
  // In addition we prevent a message to be sent out if we are still
  // working on the previous message (from IP stack perspective).
  // Typically this should never be the case except for IP_PANIC()
  // messages. The _EntranceCnt prevents us from an endless loop.
  //
  if (OS_InInterrupt() == 0) {
    if (_EntranceCnt == 0) {
      _EntranceCnt++;
      OS_SendString(s);
      _EntranceCnt--;
    }
  }
#elif (USE_RTT != 0)
  (void)SEGGER_RTT_WriteString(0u, s);
#elif (USE_SYSTEMVIEW != 0)
  SEGGER_SYSVIEW_Print(s);
#else
  char c;

  for (;;) {
    c = *s++;
    if (c == 0) {
      break;
    }
    #if USE_DCC
      JLINKDCC_SendChar(c);
    #else
      putchar(c);
    #endif
  }
#endif
}

#if SHOW_TIME
/*********************************************************************
*
*       _WriteUnsigned()
*
*  Function description
*    Stores an unsigned value into a buffer as printable string.
*
*  Parameters
*    sBuffer  : Pointer to output buffer for string.
*    v        : Value to convert.
*    NumDigits: Pad the output value to this amount of digits.
*
*  Return value
*    Pointer to end of string (on the termination character).
*/
static char* _WriteUnsigned(char* sBuffer, U32 v, int NumDigits) {
  U32      Digit;
  unsigned Base;
  unsigned Div;

  Digit = 1;
  Base  = 10;
  //
  // Count how many digits are required.
  //
  do {
    if (NumDigits <= 1) {
      Div = v / Digit;
      if (Div < Base) {
        break;
      }
    }
    NumDigits--;
    Digit *= Base;
  } while (1);
  //
  // Output digits.
  //
  do {
    Div = v / Digit;
    v  -= Div * Digit;
    *sBuffer++ = (char)('0' + Div);
    Digit /= Base;
  } while (Digit);
  *sBuffer = 0;
  return sBuffer;
}
#endif

/*********************************************************************
*
*       _ShowStamp()
*
*  Function description
*    Outputs a timestamp and task name as configured.
*/
static void _ShowStamp(void) {
#if SHOW_TIME
  {
    char* sBuffer;
    U32   Time;
    U32   v;
    char  ac[20];

    sBuffer    = &ac[0];
    Time       = IP_OS_GET_TIME();
    v          = Time / 1000u;       // Calc. the seconds.
    sBuffer    = _WriteUnsigned(sBuffer, v, 0);
    *sBuffer++ = ':';
    v          = Time - (v * 1000);  // Calc. the milliseconds.
    sBuffer    = _WriteUnsigned(sBuffer, v, 3);
    *sBuffer++ = ' ';
    *sBuffer++ = 0;
    _puts(ac);
  }
#endif

#if SHOW_TASK
  {
    const char* s;
    s = IP_OS_GetTaskName(NULL);
    if (s != NULL) {
      _puts(s);
      _puts(" - ");
    }
  }
#endif
}
#endif  // (USE_SYSTEMVIEW == 0)

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       IP_Panic()
*
*  Function description
*    This function is called if the stack encounters a critical
*    situation. In a release build, this function may not be
*    linked in.
*
*  Parameters
*    s: String to output.
*/
void IP_Panic(const char* s) {
  IP_OS_DISABLE_INTERRUPT();
#if (IP_DEBUG > 1)
#if USE_SYSTEMVIEW
  SEGGER_SYSVIEW_Error("*** Fatal error, System halted: ");
  SEGGER_SYSVIEW_PrintfTargetEx(s, SEGGER_SYSVIEW_ERROR | (1 << 8));
#else
  _puts("*** Fatal error, System halted: ");
  _puts(s);
  _puts("\n");
#endif
#endif
  while(s);
}

/*********************************************************************
*
*       IP_Log()
*
*  Function description
*    This function is called by the stack in debug builds with log
*    output. In a release build, this function may not be linked in.
*
*  Parameters
*    s: String to output.
*
*  Additional information
*    Interrupts and task switches
*      printf() has a re-entrance problem on a lot of systems if
*      interrupts are not disabled. Strings to output would be
*      scrambled if during an output from a task an output from an
*      interrupt would take place.
*      In order to avoid this problem, interrupts are disabled.
*/
void IP_Log(const char* s) {
  IP_OS_DISABLE_INTERRUPT();
#if USE_SYSTEMVIEW
  SEGGER_SYSVIEW_Print(s);
#else
  _ShowStamp();
  _puts(s);
  _puts("\n");
#endif
  IP_OS_ENABLE_INTERRUPT();
}

/*********************************************************************
*
*       IP_Warn()
*
*  Function description
*    This function is called by the stack in debug builds with warning
*    output. In a release build, this function may not be linked in.
*
*  Parameters
*    s: String to output.
*
*  Additional information
*    Interrupts and task switches
*      printf() has a re-entrance problem on a lot of systems if
*      interrupts are not disabled. Strings to output would be
*      scrambled if during an output from a task an output from an
*      interrupt would take place.
*      In order to avoid this problem, interrupts are disabled.
*/
void IP_Warn(const char* s) {
  IP_OS_DISABLE_INTERRUPT();
#if USE_SYSTEMVIEW
  SEGGER_SYSVIEW_Warn(s);
#else
  _ShowStamp();
  _puts("*** Warning *** ");
  _puts(s);
  _puts("\n");
#endif
  IP_OS_ENABLE_INTERRUPT();
}

/*************************** End of file ****************************/
