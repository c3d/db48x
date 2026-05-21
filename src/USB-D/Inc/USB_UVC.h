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
Purpose : Public header of the human interface device class
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBD_UVC_H          /* Avoid multiple inclusion */
#define USBD_UVC_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include "SEGGER.h"
#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#ifndef USBD_UVC_NUM_BUFFERS
  #define USBD_UVC_NUM_BUFFERS                  8u
#endif
#ifndef USBD_UVC_DATA_BUFFER_SIZE
  #define USBD_UVC_DATA_BUFFER_SIZE             3072u
#endif
#define USBD_UVC_CAM_FPS                        30u
#define USBD_UVC_MIN_BIT_RATE(Width, Height)    ((Width) * (Height) * 16u * USBD_UVC_CAM_FPS)// 16 bit
#define USBD_UVC_MAX_BIT_RATE(Width, Height)    ((Width) * (Height) * 16u * USBD_UVC_CAM_FPS)
#define USBD_UVC_MAX_FRAME_SIZE(Width, Height)  ((Width) * (Height) * 2u)  //YUYV2
//#define USBD_UVC_MAX_FRAME_SIZE                 (unsigned long)(USBD_UVC_WIDTH*USBD_UVC_HEIGHT*3/2) // NV12
#define USBD_UVC_INTERVAL                       (10000000uL/USBD_UVC_CAM_FPS)
#define USBD_UVC_DEFAULT_COMPRESSION_INDEX      0u

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define USBD_UVC_PROBE_REQUEST                  0x0100u
#define USBD_UVC_COMMIT_REQUEST                 0x0200u
#define USBD_UVC_STILL_PROBE                    0x0300u
#define USBD_UVC_STILL_COMMIT                   0x0400u
#define USBD_UVC_STILL_TRIGGER                  0x0500u

#define USBD_UVC_USE_BULK_MODE                  (1u << 0)
#define USBD_UVC_INPUT_TERMINAL_ID              0x01u
#define USBD_UVC_PROCESSING_UNIT_ID             0x02u
#define USBD_UVC_OUTPUT_TERMINAL_ID             0x03u

// UVC class, subclass codes
// (USB_Video_Class_1.1.pdf, 3.2 Device Descriptor)
#define UVC_DEVICE_CLASS_MISCELLANEOUS            0xFE
#define UVC_DEVICE_SUBCLASS                       0x02
#define UVC_DEVICE_PROTOCOL                       0x01

#define USBD_UVC_PAYLOAD_HEADER_SIZE              2u

#define USBD_UVC_END_OF_FRAME                     (1u << 1)
#define USBD_UVC_BIT_FRAME_ID                     (1u << 0)

#define USB_DEVICE_DESC_SIZE                      (sizeof(USB_DEVICE_DESCRIPTOR))
#define USB_CONFIGUARTION_DESC_SIZE               9
#define USB_INTERFACE_DESC_SIZE                   9
#define USB_ENDPOINT_DESC_SIZE                    7
#define UVC_INTERFACE_ASSOCIATION_DESC_SIZE       8
#define UVC_VC_ENDPOINT_DESC_SIZE                 5

#define UVC_VC_INTERFACE_HEADER_DESC_SIZE(n)      (12 + (n))
#define UVC_CAMERA_TERMINAL_DESC_SIZE(n)          (16 + (n))
#define UVC_OUTPUT_TERMINAL_DESC_SIZE(n)          (9  + (n))
#define UVC_INPUT_TERMINAL_DESC_SIZE(n)           (15 + (n))
#define UVC_PROCESSING_UNIT_DESC_SIZE(n)          (10 + (n))
#define UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(a,b) (13u + (a) * (b))


#define VS_FORMAT_UNCOMPRESSED_DESC_SIZE          (27u)
#define VS_FORMAT_MJPEG_DESC_SIZE                 (11)
#define VS_FRAME_UNCOMPRESSED_DESC_SIZE           (30u)
#define VS_STILL_IMAGE_DESC_SIZE(n)               (7u + (n) * 4u)
#define VS_COLOR_MATCHING_DESC_SIZE               (6)

#define USBD_UVC_VCIF_NUM                         0
#define USBD_UVC_VSIF_NUM                         1

#define UVC_TOTAL_IF_NUM                          2

// USB Video device class specification version 1.1
#define UVC_VERSION                               0x0110

// Video Interface Class Codes
// (USB_Video_Class_1.1.pdf, A.1 Video Interface Class Code)
#define CC_VIDEO                                  0x0E

