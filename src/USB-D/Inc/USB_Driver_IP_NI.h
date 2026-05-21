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
Purpose : Network interface driver
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USB_DRIVER_IP_NI_H
#define USB_DRIVER_IP_NI_H

/*********************************************************************
*
*       Defines, non-configurable
*/
//
// Number of bytes in the header of an Ethernet II MAC Frame header.
// Destination mac address  - 6 bytes
// Source mac address       - 6 bytes
// EtherType                - 2 bytes
//
#define USB_IP_NI_ETH_HEADER_SIZE   14u
#define USB_IP_NI_HW_ADDR_SIZE      6u         // Number of bytes in the HW address

/*********************************************************************
*
*       Link status
*/
#define USB_IP_NI_LINK_STATUS_DISCONNECTED        0
#define USB_IP_NI_LINK_STATUS_CONNECTED           1

/*********************************************************************
*
*       IDs of the statistical counters
*/
#define USB_IP_NI_STATS_WRITE_PACKET_OK           0
#define USB_IP_NI_STATS_WRITE_PACKET_ERROR        1
#define USB_IP_NI_STATS_READ_PACKET_OK            2
#define USB_IP_NI_STATS_READ_PACKET_ERROR         3
#define USB_IP_NI_STATS_READ_NO_BUFFER            4
#define USB_IP_NI_STATS_READ_ALIGN_ERROR          5
#define USB_IP_NI_STATS_WRITE_ONE_COLLISION       6
#define USB_IP_NI_STATS_WRITE_MORE_COLLISIONS     7


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef int  USB_IP_WRITE_PACKET(const void * pData, U32 NumBytes);  // Callback function to write a packet to USB host
typedef void USB_IP_REPORT_LINKSTATE(void);                          // Callback function to set the report link state
/*********************************************************************
*
*       USB_IP_NI_DRIVER_DATA
*
*  Description
*    Configuration data passed to network interface driver at initialization.
*
*  Additional information
*    When pHWAddr is NULL the MAC is automatically generated.
*/
typedef struct USB_IP_NI_DRIVER_DATA {
  const U8 * pHWAddr;         // Optional pointer to a HW address (or MAC address) of the host network interface.
  unsigned   NumBytesHWAddr;  // Number of bytes in the HW address. Typically 6 bytes.
  void     * pDriverData;     // Pointer to a user context.
} USB_IP_NI_DRIVER_DATA;

/*********************************************************************
*
*       USB_IP_NI_INIT
*
*  Description
*    Initializes the driver.
*
*  Parameters
*    pDriverData    : [IN] Pointer to driver configuration data.
*    pfWritePacket  : Callback function called by the IP stack to
*                     transmit a packet that should be sent to the USB host.
*
*  Return value
*    IP NI driver instance ID.
*
*  Additional information
*    This function is called when the RNDIS/ECM interface is added to
*    the USB stack. Typically the function makes a local copy of
*    the HW address passed in the pDriverData structure.
*    For more information this structure refer to USB_IP_NI_DRIVER_DATA.
*/
typedef unsigned (USB_IP_NI_INIT)                     (const USB_IP_NI_DRIVER_DATA * pDriverData, USB_IP_WRITE_PACKET *pfWritePacket);

/*********************************************************************
*
*       USB_IP_NI_GET_PACKET_BUFFER
*
*  Description
*    Returns a buffer for a data packet.
*
*  Parameters
*    Id          : Instance ID returned from USB_IP_NI_INIT.
*    NumBytes    : Size of the requested buffer in bytes.
*
*  Return value
*    != NULL: Pointer to allocated buffer
*    == NULL: No buffer available
*
*  Additional information
*    The function should allocate a buffer of the requested size.
*    If the buffer can not be allocated a NULL pointer should be returned.
*    The function is called when a data packet is received from PC.
*    The packet data is stored in the returned buffer.
*/
typedef void *   (USB_IP_NI_GET_PACKET_BUFFER)        (unsigned Id, unsigned NumBytes);

/*********************************************************************
*
*       USB_IP_NI_WRITE_PACKET
*
*  Description
*    Delivers a data packet to target IP stack.
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*    pData    : [IN] Data of the received packet.
*    NumBytes : Number of bytes stored in the buffer.
*
*  Additional information
*    The function is called after a data packet has been received from USB.
*    pData points to the buffer returned by the USB_IP_NI_GET_PACKET_BUFFER function.
*/
typedef void     (USB_IP_NI_WRITE_PACKET)             (unsigned Id, const void * pData, unsigned NumBytes);

/*********************************************************************
*
*       USB_IP_NI_SET_PACKET_FILTER
*
*  Description
*    Configures the type of accepted data packets.
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*    Mask     : Type of accepted data packets.
*
*  Additional information
*    The Mask parameter should be interpreted as a boolean value.
*    A value different than 0 indicates that the connection to target
*    IP stack should be established. When the function is called with
*    the Mask parameter set to 0 the connection to target IP stack
*    should be interrupted.
*/
typedef void     (USB_IP_NI_SET_PACKET_FILTER)        (unsigned Id, U32 Mask);

/*********************************************************************
*
*       USB_IP_NI_GET_LINK_STATUS
*
*  Description
*    Returns the status of the connection to target IP stack.
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*
*  Return value
*    == USB_IP_NI_LINK_STATUS_DISCONNECTED : Not connected to target IP stack.
*    == USB_IP_NI_LINK_STATUS_CONNECTED    : Connected to target IP stack.
*/
typedef int      (USB_IP_NI_GET_LINK_STATUS)          (unsigned Id);

