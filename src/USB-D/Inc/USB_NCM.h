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
Purpose : Public header of the USB Network Control Model (NCM)
          The Network Control Model (NCM) is one of the
          Communication Device Class protocols defined by usb.org to
          create a virtual Ethernet connection between a USB
          device and a host PC.
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USB_NCM_H          /* Avoid multiple inclusion */
#define USB_NCM_H

#include "USB.h"
#include "SEGGER.h"
#include "USB_Driver_IP_NI.h"

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
*       USB_NCM_INIT_DATA
*
*   Description
*     Initialization data for NCM interface.
*
*   Additional information
*     This structure holds the endpoints that should be used by
*     the NCM interface (EPIn, EPOut and EPInt). Refer to USBD_AddEP()
*     for more information about how to add an endpoint.
*/
typedef struct USB_NCM_INIT_DATA USB_NCM_INIT_DATA;
struct USB_NCM_INIT_DATA {
  U8   EPIn;      // Bulk IN endpoint for sending data to the host.
  U8   EPOut;     // Bulk OUT endpoint for receiving data from the host.
                  // The buffer associated to this endpoint must be big enough to hold a complete IP packet.
  U8   EPInt;     // Interrupt IN endpoint for sending status information.
  const USB_IP_NI_DRIVER_API * pDriverAPI;   // Pointer to the Network interface driver API.
                                             // See USB_IP_NI_DRIVER_API.
  USB_IP_NI_DRIVER_DATA        DriverData;   // Configuration data for the network interface driver.
  unsigned                     DataInterfaceNo;
};

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void USBD_NCM_Add  (const USB_NCM_INIT_DATA * pInitData);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