// Video Interface Subclass Codes
// (USB_Video_Class_1.1.pdf, A.2 Video Interface Subclass Code)
#define SC_UNDEFINED                              0x00
#define SC_VIDEOCONTROL                           0x01
#define SC_VIDEOSTREAMING                         0x02
#define SC_VIDEO_INTERFACE_COLLECTION             0x03

// Video Interface Protocol Codes
// (USB_Video_Class_1.1.pdf, A.3 Video Interface Protocol Codes)
#define PC_PROTOCOL_UNDEFINED                     0x00

// Video Class-Specific Descriptor Types
// (USB_Video_Class_1.1.pdf, A.4 Video Class-Specific Descriptor Types)
#define CS_UNDEFINED                              0x20
#define CS_DEVICE                                 0x21
#define CS_CONFIGURATION                          0x22
#define CS_STRING                                 0x23
#define CS_INTERFACE                              0x24
#define CS_ENDPOINT                               0x25

// Video Class-Specific VideoControl Interface Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.5 Video Class-Specific VC Interface Descriptor Subtypes)
#define VC_DESCRIPTOR_UNDEFINED                   0x00
#define VC_HEADER                                 0x01
#define VC_INPUT_TERMINAL                         0x02
#define VC_OUTPUT_TERMINAL                        0x03
#define VC_SELECTOR_UNIT                          0x04
#define VC_PROCESSING_UNIT                        0x05
#define VC_EXTENSION_UNIT                         0x06

// Video Class-Specific VideoStreaming Interface Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.6 Video Class-Specific VS Interface Descriptor Subtypes)
#define VS_UNDEFINED                              0x00
#define VS_INPUT_HEADER                           0x01
#define VS_OUTPUT_HEADER                          0x02
#define VS_STILL_IMAGE_FRAME                      0x03
#define VS_FORMAT_UNCOMPRESSED                    0x04
#define VS_FRAME_UNCOMPRESSED                     0x05
#define VS_FORMAT_MJPEG                           0x06
#define VS_FRAME_MJPEG                            0x07
#define VS_FORMAT_MPEG2TS                         0x0A
#define VS_FORMAT_DV                              0x0C
#define VS_COLORFORMAT                            0x0D
#define VS_FORMAT_FRAME_BASED                     0x10
#define VS_FRAME_FRAME_BASED                      0x11
#define VS_FORMAT_STREAM_BASED                    0x12

// Video Class-Specific Endpoint Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.7 Video Class-Specific Endpoint Descriptor Subtypes)
#define EP_UNDEFINED                              0x00
#define EP_GENERAL                                0x01
#define EP_ENDPOINT                               0x02
#define EP_INTERRUPT                              0x03

// Video Class-Specific Request Codes
// (USB_Video_Class_1.1.pdf, A.8 Video Class-Specific Request Codes)
#define RC_UNDEFINED                              0x00
#define SET_CUR                                   0x01
#define GET_CUR                                   0x81
#define GET_MIN                                   0x82
#define GET_MAX                                   0x83
#define GET_RES                                   0x84
#define GET_LEN                                   0x85
#define GET_INFO                                  0x86
#define GET_DEF                                   0x87

// VideoControl Interface Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.1 VideoControl Interface Control Selectors)
#define VC_CONTROL_UNDEFINED                      0x00u
#define VC_VIDEO_POWER_MODE_CONTROL               0x01u
#define VC_REQUEST_ERROR_CODE_CONTROL             0x02u

// Request Error Code Control
// (USB_Video_Class_1.1.pdf, 4.2.1.2 Request Error Code Control)
#define VC_NO_ERROR_ERR                           0x00
#define VC_NOT_READY_ERR                          0x01
#define VC_WRONG_STATE_ERR                        0x02
#define VC_POWER_ERR                              0x03
#define VC_OUT_OF_RANGE_ERR                       0x04
#define VC_INVALID_UNIT_ERR                       0x05
#define VC_INVALID_CONTROL_ERR                    0x06
#define VC_INVALID_REQUEST_ERR                    0x07
#define VC_UNKNOWN_ERR                            0xFF

// Terminal Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.2 Terminal Control Selectors)
#define TE_CONTROL_UNDEFINED                      0x00

// Selector Unit Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.3 Selector Unit Control Selectors)
#define SU_CONTROL_UNDEFINED                      0x00
#define SU_INPUT_SELECT_CONTROL                   0x01

