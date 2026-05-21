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
Purpose : Public header of the MIDI component
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBD_MIDI_H          /* Avoid multiple inclusion */
#define USBD_MIDI_H

#include "SEGGER.h"
#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, set default values
*
**********************************************************************
*/
#define USBD_MIDI_GET_CABLE(x)              ((x) >> 4)
#define USBD_MIDI_GET_CID(x)                ((x) & 0x0Fu)

#define USB_AUDIO_MIDI_IN_JACK              0x02u
#define USB_AUDIO_MIDI_OUT_JACK             0x03u

#define USB_AUDIO_MIDI_EXTERNAL_JACK        0x02u
#define USB_AUDIO_MIDI_EMBEDDED_JACK        0x01u

//
// Code Index Number Classifications
//
#define MIDI_USB_CIN_MISC_EVENT             0x00u    // Miscellaneous function code, for future expansion
#define MIDI_USB_CIN_CABLE_EVENT            0x01u    // Cable event, for future expansion
#define MIDI_USB_CIN_SYSTEM_COMMON_2BYTE    0x02u    // Two-byte System Common messages (e.g. MIDI Time Code or Song Select).
#define MIDI_USB_CIN_SYSTEM_COMMON_3BYTE    0x03u    // Three-byte System Common messages (e.g. Song Position).
#define MIDI_USB_CIN_SYSEX_BEGIN_CONT       0x04u    // System Exclusive message starts or continues, three bytes
#define MIDI_USB_CIN_SYSTEM_COMMON_1BYTE    0x05u    // One-byte System Common message
#define MIDI_USB_CIN_SYSEX_END_1BYTE        0x05u    // System Exclusive message ends with following byte
#define MIDI_USB_CIN_SYSEX_END_2BYTE        0x06u    // System Exclusive message ends with following two bytes
#define MIDI_USB_CIN_SYSEX_END_3BYTE        0x07u    // System Exclusive message ends with following three bytes
#define MIDI_USB_CIN_NOTE_OFF               0x08u    // Note Off event
#define MIDI_USB_CIN_NOTE_ON                0x09u    // Note On event
#define MIDI_USB_CIN_POLY_AFTERTOUCH        0x0Au    // Poly-KeyPress
#define MIDI_USB_CIN_CONTROL_CHANGE         0x0Bu    // Control Change event
#define MIDI_USB_CIN_PROGRAM_CHANGE         0x0Cu    // Program Change event
#define MIDI_USB_CIN_AFTERTOUCH             0x0Du    // Channel Pressure
#define MIDI_USB_CIN_PITCH_BEND             0x0Eu    // PitchBend Change
#define MIDI_USB_CIN_SINGLE_BYTE            0x0Fu    // Single byte

/*********************************************************************
*
*       MIDI status bytes
*/
#define MIDI_COMMAND_0x80_NOTE_OFF                  0x80u
#define MIDI_COMMAND_0x90_NOTE_ON                   0x90u
#define MIDI_COMMAND_0xA0_POLY_AFTERTOUCH           0xA0u
#define MIDI_COMMAND_0xB0_CONTROL_CHANGE            0xB0u
#define MIDI_COMMAND_0xC0_PROGRAM_CHANGE            0xC0u
#define MIDI_COMMAND_0xD0_AFTERTOUCH                0xD0u
#define MIDI_COMMAND_0xE0_PITCH_BEND                0xE0u
#define MIDI_COMMAND_0xF0_SYSTEM_EXCLUSIVE          0xF0u
#define MIDI_COMMAND_0xF1_TIMECODE_QUARTER_FRAME    0xF1u
#define MIDI_COMMAND_0xF2_SONG_POSITION             0xF2u
#define MIDI_COMMAND_0xF3_SONG_SELECT               0xF3u
#define MIDI_COMMAND_0xF4_UNDEFINED                 0xF4u
#define MIDI_COMMAND_0xF5_UNDEFINED                 0xF5u
#define MIDI_COMMAND_0xF6_TUNE_REQUEST              0xF6u
#define MIDI_COMMAND_0xF7_END_OF_EXCLUSIVE          0xF7u
#define MIDI_COMMAND_0xF8_TIMING_CLOCK              0xF8u
#define MIDI_COMMAND_0xF9_UNDEFINED                 0xF9u
#define MIDI_COMMAND_0xFA_START                     0xFAu
#define MIDI_COMMAND_0xFB_CONTINUE                  0xFBu
#define MIDI_COMMAND_0xFC_STOP                      0xFCu
#define MIDI_COMMAND_0xFD_UNDEFINED                 0xFDu
#define MIDI_COMMAND_0xFE_ACTIVE_SENSING            0xFEu
#define MIDI_COMMAND_0xFF_RESET                     0xFFu

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef int USBD_MIDI_HANDLE;

