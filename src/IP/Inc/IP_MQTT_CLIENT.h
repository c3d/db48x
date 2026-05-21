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

File    : IP_MQTT_CLIENT.h
Purpose : Header file for the MQTT (MQ Telemetry Transport) client.

Literature:
  [1] MQTT Version 3.1.1  - (http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.pdf)
  [2] MQTT Version 5.0    - (https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.pdf)
*/

#ifndef IP_MQTT_CLIENT_H      // Avoid multiple inclusion.
#define IP_MQTT_CLIENT_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

#include "SEGGER.h"
#include "IP_MQTT_CLIENT_Conf.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// For processing QoS 2 messages a subscriber needs to remember
// the packet IDs to be able to process data using only one buffer.
// A small amount of packet IDs to remember should be sufficient
// as QoS 2 messages not completely processed will be retransmitted anyhow.
//
#ifndef   IP_MQTT_CLIENT_RECEIVED_ID_CACHE
  #define IP_MQTT_CLIENT_RECEIVED_ID_CACHE  10
#endif

//
// Enables MQTT V5 support. This define can be set to 0 to save some code.
// With this define set to zero emNet MQTT Client only supports version 3.1.1 of MQTT.
//
#ifndef   IP_MQTT_CLIENT_SUPPORT_V5
  #define IP_MQTT_CLIENT_SUPPORT_V5  1
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

//
// [2] The Server MUST allow ClientID's which are between 1 and 23 UTF-8 encoded bytes in length, and that contain only the characters
// "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" [MQTT-3.1.3-5].
//
#ifndef IP_MQTT_CLIENT_ID_MAX_LEN
  #define IP_MQTT_CLIENT_ID_MAX_LEN                23
#endif

#define IP_MQTT_CLIENT_CLEAN_SESSION              1

//
//  MQTT Control Packet types
//
#define IP_MQTT_CLIENT_TYPES_CONNECT              1u   // Client request to connect to Server        (Client to Server)
#define IP_MQTT_CLIENT_TYPES_CONNACK              2u   // Connect acknowledgment                     (Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBLISH              3u   // Client to Server or Publish message        (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBACK               4u   // Publish acknowledgment                     (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBREC               5u   // Publish received (assured delivery part 1) (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBREL               6u   // Publish release (assured delivery part 2)  (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBCOMP              7u   // Publish complete (assured delivery part 3) (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_SUBSCRIBE            8u   // Client subscribe request                   (Client to Server)
#define IP_MQTT_CLIENT_TYPES_SUBACK               9u   // Subscribe acknowledgment                   (Server to Client)
#define IP_MQTT_CLIENT_TYPES_UNSUBSCRIBE         10u   // Unsubscribe request                        (Client to Server)
#define IP_MQTT_CLIENT_TYPES_UNSUBACK            11u   // Unsubscribe acknowledgment                 (Server to Client)
#define IP_MQTT_CLIENT_TYPES_PINGREQ             12u   // PING request                               (Client to Server)
#define IP_MQTT_CLIENT_TYPES_PINGRESP            13u   // PING response                              (Server to Client)
#define IP_MQTT_CLIENT_TYPES_DISCONNECT          14u   // Client is disconnecting                    (Client to Server) (5 only: or Server to Client)
#define IP_MQTT_CLIENT_TYPES_AUTH                15u   // Authentication exchange                    (5 only: Client to Server or Server to Client)