// Camera Terminal Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.4 Camera Terminal Control Selectors)
#define CT_CONTROL_UNDEFINED                      0x00
#define CT_SCANNING_MODE_CONTROL                  0x01
#define CT_AE_MODE_CONTROL                        0x02
#define CT_AE_PRIORITY_CONTROL                    0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL         0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL         0x05
#define CT_FOCUS_ABSOLUTE_CONTROL                 0x06
#define CT_FOCUS_RELATIVE_CONTROL                 0x07
#define CT_FOCUS_AUTO_CONTROL                     0x08
#define CT_IRIS_ABSOLUTE_CONTROL                  0x09
#define CT_IRIS_RELATIVE_CONTROL                  0x0A
#define CT_ZOOM_ABSOLUTE_CONTROL                  0x0B
#define CT_ZOOM_RELATIVE_CONTROL                  0x0C
#define CT_PANTILT_ABSOLUTE_CONTROL               0x0D
#define CT_PANTILT_RELATIVE_CONTROL               0x0E
#define CT_ROLL_ABSOLUTE_CONTROL                  0x0F
#define CT_ROLL_RELATIVE_CONTROL                  0x10
#define CT_PRIVACY_CONTROL                        0x11

// Processing Unit Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.5 Processing Unit Control Selectors)
#define PU_CONTROL_UNDEFINED                      0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL         0x01
#define PU_BRIGHTNESS_CONTROL                     0x02
#define PU_CONTRAST_CONTROL                       0x03
#define PU_GAIN_CONTROL                           0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL           0x05
#define PU_HUE_CONTROL                            0x06
#define PU_SATURATION_CONTROL                     0x07
#define PU_SHARPNESS_CONTROL                      0x08
#define PU_GAMMA_CONTROL                          0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL      0x0A
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL 0x0B
#define PU_WHITE_BALANCE_COMPONENT_CONTROL        0x0C
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL   0x0D
#define PU_DIGITAL_MULTIPLIER_CONTROL             0x0E
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL       0x0F
#define PU_HUE_AUTO_CONTROL                       0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL          0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL             0x12

// Extension Unit Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.6 Extension Unit Control Selectors)
#define XU_CONTROL_UNDEFINED                      0x00

// VideoStreaming Interface Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.7 VideoStreaming Interface Control Selectors)
#define VS_CONTROL_UNDEFINED                      0x00
#define VS_PROBE_CONTROL                          0x01
#define VS_COMMIT_CONTROL                         0x02
#define VS_STILL_PROBE_CONTROL                    0x03
#define VS_STILL_COMMIT_CONTROL                   0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL            0x05
#define VS_STREAM_ERROR_CODE_CONTROL              0x06
#define VS_GENERATE_KEY_FRAME_CONTROL             0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL           0x08
#define VS_SYNC_DELAY_CONTROL                     0x09

// Defined Bits Containing Capabilities of the Control
// (USB_Video_Class_1.1.pdf, 4.1.2 Table 4-3 Defined Bits Containing Capabilities of the Control)
#define SUPPORTS_GET                              0x01
#define SUPPORTS_SET                              0x02
#define STATE_DISABLED                            0x04
#define AUTOUPDATE_CONTROL                        0x08
#define ASYNCHRONOUS_CONTROL                      0x10

// USB Terminal Types
// (USB_Video_Class_1.1.pdf, B.1 USB Terminal Types)
#define TT_VENDOR_SPECIFIC                        0x0100
#define TT_STREAMING                              0x0101

// Input Terminal Types
// (USB_Video_Class_1.1.pdf, B.2 Input Terminal Types)
#define ITT_VENDOR_SPECIFIC                       0x0200
#define ITT_CAMERA                                0x0201
#define ITT_MEDIA_TRANSPORT_INPUT                 0x0202

// Output Terminal Types
// (USB_Video_Class_1.1.pdf, B.3 Output Terminal Types)
#define OTT_VENDOR_SPECIFIC                       0x0300
#define OTT_DISPLAY                               0x0301
#define OTT_MEDIA_TRANSPORT_OUTPUT                0x0302

// External Terminal Types
// (USB_Video_Class_1.1.pdf, B.4 External Terminal Types)
#define EXTERNAL_VENDOR_SPECIFIC                  0x0400
#define COMPOSITE_CONNECTOR                       0x0401
#define SVIDEO_CONNECTOR                          0x0402
#define COMPONENT_CONNECTOR                       0x0403


