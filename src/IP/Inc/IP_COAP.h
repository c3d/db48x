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

File    : IP_COAP.h
Purpose : Header file for the Constrained Application Protocol (CoAP)
          Client / Server application.
*/

#ifndef IP_COAP_H            // Avoid multiple inclusion.
#define IP_COAP_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

#include "SEGGER.h"
#include "IP_COAP_Conf.h"

/*********************************************************************
*
*       Defines, configurable
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
// UDP connection definitions.
//
#define IP_COAP_IPV4         0u
#define IP_COAP_IPV6         1u
#define IP_COAP_DEFAULT_PORT 5683u

//
// CoAP types.
//
#define IP_COAP_TYPE_CON  0x0u
#define IP_COAP_TYPE_NON  0x1u
#define IP_COAP_TYPE_ACK  0x2u
#define IP_COAP_TYPE_RST  0x3u

//
// CoAP codes.
//
#define IP_COAP_CODE2NUM(c,d)             (U8)((((c) & 0x7u) << 5) | ((d) & 0x1Fu))
#define IP_COAP_GET_CLASS(n)              (((n) >> 5) & 0x7u)

#define IP_COAP_CLASS_REQUEST             0u
#define IP_COAP_CLASS_SUCCESS             2u
#define IP_COAP_CLASS_CLIENT_ERROR        4u
#define IP_COAP_CLASS_SERVER_ERROR        5u

#define IP_COAP_CODE_EMPTY                IP_COAP_CODE2NUM(0u, 0u)   // Empty

#define IP_COAP_CODE_REQ_GET              IP_COAP_CODE2NUM(0u, 1u)   // GET
#define IP_COAP_CODE_REQ_POST             IP_COAP_CODE2NUM(0u, 2u)   // POST
#define IP_COAP_CODE_REQ_PUT              IP_COAP_CODE2NUM(0u, 3u)   // PUT
#define IP_COAP_CODE_REQ_DEL              IP_COAP_CODE2NUM(0u, 4u)   // DELETE

#define IP_COAP_CODE_SUCCESS_CREATED      IP_COAP_CODE2NUM(2u, 1u)   // 2.01 Created
#define IP_COAP_CODE_SUCCESS_DELETED      IP_COAP_CODE2NUM(2u, 2u)   // 2.02 Deleted
#define IP_COAP_CODE_SUCCESS_VALID        IP_COAP_CODE2NUM(2u, 3u)   // 2.03 Valid
#define IP_COAP_CODE_SUCCESS_CHANGED      IP_COAP_CODE2NUM(2u, 4u)   // 2.04 Changed
#define IP_COAP_CODE_SUCCESS_CONTENT      IP_COAP_CODE2NUM(2u, 5u)   // 2.05 Content
#define IP_COAP_CODE_SUCCESS_CONTINUE     IP_COAP_CODE2NUM(2u,31u)   // 2.31 Continue

#define IP_COAP_CODE_CERROR_BAD_REQ       IP_COAP_CODE2NUM(4u, 0u)   // 4.00 Bad Request
#define IP_COAP_CODE_CERROR_UNAUTHOR      IP_COAP_CODE2NUM(4u, 1u)   // 4.01 Unauthorized
#define IP_COAP_CODE_CERROR_BAP_OPT       IP_COAP_CODE2NUM(4u, 2u)   // 4.02 Bad Option
#define IP_COAP_CODE_CERROR_FORBID        IP_COAP_CODE2NUM(4u, 3u)   // 4.03 Forbidden
#define IP_COAP_CODE_CERROR_NOT_FOUND     IP_COAP_CODE2NUM(4u, 4u)   // 4.04 Not Found
#define IP_COAP_CODE_CERROR_NOT_ALLOW     IP_COAP_CODE2NUM(4u, 5u)   // 4.05 Method Not Allowed
#define IP_COAP_CODE_CERROR_NOT_ACCEPT    IP_COAP_CODE2NUM(4u, 6u)   // 4.06 Not Acceptable
#define IP_COAP_CODE_CERROR_REQ_INCOMP    IP_COAP_CODE2NUM(4u, 8u)   // 4.08 Request Entity Incomplete
#define IP_COAP_CODE_CERROR_PRECOND       IP_COAP_CODE2NUM(4u,12u)   // 4.12 Precondition Failed
#define IP_COAP_CODE_CERROR_TOO_LARGE     IP_COAP_CODE2NUM(4u,13u)   // 4.13 Request Entity Too Large
#define IP_COAP_CODE_CERROR_CONT_FORMAT   IP_COAP_CODE2NUM(4u,15u)   // 4.15 Unsupported Content-Format

#define IP_COAP_CODE_SERROR_INTERNAL      IP_COAP_CODE2NUM(5u, 0u)   // 5.00 Internal Server Error
#define IP_COAP_CODE_SERROR_NOT_IMP       IP_COAP_CODE2NUM(5u, 1u)   // 5.01 Not Implemented
#define IP_COAP_CODE_SERROR_BAD_GW        IP_COAP_CODE2NUM(5u, 2u)   // 5.02 Bad Gateway
#define IP_COAP_CODE_SERROR_UNAVAIL       IP_COAP_CODE2NUM(5u, 3u)   // 5.03 Service Unavailable
#define IP_COAP_CODE_SERROR_GW_TIME       IP_COAP_CODE2NUM(5u, 4u)   // 5.04 Gateway Timeout
#define IP_COAP_CODE_SERROR_NO_PROXY      IP_COAP_CODE2NUM(5u, 5u)   // 5.05 Proxying Not Supported

//
// Internal code for client error report.
// Not to be used in client/server exchange.
//
#define IP_COAP_CODE_INTERN_REQ_TIMEOUT   IP_COAP_CODE2NUM(6u, 0u)   // 6.00 Request timeout.

//
// CoAP options.
//
#define IP_COAP_OPTION_IF_MATCH            1u
#define IP_COAP_OPTION_URI_HOST            3u
#define IP_COAP_OPTION_ETAG                4u
#define IP_COAP_OPTION_IF_NONE_MATCH       5u
#define IP_COAP_OPTION_OBSERVE             6u
#define IP_COAP_OPTION_URI_PORT            7u
#define IP_COAP_OPTION_LOCATION_PATH       8u
#define IP_COAP_OPTION_URI_PATH           11u
#define IP_COAP_OPTION_CONTENT_FORMAT     12u
#define IP_COAP_OPTION_MAX_AGE            14u
#define IP_COAP_OPTION_URI_QUERY          15u
#define IP_COAP_OPTION_ACCEPT             17u
#define IP_COAP_OPTION_LOCATION_QUERY     20u
#define IP_COAP_OPTION_BLOCK2             23u
#define IP_COAP_OPTION_BLOCK1             27u
#define IP_COAP_OPTION_SIZE2              28u
#define IP_COAP_OPTION_PROXY_URI          35u
#define IP_COAP_OPTION_PROXY_SCHEME       39u
#define IP_COAP_OPTION_SIZE1              60u

//
// Options internal mask (needs to be ordered in
// growing CoAP option number).
//
#define IP_COAP_OPTMASK_IF_MATCH          (1uL      )
#define IP_COAP_OPTMASK_URI_HOST          (1uL <<  1)
#define IP_COAP_OPTMASK_ETAG              (1uL <<  2)
#define IP_COAP_OPTMASK_IF_NONE_MATCH     (1uL <<  3)
#define IP_COAP_OPTMASK_OBSERVE           (1uL <<  4)
#define IP_COAP_OPTMASK_URI_PORT          (1uL <<  5)
#define IP_COAP_OPTMASK_LOCATION_PATH     (1uL <<  6)
#define IP_COAP_OPTMASK_URI_PATH          (1uL <<  7)
#define IP_COAP_OPTMASK_CONTENT_FORMAT    (1uL <<  8)
#define IP_COAP_OPTMASK_MAX_AGE           (1uL <<  9)
#define IP_COAP_OPTMASK_URI_QUERY         (1uL << 10)
#define IP_COAP_OPTMASK_ACCEPT            (1uL << 11)
#define IP_COAP_OPTMASK_LOCATION_QUERY    (1uL << 12)
#define IP_COAP_OPTMASK_BLOCK2            (1uL << 13)
#define IP_COAP_OPTMASK_BLOCK1            (1uL << 14)
#define IP_COAP_OPTMASK_SIZE2             (1uL << 15)
#define IP_COAP_OPTMASK_PROXY_URI         (1uL << 16)
#define IP_COAP_OPTMASK_PROXY_SCHEME      (1uL << 17)
#define IP_COAP_OPTMASK_SIZE1             (1uL << 18)

//
// Content Format.
//
#define IP_COAP_CT_TXT                     0u  // plain/text
#define IP_COAP_CT_LINK_FORMAT            40u  // application/link-format
#define IP_COAP_CT_XML                    41u  // application/xml
#define IP_COAP_CT_OCTET_STREAM           42u  // application/octet-stream
#define IP_COAP_CT_EXI                    47u  // application/exi
#define IP_COAP_CT_JSON                   50u  // application/json

//
// Flag to disable major features.
//
#define IP_COAP_CONFIG_DISABLE_BLOCK1     (1u     )
#define IP_COAP_CONFIG_DISABLE_BLOCK2     (1u << 1)
#define IP_COAP_CONFIG_DISABLE_BLOCKS     (IP_COAP_CONFIG_DISABLE_BLOCK1 | IP_COAP_CONFIG_DISABLE_BLOCK2)
#define IP_COAP_CONFIG_DISABLE_OBSERVE    (1u << 2)

//
// Observer configuration
//
#define IP_COAP_OBS_OBSERVABLE            (1u     )
#define IP_COAP_OBS_AUTO_CON_ON_MAX_AGE   (1u << 1)
#define IP_COAP_OBS_AUTO_NON_ON_MAX_AGE   (1u << 2)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

//
// Status of the connection between a client and a server.
//
enum {
  IP_COAP_CONNECTION_NONE                  = 0,
  IP_COAP_CONNECTION_WAIT_NEXT_BLOCK_REQ,          // Waiting for the next GET when doing block transfer.
  IP_COAP_CONNECTION_WAIT_DATA_READY,              // Separate ACK sent, waiting for the data to be ready
  IP_COAP_CONNECTION_DATA_READY,                   // Users data are ready, need to send a CON/NON.
  IP_COAP_CONNECTION_WAIT_ACK,                     // CON/NON is sent, wait for ACK/NON.
  IP_COAP_CONNECTION_WAIT_MULTICAST,               // Multicast request received, waiting to send the reply.
  IP_COAP_CONNECTION_CLOSE_ON_END,                 // Connection will be closed (error occured).
  IP_COAP_CONNECTION_CLOSE_ON_ERROR                // Connection will be closed, transfer is completed.
};


//
// Return values for the CoAP functions and callbacks.
//
enum {
  //
  // Negative values are errors.
  //
  IP_COAP_RETURN_NO_DATA                  = -9,
  IP_COAP_RETURN_CONDITION_FAILED         = -8,
  IP_COAP_RETURN_PROXY_NOT_SUPPORTED      = -7,
  IP_COAP_RETURN_CT_FORMAT_ERROR          = -6,
  IP_COAP_RETURN_OPTION_ERROR             = -5,
  IP_COAP_RETURN_BUFFER_TOO_SMALL         = -4,
  IP_COAP_RETURN_NO_PAYLOAD               = -3,
  IP_COAP_RETURN_NOT_ALLOWED              = -2,
  IP_COAP_RETURN_ERR                      = -1,

  //
  // Positive values are information.
  //
  IP_COAP_RETURN_OK                =  0,
  IP_COAP_RETURN_SEND_SEPARATE,
  IP_COAP_RETURN_SEND_BLOCK,
  IP_COAP_RETURN_SEND_END,
  IP_COAP_RETURN_IGNORE_BLOCK,
  IP_COAP_RETURN_IGNORE_END,
  IP_COAP_RETURN_WAIT_ACK,
  IP_COAP_RETURN_RESET,
  IP_COAP_RETURN_OK_NO_DELETE
};

//
// UDP connection definition structure.
//
typedef struct {
  void* hSock;
  union {
    U8  IPAddrV6[16];
    U32 IPAddrV4;
  } Addr;
  U16   Port;
  U8    Family;
} IP_COAP_CONN_INFO;

//
// CoAP header structure.
//
typedef struct {
  U8  Type;
  U8  Code;
  U16 MessageId;
  U8  aToken[8];
  U8  TokenLength;
} IP_COAP_HEADER_INFO;

//
// Definition of the block.
//
typedef struct {
  U16 Index;
  U16 Size;
} IP_COAP_BLOCK_INFO;

//
// If-Match structure used by the client
// to send many If-Match options in one
// request.
//
typedef struct IP_COAP_IF_MATCH_INFO_STRUCT IP_COAP_IF_MATCH_INFO;
struct IP_COAP_IF_MATCH_INFO_STRUCT {
  IP_COAP_IF_MATCH_INFO*   pNext;
  U8*                      pETag;
  U8                       ETagLength;
};

//
// Option desciption structure.
//
typedef struct {
  U32                      OptionPresentMask;  // Mask of IP_COAP_OPTMASK_xxx.
  //
  U8*                      pHost;
  U8*                      pURI;
  U8*                      pQuery;
  U8*                      pETag;
  U8*                      pBlockLast;
  U8*                      pProxyURI;
  U8*                      pProxyScheme;
  IP_COAP_IF_MATCH_INFO*   pIfMatch;
  U32                      Observe;
  U32                      MaxAge;
  U32                      Size1;
  U32                      Block2;
  U32                      Block1;
  U32                      Size2;
  U16                      URIPort;
  U16                      Accept;
  U16                      ContentFormat;
  U16                      ProxyURILength;
  U8                       ProxySchemeLength;
  U8                       HostLength;
  U8                       URILength;
  U8                       QueryLength;
  U8                       ETagLength;
} IP_COAP_OPTIONS_INFO;

//
// Parameter of the callbacks.
//
typedef struct {
  IP_COAP_BLOCK_INFO*    pBlock;
  IP_COAP_CONN_INFO*     pConnInfo;
  IP_COAP_HEADER_INFO*   pHeader;
  IP_COAP_OPTIONS_INFO*  pOptDesc;
} IP_COAP_CALLBACK_PARAM;


//
// CoAP sytem APIs (UDP transfer + time).
//
typedef struct {
  int   (*pfReceive)  (U8* pBuffer, unsigned BufferSize, IP_COAP_CONN_INFO* pInfo, unsigned* pIsMulticast);
  int   (*pfSend)     (U8* pBuffer, unsigned BufferSize, IP_COAP_CONN_INFO* pInfo);
  U32   (*pfGetTimeMs)(void);
} IP_COAP_API;

/////////////////////////////////////////////////////////
// Server.
/////////////////////////////////////////////////////////


typedef struct IP_COAP_SERVER_CONTEXT_STRUCT  IP_COAP_SERVER_CONTEXT;
typedef struct IP_COAP_CONNECTION_STRUCT      IP_COAP_CONNECTION;
typedef struct IP_COAP_SERVER_DATA_STRUCT     IP_COAP_SERVER_DATA;

//
// Server data structure. Defines a item of the server.
// The first '/' shall be ommited in the sURI string.
//
struct IP_COAP_SERVER_DATA_STRUCT {
  IP_COAP_SERVER_DATA*    pNext;
  //
  const char*             sURI;
  const char*             sDescription;
  U8                      ETag[8];
  U32                     MaxAge;
  U32                     Size2;
  U32                     DefGetOptMask;
  U16                     ContentFormat;
  U8                      ETagLength;
  U8                      ObsConfig;
  //
  // Callbacks to handle the requests.
  //
  int                     (*pfGETPayload)(IP_COAP_SERVER_CONTEXT* pContext, U8** ppBuffer, U16* pLength, IP_COAP_CALLBACK_PARAM* pParam);
  int                     (*pfPUTPayload)(IP_COAP_SERVER_CONTEXT* pContext, U8* pPayload, U16 Length, IP_COAP_CALLBACK_PARAM* pParam);
  int                     (*pfDELHandler)(IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_CALLBACK_PARAM* pParam);
} ;


//
// Server definition of a connection with a client.
//
struct IP_COAP_CONNECTION_STRUCT {
  U8                         ConnStatus;
  U8                         Procedure;
  IP_COAP_SERVER_DATA*       pServerData;
  IP_COAP_HEADER_INFO        Header;
  IP_COAP_OPTIONS_INFO       Options;
  //
  // Retry.
  //
  U32                        UpdateTime;
  U32                        RetryTimeout;
  U8                         RetryNum;
  //
  // Block information.
  //
  U8                         BlockSzx;  // 0xFF for no block.
  U8                         FirstBlockSent;
};


//
// Server definition of a client.
//
typedef struct IP_COAP_SERVER_CLIENT_INFO_STRUCT IP_COAP_SERVER_CLIENT_INFO;
struct IP_COAP_SERVER_CLIENT_INFO_STRUCT {
  IP_COAP_SERVER_CLIENT_INFO*    pNext;
  //
  IP_COAP_CONN_INFO              ConnInfo;
  //
  // According to RFC, there is only NSTART ongoing connections
  // between a client and a server.
  //
  IP_COAP_CONNECTION             aConnection[IP_COAP_NSTART];
  int                            ConnectionIndex;
};

//
// Server definition of an observer.
//
typedef struct IP_COAP_OBSERVER_STRUCT IP_COAP_OBSERVER;
struct IP_COAP_OBSERVER_STRUCT {
  IP_COAP_OBSERVER*       pNext;
  //
  IP_COAP_SERVER_DATA*    pServerData;
  IP_COAP_CONN_INFO       ConnInfo;
  IP_COAP_CONNECTION*     pConnection;
  U8                      aToken[8];
  U32                     Observe;
  U32                     Time;
  U32                     ForceConTimeout;  // Timestamp after which we force sending a CON instead of NON to check if the client is still alive.
  U8                      TokenLength;
  U8                      Status;
  U8                      Szx;
};

//
// Callback to perform a PORT.
//
typedef int (*PF_POST_HANDLER)(IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_CALLBACK_PARAM* pParam, U32 PayloadLength, IP_COAP_SERVER_DATA** ppServerData);

//
// Main server context structure.
//
struct IP_COAP_SERVER_CONTEXT_STRUCT {
  //
  // APIs for UDP transmission and time.
  //
  int                          (*pfReceive)  (U8* pBuffer, unsigned BufferSize, IP_COAP_CONN_INFO* pInfo, unsigned* pIsMulticast);
  int                          (*pfSend)     (U8* pBuffer, unsigned BufferSize, IP_COAP_CONN_INFO* pInfo);
  U32                          (*pfGetTimeMs)(void);
  //
  // Server parameters.
  //
  const char*                  sHostName;
  const char*                  sErrorDesc;
  IP_COAP_SERVER_DATA*         pFirstData;
  IP_COAP_SERVER_DATA          DataWellKnownCore;
  //
  IP_COAP_SERVER_CLIENT_INFO*  pFirstClient;
  IP_COAP_SERVER_CLIENT_INFO*  pFreeCLient;
  //
  IP_COAP_OBSERVER*            pFirstObs;
  IP_COAP_OBSERVER*            pFreeObs;
  //
  PF_POST_HANDLER              pfPOSTCreateEntry;
  //
  // Internal data.
  //
  IP_COAP_SERVER_DATA*         pDataFirstFound;
  U8*                          pPayload;
  U32                          OptionMask;
  U16                          UDPPort;
  U16                          PayloadLength;
  U16                          StructureETag;
  U16                          URILengthReq;
  U16                          RxMsgLength;  // Length of the received message.
  U16                          MsgLength;    // Length of the message to send.
  U16                          BadOption;
  U16                          MessageId;
  U8                           DefaultSzx;
  U8                           ConfigMask;
  //
  // Buffer for the UDP packet handling (Rx/Tx).
  //
  U8*                          pMsgBuffer;
  U16                          MsgBufferSize;
};


/////////////////////////////////////////////////////////
// Client.
/////////////////////////////////////////////////////////

typedef struct IP_COAP_CLIENT_CONTEXT_STRUCT IP_COAP_CLIENT_CONTEXT;

//
// Callback for the client payload handling.
//
typedef int    (*PF_CLIENT_PAYLOAD)(IP_COAP_CLIENT_CONTEXT* pContext, U8** ppPayload, U16* pLength, IP_COAP_CALLBACK_PARAM* pParam);
//
// Callback to mark the end of an observe transfer.
//
typedef void   (*PF_OBS_END_TRANSFER)(U8 Code, int IsFinal, void* pParam);

//
// Client observer definition.
//
typedef struct IP_COAP_CLIENT_OBS_STRUCT IP_COAP_CLIENT_OBS;
struct IP_COAP_CLIENT_OBS_STRUCT {
  IP_COAP_CLIENT_OBS*     pNext;
  //
  // Observe is identified by it's Token and Observe number.
  //
  U8                      aToken[8];
  U8                      TokenLength;
  U8                      Type;
  U32                     Observe;
  //
  // Callbacks.
  //
  PF_CLIENT_PAYLOAD       pfPayloadCallback;
  PF_OBS_END_TRANSFER     pfObsEndTransfer;
  void*                   pParam;
};

//
// Definition of a request made by the client.
//
typedef struct {
  IP_COAP_HEADER_INFO     Header;
  IP_COAP_OPTIONS_INFO    Options;
  IP_COAP_OPTIONS_INFO    OptionsSaved;  // Workaround when using the "Options" member to parse received options
                                         // but we need to send more "original" options for example for further blocks.
  U32                     OptionsMask;
  //
  U8*                     pPayload;
  PF_CLIENT_PAYLOAD       pfPayloadCallback;
  U16                     PayloadLength;
  U8                      ConnStatus;
  U8                      Procedure;
  //
  // Retry.
  //
  U32                     UpdateTime;
  U32                     RetryTimeout;
  U8                      RetryNum;
  //
  // Observable context
  //
  IP_COAP_CLIENT_OBS*     pObs;
} IP_COAP_CLIENT_REQUEST;


//
// Main client context structure.
//
struct IP_COAP_CLIENT_CONTEXT_STRUCT {
  //
  // Buffer for the UDP packet handling (Rx/Tx).
  //
  U8*                          pMsgBuffer;
  U16                          MsgBufferSize;
  //
  // UDP transmission function.
  //
  int                          (*pfReceive)  (U8* pBuffer, unsigned BufferSize, IP_COAP_CONN_INFO* pInfo, unsigned* pIsMulticast);
  int                          (*pfSend)     (U8* pBuffer, unsigned BufferSize, IP_COAP_CONN_INFO* pInfo);
  U32                          (*pfGetTimeMs)(void);
  //
  // Client parameters.
  //
  IP_COAP_CLIENT_REQUEST       aRequest[IP_COAP_NSTART];
  IP_COAP_CONN_INFO            ConnInfo;
  IP_COAP_CLIENT_OBS*          pFirstObs;
  //
  // Internal data.
  //
  U8*                          pPayload;
  U16                          PayloadLength;
  U16                          MessageId;
  U8                           DefaultSzx;
};

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

/*********************************************************************
*
*       API functions / Function prototypes
*
**********************************************************************
*/