//
//  MQTT Control Packet flags
//
// Control Packet | Fixed header flags         | Bit 3 | Bit 2 | Bit 1 | Bit 0
// --------------------------------------------------------------------
// CONNECT        | Reserved                   | 0     | 0     | 0     |  0
// CONNACK        | Reserved                   | 0     | 0     | 0     |  0
// PUBLISH        | Used in MQTT 3.1.1 and 5   | DUP   | QoS   | QoS   | RETAIN
// PUBACK         | Reserved                   | 0     | 0     | 0     |  0
// PUBREC         | Reserved                   | 0     | 0     | 0     |  0
// PUBREL         | Reserved                   | 0     | 0     | 1     |  0
// PUBCOMP        | Reserved                   | 0     | 0     | 0     |  0
// SUBSCRIBE      | Reserved                   | 0     | 0     | 1     |  0
// SUBACK         | Reserved                   | 0     | 0     | 0     |  0
// UNSUBSCRIBE    | Reserved                   | 0     | 0     | 1     |  0
// UNSUBACK       | Reserved                   | 0     | 0     | 0     |  0
// PINGREQ        | Reserved                   | 0     | 0     | 0     |  0
// PINGRESP       | Reserved                   | 0     | 0     | 0     |  0
// DISCONNECT     | Reserved                   | 0     | 0     | 0     |  0
// AUTH           | Reserved                   | 0     | 0     | 0     |  0
//
#define IP_MQTT_CLIENT_FLAGS_PUBREL               (1uL << 1u)
#define IP_MQTT_CLIENT_FLAGS_SUBSCRIBE            (1uL << 1u)
#define IP_MQTT_CLIENT_FLAGS_UNSUBSCRIBE          (1uL << 1u)
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_RETAIN       (1uL << 0u)
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS0                0uL    // At most once delivery
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS1                1uL    // At least once delivery
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS2                2uL    // Exactly once delivery
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS_DUP      (1uL << 3u)   // Duplicate delivery of a PUBLISH packet


// Properties:
//
// [2] says in section 2.2.2.2:
//   A Property consists of an Identifier which defines its usage and data type, followed by a value. The
//   Identifier is encoded as a Variable Byte Integer. A Control Packet which contains an Identifier which is not
//   valid for its packet type, or contains a value not of the specified data type, is a Malformed Packet. If
//   received, use a CONNACK or DISCONNECT packet with Reason Code 0x81 (Malformed Packet) as
//   described in section 4.13 Handling errors. There is no significance in the order of Properties with different
//   Identifiers.
//
//lint -esym(621,IP_MQTT_PROP_TYPE_*)
#define IP_MQTT_PROP_TYPE_PAYLOAD_FORMAT_INDICATOR          0x01u  // | Byte                    | PUBLISH, Will Properties
#define IP_MQTT_PROP_TYPE_MESSAGE_EXPIRY_INTERVAL           0x02u  // | Four Byte Integer       | PUBLISH, Will Properties
#define IP_MQTT_PROP_TYPE_CONTENT_TYPE                      0x03u  // | UTF-8 Encoded String    | PUBLISH, Will Properties
#define IP_MQTT_PROP_TYPE_RESPONSE_TOPIC                    0x08u  // | UTF-8 Encoded String    | PUBLISH, Will Properties
#define IP_MQTT_PROP_TYPE_CORRELATION_DATA                  0x09u  // | Binary Data             | PUBLISH, Will Properties
#define IP_MQTT_PROP_TYPE_SUBSCRIPTION_ID                   0x0Bu  // | Variable Byte Integer   | PUBLISH, SUBSCRIBE
#define IP_MQTT_PROP_TYPE_SESSION_EXPIRY_INTERVAL           0x11u  // | Four Byte Integer       | CONNECT, CONNACK, DISCONNECT
#define IP_MQTT_PROP_TYPE_ASSIGNED_CLIENT_IDENTIFIER        0x12u  // | UTF-8 Encoded String    | CONNACK
#define IP_MQTT_PROP_TYPE_SERVER_KEEP_ALIVE                 0x13u  // | Two Byte Integer        | CONNACK
#define IP_MQTT_PROP_TYPE_AUTHENTICATION_METHOD             0x15u  // | UTF-8 Encoded String    | CONNECT, CONNACK, AUTH
#define IP_MQTT_PROP_TYPE_AUTHENTICATION_DATA               0x16u  // | Binary Data             | CONNECT, CONNACK, AUTH
#define IP_MQTT_PROP_TYPE_REQUEST_PROBLEM_INFORMATION       0x17u  // | Byte                    | CONNECT
#define IP_MQTT_PROP_TYPE_WILL_DELAY_INTERVAL               0x18u  // | Four Byte Integer       | Will Properties
#define IP_MQTT_PROP_TYPE_REQUEST_RESPONSE_INFORMATION      0x19u  // | Byte                    | CONNECT
#define IP_MQTT_PROP_TYPE_RESPONSE_INFORMATION              0x1Au  // | UTF-8 Encoded String    | CONNACK
#define IP_MQTT_PROP_TYPE_SERVER_REFERENCE                  0x1Cu  // | UTF-8 Encoded String    | CONNACK, DISCONNECT
#define IP_MQTT_PROP_TYPE_REASON_STRING                     0x1Fu  // | UTF-8 Encoded String    | CONNACK, PUBACK, PUBREC, PUBREL, PUBCOMP, SUBACK, UNSUBACK, DISCONNECT, AUTH
#define IP_MQTT_PROP_TYPE_RECEIVE_MAXIMUM                   0x21u  // | Two Byte Integer        | CONNECT, CONNACK
#define IP_MQTT_PROP_TYPE_TOPIC_ALIAS_MAXIMUM               0x22u  // | Two Byte Integer        | PUBLISH
#define IP_MQTT_PROP_TYPE_TOPIC_ALIAS                       0x23u  // | Two Byte Integer        | CONNACK
#define IP_MQTT_PROP_TYPE_MAXIMUM_QOS                       0x24u  // | Byte                    | CONNACK
#define IP_MQTT_PROP_TYPE_RETAIN_AVAILABLE                  0x25u  // | Byte                    | CONNACK
#define IP_MQTT_PROP_TYPE_USER_PROPERTY                     0x26u  // | UTF-8 String Pair       | CONNECT, CONNACK, PUBLISH, Will Properties, PUBACK, PUBREC, PUBREL, PUBCOMP, SUBSCRIBE, SUBACK, UNSUBSCRIBE, UNSUBACK, DISCONNECT, AUTH
#define IP_MQTT_PROP_TYPE_MAXIMUM_PACKET_SIZE               0x27u  // | Four Byte Integer       | CONNACK
#define IP_MQTT_PROP_TYPE_WILDCARD_SUBSCRIPTION_AVAILABLE   0x28u  // | Byte                    | CONNACK
#define IP_MQTT_PROP_TYPE_SUBSCRIPTION_IDENTIFIER_AVAILABLE 0x29u  // | Byte                    | CONNACK
#define IP_MQTT_PROP_TYPE_SHARED_SUBSCRIPTION_AVAILABLE     0x2Au  // | Byte                    | CONNACK