/*********************************************************************
*
*       USBD_MIDI_PACKET
*
*  Description
*    Structure describing a MIDI packet.
*/
typedef struct {
  U8                        CableNumber_and_CIN; // b0-b3 - Code Index Number (CIN).
                                                 // b4-b7 - Cable Number (embedded MIDI Jack ID).
  U8                        MIDI_0;              // First MIDI byte:
                                                 // b0-b3 - For CIN < 0xF: MIDI channel number.
                                                 // b4-b7 - Code Index Number (same as in b0-b3 of CableNumber_and_CIN).
  U8                        MIDI_1;              // Second MIDI byte - Content depends on CIN.
  U8                        MIDI_2;              // Third MIDI byte - Content depends on CIN.
} USBD_MIDI_PACKET;

/*********************************************************************
*
*       USBD_MIDI_JACK
*
*  Description
*    Structure describing a MIDI IN or OUT jack.
*/
typedef struct {
  U8                        JackType;          // USB_AUDIO_MIDI_EMBEDDED_JACK or USB_AUDIO_MIDI_EXTERNAL_JACK
  U8                        JackID;            // Unique ID for the jack. Must not be zero.
  U8                        JackDir;           // Jack direction, USB_AUDIO_MIDI_IN_JACK or USB_AUDIO_MIDI_OUT_JACK
  U8                        NrInputPins;       // For IN jacks - set to zero. For OUT jacks - number of input pins for this MIDI OUT jack.
  U8                      * paSourceID;        // Only for OUT jacks. Pointer to an array containing the IDs of the entities to which the pin of this MIDI OUT Jack is connected.
  U8                      * paSourcePin;       // Only for OUT jacks. Pointer to an array containing the output pin numbers of the entities to which the input pins of this MIDI OUT Jack are connected.
  const char              * pJackName;         // String describing the jack. Can be NULL.
} USBD_MIDI_JACK;

/*********************************************************************
*
*       USBD_MIDI_INIT_DATA
*
*  Description
*    Initialization structure that is needed when adding a MIDI interface to emUSB-Device.
*/
typedef struct {
  U16                       Flags;             // Reserved for future use, must be 0.
  U8                        EPIn;              // Bulk IN endpoint for sending data to the host.
  U8                        EPOut;             // Bulk OUT endpoint for receiving data from the host.
  const USBD_MIDI_JACK    * paJackList;        // Pointer to an array containing all jacks for the MIDI interface.
  unsigned                  NumJacks;          // Number of elements inside the paJackList array.
} USBD_MIDI_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void              USBD_MIDI_Init                    (void);
USBD_MIDI_HANDLE  USBD_MIDI_Add                     (const USBD_MIDI_INIT_DATA * pInitData);
int               USBD_MIDI_ReceivePackets          (USBD_MIDI_HANDLE hInst, USBD_MIDI_PACKET * paPacket, unsigned NumPackets, int Timeout);
unsigned          USBD_MIDI_GetNumPacketsInBuffer   (USBD_MIDI_HANDLE hInst);
int               USBD_MIDI_ConvertPackets          (const USBD_MIDI_PACKET * paPacket, unsigned NumPackets, U8 * pBuf);
int               USBD_MIDI_WritePackets            (USBD_MIDI_HANDLE hInst, const USBD_MIDI_PACKET * paPacket, unsigned NumPackets, int Timeout);
int               USBD_MIDI_WriteStream             (USBD_MIDI_HANDLE hInst, U8 JackID, const U8 *pData, unsigned NumBytes, int Timeout);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