//
// Utility.
//
int IP_COAP_CheckAcceptFormat (IP_COAP_CALLBACK_PARAM* pParam, U16 Format);
int IP_COAP_GetAcceptFormat   (IP_COAP_CALLBACK_PARAM* pParam, U16* pFormat);
int IP_COAP_CheckContentFormat(IP_COAP_CALLBACK_PARAM* pParam, U16 Format, unsigned OptionMandatory);
int IP_COAP_GetContentFormat  (IP_COAP_CALLBACK_PARAM* pParam, U16* pFormat);
int IP_COAP_IsLastBlock       (IP_COAP_CALLBACK_PARAM* pParam, U8 Code);
int IP_COAP_GetURIHost        (IP_COAP_CALLBACK_PARAM* pParam, U8** ppHost, U8* pHostLength);
int IP_COAP_GetURIPath        (IP_COAP_CALLBACK_PARAM* pParam, U8** ppURI, U8* pURILength);
int IP_COAP_GetURIPort        (IP_COAP_CALLBACK_PARAM* pParam, U16* pURIPort);
int IP_COAP_GetQuery          (IP_COAP_CALLBACK_PARAM* pParam, U8** ppQuery, U16* pQueryLength);
int IP_COAP_GetETag           (IP_COAP_CALLBACK_PARAM* pParam, U8** ppETag, U8* pETagLength);
int IP_COAP_GetMaxAge         (IP_COAP_CALLBACK_PARAM* pParam, U32* pMaxAge);
int IP_COAP_GetSize1          (IP_COAP_CALLBACK_PARAM* pParam, U32* pSize1);
int IP_COAP_GetSize2          (IP_COAP_CALLBACK_PARAM* pParam, U32* pSize2);
int IP_COAP_GetLocationPath   (IP_COAP_CALLBACK_PARAM* pParam, U8** ppLoc, U8* pLocLength);
int IP_COAP_GetLocationQuery  (IP_COAP_CALLBACK_PARAM* pParam, U8** ppQuery, U16* pQueryLength);
int IP_COAP_GetObserve        (IP_COAP_CALLBACK_PARAM* pParam, U32* pObserve);