//
// [2] Table 2-6 - Reason Codes
//
//
// Regular reason codes.
//
//lint -esym(621,IP_MQTT_REASON_*)
#define IP_MQTT_REASON_SUCCESS                0x00u  // Success.
#define IP_MQTT_REASON_NORMAL_DISCONNECT      0x00u  // Close the connection normally. Do not send the Will Message.
#define IP_MQTT_REASON_GRANTED_QOS_0          0x00u  // The subscription is accepted and the maximum QoS sent will be QoS 0. This might be a lower QoS than was requested.
#define IP_MQTT_REASON_GRANTED_QOS_1          0x01u  // The subscription is accepted and the maximum QoS sent will be QoS 1. This might be a lower QoS than was requested.
#define IP_MQTT_REASON_GRANTED_QOS_2          0x02u  // The subscription is accepted and any received QoS will be sent to this subscription.
#define IP_MQTT_REASON_DISCONNECT_WITH_WILL   0x04u  // The Client wishes to disconnect but requires that the Server also publishes its Will Message.
#define IP_MQTT_REASON_NO_MATCH_SUB           0x10u  // The message is accepted but there are no subscribers. This is sent only by the Server. If the Server knows that there are no matching subscribers, it MAY use this Reason Code instead of 0x00 (Success).
#define IP_MQTT_REASON_NO_EXIST_SUB           0x11u  // No subscription existed
#define IP_MQTT_REASON_CONTINUE_AUTH          0x18u  // Continue the authentication with another step
#define IP_MQTT_REASON_RE_AUTH                0x19u  // Initiate a re-authentication
//
// Error reason codes.
//
#define IP_MQTT_REASON_UNSPECIFIED_ERR        0x80u  // The subscription is not accepted and the Server either does not wish to reveal the reason or none of the other Reason Codes apply.
#define IP_MQTT_REASON_MALFORMED_PACKET       0x81u  // The received packet does not conform to this specification.
#define IP_MQTT_REASON_PROTOCOL_ERR           0x82u  // An unexpected or out of order packet was received.
#define IP_MQTT_REASON_IMPL_SPECIFIC_ERR      0x83u  // The SUBSCRIBE is valid but the Server does not accept it.
#define IP_MQTT_REASON_UNSUP_PROTO_VER        0x84u  // The Server does not support the version of the MQTT protocol requested by the Client.
#define IP_MQTT_REASON_CLIENT_ID_NOT_VALID    0x85u  // The Client Identifier is a valid string but is not allowed by the Server.
#define IP_MQTT_REASON_BAD_USER_OR_PASS       0x86u  // The Server does not accept the User Name or Password specified by the Client
#define IP_MQTT_REASON_NOT_AUTHORIZED         0x87u  // The Client is not authorized to make this subscription.
#define IP_MQTT_REASON_SERVER_UNAVAILABLE     0x88u  // The MQTT Server is not available.
#define IP_MQTT_REASON_SERVER_BUSY            0x89u  // The Server is busy. Try again later.
#define IP_MQTT_REASON_BANNED                 0x8Au  // This Client has been banned by administrative action. Contact the server administrator.
#define IP_MQTT_REASON_SERVER_SHUTTING_DOWN   0x8Bu  // The Server is shutting down.
#define IP_MQTT_REASON_BAD_AUTH_METHOD        0x8Cu  // The authentication method is not supported or does not match the authentication method currently in use.
#define IP_MQTT_REASON_KEEP_ALIVE_TIMEOUT     0x8Du  // The Connection is closed because no packet has been received for 1.5 times the Keepalive time.
#define IP_MQTT_REASON_SESSION_TAKEN_OVER     0x8Eu  // Another Connection using the same ClientID has connected causing this Connection to be closed.
#define IP_MQTT_REASON_TOPIC_FILTER_INVALID   0x8Fu  // The Topic Filter is correctly formed but is not allowed for this Client.
#define IP_MQTT_REASON_TOPIC_NAME_INVALID     0x90u  // The Topic Name is not malformed, but is not accepted by this Client or Server.
#define IP_MQTT_REASON_PACKET_ID_IN_USE       0x91u  // The specified Packet Identifier is already in use.
#define IP_MQTT_REASON_PACKET_ID_NOT_FOUND    0x92u  // The Packet Identifier is not known. This is not an error during recovery, but at other times indicates a mismatch between the Session State on the Client and Server.
#define IP_MQTT_REASON_RX_MAX_EXCEEDED        0x93u  // The Client or Server has received more than Receive Maximum publication for which it has not sent PUBACK or PUBCOMP.
#define IP_MQTT_REASON_TOPIC_ALIAS_INVALID    0x94u  // The Client or Server has received a PUBLISH packet containing a Topic Alias which is greater than the Maximum Topic Alias it sent in the CONNECT or CONNACK packet.
#define IP_MQTT_REASON_PACKET_TOO_LARGE       0x95u  // The packet size is greater than Maximum Packet Size for this Client or Server.
#define IP_MQTT_REASON_MSG_RATE_TOO_HIGH      0x96u  // The received data rate is too high.
#define IP_MQTT_REASON_QUOTA_EXCEEDED         0x97u  // An implementation or administrative imposed limit has been exceeded.
#define IP_MQTT_REASON_ADMIN_ACTION           0x98u  // The Connection is closed due to an administrative action.
#define IP_MQTT_REASON_PAYLOAD_FORMAT_INVALID 0x99u  // The payload format does not match the one specified in the Payload Format Indicator.
#define IP_MQTT_REASON_RETAIN_NOT_SUP         0x9Au  // The Server has does not support retained messages.
#define IP_MQTT_REASON_QOS_NOT_SUP            0x9Bu  // The Client specified a QoS greater than the QoS specified in a Maximum QoS in the CONNACK.
#define IP_MQTT_REASON_USE_ANOTHER_SERVER     0x9Cu  // The Client should temporarily change its Server.
#define IP_MQTT_REASON_SERVER_MOVED           0x9Du  // The Server is moved and the Client should permanently change its server location.
#define IP_MQTT_REASON_SHARED_SUB_NOT_SUP     0x9Eu  // The Server does not support Shared Subscriptions for this Client.
#define IP_MQTT_REASON_CON_RATE_EXCEED        0x9Fu  // This connection is closed because the connection rate is too high.
#define IP_MQTT_REASON_MAX_CON_TIME           0xA0u  // The maximum connection time authorized for this connection has been exceeded.
#define IP_MQTT_REASON_SUB_ID_NOT_SUP         0xA1u  // The Server does not support Subscription Identifiers; the subscription is not accepted.
#define IP_MQTT_REASON_WILDCARD_SUB_NOT_SUP   0xA2u  // The Server does not support Wildcard Subscriptions; the subscription is not accepted.


