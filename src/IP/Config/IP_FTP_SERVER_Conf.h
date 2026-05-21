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

File    : IP_FTP_SERVER_Conf.h
Purpose : emFTP server add-on configuration file.
*/

#ifndef IP_FTP_SERVER_CONF_H  // Avoid multiple inclusion.
#define IP_FTP_SERVER_CONF_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   DEBUG
  #define DEBUG  0
#endif

//
// Selection to our best knowledge.
//
#if defined(__linux__)
  #include <stdio.h>
  #define FTPS_FUNC_WARN(p)  printf p ; \
                             printf("\n");
  #define FTPS_FUNC_LOG(p)   printf p ; \
                             printf("\n");
#elif defined(WIN32)
  //
  // - Microsoft Visual Studio
  //
  void WIN32_OutputDebugStringf(const char * sFormat, ...);
  #define FTPS_FUNC_WARN(p)  WIN32_OutputDebugStringf p
  #define FTPS_FUNC_LOG(p)   WIN32_OutputDebugStringf p
#elif (defined(__ICCARM__) || defined(__ICCRX__) || defined(__GNUC__) || defined(__SEGGER_CC__))
  //
  // - IAR ARM
  // - IAR RX
  // - GCC based
  // - SEGGER
  //
  #include "IP.h"
  #define FTPS_FUNC_WARN(p)  IP_Warnf_Application p
  #define FTPS_FUNC_LOG(p)   IP_Logf_Application  p
#else
  //
  // Other toolchains
  //
  #define FTPS_FUNC_WARN(p)
  #define FTPS_FUNC_LOG(p)
#endif

//
// Final selection that can be overridden.
//
#ifndef       FTPS_WARN
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   FTPS_WARN(p)      FTPS_FUNC_WARN(p)
  #else
    //
    // Release builds
    //
    #define   FTPS_WARN(p)
  #endif
#endif

#ifndef       FTPS_LOG
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   FTPS_LOG(p)       FTPS_FUNC_LOG(p)
  #else
    //
    // Release builds
    //
    #define   FTPS_LOG(p)
  #endif
#endif

#ifndef       FTPS_APP_WARN
  #define     FTPS_APP_WARN(p)  FTPS_FUNC_WARN(p)
#endif

#ifndef       FTPS_APP_LOG
  #define     FTPS_APP_LOG(p)   FTPS_FUNC_LOG(p)
#endif

//
// Old configuration defines for legacy IP_FTPS_Process() API.
// New integrations should use IP_FTPS_ProcessEx() with the
// dynamic buffer configuration via IP_FTPS_ConfigBufSizes() .
//
#define FTPS_BUFFER_SIZE       512  // Two buffers of this size will be used, one for IN and one for OUT.
#define FTPS_MAX_PATH          128  // Max. length of complete path including directory and filename. One buffer.
#define FTPS_MAX_PATH_DIR      128  // Max. length of dirname (path without filename). One buffer.
#define FTPS_MAX_FILE_NAME      64  // The default is 13 characters, because filenames can not be longer than an 8.3 without long file name support.
                                    // 8.3 + 1 character for string termination. One buffer.
//
// Sign on message. Can also be set with new API IP_FTPS_SetSignOnMsg() .
//
#define FTPS_SIGN_ON_MSG  "Welcome to emFTP server"


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