#define USB_VIDEO_DESC_SIZ (unsigned)(\
          UVC_VC_INTERFACE_HEADER_DESC_SIZE(1) + \
          UVC_INPUT_TERMINAL_DESC_SIZE(3) + \
          UVC_OUTPUT_TERMINAL_DESC_SIZE(0) + \
          UVC_PROCESSING_UNIT_DESC_SIZE(2))

#define VC_TERMINAL_SIZ (unsigned int)(UVC_VC_INTERFACE_HEADER_DESC_SIZE(1) + UVC_CAMERA_TERMINAL_DESC_SIZE(2) + UVC_OUTPUT_TERMINAL_DESC_SIZE(0))

#define UVC_DBVAL(x) ((x) & 0xFFu), (((x) >> 8) & 0xFFu), (((x) >> 16) & 0xFFu), (((x) >> 24) & 0xFFu)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef int USBD_UVC_HANDLE;

/*********************************************************************
*
*       USB_UVC_ON_RESOLUTION_CHANGE
*
*   Description
*     Callback function description which is set via
*     USBD_UVC_SetOnResolutionChange().
*
*   Parameters
*     FrameIndex  - 1-based index of the frame resolution.
*/
typedef void USB_UVC_ON_RESOLUTION_CHANGE(unsigned FrameIndex);

/*********************************************************************
*
*       USB_UVC_ON_CONTROL_8
*
*   Description
*     Callback function for a specific control.
*
*   Parameters
*     IsSetRequest - The callback is called for both SET_CUR and GET_CUR requests.
*                    When this parameter is 1 the request is SET_CUR,
*                    when it is 0 the request is GET_CUR
*     Control      - Control Selector ID.
*     Value        - For SET_CUR this is the new value for the control.
*                    For GET_CUR this parameter is 0.
*
*   Return value
*     In case of a SET_CUR return 0 on success, 1 on failure (the UVC module will STALL the request then).
*     In case of a GET_CUR return the current value of the control.
*
*   Additional information
*     This callback is called in an interrupt context. It must not block and must
*     execute swiftly to avoid time outs on the host side.
*/
typedef U8 USB_UVC_ON_CONTROL_8(U8 IsSetRequest, unsigned Control, U8 Value);

/*********************************************************************
*
*       USB_UVC_ON_CONTROL_8_8
*
*   Description
*     Callback function for a specific control.
*
*   Parameters
*     IsSetRequest - The callback is called for both SET_CUR and GET_CUR requests.
*                    When this parameter is 1 the request is SET_CUR,
*                    when it is 0 the request is GET_CUR
*     Control      - Control Selector ID.
*     Value1       - For SET_CUR this is a pointer to the first new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*     Value2       - For SET_CUR this is a pointer to the second new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*   Return value
*     In case of a SET_CUR return 0 on success, 1 on failure (the UVC module will STALL the request then).
*     In case of a GET_CUR return 0.
*
*   Additional information
*     This callback is called in an interrupt context. It must not block and must
*     execute swiftly to avoid time outs on the host side.
*/
typedef U8 USB_UVC_ON_CONTROL_8_8(U8 IsSetRequest, unsigned Control, U8 * Value1, U8 * Value2);

/*********************************************************************
*
*       USB_UVC_ON_CONTROL_8_8_8
*
*   Description
*     Callback function for a specific control.
*
*   Parameters
*     IsSetRequest - The callback is called for both SET_CUR and GET_CUR requests.
*                    When this parameter is 1 the request is SET_CUR,
*                    when it is 0 the request is GET_CUR
*     Control      - Control Selector ID.
*     Value1       - For SET_CUR this is a pointer to the first new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*     Value2       - For SET_CUR this is a pointer to the second new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*     Value3       - For SET_CUR this is a pointer to the third new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*   Return value
*     In case of a SET_CUR return 0 on success, 1 on failure (the UVC module will STALL the request then).
*     In case of a GET_CUR return 0.
*
*   Additional information
*     This callback is called in an interrupt context. It must not block and must
*     execute swiftly to avoid time outs on the host side.
*/
typedef U8 USB_UVC_ON_CONTROL_8_8_8(U8 IsSetRequest, unsigned Control, U8 * Value1, U8 * Value2, U8 * Value3);