/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef void* IP_MQTT_CLIENT_SOCKET;

/*********************************************************************
*
*       IP_MQTT_CLIENT_TRANSPORT_API
*
*  Description
*    Structure with pointers to the required socket interface functions.
*/
typedef struct {
  void* (*pfConnect)   (const char* sSrvAddr, unsigned SrvPort);    // Pointer to the connect function (for example, connect()).
  void  (*pfDisconnect)(void* pSocket);                             // Pointer to the disconnect function (for example, closesocket()).
  int   (*pfReceive)   (void* pSocket, char* pData, int Len);       // Pointer to a callback function (for example, send()).
  int   (*pfSend)      (void* pSocket, const char* pData, int Len); // Pointer to a callback function (for example, recv()).
} IP_MQTT_CLIENT_TRANSPORT_API;


typedef struct {
  const IP_MQTT_CLIENT_TRANSPORT_API* pAPI;
        IP_MQTT_CLIENT_SOCKET         Socket;  // Socket used for the connection to the MQTT broker
} IP_MQTT_CLIENT_CONNECTION_CONTEXT;

/*********************************************************************
*
*       MQTT_DLIST
*
*  Doubly linked lists.
*/
typedef struct IP_MQTT_DLIST_ITEM_STRUCT IP_MQTT_DLIST_ITEM;
struct IP_MQTT_DLIST_ITEM_STRUCT {
  IP_MQTT_DLIST_ITEM* pNext;
  IP_MQTT_DLIST_ITEM* pPrev;
  unsigned            MessageId;
};

