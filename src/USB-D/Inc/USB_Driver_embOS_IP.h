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
File    : USB_Driver_embOS_IP.h
Purpose : Network interface driver for embOS/IP
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USB_DRIVER_EMBOS_IP_H
#define USB_DRIVER_EMBOS_IP_H

#include "USB_Driver_IP_NI.h"
#include "IP.h"

/*********************************************************************
*
*       defines, non-configurable
*/


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       Communication drivers
*
**********************************************************************
*/
extern const IP_HW_DRIVER USB_IP_Driver;

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