//
// Server.
//
int IP_COAP_SERVER_Init               (IP_COAP_SERVER_CONTEXT* pContext, U8* pMsgBuffer, U16 MsgBufferSize, const IP_COAP_API* pAPI);
int IP_COAP_SERVER_Process            (IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_CONN_INFO* pConnInfo);
U8* IP_COAP_SERVER_GetMsgBuffer       (IP_COAP_SERVER_CONTEXT* pContext, U16* pMsgLength);
int IP_COAP_SERVER_AddData            (IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_SERVER_DATA* pData);
int IP_COAP_SERVER_RemoveData         (IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_SERVER_DATA* pData);
int IP_COAP_SERVER_AddClientBuffer    (IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_SERVER_CLIENT_INFO* pClientInfo, unsigned NumClientInfo);
int IP_COAP_SERVER_AddObserverBuffer  (IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_OBSERVER* pObs, unsigned NumObservers);
int IP_COAP_SERVER_UpdateData         (IP_COAP_SERVER_CONTEXT* pContext, IP_COAP_SERVER_DATA* pData, U8 ObsUpdateType, unsigned AutoETag);
int IP_COAP_SERVER_SetDefaultBlockSize(IP_COAP_SERVER_CONTEXT* pContext, U16 BlockSize);
int IP_COAP_SERVER_SetPOSTHandler     (IP_COAP_SERVER_CONTEXT* pContext, PF_POST_HANDLER pfPOSTCreateEntry);
int IP_COAP_SERVER_ConfigSet          (IP_COAP_SERVER_CONTEXT* pContext, U8 ConfigMask);
int IP_COAP_SERVER_ConfigClear        (IP_COAP_SERVER_CONTEXT* pContext, U8 ConfigMask);
int IP_COAP_SERVER_SetURIPort         (IP_COAP_SERVER_CONTEXT* pContext, U16 Port);
int IP_COAP_SERVER_SetHostName        (IP_COAP_SERVER_CONTEXT* pContext, const char* sHostName);
int IP_COAP_SERVER_SetErrorDescription(IP_COAP_SERVER_CONTEXT* pContext, const char* sErrorDesc);

