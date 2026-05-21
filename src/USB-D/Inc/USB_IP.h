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
Purpose : Public header of the emUSB-Device-IP component.
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USB_IP_H          /* Avoid multiple inclusion */
#define USB_IP_H

#include "USB.h"
#include "SEGGER.h"
#include "USB_Driver_IP_NI.h"
#include "USB_RNDIS.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       USB_IP_INIT_DATA
*
*  Description
*    Structure which stores the parameters of the IP component.
*
*  Additional information
*    This structure holds the endpoints that should be used with the
*    IP component. Refer to USBD_AddEP() for more information about
*    how to add an endpoint.
*/
typedef struct USB_IP_INIT_DATA {
  U8   EPIn;      // Bulk IN endpoint to send data packets to the USB host.
  U8   EPOut;     // Bulk OUT endpoint to receive data packets from the USB host.
  U8   EPInt;     // Interrupt IN endpoint to send notifications to the USB host.
  const USB_IP_NI_DRIVER_API  * pDriverAPI;     // Network interface driver API.
  USB_IP_NI_DRIVER_DATA         DriverData;     // Data passed at initialization to low-level driver.
  const USB_RNDIS_DEVICE_INFO * pRndisDevInfo;  // Pointer to a filled USB_RNDIS_DEVICE_INFO structure.
} USB_IP_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void USBD_IP_Add  (const USB_IP_INIT_DATA * pInitData);
void USBD_IP_NCM_Add(const USB_IP_INIT_DATA * pInitData);
void USBD_IP_Task (void);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