typedef struct {
  IP_MQTT_DLIST_ITEM* pFirst;
  int                 NumItems;
#if DEBUG != 0
  int                 MaxItems;
#endif
} IP_MQTT_DLIST_HEAD;

/*********************************************************************
*
*       IP_MQTT_BIN_DATA
*
*  Description
*     Structure describing a MQTT 5 Binary Property.
*/
typedef struct {
  U16 Len;                                  // Number of bytes inside the buffer pointed to by pData.
  const U8 * pData;                         // Pointer to a buffer containing the data.
} IP_MQTT_BIN_DATA;

/*********************************************************************
*
*       IP_MQTT_STR_DATA
*
*  Description
*     Structure describing a MQTT 5 String Property.
*     Strings should not be terminated with \0.
*/
typedef struct {
  U16 Len;                                  // Length of the string.
  const char * pData;                       // Pointer to a buffer containing the string.
} IP_MQTT_STR_DATA;

/*********************************************************************
*
*       IP_MQTT_STR_PAIR_DATA
*
*  Description
*    Structure describing a MQTT 5 String Pair Property.
*    This is exclusivly used by IP_MQTT_PROP_TYPE_USER_PROPERTY.
*/
typedef struct {
  U16 Len1;                                 // Length of the first string.
  U16 Len2;                                 // Length of the second string.
  const char * pData1;                      // Pointer to a buffer containing the first string.
  const char * pData2;                      // Pointer to a buffer containing the second string.
} IP_MQTT_STR_PAIR_DATA;

/*********************************************************************
*
*       IP_MQTT_PROPERTY
*
*  Description
*    Structure describing a MQTT 5 Property.
*/
typedef struct {
  U8 PropType;                              // Type of the property.
  union {
    U8  Data_U8;                            // Byte storage.
    U16 Data_U16;                           // Half-word storage.
    U32 Data_U32;                           // Word storage.
    U32 Data_VarInt;                        // Variable integers are stored by the user in a U32,
                                            // the MQTT module will do the encoding in the Variable Integer format.
    IP_MQTT_BIN_DATA Data_Bin;              // Structure of type IP_MQTT_BIN_DATA.
    IP_MQTT_STR_DATA Data_Str;              // Structure of type IP_MQTT_STR_DATA.
    IP_MQTT_STR_PAIR_DATA Data_StrPair;     // Structure of type IP_MQTT_STR_PAIR_DATA.
  } PropData;
} IP_MQTT_PROPERTY;