/*********************************************************************
*
*       USB_IP_NI_GET_LINK_SPEED
*
*  Description
*    Returns the connection speed.
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*
*  Return value
*    != 0 : The connection speed in units of 100 bits/sec.
*    == 0 : Not connected.
*/
typedef U32      (USB_IP_NI_GET_LINK_SPEED)           (unsigned Id);

/*********************************************************************
*
*       USB_IP_NI_GET_HWADDR
*
*  Description
*    Returns the HW address (MAC address) of the host network interface (PC).
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*    pAddr    : [OUT] The HW address.
*    NumBytes : Maximum number of bytes to store into pAddr.
*
*  Additional information
*    The returned HW address is the one passed to the driver in the call to USB_IP_NI_INIT.
*    Typically the HW address is 6 bytes long.
*/
typedef void     (USB_IP_NI_GET_HWADDR)               (unsigned Id, U8 * pAddr, unsigned NumBytes);

/*********************************************************************
*
*       USB_IP_NI_GET_STATS
*
*  Description
*    Returns statistical counters.
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*    Type     : The type of information requested. See table below.
*
*  Return value
*    Value of the requested statistical counter.
*
*  Additional information
*    The counters should be set to 0 when the USB_IP_NI_RESET function is called.
*/
typedef U32      (USB_IP_NI_GET_STATS)                (unsigned Id, int Type);

/*********************************************************************
*
*       USB_IP_NI_GET_MTU
*
*  Description
*    Returns the maximum transmission unit, the size of the largest data packet which can be transferred.
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*
*  Return value
*    The MTU size in bytes. Typically 1500 bytes.
*/
typedef U32      (USB_IP_NI_GET_MTU)                  (unsigned Id);

/*********************************************************************
*
*       USB_IP_NI_RESET
*
*  Description
*    Resets the driver.
*
*  Parameters
*    Id       : Instance ID returned from USB_IP_NI_INIT.
*/
typedef void     (USB_IP_NI_RESET)                    (unsigned Id);

/*********************************************************************
*
*       USB_IP_NI_SET_WRITE_PACKET_FUNC
*
*  Description
*    Changes the USB_IP_WRITE_PACKET callback which was added via
*    USB_IP_NI_INIT to a different callback function.
*    This function is only called by the stack when IP-over-USB is used.
*    It is not called when RNDIS or ECM is used standalone.
*
*  Parameters
*    Id             : Instance ID returned from USB_IP_NI_INIT.
*    pfWritePacket  : Callback function called by the IP stack to
*                     transmit a packet that should be sent to the USB host.
*/
typedef void     (USB_IP_NI_SET_WRITE_PACKET_FUNC)    (unsigned Id, USB_IP_WRITE_PACKET *pfWritePacket);


/*********************************************************************
*
*       USB_IP_NI_SET_REPORT_LINKSTATE_FUNC
*
*  Description
*    Changes the USB_IP_REPORT_LINKSTATE callback. Normally this is called
*    only once per initialization in order to allow to send notification to
*    the host that the link state has been changed.
*
*  Parameters
*    Id                : Instance ID returned from USB_IP_NI_INIT.
*    pfReportLinkState : Callback function called by the IP stack to
*                        notify the host that the link state has been changed.
*/
typedef void     (USB_IP_NI_SET_REPORT_LINKSTATE_FUNC)(unsigned Id, USB_IP_REPORT_LINKSTATE *pfReportLinkState);

/*********************************************************************
*
*       USB_IP_NI_DRIVER_API
*
*  Description
*    This structure contains the callback functions for the network interface driver.
*
*  Additional information
*    The emUSB-Device-RNDIS/emUSB-Device-CDC-ECM component calls the functions of this API
*    to exchange data and status information with the IP stack running on the target.
*/
typedef struct USB_IP_NI_DRIVER_API {
  USB_IP_NI_INIT                       *pfInit;                   // Initializes the driver.
  USB_IP_NI_GET_PACKET_BUFFER          *pfGetPacketBuffer;        // Returns a buffer for a data packet.
  USB_IP_NI_WRITE_PACKET               *pfWritePacket;            // Delivers a data packet to target IP stack.
  USB_IP_NI_SET_PACKET_FILTER          *pfSetPacketFilter;        // Configures the type of accepted data packets.
  USB_IP_NI_GET_LINK_STATUS            *pfGetLinkStatus;          // Returns the status of the connection to target IP stack.
  USB_IP_NI_GET_LINK_SPEED             *pfGetLinkSpeed;           // Returns the connection speed.
  USB_IP_NI_GET_HWADDR                 *pfGetHWAddr;              // Returns the HW address of the PC.
  USB_IP_NI_GET_STATS                  *pfGetStats;               // Returns statistical counters.
  USB_IP_NI_GET_MTU                    *pfGetMTU;                 // Returns the size of the largest data packet which can be transferred.
  USB_IP_NI_RESET                      *pfReset;                  // Resets the driver.
  USB_IP_NI_SET_WRITE_PACKET_FUNC      *pfSetWritePacketFunc;     // Allows to change the WritePacket callback which was set by pfInit.
  USB_IP_NI_SET_REPORT_LINKSTATE_FUNC  *pfSetReportLinkstateFunc; // Allows to set the report link state change.
} USB_IP_NI_DRIVER_API;

/*********************************************************************
*
*       Communication drivers
*
**********************************************************************
*/
extern USB_IP_NI_DRIVER_API USB_Driver_IP_NI;    // Network interface

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