//
// Client.
//
int IP_COAP_CLIENT_Init                  (IP_COAP_CLIENT_CONTEXT* pContext, U8* pMsgBuffer, U16 MsgBufferSize, const IP_COAP_API* pAPI);
int IP_COAP_CLIENT_Process               (IP_COAP_CLIENT_CONTEXT* pContext);
int IP_COAP_CLIENT_GetFreeRequestIdx     (IP_COAP_CLIENT_CONTEXT* pContext, unsigned* pIndex);
int IP_COAP_CLIENT_AbortRequestIdx       (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index);
int IP_COAP_CLIENT_SetServerAddress      (IP_COAP_CLIENT_CONTEXT* pContext, IP_COAP_CONN_INFO* pConnInfo);
int IP_COAP_CLIENT_SetDefaultBlockSize   (IP_COAP_CLIENT_CONTEXT* pContext, U16 BlockSize);
int IP_COAP_CLIENT_SetCommand            (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8 Type, U8 Code);
int IP_COAP_CLIENT_SetToken              (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pToken, U8 TokenLength);
int IP_COAP_CLIENT_SetPayloadHandler     (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, PF_CLIENT_PAYLOAD pf);
int IP_COAP_CLIENT_SetReplyWaitTime      (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U32 Seconds);
int IP_COAP_CLIENT_BuildAndSend          (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index);
int IP_COAP_CLIENT_GetLastResult         (IP_COAP_CLIENT_CONTEXT* pContext, U8* pCode, U8** ppError, U16* pLength);
U8* IP_COAP_CLIENT_GetMsgBuffer          (IP_COAP_CLIENT_CONTEXT* pContext, U16* pMsgLength);
int IP_COAP_CLIENT_GetLocationPath       (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8** ppLoc, U8* pLocLength);
int IP_COAP_CLIENT_GetLocationQuery      (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8** ppQuery, U8* pQueryLength);
int IP_COAP_CLIENT_SetOptionURIPath      (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pURI, U8 URILength);
int IP_COAP_CLIENT_SetOptionURIHost      (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pHost, U8 HostLength);
int IP_COAP_CLIENT_SetOptionURIPort      (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U16 Port);
int IP_COAP_CLIENT_SetOptionURIQuery     (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pQuery, U8 QueryLength);
int IP_COAP_CLIENT_SetOptionETag         (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pETag, U8 ETagLength);
int IP_COAP_CLIENT_SetOptionBlock        (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U16 Size);
int IP_COAP_CLIENT_SetOptionAccept       (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U16 Value);
int IP_COAP_CLIENT_SetOptionContentFormat(IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U16 Value);
int IP_COAP_CLIENT_SetOptionIfNoneMatch  (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index);
int IP_COAP_CLIENT_SetOptionLocationPath (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pLocation, U8 LocationLength);
int IP_COAP_CLIENT_SetOptionLocationQuery(IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pQuery, U8 QueryLength);
int IP_COAP_CLIENT_SetOptionProxyURI     (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pURI, U16 URILength);
int IP_COAP_CLIENT_SetOptionProxyScheme  (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U8* pScheme, U8 SchemeLength);
int IP_COAP_CLIENT_SetOptionSize1        (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, U32 Value);
int IP_COAP_CLIENT_SetOptionAddIFMatch   (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, IP_COAP_IF_MATCH_INFO* pIFMatch);
//
// Client observe.
//
int IP_COAP_CLIENT_OBS_Init             (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, IP_COAP_CLIENT_OBS* pObs, unsigned AutoToken);
int IP_COAP_CLIENT_OBS_Abort            (IP_COAP_CLIENT_CONTEXT* pContext, unsigned Index, IP_COAP_CLIENT_OBS* pObs, unsigned TryActiveAbort);
int IP_COAP_CLIENT_OBS_SetEndCallback   (IP_COAP_CLIENT_OBS* pObs, PF_OBS_END_TRANSFER pfObsEndTransfer, void* pParam);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
