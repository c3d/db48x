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

File    : IP_COAP_Conf.h
Purpose : COAP configuration file.
*/

#ifndef IP_COAP_CONF_H  // Avoid multiple inclusion.
#define IP_COAP_CONF_H

#if defined(__cplusplus)
  extern "C" {                 // Make sure we have C-declarations in C++ programs.
#endif

//
// Includes for memset(), memcpy() and strlen().
//
#include <stdio.h>
#include <string.h>

#include "SEGGER.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/


#ifndef   IP_COAP_NSTART
  #define IP_COAP_NSTART                 (1u)                       // Number of simultaneous activities between a client and a server.
#endif

#ifndef   IP_COAP_MAX_RETRANSMIT
  #define IP_COAP_MAX_RETRANSMIT         (4u)                       // Number of CON retry.
#endif

#ifndef   IP_COAP_ACK_TIMEOUT                                       // Time [ms] to wait for an ACK. Please note that this is the base timeout to wait.
  #define IP_COAP_ACK_TIMEOUT            (2000u)                    // This value gets randomized between 1x and 1.5x for the first CON and is used as is for the
#endif                                                              // last retry. For all other retries the randomized value gets doubled for each retry.

#ifndef   IP_COAP_DEFAULT_LEISURE
  #define IP_COAP_DEFAULT_LEISURE        (5000u)                    // Default leisure time in milliseconds to randomize the time to reply to a multicast NON request.
#endif

#ifndef   IP_COAP_OBS_FORCE_CON_TIMEOUT
  #define IP_COAP_OBS_FORCE_CON_TIMEOUT  (1000u * 60u * 60u * 12u)  // Time [ms] after which the next observable report is sent as CON instead of NON to check
#endif                                                              // if the client is still alive. The RFC suggests maximum 24 hours, we use 12 by default.

#ifndef   DEBUG
  #define DEBUG                          0
#endif

#ifndef   IP_COAP_MEMSET
  #define IP_COAP_MEMSET                 memset
#endif

#ifndef   IP_COAP_MEMCPY
  #define IP_COAP_MEMCPY                 memcpy
#endif

#ifndef   IP_COAP_MEMCMP
  #define IP_COAP_MEMCMP                 memcmp
#endif

#ifndef   IP_COAP_MEMMOVE
  #define IP_COAP_MEMMOVE                memmove
#endif

#ifndef   IP_COAP_STRLEN
  #define IP_COAP_STRLEN                 SEGGER_strlen
#endif

//
// Selection to our best knowledge.
//
#if defined(__linux__)
  #define IP_COAP_FUNC_WARN(p)  printf p ; \
                                printf("\n");
  #define IP_COAP_FUNC_LOG(p)   printf p ; \
                                printf("\n");
#elif defined(WIN32)
  //
  // - Microsoft Visual Studio
  //
  void WIN32_OutputDebugStringf(const char * sFormat, ...);
  #define IP_COAP_FUNC_WARN(p)  WIN32_OutputDebugStringf p
  #define IP_COAP_FUNC_LOG(p)   WIN32_OutputDebugStringf p
#elif (defined(__ICCARM__) || defined(__ICCRX__) || defined(__GNUC__) || defined(__SEGGER_CC__))
  //
  // - IAR ARM
  // - IAR RX
  // - GCC based
  // - SEGGER
  //
  #include "IP.h"
  #define IP_COAP_FUNC_WARN(p)  IP_Warnf_Application p
  #define IP_COAP_FUNC_LOG(p)   IP_Logf_Application  p
#else
  //
  // Other toolchains
  //
  #define IP_COAP_FUNC_WARN(p)
  #define IP_COAP_FUNC_LOG(p)
#endif

//
// Final selection that can be overridden.
//
#ifndef       IP_COAP_WARN
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   IP_COAP_WARN(p)      IP_COAP_FUNC_WARN(p)
  #else
    //
    // Release builds
    //
    #define   IP_COAP_WARN(p)
  #endif
#endif

#ifndef       IP_COAP_LOG
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   IP_COAP_LOG(p)       IP_COAP_FUNC_LOG(p)
  #else
    //
    // Release builds
    //
    #define   IP_COAP_LOG(p)
  #endif
#endif

#ifndef       IP_COAP_APP_WARN
  #define     IP_COAP_APP_WARN(p)  IP_COAP_FUNC_WARN(p)
#endif

#ifndef       IP_COAP_APP_LOG
  #define     IP_COAP_APP_LOG(p)   IP_COAP_FUNC_LOG(p)
#endif

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