/*********************************************************************
*
*       IP_MQTT_CLIENT_MESSAGE
*
*  Description
*    Message maintenance structure.
*/
typedef struct {
  IP_MQTT_DLIST_ITEM      Link;             // Internal link.
  const char*             sTopic;           // Pointer to the topic string.
  const char*             pData;            // Pointer to the payload which should be published.
  U32                     DataLen;          // Number of payload bytes.
  U16                     PacketId;         // Packet ID. (Will be filled by the MQTT client module)
  U8                      QoS;              // QoS flag.
  U8                      Retain;           // Retain flag.
  U8                      Duplicate;        // Duplicate flag. (Will be filled by the MQTT client module)
#if IP_MQTT_CLIENT_SUPPORT_V5
  const IP_MQTT_PROPERTY ** paProperties;   // [MQTT 5 only] Array containing pointers to IP_MQTT_PROPERTY structures. Can be NULL.
  U8                        NumProperties;  // [MQTT 5 only] Number of elements inside paProperties.
#endif
} IP_MQTT_CLIENT_MESSAGE;

/*********************************************************************
*
*       IP_MQTT_CLIENT_APP_API
*
*  Description
*    Structure with pointers to the required functions.
*/
typedef struct {
  U16   (*pfGenRandom)          (void);                                                                     // Pointer to a function which returns a random unsigned short value. The random value is used for the packet ID of MQTT packets.
  void* (*pfAlloc)              (U32 NumBytesReq);                                                          // Pointer to the Alloc() function. This function is only required, if you plan to call the MQTT client API from several tasks (e.g. MQTT client acts as subscriber and publisher.)
  void  (*pfFree)               (void* p);                                                                  // Pointer to the Free () function. This function is only required, if you plan to call the MQTT client API from several tasks (e.g. MQTT client acts as subscriber and publisher.)
  void  (*pfLock)               (void);                                                                     // Pointer to a function which acquires a lock to ensure MQTT API is thread safe. This function is only required, if you plan to call the MQTT client API from several tasks (e.g. MQTT client acts as subscriber and publisher.)
  void  (*pfUnlock)             (void);                                                                     // Pointer to a function which releases a previously acquired lock for thread safety MQTT API. This function is only required, if you plan to call the MQTT client API from several tasks (e.g. MQTT client acts as subscriber and publisher.)
  int   (*pfRecvMessage)        (void* pMQTTClient, void* pPublish, int NumBytesRem);                       // Deprecated, use pfRecvMessageEx().
  int   (*pfOnMessageConfirm)   (void* pMQTTClient, U8 Type, U16 PacketId);                                 // Deprecated, use pfOnMessageConfirmEx().
  int   (*pfHandleError)        (void* pMQTTClient);                                                        // Pointer to a callback which is called in case of an error.
  void  (*pfHandleDisconnect)   (void* pMQTTClient, U8 ReasonCode);                                         // [MQTT 5 only] Pointer to an optional callback which is called when the server sends a Disconnect Request. Can be NULL.
  int   (*pfOnMessageConfirmEx) (void* pMQTTClient, U8 Type, U16 PacketId, U8 ReasonCode);                  // Pointer to a callback which is called when all QoS related messages are processed for a message. Sent messages can be freed after this callback has been called. Received messages can be processed by the application after this callback has been called.
  int   (*pfRecvMessageEx)      (void* pMQTTClient, void* pPublish, int NumBytesRem, U8 * pReasonCode);     // Pointer to a callback which is called when a PUBLISH message is received.
  void  (*pfOnProperty)         (void* pMQTTClient, U16 PacketId, U8 PacketType, IP_MQTT_PROPERTY * pProp); // [MQTT 5 only] Pointer to an optional callback which is called when a non-PUBLISH (CONNACK, PUBACK, PUBREC, PUBCOMP, SUBACK, UNSUBACK, DISCONNECT, AUTH) message is received with a Property. The callback is called for each contained property individually. Can be NULL.
} IP_MQTT_CLIENT_APP_API;