/*********************************************************************
*
*       USB_UVC_ON_CONTROL_16
*
*   Description
*     Callback function for a specific control.
*
*   Parameters
*     IsSetRequest - The callback is called for both SET_CUR and GET_CUR requests.
*                    When this parameter is 1 the request is SET_CUR,
*                    when it is 0 the request is GET_CUR
*     Control      - Control Selector ID.
*     Value        - For SET_CUR this is the new value for the control.
*                    For GET_CUR this parameter is 0.
*
*   Return value
*     In case of a SET_CUR return 0 on success, 1 on failure (the UVC module will STALL the request then).
*     In case of a GET_CUR return the current value of the control.
*
*   Additional information
*     This callback is called in an interrupt context. It must not block and must
*     execute swiftly to avoid time outs on the host side.
*/
typedef U16 USB_UVC_ON_CONTROL_16(U8 IsSetRequest, unsigned Control, U16 Value);

/*********************************************************************
*
*       USB_UVC_ON_CONTROL_16_16
*
*   Description
*     Callback function for a specific control.
*
*   Parameters
*     IsSetRequest - The callback is called for both SET_CUR and GET_CUR requests.
*                    When this parameter is 1 the request is SET_CUR,
*                    when it is 0 the request is GET_CUR
*     Control      - Control Selector ID.
*     Value1       - For SET_CUR this is a pointer to the first new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*     Value2       - For SET_CUR this is a pointer to the second new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*   Return value
*     In case of a SET_CUR return 0 on success, 1 on failure (the UVC module will STALL the request then).
*     In case of a GET_CUR return 0.
*
*   Additional information
*     This callback is called in an interrupt context. It must not block and must
*     execute swiftly to avoid time outs on the host side.
*/
typedef U8 USB_UVC_ON_CONTROL_16_16(U8 IsSetRequest, unsigned Control, U16 * Value1, U16 * Value2);

/*********************************************************************
*
*       USB_UVC_ON_CONTROL_32
*
*   Description
*     Callback function for a specific control.
*
*   Parameters
*     IsSetRequest - The callback is called for both SET_CUR and GET_CUR requests.
*                    When this parameter is 1 the request is SET_CUR,
*                    when it is 0 the request is GET_CUR
*     Control      - Control Selector ID.
*     Value        - For SET_CUR this is the new value for the control.
*                    For GET_CUR this parameter is 0.
*
*   Return value
*     In case of a SET_CUR return 0 on success, 1 on failure (the UVC module will STALL the request then).
*     In case of a GET_CUR return the current value of the control.
*
*   Additional information
*     This callback is called in an interrupt context. It must not block and must
*     execute swiftly to avoid time outs on the host side.
*/
typedef U32 USB_UVC_ON_CONTROL_32(U8 IsSetRequest, unsigned Control, U32 Value);

/*********************************************************************
*
*       USB_UVC_ON_CONTROL_32_32
*
*   Description
*     Callback function for a specific control.
*
*   Parameters
*     IsSetRequest - The callback is called for both SET_CUR and GET_CUR requests.
*                    When this parameter is 1 the request is SET_CUR,
*                    when it is 0 the request is GET_CUR
*     Control      - Control Selector ID.
*     Value1       - For SET_CUR this is a pointer to the first new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*     Value2       - For SET_CUR this is a pointer to the second new value for the control.
*                    For GET_CUR this parameter should be set to the current value.
*   Return value
*     In case of a SET_CUR return 0 on success, 1 on failure (the UVC module will STALL the request then).
*     In case of a GET_CUR return 0.
*
*   Additional information
*     This callback is called in an interrupt context. It must not block and must
*     execute swiftly to avoid time outs on the host side.
*/
typedef U8 USB_UVC_ON_CONTROL_32_32(U8 IsSetRequest, unsigned Control, U32 * Value1, U32 * Value2);

/*********************************************************************
*
*       USBD_UVC_DATA_BUFFER
*
*   Description
*     Structure which contains values for a single buffer.
*
*   Additional information
*     The size of the buffers can be set with the USBD_UVC_DATA_BUFFER_SIZE
*     define. Ideally it should match the MaxPacketSize for the isochronous
*     endpoint.
*/
typedef struct _USBD_UVC_DATA_BUFFER {
  U8      * pData;                           // Pointer to a data buffer.
                                             // When USBD_UVC_Write() is used the user must set this pointer to a valid buffer of size USBD_UVC_DATA_BUFFER_SIZE.
                                             // When USBD_UVC_WriteEx() is used the user must not modify this value.
  unsigned  NumBytesIn;                      // Size of the packet.
  U8        Flags;                           // Flags which will be sent with the packet.
  U8        FrameID;                         // ID of the frame.
} USBD_UVC_DATA_BUFFER;

