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
Purpose : Public header of DFU device class
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBD_DFU_H          /* Avoid multiple inclusion */
#define USBD_DFU_H

#include "SEGGER.h"
#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
//
// Interface modes
//
#define USB_DFU_MODE_RUNTIME      0
#define USB_DFU_MODE_DFU          1
#define USB_DFU_MODE_MIXED        2

//
// DFU attributes
//
#define USB_DFU_ATTR_DOWNLOAD                 (1u << 0)   // Device is download capable.
#define USB_DFU_ATTR_UPLOAD                   (1u << 1)   // Device is upload capable.
#define USB_DFU_ATTR_MANIFEST_TOLERANT        (1u << 2)   // Device is able to communicate via USB after Manifestation phase.
#define USB_DFU_ATTR_WILL_DETACH              (1u << 3)   // device will perform a bus detach-attach sequence when it receives
                                                          // a DFU_DETACH request. The host must not issue a USB Reset.

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
//
// DFU error status
//
typedef enum {
  errNone = 0,          // No error condition is present.
  errTARGET,            // File is not targeted for use by this device.
  errFILE,              // File is for this device but fails some vendor-specific verification test.
  errWRITE,             // Device is unable to write memory.
  errERASE,             // Memory erase function failed.
  errCHECK_ERASED,      // Memory erase check failed.
  errPROG,              // Program memory function failed.
  errVERIFY,            // Programmed memory failed verification.
  errADDRESS,           // Cannot program memory due to received address that is out of range.
  errNOTDONE,           // Received DFU_DNLOAD with wLength = 0, but device does not think it has all of the data yet.
  errFIRMWARE,          // Device's firmware is corrupt. It cannot return to run-time (non-DFU) operations.
  errVENDOR,            // iString indicates a vendor-specific error.
  errUSBR,              // Device detected unexpected USB reset signaling.
  errPOR,               // Device detected unexpected power on reset.
  errUNKNOWN,           // Something went wrong, but the device does not know what it was.
  errSTALLEDPKT         // Device stalled an unexpected request.
} USB_DFU_ERROR_STATE;

/*********************************************************************
*
*       USBD_DFU_DOWNLOAD
*
*  Description
*    Callback function to handle download data to the application that was received from the host.
*    The function is called in interrupt context and should return as fast as possible.
*    Especially flash programming must not be done within this function.
*    If NumBytes >= 0, the application must respond either with a call to USBD_DFU_Ack() if the data could
*    be processed successfully or by calling USBD_DFU_SetError() if an error occurred.
*    These functions need not to be called from the USBD_DFU_DOWNLOAD function, but may be called
*    later after processing the data. The host will wait for either USBD_DFU_Ack() or
*    USBD_DFU_SetError() before starting another download.
*
*  Parameters
*    NumBytes     : Number of bytes received from the host. The data is stored in the buffer provided
*                   by USB_DFU_INIT_DATA.pBuffer.
*                   A value of 0 indicates the end of the data to be downloaded.
*                   A negative value means that the host has aborted the download.
*    BlockNum     : Block sequence number provided by the host.
*/
typedef void USBD_DFU_DOWNLOAD(int NumBytes, U16 BlockNum);

/*********************************************************************
*
*       USBD_DFU_UPLOAD
*
*  Description
*    Callback function to get upload data to be transferred to the host.
*    The function is called in interrupt context and should return as fast as possible.
*
*  Parameters
*    bStart       : 1 = Start upload, 0 = continue upload.
*    BlockNum     : Block sequence number provided by the host.
*    NumBytes     : Number of bytes requested by the host.
*    ppData       : [OUT] Pointer to the data to be transfered to the host.
*
*  Return value
*    Size of the data provided by the function (in bytes).
*    A value < NumBytes (including 0) indicate the last part of the data.
*    A negative value indicates an error. In case of an error, the function should
*    also call USBD_DFU_SetError().
*/
typedef int USBD_DFU_UPLOAD(int bStart, U16 BlockNum, U16 NumBytes, const U8 ** ppData);

/*********************************************************************
*
*       USBD_DFU_DETACH_REQUEST
*
*  Description
*   Callback function is called when the host requests a DETACH, prompting the
*   device to enter DFU mode. This function is executed in interrupt context.
*   The detach and/or reinitialization must not be performed inside this function.
*   Instead this function should only trigger a task to perform the required operation.
*
*  Parameters
*    Timeout     : Timeout provided by the host.
*/
typedef void USBD_DFU_DETACH_REQUEST(U16 Timeout);

/*********************************************************************
*
*       USBD_DFU_INIT_DATA
*
*  Description
*    Initialization data for the DFU interface.
*/
typedef struct {
  I8                         Mode;            // Operation mode of the DFU interface:
                                              // + USB_DFU_MODE_RUNTIME: The interface is in runtime mode only. Download of firmware data is not supported.
                                              // + USB_DFU_MODE_DFU: The interface is in DFU mode.
                                              // + USB_DFU_MODE_MIXED: The interface is in runtime mode but allows download of firmware data in this mode.
  U8                         Attributes;      // Bit mask containing the DFU attributes. Combination of the
                                              // USB_DFU_ATTR_... flags.
  U16                        DetachTimeout;   // Time, in milliseconds, that the device will wait after receipt
                                              // of the DFU_DETACH request.
  U16                        TransferSize;    // Maximum number of bytes that the device can accept per
                                              // control-write transaction.
  U16                        Flags;           // RFU. Must be 0.
  const char               * pInterfaceName;  // Name of the interface. Optional, may be NULL.
  USBD_DFU_DETACH_REQUEST  * pfDetachRequest; // Pointer to the callback function to request a detach.
                                              // Used for Mode == USB_DFU_MODE_RUNTIME only.
  USBD_DFU_DOWNLOAD        * pfDownload;      // Pointer to the callback function to receive download data.
                                              // Used for Mode != USB_DFU_MODE_RUNTIME only.
  U8                       * pBuffer;         // Pointer to a buffer to store download data.
                                              // The size of the buffer must be 'TransferSize' bytes.
  USBD_DFU_UPLOAD          * pfUpload;        // Pointer to the callback function to get upload data.
                                              // Optional. Used for Mode != USB_DFU_MODE_RUNTIME only.
} USB_DFU_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void              USBD_DFU_Add                   (const USB_DFU_INIT_DATA * pInitData);
void              USBD_DFU_Add_RunTime           (const USB_DFU_INIT_DATA * pInitData);
void              USBD_DFU_AddAlternateInterface (const char *pInterfaceName);
void              USBD_DFU_SetMSDescInfo         (void);
void              USBD_DFU_Ack                   (void);
void              USBD_DFU_SetPollTimeout        (U32 PollTimeout);
void              USBD_DFU_SetError              (USB_DFU_ERROR_STATE Err);
void              USBD_DFU_ManifestComplt        (void);
unsigned          USBD_DFU_GetStatusReqCnt       (void);
unsigned          USBD_DFU_GetAlternateSetting   (void);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
