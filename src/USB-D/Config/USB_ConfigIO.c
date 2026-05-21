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
File        : USB_ConfigIO.c
Purpose     : Sample implementation of log and warn function
              for emUSB-Device and embOS.
-------------------------- END-OF-HEADER -----------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <stdio.h>
#include "USB.h"

#if defined (__CROSSWORKS_ARM)
  #include "__putchar.h"
#endif

/*********************************************************************
*
*       Defines, configurable
*
*       This section is normally the only section which requires
*       changes on most embedded systems.
*/
#ifndef   USE_RTT
  #define USE_RTT    0    // SEGGER's Real Time Terminal: https://www.segger.com/jlink-real-time-terminal.html
#endif

#ifndef   USE_DCC
  #define USE_DCC    0
#endif

#ifndef   SHOW_TIME
  #define SHOW_TIME  1
#endif

#ifndef   SHOW_TASK
  #define SHOW_TASK  1
#endif

#if SHOW_TASK > 0
  #include "RTOS.h"
#endif

#if USE_RTT
  #include "SEGGER_RTT.h"
#endif

#if USE_DCC
  #include "JLINKDCC.h"
#endif

/*********************************************************************
*
*       _puts
*
*  Function description
*    Local (static) replacement for puts.
*    The reason why puts is not used is that puts always appends
*    a new-line character, which screws up our formatting.
*
*  Parameters
*    s - Pointer to a string.
*/
static void _puts(const char * s) {
#if USE_RTT
  SEGGER_RTT_WriteString(0, s);
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

/*********************************************************************
*
*       _WriteUnsigned
*
*  Function description
*    Prints an unsigned integer as a string.
*
*  Parameters
*    s          - Pointer to a string into which the value should be written.
*    v          - Integer value to print.
*    NumDigits  - Number of digits to print (0 for all digits).
*
*  Return value
*    s - Pointer to a string, same as the s parameter.
*/
#if SHOW_TIME
static char * _WriteUnsigned(char * s, U32 v, int NumDigits) {
  unsigned   Base;
  unsigned   Div;
  U32        Digit;
  Digit    = 1;
  Base     = 10;
  //
  // Count how many digits are required
  //
  while (((v / Digit) >= Base) || (NumDigits > 1)) {
    NumDigits--;
    Digit *= Base;
  }
  //
  // Output digits
  //
  do {
    Div = v / Digit;
    v  -= Div * Digit;
    *s++ = (char)('0' + Div);
    Digit /= Base;
  } while (Digit);
  *s = 0;
  return s;
}
#endif

/*********************************************************************
*
*       _ShowStamp
*
*  Function description
*    Prints a time-stamp and the name of the task from which
*    the function was executed.
*
*  Notes
*    This function is only designed to work with embOS.
*/
static void _ShowStamp(void) {
#if SHOW_TIME
  I32    Time;
  char   ac[20];
  char * sBuffer = &ac[0];
  Time           = USB_OS_GetTickCnt();
  sBuffer        = _WriteUnsigned(sBuffer, Time / 1000, 0);
  *sBuffer++     = ':';
  sBuffer        = _WriteUnsigned(sBuffer, Time % 1000, 3);
  *sBuffer++     = ' ';
  *sBuffer++     = 0;
  _puts(ac);
#endif

#if SHOW_TASK
{
  const char * s;
#if OS_VERSION_GENERIC == 38803
  s = OS_GetTaskName(OS_Global.pCurrentTask);
#else
#if (OS_DEBUG) && (OS_VERSION_GENERIC >= 41201u)
  if (OS_Global.InInt) {
    s = "Interrupt";
  } else
#endif
  {
    s = OS_GetTaskName(OS_pCurrentTask);
  }
#endif
  if (s) {
    _puts(s);
    _puts(" - ");
  }
}
#endif
}

/*********************************************************************
*
*       USB_OS_Panic
*
*  Function description
*    Is called if the stack encounters a fatal error.
*
*  Parameters
*    pErrMsg - Pointer to a string holding the error message.
*
*  Additional information
*    In a release build this function is not linked in. The default
*    implementation of this function disables all interrupts to avoid
*    further task switches, outputs the error string via terminal I/O
*    and loops forever. When using an emulator, you should set a
*    break-point at the beginning of this routine or simply stop the
*    program after a failure.
*/
void USB_OS_Panic(const char * pErrMsg) {
  USB_LOG((USB_MTYPE_INFO, "PANIC: %s", pErrMsg));
  USB_OS_IncDI();
  while (pErrMsg);
}

/*********************************************************************
*
*       USB_X_Log
*
*  Function description
*    This function is called by the stack in debug builds with log output.
*    In a release build, this function is not be linked in.
*
*  Parameters
*    s - Pointer to a string holding the log message.
*/
void USB_X_Log(const char * s) {
  USB_OS_IncDI();
  _ShowStamp();
  _puts(s);
  _puts("\n");
  USB_OS_DecRI();
}

/*********************************************************************
*
*       USB_X_Warn
*
*  Function description
*    This function is called by the stack in debug builds with warning output.
*    In a release build, this function is not be linked in.
*
*  Parameters
*    s - Pointer to a string holding the warning message.
*/
void USB_X_Warn(const char * s) {
  USB_OS_IncDI();
  _ShowStamp();
  _puts("*** Warning *** ");
  _puts(s);
  _puts("\n");
  USB_OS_DecRI();
}

/*************************** End of file ****************************/
