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

File    : IP_MQTT_CLIENT_Conf.h
Purpose : MQTT client configuration file.
*/

#ifndef IP_MQTT_CLIENT_CONF_H  // Avoid multiple inclusion.
#define IP_MQTT_CLIENT_CONF_H

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

#ifndef   DEBUG
  #define DEBUG                  0
#endif

#ifndef   IP_MQTT_CLIENT_MEMSET
  #define IP_MQTT_CLIENT_MEMSET  memset
#endif

#ifndef   IP_MQTT_CLIENT_MEMCPY
  #define IP_MQTT_CLIENT_MEMCPY  memcpy
#endif

#ifndef   IP_MQTT_CLIENT_STRLEN
  #define IP_MQTT_CLIENT_STRLEN  SEGGER_strlen
#endif

//
// Selection to our best knowledge.
//
#if defined(__linux__)
  #define IP_MQTT_CLIENT_FUNC_WARN(p)  printf p ; \
                                       printf("\n");
  #define IP_MQTT_CLIENT_FUNC_LOG(p)   printf p ; \
                                       printf("\n");
#elif defined(WIN32)
  //
  // - Microsoft Visual Studio
  //
  void WIN32_OutputDebugStringf(const char * sFormat, ...);
  #define IP_MQTT_CLIENT_FUNC_WARN(p)  WIN32_OutputDebugStringf p
  #define IP_MQTT_CLIENT_FUNC_LOG(p)   WIN32_OutputDebugStringf p
#elif (defined(__ICCARM__) || defined(__ICCRX__) || defined(__GNUC__) || defined(__SEGGER_CC__))
  //
  // - IAR ARM
  // - IAR RX
  // - GCC based
  // - SEGGER
  //
  #include "IP.h"
  #define IP_MQTT_CLIENT_FUNC_WARN(p)  IP_Warnf_Application p
  #define IP_MQTT_CLIENT_FUNC_LOG(p)   IP_Logf_Application  p
#else
  //
  // Other toolchains
  //
  #define IP_MQTT_CLIENT_FUNC_WARN(p)
  #define IP_MQTT_CLIENT_FUNC_LOG(p)
#endif

//
// Final selection that can be overridden.
//
#ifndef       IP_MQTT_CLIENT_WARN
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   IP_MQTT_CLIENT_WARN(p)      IP_MQTT_CLIENT_FUNC_WARN(p)
  #else
    //
    // Release builds
    //
    #define   IP_MQTT_CLIENT_WARN(p)
  #endif
#endif

#ifndef       IP_MQTT_CLIENT_LOG
  #if (DEBUG != 0)
    //
    // Debug builds
    //
    #define   IP_MQTT_CLIENT_LOG(p)       IP_MQTT_CLIENT_FUNC_LOG(p)
  #else
    //
    // Release builds
    //
    #define   IP_MQTT_CLIENT_LOG(p)
  #endif
#endif

#ifndef       IP_MQTT_CLIENT_APP_WARN
  #define     IP_MQTT_CLIENT_APP_WARN(p)  IP_MQTT_CLIENT_FUNC_WARN(p)
#endif

#ifndef       IP_MQTT_CLIENT_APP_LOG
  #define     IP_MQTT_CLIENT_APP_LOG(p)   IP_MQTT_CLIENT_FUNC_LOG(p)
#endif

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
