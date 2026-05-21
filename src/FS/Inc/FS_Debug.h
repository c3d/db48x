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
File    : FS_Debug.h
Purpose : Debug macros for logging
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_DEBUG_H                // Avoid recursive and multiple inclusion
#define FS_DEBUG_H

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
//lint -save -e829 +headerwarn option was previously issued for header 'stdarg.h' N:102. Reason: we need access to the va_start() and va_end() macros.
#include <stdarg.h>
//lint -restore
#include "SEGGER.h"
#include "FS_ConfDefaults.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_DEBUG_ERROROUT
*/
#if (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ERRORS)
  #define FS_DEBUG_ERROROUT(s)              FS_ErrorOutf s
#else
  #define FS_DEBUG_ERROROUT(s)
#endif

/*********************************************************************
*
*       FS_DEBUG_WARN
*/
#if (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_WARNINGS)
  #define FS_DEBUG_WARN(s)                  FS_Warnf s
#else
  #define FS_DEBUG_WARN(s)
#endif

/*********************************************************************
*
*       FS_DEBUG_LOG
*/
#if (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ALL)
  #define FS_DEBUG_LOG(s)                   FS_Logf s
#else
  #define FS_DEBUG_LOG(s)
#endif

#define EXPR2STRING(Expr)         #Expr   //lint !e9024 '#/##' operator used in macro 'FS_DEBUG_ASSERT' [MISRA 2012 Rule 20.10, advisory] N:102. Rationale: This macro is used only in debug builds.

/*********************************************************************
*
*       FS_DEBUG_ASSERT
*/
#if (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_LOG_ERRORS)
   #define FS_DEBUG_ASSERT(MType, Expr)              \
   if (!(Expr)) {                                    \
      FS_DEBUG_ERROROUT((MType, EXPR2STRING(Expr))); \
      FS_X_Panic(FS_ERRCODE_ASSERT_FAILURE);         \
    }
#else
  #define FS_DEBUG_ASSERT(MType, Expr)
#endif

/*********************************************************************
*
*       Logging (for debugging primarily)
*/
void FS_ErrorOutf     (U32 Type, const char * sFormat, ...);
void FS_Logf          (U32 Type, const char * sFormat, ...);
void FS_Warnf         (U32 Type, const char * sFormat, ...);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FS_DEBUG_H

/*************************** End of file ****************************/