/*********************************************************************
*
*       USBD_UVC_BUFFER
*
*   Description
*     Structure which contains information about the UVC ring buffer.
*
*   Additional information
*     The number of buffers can be set with the USBD_UVC_NUM_BUFFERS
*     define. Generally the user does not have to interact with this
*     structure, but he has to provide the memory for it.
*     When USBD_UVC_USE_BULK_MODE is used USBD_UVC_NUM_BUFFERS can be reduced to 1.
*/
typedef struct _USBD_UVC_BUFFER {
  USBD_UVC_DATA_BUFFER  Buf[USBD_UVC_NUM_BUFFERS]; // Array of USBD_UVC_DATA_BUFFER elements.
  volatile U8           NumBlocksIn;               // Number of currently used buffers.
  U8                    RdPos;                     // Buffer read position.
  U8                    WrPos;                     // Buffer write position.
  U8                    Flags;                     // Used by the UVC module automatically. Do not modify. 1 - WriteEx used.
} USBD_UVC_BUFFER;

/*********************************************************************
*
*       USBD_UVC_RESOLUTION
*
*   Description
*     Structure describing a valid image resolution.
*/
typedef struct _USBD_UVC_RESOLUTION {
  unsigned Width;     // Width in pixels.
  unsigned Height;    // Height in pixels.
} USBD_UVC_RESOLUTION;

