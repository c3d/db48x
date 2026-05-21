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
File    : SMTPC_Conf.h
Purpose : SMTP client add-on configuration file
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _SMTPC_CONF_H_
#define _SMTPC_CONF_H_ 1

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define SMTPC_MSGID_DOMAIN           "@segger.com"
#define SMTPC_SERVER_PORT             25
#define SMTPC_IN_BUFFER_SIZE         256
#define SMTPC_AUTH_USER_BUFFER_SIZE   48
#define SMTPC_AUTH_PASS_BUFFER_SIZE   48

#ifndef   DEBUG
  #define DEBUG  0
#endif

//
// Macros
//
#ifdef __ICCARM__     // IAR ARM toolchain
  #include "IP.h"
#else                 // Other toolchains
  #include <string.h>
  #define IP_MEMSET memset
#endif

//
// Selection to our best knowledge.
//
#if defined(__linux__)
  #include <stdio.h>
  #define SMTPC_FUNC_WARN(p)  printf p ; \
                              printf("\n");
  #define SMTPC_FUNC_LOG(p)   printf p ; \
                              printf("\n");
#elif defined(WIN32)
  //
  // - Microsoft Visual Studio
  //
  void WIN32_OutputDebugStringf(const char * sFormat, ...);
  #define SMTPC_FUNC_WARN(p)  WIN32_OutputDebugStringf p
  #define SMTPC_FUNC_LOG(p)   WIN32_OutputDebugStringf p
#elif (defined(__ICCARM__) || defined(__ICCRX__) || defined(__GNUC__) || defined(__SEGGER_CC__))
  //
  // - IAR ARM
  // - IAR RX
  // - GCC based
  // - SEGGER
  //
  #include "IP.h"
  #define SMTPC_FUNC_WARN(p)  IP_Warnf_Application p
  #define SMTPC_FUNC_LOG(p)   IP_Logf_Application  p
#else
  //
  // Other toolchains
  //
  #define SMTPC_FUNC_WARN(p)
  #define SMTPC_FUNC_LOG(p)
#endif

//
// Final selection that can be overridden.
//
#ifndef       SMTPC_WARN
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   SMTPC_WARN(p)      SMTPC_FUNC_WARN(p)
  #else
    //
    // Release builds
    //
    #define   SMTPC_WARN(p)
  #endif
#endif

#ifndef       SMTPC_LOG
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   SMTPC_LOG(p)       SMTPC_FUNC_LOG(p)
  #else
    //
    // Release builds
    //
    #define   SMTPC_LOG(p)
  #endif
#endif

#ifndef       SMTPC_APP_WARN
  #define     SMTPC_APP_WARN(p)  SMTPC_FUNC_WARN(p)
#endif

#ifndef       SMTPC_APP_LOG
  #define     SMTPC_APP_LOG(p)   SMTPC_FUNC_LOG(p)
#endif

#endif     // Avoid multiple inclusion

/*************************** End of file ****************************/
