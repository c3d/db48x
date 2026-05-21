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

File    : IP_WEBSOCKET.h
Purpose : Header file for WebSocket add-on.
*/

#ifndef IP_WEBSOCKET_H        // Avoid multiple inclusion.
#define IP_WEBSOCKET_H

#include "SEGGER.h"

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   IP_WEBSOCKET_USE_PARA                    // Some compiler complain about unused parameters.
  #define IP_WEBSOCKET_USE_PARA(Para)  (void)Para  // This works for most compilers.
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define IP_WEBSOCKET_ERR_ALL_DATA_READ  -100  // All data of the current message has been read. Get next message with IP_WEBSOCKET_WaitForNextMessage().
#define IP_WEBSOCKET_ERR_LOGIC          -101
#define IP_WEBSOCKET_ERR_AGAIN          -102  // Execute the same call again. Not directly a WOULDBLOCK error as we look at the payload only but might have processed headers.

enum {
  IP_WEBSOCKET_STATE_CONNECTING = 0,
  IP_WEBSOCKET_STATE_OPEN,
  IP_WEBSOCKET_STATE_CLOSING,
  IP_WEBSOCKET_STATE_CLOSED
};

enum {
  IP_WEBSOCKET_FRAME_TYPE_CONTINUE = 0x00,
  IP_WEBSOCKET_FRAME_TYPE_TEXT,             // Text means an UTF-8 encoded text.
  IP_WEBSOCKET_FRAME_TYPE_BINARY,
  IP_WEBSOCKET_FRAME_TYPE_CLOSE    = 0x08,
  IP_WEBSOCKET_FRAME_TYPE_PING,
  IP_WEBSOCKET_FRAME_TYPE_PONG,
  IP_WEBSOCKET_FRAME_TYPE_NONE     = 0xFF  // No previous frame has been received.
};

//
// Close codes as defined by RFC 6455 section 11.7. "WebSocket Close Code Number Registry".
//
#define IP_WEBSOCKET_CLOSE_CODE_NORMAL_CLOSURE              1000
#define IP_WEBSOCKET_CLOSE_CODE_GOING_AWAY                  1001
#define IP_WEBSOCKET_CLOSE_CODE_PROTOCOL_ERROR              1002
#define IP_WEBSOCKET_CLOSE_CODE_UNSUPPORTED_DATA            1003
#define IP_WEBSOCKET_CLOSE_CODE_NO_STATUS_RCVD              1005
#define IP_WEBSOCKET_CLOSE_CODE_ABNORMAL_CLOSURE            1006
#define IP_WEBSOCKET_CLOSE_CODE_INVALID_FRAME_PAYLOAD_DATA  1007
#define IP_WEBSOCKET_CLOSE_CODE_POLICY_VIOLATION            1008
#define IP_WEBSOCKET_CLOSE_CODE_MESSAGE_TOO_BIG             1009
#define IP_WEBSOCKET_CLOSE_CODE_MANDATORY_EXT               1010
#define IP_WEBSOCKET_CLOSE_CODE_INTERNAL_SERVER_ERROR       1011
#define IP_WEBSOCKET_CLOSE_CODE_TLS_HANDSHAKE               1015

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef void* IP_WEBSOCKET_CONNECTION;

typedef struct IP_WEBSOCKET_TRANSPORT_API_STRUCT IP_WEBSOCKET_TRANSPORT_API;
typedef struct IP_WEBSOCKET_CONTEXT_STRUCT IP_WEBSOCKET_CONTEXT;

struct IP_WEBSOCKET_TRANSPORT_API_STRUCT {
  //
  // Communication/IP API.
  //
  int (*pfRecv)      (IP_WEBSOCKET_CONTEXT* pContext, IP_WEBSOCKET_CONNECTION* pConnection,       void* pData, unsigned NumBytes);
  int (*pfSend)      (IP_WEBSOCKET_CONTEXT* pContext, IP_WEBSOCKET_CONNECTION* pConnection, const void* pData, unsigned NumBytes);
  U32 (*pfGenMaskKey)(void);
};

//
// WebSocket state is typically required for non-blocking socket API
// as one call might not result in receiving/sending all data as
// a socket would in default blocking mode.
//
typedef struct {
  U32 Stack;              // State stack used in units of 4 x U8.
  int NumBytes;           // NumBytes already read/written.
  U8  abHeader[16];       // Buffer for maximum header size of FlagsOpcode(1) + MaskPayloadLen(1) + ExtPayloadLen(0/2/8) + MaskKey(4) + CloseCode(2) .
  U8  StackShift;         // Shift in units of 8-bits of current continue state stack.
} IP_WEBSOCKET_CONTINUE;

struct IP_WEBSOCKET_CONTEXT_STRUCT {
  const IP_WEBSOCKET_TRANSPORT_API* pAPI;
  //
  // Continue contexts in case non-blocking socket API is used.
  //
  IP_WEBSOCKET_CONTINUE RxContinue;
  IP_WEBSOCKET_CONTINUE TxContinue;
  //
  // Connection parameters.
  //
  IP_WEBSOCKET_CONNECTION* pConnection;   // Socket handle or SSL session handle passed to communication callbacks.
  U64                      NumBytesLeft;  // NumBytes left in the current websocket frame. Set by IP_WEBSOCKET_WaitForNextMessage() and IP_WEBSOCKET_Recv().
  U8                       aMaskKey[4];   // Mask key used when frames are sent from client to server.
  U8                       MaskKeyIndex;  // Current index 0..3 of the mask key.
  U8                       State;         // IP_WEBSOCKET_STATE_*.
  U8                       FrameType;     // Type of current frame identified by IP_WEBSOCKET_WaitForNextMessage(), IP_WEBSOCKET_FRAME_TYPE_*.
  U8                       FrameFin;      // Has the last message (FIN bit set) of the currently processed application frame been received ?
  U8                       SendMore;      // Did the last frame sent by the application has the FIN bit cleared ?
  U8                       IsClient;      // Is this a server or client context ?
};

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

int  IP_WEBSOCKET_Close             (IP_WEBSOCKET_CONTEXT* pContext, char* sReason, U16 CloseCode);
int  IP_WEBSOCKET_DiscardMessage    (IP_WEBSOCKET_CONTEXT* pContext);
int  IP_WEBSOCKET_GenerateAcceptKey (void* pSecWebSocketKey, int SecWebSocketKeyLen, void* pBuffer, int BufferSize);
void IP_WEBSOCKET_InitClient        (IP_WEBSOCKET_CONTEXT* pContext, const IP_WEBSOCKET_TRANSPORT_API* pAPI, IP_WEBSOCKET_CONNECTION* pConnection);
void IP_WEBSOCKET_InitServer        (IP_WEBSOCKET_CONTEXT* pContext, const IP_WEBSOCKET_TRANSPORT_API* pAPI, IP_WEBSOCKET_CONNECTION* pConnection);
int  IP_WEBSOCKET_Recv              (IP_WEBSOCKET_CONTEXT* pContext, void* pData, int NumBytes);
int  IP_WEBSOCKET_Send              (IP_WEBSOCKET_CONTEXT* pContext, void* pData, int NumBytes, U8 MessageType, U8 SendMore);
int  IP_WEBSOCKET_WaitForNextMessage(IP_WEBSOCKET_CONTEXT* pContext, U8* pMessageType);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