/*********************************************************************
*
*       IP_MQTT_CLIENT_TOPIC_FILTER
*
*  Description
*    Structure used to subscribe to a particular topic.
*/
typedef struct {
  const char* sTopicFilter;                         // Pointer to the topic filter string (zero-terminated).
  U16         Length;                               // This field is deprecated! It may be removed in future versions.
                                                    // Make sure sTopicFilter is zero-terminated.
  U8          QoS;                                  // Quality of service flag.
} IP_MQTT_CLIENT_TOPIC_FILTER;

/*********************************************************************
*
*       IP_MQTT_CLIENT_SUBSCRIBE
*
*  Description
*    Structure used by the IP_MQTT_CLIENT_Subscribe() function to subscribe to many topics at once.
*    The topics are described in an array of IP_MQTT_CLIENT_TOPIC_FILTER.
*/
typedef struct {
  IP_MQTT_DLIST_ITEM           Link;                // Internal link.
  IP_MQTT_CLIENT_TOPIC_FILTER* pTopicFilter;        // Pointer to the first topic filter structure.
  int                          TopicCnt;            // Number of added topics.
  U16                          PacketId;            // Packet ID. (Will be filled by the MQTT client module.)
  U8                           ReturnCode;          // Return code. (Will be filled by the MQTT client module.)
#if IP_MQTT_CLIENT_SUPPORT_V5
  U8                         * paReasonCodes;       // [MQTT 5 only] Pointer to an array to store the received Reason Codes. Can be NULL.
                                                    // In case the application subscribes to multiple topics at once this buffer will be filled with the reason codes for each topic.
                                                    // Reason codes will appear in the same order as the subscribed topics, e.g.
                                                    // For a subscription with two topics t1 (QoS1) and t2 (QoS2) you will
                                                    // receive IP_MQTT_REASON_GRANTED_QOS_1, IP_MQTT_REASON_GRANTED_QOS_2 in the buffer.
                                                    // If the pointer is not NULL the size of this buffer MUST be the same as TopicCnt.
                                                    // When using pfRecvMessageEx care must be taken because the buffer is filled with
                                                    // valid values only after the pfRecvMessageEx callback is called with PacketType SUBACK and the correct Packet ID.
#endif
} IP_MQTT_CLIENT_SUBSCRIBE;

/*********************************************************************
*
*       IP_MQTT_CLIENT_CONTEXT
*
*  Description
*    MQTT client instance structure.
*/
typedef struct IP_MQTT_CLIENT_CONTEXT_STRUCT IP_MQTT_CLIENT_CONTEXT;
struct IP_MQTT_CLIENT_CONTEXT_STRUCT {
        IP_MQTT_CLIENT_CONTEXT*           pNext;                // Link element for internal management.
        IP_MQTT_DLIST_HEAD                Head;
  const IP_MQTT_CLIENT_APP_API*           pAppApi;
  const char*                             sId;                  // Client Id.
  const char*                             sUser;
  const char*                             sPass;
        IP_MQTT_CLIENT_MESSAGE*           pLastWill;
        char*                             pBuffer;              // Buffer of the MQTT client.
        IP_MQTT_CLIENT_CONNECTION_CONTEXT Connection;           // API which should be used.
        U32                               BufferSize;           // Buffer size.
        U32                               ReceivedNumBytes;     // Number of bytes of the received payload.
        unsigned                          State;                // State of the MQTT connection.
        U16                               ReceivedID;           // Packet ID of the received packet.
        U16                               KeepAlive;            // Keep alive time in seconds. The application must send KeepAlive packets (PINGREQ) within the configured timeout.
        U16                               aQOS2Pending[IP_MQTT_CLIENT_RECEIVED_ID_CACHE];
        U8                                ReceivedQOS;          // QoS of the received packet.
        U8                                Version;              // 4 for MQTT 3.1.1, 5 for MQTT 5
#if IP_MQTT_CLIENT_SUPPORT_V5
        U16                               ReceiveMax;
        U32                               SessionExpiryInterval;
        U8                                ServerMaxQoS;
        U8                                ServerRetainAvailable;
        U16                               MaxTopicAlias;        // Maximum number of topic aliases. Currently not supported and must be set to 0.
        U32                               MaxPacketSize;        // Maximum packet size. Can be used to tell the broker to limit the packet size (total number of bytes in an MQTT Control Packet) to a specific value.
                                                                // The default of 0 means no limit.
#endif
};