/*********************************************************************
*
*       USBD_UVC_CONTROL_8
*
*   Description
*     Structure describing a control setting which uses U8 as a parameter.
*/
typedef struct _USBD_UVC_CONTROL_8 {
  U16 Min;                      // Minimum value of the control.
  U16 Max;                      // Maximum value of the control.
  U16 Res;                      // Control resolution (steps in which the host can change
                                // the control e.g. 1 means 1,2,3,4... is allowed, 5 means 5,10,15,20... is allowed).
  USB_UVC_ON_CONTROL_8 * pfCb;  // Callback to be executed when the control is changed by the host.
                                // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_8;

/*********************************************************************
*
*       USBD_UVC_CONTROL_8_8
*
*   Description
*     Structure describing a control setting which uses two U8 as parameters.
*/
typedef struct _USBD_UVC_CONTROL_8_8 {
  U8 Min1;                          // Minimum value of the first control.
  U8 Min2;                          // Minimum value of the second control.
  U8 Max1;                          // Maximum value of the first control.
  U8 Max2;                          // Maximum value of the second control.
  U8 Res1;                          // Control resolution of the first control (steps in which the host can change
                                    // the control e.g. 1 means 1,2,3,4... is allowed, 5 means 5,10,15,20... is allowed).
  U8 Res2;                          // Control resolution of the second control.
  USB_UVC_ON_CONTROL_8_8 * pfCb;    // Callback to be executed when the control is changed by the host.
                                    // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_8_8;


/*********************************************************************
*
*       USBD_UVC_CONTROL_8_8_8
*
*   Description
*     Structure describing a control setting which uses three U8 as parameters.
*/
typedef struct _USBD_UVC_CONTROL_8_8_8 {
  U8 Min1;                          // Minimum value of the first control.
  U8 Min2;                          // Minimum value of the second control.
  U8 Min3;                          // Minimum value of the third control.
  U8 Max1;                          // Maximum value of the first control.
  U8 Max2;                          // Maximum value of the second control.
  U8 Max3;                          // Maximum value of the third control.
  U8 Res1;                          // Control resolution of the first control (steps in which the host can change
                                    // the control e.g. 1 means 1,2,3,4... is allowed, 5 means 5,10,15,20... is allowed).
  U8 Res2;                          // Control resolution of the second control.
  U8 Res3;                          // Control resolution of the third control.
  USB_UVC_ON_CONTROL_8_8_8 * pfCb;  // Callback to be executed when the control is changed by the host.
                                    // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_8_8_8;


/*********************************************************************
*
*       USBD_UVC_CONTROL_16
*
*   Description
*     Structure describing a control setting which uses U16 as a parameter.
*/
typedef struct _USBD_UVC_CONTROL_16 {
  U16 Min;                      // Minimum value of the control.
  U16 Max;                      // Maximum value of the control.
  U16 Res;                      // Control resolution (steps in which the host can change
                                // the control e.g. 1 means 1,2,3,4... is allowed, 5 means 5,10,15,20... is allowed).
  USB_UVC_ON_CONTROL_16 * pfCb; // Callback to be executed when the control is changed by the host.
                                // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_16;

/*********************************************************************
*
*       USBD_UVC_CONTROL_16_16
*
*   Description
*     Structure describing a control setting which uses two U16 as parameters.
*/
typedef struct _USBD_UVC_CONTROL_16_16 {
  U16 Min1;                          // Minimum value of the first control.
  U16 Min2;                          // Minimum value of the second control.
  U16 Max1;                          // Maximum value of the first control.
  U16 Max2;                          // Maximum value of the second control.
  U16 Res1;                          // Control resolution of the first control (steps in which the host can change
                                     // the control e.g. 1 means 1,2,3,4... is allowed, 5 means 5,10,15,20... is allowed).
  U16 Res2;                          // Control resolution of the second control.
  USB_UVC_ON_CONTROL_16_16 * pfCb;   // Callback to be executed when the control is changed by the host.
                                     // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_16_16;


/*********************************************************************
*
*       USBD_UVC_CONTROL_32
*
*   Description
*     Structure describing a control setting which uses U32 as a parameter.
*/
typedef struct _USBD_UVC_CONTROL_32 {
  U32 Min;                      // Minimum value of the control.
  U32 Max;                      // Maximum value of the control.
  U32 Res;                      // Control resolution (steps in which the host can change
                                // the control e.g. 1 means 1,2,3,4... is allowed, 5 means 5,10,15,20... is allowed).
  USB_UVC_ON_CONTROL_32 * pfCb; // Callback to be executed when the control is changed by the host.
                                // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_32;

/*********************************************************************
*
*       USBD_UVC_CONTROL_32_32
*
*   Description
*     Structure describing a control setting which uses two U32 as parameters.
*/
typedef struct _USBD_UVC_CONTROL_32_32 {
  U32 Min1;                          // Minimum value of the first control.
  U32 Min2;                          // Minimum value of the second control.
  U32 Max1;                          // Maximum value of the first control.
  U32 Max2;                          // Maximum value of the second control.
  U32 Res1;                          // Control resolution of the first control (steps in which the host can change
                                     // the control e.g. 1 means 1,2,3,4... is allowed, 5 means 5,10,15,20... is allowed).
  U32 Res2;                          // Control resolution of the second control.
  USB_UVC_ON_CONTROL_32_32 * pfCb;   // Callback to be executed when the control is changed by the host.
                                     // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_32_32;

/*********************************************************************
*
*       USBD_UVC_CONTROL_BOOL
*
*   Description
*     Structure describing a boolean control setting.
*/
typedef struct _USBD_UVC_CONTROL_BOOL {
  USB_UVC_ON_CONTROL_8 * pfCb;  // Callback to be executed when the control is changed by the host.
                                // This callback is called in an interrupt context.
} USBD_UVC_CONTROL_BOOL;



/*********************************************************************
*
*       USBD_UVC_CONTROLS
*
*   Description
*     Structure describing control settings. See USBD_UVC_CONTROLS.
*/
typedef struct _USBD_UVC_CONTROLS {
  //
  // Camera Terminal controls
  //
  USBD_UVC_CONTROL_BOOL   * pScanning;            // Scanning Mode Control
  USBD_UVC_CONTROL_8      * pAuto_Exp_Mode;       // Auto-Exposure Mode Control
  USBD_UVC_CONTROL_8      * pAuto_Exp_Prio;       // Auto-Exposure Priority Control
  USBD_UVC_CONTROL_32     * pExp_Time_Abs;        // Exposure Time (Absolute) Control
  USBD_UVC_CONTROL_8      * pExp_Time_Rel;        // Exposure Time (Relative) Control
  USBD_UVC_CONTROL_16     * pFocus_Abs;           // Focus (Absolute) Control
  USBD_UVC_CONTROL_8_8    * pFocus_Rel;           // Focus (Relative) Control
  USBD_UVC_CONTROL_BOOL   * pFocus_Auto;          // Focus, Auto Control
  USBD_UVC_CONTROL_16     * pIris_Abs;            // Iris (Absolute) Control
  USBD_UVC_CONTROL_8      * pIris_Rel;            // Iris (Relative) Control
  USBD_UVC_CONTROL_16     * pZoom_Abs;            // Zoom (Absolute) Control
  USBD_UVC_CONTROL_8_8_8  * pZoom_Rel;            // Zoom (Relative) Control
  USBD_UVC_CONTROL_32_32  * pPanTilt_Abs;         // PanTilt (Absolute) Control
  USBD_UVC_CONTROL_32     * pPanTilt_Rel;         // PanTilt (Relative) Control
  USBD_UVC_CONTROL_16     * pRoll_Abs;            // Roll (Absolute) Control
  USBD_UVC_CONTROL_8_8    * pRoll_Rel;            // Roll (Relative) Control
  USBD_UVC_CONTROL_BOOL   * pPrivacy;             // Privacy Control
  //
  // Processing Unit controls
  //
  USBD_UVC_CONTROL_16     * pBrightness;          // Brightness Control
  USBD_UVC_CONTROL_16     * pContrast;            // Contrast Control
  USBD_UVC_CONTROL_16     * pGain;                // Gain Control
  USBD_UVC_CONTROL_8      * pPL_Freq;             // Power Line Frequency Control
  USBD_UVC_CONTROL_16     * pHue;                 // Hue Control
  USBD_UVC_CONTROL_BOOL   * pHue_Auto;            // Hue, Auto Control
  USBD_UVC_CONTROL_16     * pSaturation;          // Saturation Control
  USBD_UVC_CONTROL_16     * pSharpness;           // Sharpness Control
  USBD_UVC_CONTROL_16     * pGamma;               // Gamma Control
  USBD_UVC_CONTROL_16     * pWB_Temp;             // White Balance Temperature Control
  USBD_UVC_CONTROL_16_16  * pWB_Comp;             // White Balance Component Control
  USBD_UVC_CONTROL_16     * pBacklight_C;         // Backlight Compensation Control
  USBD_UVC_CONTROL_BOOL   * pWB_Temp_Auto;        // White Balance Temperature, Auto Control
  USBD_UVC_CONTROL_BOOL   * pWB_Comp_Auto;        // White Balance Component, Auto Control
  USBD_UVC_CONTROL_16     * pDigital_Mult;        // Digital Multiplier Control
  USBD_UVC_CONTROL_16     * pDigital_Mult_Limit;  // Digital Multiplier Limit Control
} USBD_UVC_CONTROLS;

/*********************************************************************
*
*       USBD_UVC_INIT_DATA
*
*   Description
*     Initialization data for UVC interface.
*
*   Additional information
*     This structure holds the endpoint that should be used by
*     the UVC interface (EPIn). Refer to USBD_AddEPEx()
*     for more information about how to add an endpoint.
*/
typedef struct _USBD_UVC_INIT_DATA {              // NOLINT(clang-analyzer-optin.performance.Padding)
  U8                          EPIn;               // Isochronous IN endpoint for sending data to the host.
  USBD_UVC_BUFFER           * pBuf;               // Pointer to a USBD_UVC_BUFFER structure.
  const USBD_UVC_RESOLUTION * aResolutions;       // Pointer to an array of USBD_UVC_RESOLUTION structures.
  U8                          NumResolutions;     // Number of elements inside the aResolutions array.
  U8                          StillCaptureMethod; // Method of "still image capture" to use.
                                                  // Valid values:
                                                  // * 1 - The host software will extract the next available
                                                  //       video frame. (default)
                                                  // * 2 - When the host requests a still image a callback
                                                  //       will be called which has to provide a new (still) image frame.
                                                  //       It only makes sense to use this method if your data
                                                  //       source is able to provide better quality still images
                                                  //       than the default quality of the video stream.
  U8                          Flags;              // Various flags.
                                                  // Valid bits:
                                                  // * USBD_UVC_USE_BULK_MODE - In this mode UVC uses bulk endpoints
                                                  //                            instead of isochronous endpoints.
  USBD_UVC_CONTROLS         * Controls;           // Pointer to a structure of type USBD_UVC_CONTROLS.
                                                  // The structure memory must remain available to the UVC class.
} USBD_UVC_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
int             USBD_UVC_Add                    (const USBD_UVC_INIT_DATA * pInitData);
int             USBD_UVC_Write                  (const U8 * pData, unsigned NumBytes, U8 UserFlags);
int             USBD_UVC_WriteEx                (U8 * pData, unsigned NumBytes, U8 UserFlags);
void            USBD_UVC_SetOnResolutionChange  (USB_UVC_ON_RESOLUTION_CHANGE * pfOnResChange);
#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