/*********************************************************************
*
*       IP_MQTT_CONNECT_PARAM
*
*  Description
*    Structure containing parameters required for a new connection to a MQTT broker.
*/
typedef struct {
  const char*               sAddr;          // String with address of the broker.
  U16                       Port;           // Listening port of the broker.
  U8                        CleanSession;   // * 0: Reuse old session data to continue.
                                            // * 1: Start with a clean session.
  U8                        Version;        // Following values are allowed:
                                            // * 4 - use MQTT 3.1.1
                                            // * 5 - use MQTT 5
#if IP_MQTT_CLIENT_SUPPORT_V5
  const IP_MQTT_PROPERTY ** paProperties;   // [MQTT 5 only] Array containing pointers to IP_MQTT_PROPERTY structures. Can be NULL.
  U8                        NumProperties;  // [MQTT 5 only] Number of elements inside paProperties.
#endif
} IP_MQTT_CONNECT_PARAM;

/*********************************************************************
*
*       API functions / Function prototypes
*
**********************************************************************
*/
int  IP_MQTT_CLIENT_Init                (IP_MQTT_CLIENT_CONTEXT* pClient, char* pBuffer, U32 BufferSize, const IP_MQTT_CLIENT_TRANSPORT_API* pAPI, const IP_MQTT_CLIENT_APP_API* pAppAPI, const char* sId);
int  IP_MQTT_CLIENT_SetLastWill         (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_MESSAGE* pLastWill);
int  IP_MQTT_CLIENT_SetUserPass         (IP_MQTT_CLIENT_CONTEXT* pClient, const char* sUser, const char* sPass);
int  IP_MQTT_CLIENT_SetKeepAlive        (IP_MQTT_CLIENT_CONTEXT* pClient, U16 KeepAlive);
void IP_MQTT_CLIENT_Timer               (void);

int  IP_MQTT_CLIENT_Connect             (IP_MQTT_CLIENT_CONTEXT* pClient, const char* sAddr, U16 Port, U8 CleanSession);
int  IP_MQTT_CLIENT_ConnectEx           (IP_MQTT_CLIENT_CONTEXT* pClient, const IP_MQTT_CONNECT_PARAM* pConnectPara, U8 * pReasonCode);
int  IP_MQTT_CLIENT_Disconnect          (IP_MQTT_CLIENT_CONTEXT* pClient);
int  IP_MQTT_CLIENT_Publish             (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_MESSAGE* pPublish);
int  IP_MQTT_CLIENT_Subscribe           (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_SUBSCRIBE* pSubscribe); //lint !e9046 Typographical ambiguity
int  IP_MQTT_CLIENT_Unsubscribe         (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_SUBSCRIBE* pUnsubscribe);
int  IP_MQTT_CLIENT_WaitForNextMessage  (IP_MQTT_CLIENT_CONTEXT* pClient, U32* pType, U32* pNumBytesRecv, char* pBuffer, U32 BufferSize);
int  IP_MQTT_CLIENT_Recv                (IP_MQTT_CLIENT_CONTEXT* pClient, char* pBuffer, U32 BufferSize);

int  IP_MQTT_CLIENT_Exec                (IP_MQTT_CLIENT_CONTEXT* pClient);
int  IP_MQTT_CLIENT_ParsePublish        (IP_MQTT_CLIENT_MESSAGE* pPublish, char* pBuffer, int NumBytes, char** ppTopic, int* pNumBytesTopic, char** ppPayload, int* pNumBytesPayload);
int  IP_MQTT_CLIENT_ParsePublishEx      (const IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_MESSAGE* pPublish, char* pBuffer, int NumBytes, char** ppTopic, int* pNumBytesTopic, char** ppPayload, int* pNumBytesPayload, char ** ppProperties, int* pNumBytesProperties);
int  IP_MQTT_CLIENT_IsClientConnected   (const IP_MQTT_CLIENT_CONTEXT* pClient);

const char * IP_MQTT_Property2String    (U8 x);
const char * IP_MQTT_ReasonCode2String  (U8 x);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif // #ifndef IP_MQTT_CLIENT_H // Avoid multiple inclusion.

/*************************** End of file ****************************/
