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

File    : IP_SNMP_AGENT.h
Purpose : Header file for SNMP Agent.
*/

#include "SEGGER.h"
#include "IP_SNMP_AGENT_Conf.h"

#ifndef IP_SNMP_AGENT_H       // Avoid multiple inclusion.
#define IP_SNMP_AGENT_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   IP_SNMP_AGENT_SUPPORT_64_BIT_TYPES
  #define IP_SNMP_AGENT_SUPPORT_64_BIT_TYPES  1           // Can be disabled to support compiling on older toolchains (not C99 compliant).
#endif

#ifndef   IP_SNMP_AGENT_USE_PARA                          // Some compiler complain about unused parameters.
  #define IP_SNMP_AGENT_USE_PARA(Para)        (void)Para  // This works for most compilers.
#endif

#ifndef   IP_SNMP_AGENT_MEMCPY
  #include "string.h"  // For memcpy() .
  #define IP_SNMP_AGENT_MEMCPY                memcpy
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

//
// SNMP versions.
//
#define IP_SNMP_VERSION_1  0
#define IP_SNMP_VERSION_2  1
#define IP_SNMP_VERSION_3  3

//
// Access permissions.
//
#define IP_SNMP_AGENT_PERM_READ_MASK   (1 << 0)
#define IP_SNMP_AGENT_PERM_WRITE_MASK  (1 << 1)

//
// SNMPv3 "msgFlags".
//
#define IP_SNMPV3_MSG_FLAG_AUTH_MASK    (1u << 0)
#define IP_SNMPV3_MSG_FLAG_PRIV_MASK    (1u << 1)
#define IP_SNMPV3_MSG_FLAG_REPORT_MASK  (1u << 2)

//
// SNMP PDU types.
//
#define IP_SNMP_PDU_TYPE_GET_REQUEST       0xA0
#define IP_SNMP_PDU_TYPE_GET_NEXT_REQUEST  0xA1
#define IP_SNMP_PDU_TYPE_GET_RESPONSE      0xA2
#define IP_SNMP_PDU_TYPE_SET_REQUEST       0xA3
#define IP_SNMP_PDU_TYPE_TRAPV1            0xA4
#define IP_SNMP_PDU_TYPE_GET_BULK_REQUEST  0xA5
#define IP_SNMP_PDU_TYPE_INFORMV2          0xA6
#define IP_SNMP_PDU_TYPE_TRAPV2            0xA7
#define IP_SNMP_PDU_TYPE_REPORT            0xA8

//
// SNMP generic-trap OID values.
//
#define IP_SNMP_GENERIC_TRAP_OID_COLD_START              { 0x2B, 0x06, 0x01, 0x06, 0x03, 0x01, 0x01, 0x05, 0x01 }  // Generic SMIv1 TRAP #0 OID value.
#define IP_SNMP_GENERIC_TRAP_OID_WARM_START              { 0x2B, 0x06, 0x01, 0x06, 0x03, 0x01, 0x01, 0x05, 0x02 }  // Generic SMIv1 TRAP #1 OID value.
#define IP_SNMP_GENERIC_TRAP_OID_LINK_DOWN               { 0x2B, 0x06, 0x01, 0x06, 0x03, 0x01, 0x01, 0x05, 0x03 }  // Generic SMIv1 TRAP #2 OID value.
#define IP_SNMP_GENERIC_TRAP_OID_LINK_UP                 { 0x2B, 0x06, 0x01, 0x06, 0x03, 0x01, 0x01, 0x05, 0x04 }  // Generic SMIv1 TRAP #3 OID value.
#define IP_SNMP_GENERIC_TRAP_OID_AUTHENTICATION_FAILURE  { 0x2B, 0x06, 0x01, 0x06, 0x03, 0x01, 0x01, 0x05, 0x05 }  // Generic SMIv1 TRAP #4 OID value.
#define IP_SNMP_GENERIC_TRAP_OID_EGP_NEIGHBOR_LOSS       { 0x2B, 0x06, 0x01, 0x06, 0x03, 0x01, 0x01, 0x05, 0x06 }  // Generic SMIv1 TRAP #5 OID value.
#define IP_SNMP_GENERIC_TRAP_OID_ENTERPRISE_SPECIFIC     { 0x2B, 0x06, 0x01, 0x06, 0x03, 0x01, 0x01, 0x05, 0x07 }  // Generic SMIv1 TRAP #6 OID value.

//
// SNMP field types.
//
#define IP_SNMP_TYPE_INTEGER       0x02
#define IP_SNMP_TYPE_OCTET_STRING  0x04
#define IP_SNMP_TYPE_NULL          0x05
#define IP_SNMP_TYPE_OID           0x06
#define IP_SNMP_TYPE_IP_ADDRESS    0x40  // Bytes can not be truncated, always 4 bytes. This is IPv4 only. There is no type for IPv6.
#define IP_SNMP_TYPE_COUNTER32     0x41
#define IP_SNMP_TYPE_UNSIGNED32    0x42
#define IP_SNMP_TYPE_TIME_TICKS    0x43
#define IP_SNMP_TYPE_OPAQUE        0x44  // Used to encapsulate non standard types such as float(120). The real tag to use is BER encoded.

//
// Non-standard SNMP field types.
// This are typically SNMP types with a value above 32
// and typically come wrapped in an Opaque type.
// Although not specified in the main RFCs of SNMP
// they are agreed to be implemented in a specific
// way and commonly used by various tools.
// When testing with tools like Net-SNMP a response
// of an Opaque wrapped type might only display the
// bytes of the Opaque instead of a readable result.
//
// float and double are special cases as well as
// typically the value of a type wrapped inside an
// Opaque needs to be BER serialized. As float and double
// are already IEEE 754 encoded they will not be encoded
// again.
//
#if IP_SNMP_AGENT_SUPPORT_64_BIT_TYPES
#define IP_SNMP_TYPE_COUNTER64   0x46
#define IP_SNMP_TYPE_DOUBLE      0x79
#define IP_SNMP_TYPE_INTEGER64   0x7A
#define IP_SNMP_TYPE_UNSIGNED64  0x7B
#endif

#define IP_SNMP_TYPE_FLOAT       0x78

//
// SNMP field types that use the same tag ID as other types.
// These types are compatible and are therefore only remapped.
//
#define IP_SNMP_TYPE_INTEGER32  IP_SNMP_TYPE_INTEGER
#define IP_SNMP_TYPE_BITS       IP_SNMP_TYPE_OCTET_STRING
#define IP_SNMP_TYPE_COUNTER    IP_SNMP_TYPE_COUNTER32
#define IP_SNMP_TYPE_GAUGE      IP_SNMP_TYPE_UNSIGNED32
#define IP_SNMP_TYPE_GAUGE32    IP_SNMP_TYPE_UNSIGNED32

//
// SNMP return/error codes. To be used as return values in MIB callbacks.
//
#define IP_SNMP_OK                 0

#define IP_SNMP_ERR_TOO_BIG        1
#define IP_SNMP_ERR_NO_SUCH_NAME   2
#define IP_SNMP_ERR_BAD_VALUE      3
#define IP_SNMP_ERR_GENERIC        5
#define IP_SNMP_ERR_NO_ACCESS      6
#define IP_SNMP_ERR_WRONG_TYPE     7
#define IP_SNMP_ERR_NO_CREATION   11
#define IP_SNMP_ERR_AUTH          16

//
// IP_SNMP_AGENT return/error codes. Returned by the SNMP Agent API.
//
#define IP_SNMP_AGENT_OK                        0

#define IP_SNMP_AGENT_ERR_MISC                 -1
#define IP_SNMP_AGENT_ERR_UNSUPPORTED_VERSION  -2
#define IP_SNMP_AGENT_ERR_AUTH                 -3
#define IP_SNMP_AGENT_ERR_MALFORMED_MESSAGE    -4
#define IP_SNMP_AGENT_ERR_TOO_BIG              -5

//
// IP_SNMP_AGENT INFORM status codes.
//
#define IP_SNMP_AGENT_INFORM_STATUS_CANCELED         -2
#define IP_SNMP_AGENT_INFORM_STATUS_TIMEOUT          -1
#define IP_SNMP_AGENT_INFORM_STATUS_WAITING_FOR_ACK   0
#define IP_SNMP_AGENT_INFORM_STATUS_ACK_RECEIVED      1
#define IP_SNMP_AGENT_INFORM_STATUS_NACK_RECEIVED     2

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct IP_SNMP_AGENT_CONTEXT_STRUCT IP_SNMP_AGENT_CONTEXT;

typedef struct {
  void  (*pfInit)             (void);
  void  (*pfDeInit)           (void);
  void  (*pfLock)             (void);
  void  (*pfUnlock)           (void);
  void* (*pfAllocSendBuffer)  (void* pUserContext, U8** ppBuffer, U32 NumBytes, U8 IPAddrLen);
  void  (*pfFreeSendBuffer)   (void* pUserContext, void* p, char SendCalled, int r);
  int   (*pfSendTrapInform)   (void* pContext, void* pUserContext, void* hBuffer, const U8* pData, U32 NumBytes, U8* pIPAddr, U16 Port, U8 IPAddrLen);
  U32   (*pfGetTime)          (void);
  U32   (*pfSysTicks2SnmpTime)(U32 SysTicks);
  U32   (*pfSnmpTime2SysTicks)(U32 SnmpTime);
} IP_SNMP_AGENT_API;

typedef struct {
  const U8* pOID;
        U16 Len;
        U8  Perm;
} IP_SNMP_AGENT_PERM;

typedef struct IP_SNMP_AGENT_COMMUNITY {
  struct IP_SNMP_AGENT_COMMUNITY* pNext;
  const  char*                    sCommunity;
         U32                      Len;
  const  IP_SNMP_AGENT_PERM*      pPerm;
} IP_SNMP_AGENT_COMMUNITY;

typedef struct {
  U8* pBuffer;
  U8* pData;
  U32 BufferSize;
  U32 NumBytesLeft;
} IP_SNMP_AGENT_BUFFER_DESC;

/*********************************************************************
*
*       IP_SNMP_SM_USM_AUTH_API_CALC_HMAC_FUNC
*
*  Function description
*    Calculates the AUTH(entication) specific HMAC.
*
*  Parameters
*    pContext        : Pointer to an SNMP Agent context.
*    pBuffer         : Pointer to a buffer descriptor of type
*                      IP_SNMP_AGENT_BUFFER_DESC .
*    pAuthParamsField: Location of the "msgAuthenticationParameters"
*                      field in the mesaage.
*/
typedef void IP_SNMP_SM_USM_AUTH_API_CALC_HMAC_FUNC(IP_SNMP_AGENT_CONTEXT* pContext, IP_SNMP_AGENT_BUFFER_DESC* pBuffer, U8* pAuthParamsField);

/*********************************************************************
*
*       IP_SNMP_SM_USM_AUTH_API
*
*  Function description
*    Hash specific driver-like API for the
*    User-basedSecurityModel (USM) AUTH(entication) of a user.
*/
typedef struct {
  IP_SNMP_SM_USM_AUTH_API_CALC_HMAC_FUNC* pfCalcHMAC;  // Callback caclulating the hash specific AUTHentication HMAC.
  U8                                      DigestLen;   // Length of the digest depending on the hash algorithm e.g.:
                                                       //   * MD5  : 16 bytes
                                                       //   * SHA-1: 20 bytes
  U8                                      HmacLen;     // Length of the SNMP specific HMAC for the selected hash
                                                       // algorithm according to the SNMP RFCs:
                                                       //   * MD5  : 12 bytes
                                                       //   * SHA-1: 12 bytes
} IP_SNMP_SM_USM_AUTH_API;

extern const IP_SNMP_SM_USM_AUTH_API IP_SNMP_SM_USM_AuthMD5;
extern const IP_SNMP_SM_USM_AUTH_API IP_SNMP_SM_USM_AuthSHA1;

/*********************************************************************
*
*       IP_SNMP_HASH_INIT_FUNC
*
*  Function description
*    Returns/initializes a fresh hash context.
*
*  Return value
*    Initialized hash context.
*
*  Additional information
*    Calculating hashes is done from a task that uses the API lock.
*    Therefore it is typically sufficient to use a single static
*    hash context.
*
*    For the moment the routine is not expected to fail and
*    return a NULL pointer.
*/
typedef void* IP_SNMP_HASH_INIT_FUNC(void);

/*********************************************************************
*
*       IP_SNMP_HASH_ADD_FUNC
*
*  Function description
*    Adds data to the hash calculation.
*
*  Parameters
*    pContext: Pointer to hash context returned from init callback.
*    pInput  : Pointer to data to add.
*    InputLen: Length of the data to add from pInput .
*/
typedef void IP_SNMP_HASH_ADD_FUNC(void* pContext, const U8* pInput, unsigned InputLen);

/*********************************************************************
*
*       IP_SNMP_HASH_FINAL_FUNC
*
*  Function description
*    Finalizes the hash calculation and returns the digest.
*
*  Parameters
*    pContext : Pointer to hash context returned from init callback.
*    pDigest  : Pointer where to store the result.
*    DigestLen: Maximum size of the buffer where to store the result.
*/
typedef void IP_SNMP_HASH_FINAL_FUNC(void* pContext, U8* pDigest, unsigned DigestLen);

/*********************************************************************
*
*       IP_SNMP_HASH_API
*
*  Function description
*    Hash API for the User-basedSecurityModel (USM) AUTH(entication)
*    of a user.
*/
typedef struct {
  IP_SNMP_HASH_INIT_FUNC*  pfInit;   // Calllback to allocate a fresh hash algorithm context.
  IP_SNMP_HASH_ADD_FUNC*   pfAdd;    // Calllback to add more data into the hash algorithm.
  IP_SNMP_HASH_FINAL_FUNC* pfFinal;  // Calllback to finalize hashing and return the digest.
} IP_SNMP_HASH_API;

/*********************************************************************
*
*       IP_SNMP_SM_USM_AUTH_PARAMS
*
*  Function description
*    Configuration parameters for the User-basedSecurityModel (USM)
*    authentication for a user.
*/
typedef struct {
  const IP_SNMP_SM_USM_AUTH_API* pAuthAPI;  // Pointer to the AUTH(entication) specific handling API.
  const IP_SNMP_HASH_API*        pHashAPI;  // Pointer to the hash API to use.
} IP_SNMP_SM_USM_AUTH_PARAMS;

typedef enum {
  IP_SNMP_CIPHER_DIR_DECRYPT = 0,
  IP_SNMP_CIPHER_DIR_ENCRYPT
} IP_SNMP_CIPHER_DIR;

/*********************************************************************
*
*       IP_SNMP_SM_USM_PRIV_API_EXEC_FUNC
*
*  Function description
*    Executes the PRIV(acy) specific cipher handling.
*
*  Parameters
*    pContext : Pointer to an SNMP Agent context.
*    pData    : Pointer to the data to decrypt or encrypt in-place.
*    NumBytes : Number of bytes to decrypt/encrypt.
*    SaltLen  : Length of the value of the "msgPrivacyParameters" field.
*    Direction: Decrypt or encrypt direction of type IP_SNMP_CIPHER_DIR .
*                 * IP_SNMP_CIPHER_DIR_DECRYPT
*                 * IP_SNMP_CIPHER_DIR_ENCRYPT
*
*  Return value
*    == 0: O.K.
*    <  0: Error (not enough bytes in buffer?)
*/
typedef int IP_SNMP_SM_USM_PRIV_API_EXEC_FUNC(IP_SNMP_AGENT_CONTEXT* pContext, U8* pData, unsigned NumBytes, unsigned SaltLen, IP_SNMP_CIPHER_DIR Direction);

/*********************************************************************
*
*       IP_SNMP_SM_USM_PRIV_API
*
*  Function description
*    Cipher specific driver-like API for the
*    User-basedSecurityModel (USM) PRIV(acy) of a user.
*/
typedef struct {
  IP_SNMP_SM_USM_PRIV_API_EXEC_FUNC* pfExec;    // Callback executing the cipher specific PRIV(acy) handling.
  U8                                 BlockLen;  // Length of each individual ciphertext block.
} IP_SNMP_SM_USM_PRIV_API;

extern const IP_SNMP_SM_USM_PRIV_API IP_SNMP_SM_USM_PrivDES;

/*********************************************************************
*
*       IP_SNMP_CIPHER_INIT_FUNC
*
*  Function description
*    Returns/initializes a fresh cipher context for a decrypt or
*    encrypt operation.
*
*  Parameters
*    pKey     : Pointer to the cipher key to use. Its length is
*               determined by the PRIV(acy) cipher selected via the
*               user table.
*    KeyLen   : Length of the key at pKey .
*    Direction: Decrypt or encrypt direction of type IP_SNMP_CIPHER_DIR .
*                 * IP_SNMP_CIPHER_DIR_DECRYPT
*                 * IP_SNMP_CIPHER_DIR_ENCRYPT
*
*  Return value
*    Initialized cipher context.
*
*  Additional information
*    Decrypt/encrypt is done from a task that uses the API lock.
*    Therefore it is typically sufficient to use a single static
*    cipher context.
*
*    For the moment the routine is not expected to fail and
*    return a NULL pointer.
*/
typedef void* IP_SNMP_CIPHER_INIT_FUNC(const U8* pKey, unsigned KeyLen, IP_SNMP_CIPHER_DIR Direction);

/*********************************************************************
*
*       IP_SNMP_CIPHER_EXEC_FUNC
*
*  Function description
*    Decrypts/encrypts data.
*
*  Parameters
*    pContext : Pointer to cipher context returned from init callback.
*    pOutput  : Pointer where to store the decrypted output.
*    pInput   : Pointer to the encrypted input.
*    InputLen : Length of the data to decrypt from pInput .
*    pIV      : Pointer to the IV (InitializationVector) to use. The
*               size of the IV is determined by the PRIV(acy) cipher
*               selected via the user table.
*    Direction: Decrypt or encrypt direction of type IP_SNMP_CIPHER_DIR .
*                 * IP_SNMP_CIPHER_DIR_DECRYPT
*                 * IP_SNMP_CIPHER_DIR_ENCRYPT
*/
typedef void IP_SNMP_CIPHER_EXEC_FUNC(void* pContext, U8* pOutput, const U8* pInput, unsigned InputLen, U8* pIV, IP_SNMP_CIPHER_DIR Direction);

/*********************************************************************
*
*       IP_SNMP_CIPHER_FINAL_FUNC
*
*  Function description
*    Finalizes the decrypt or encrypt operation.
*
*  Parameters
*    pContext : Pointer to hash context returned from init callback.
*    Direction: Decrypt or encrypt direction of type IP_SNMP_CIPHER_DIR .
*                 * IP_SNMP_CIPHER_DIR_DECRYPT
*                 * IP_SNMP_CIPHER_DIR_ENCRYPT
*
*  Additional information
*    This callback can be used to free resources allocated during init
*    or to kill any security related leftovers from the cipher operation.
*/
typedef void IP_SNMP_CIPHER_FINAL_FUNC(void* pContext, IP_SNMP_CIPHER_DIR Direction);

/*********************************************************************
*
*       IP_SNMP_CIPHER_API
*
*  Function description
*    Cipher API for the User-basedSecurityModel (USM) PRIV(acy) of a user.
*/
typedef struct {
  IP_SNMP_CIPHER_INIT_FUNC*  pfInit;   // Calllback to allocate a fresh cipher algorithm context.
  IP_SNMP_CIPHER_EXEC_FUNC*  pfExec;   // Calllback to decrypt/encrypt data.
  IP_SNMP_CIPHER_FINAL_FUNC* pfFinal;  // Calllback to finalize cipher operations and free resources.
} IP_SNMP_CIPHER_API;

/*********************************************************************
*
*       IP_SNMP_SM_USM_PRIV_PARAMS
*
*  Function description
*    Configuration parameters for the User-basedSecurityModel (USM)
*    data encryption for a user.
*/
typedef struct {
  const IP_SNMP_SM_USM_PRIV_API* pPrivAPI;    // Pointer to the PRIV(acy) specific handling API.
  const IP_SNMP_CIPHER_API*      pCipherAPI;  // Pointer to the cipher API to use.
} IP_SNMP_SM_USM_PRIV_PARAMS;

/*********************************************************************
*
*       IP_SNMP_SM_USM_ENGINE_ENTRY
*
*  Function description
*    Information related to an SNMP Engine.
*
*  Additional information
*    An SNMP(v3) Engine not only has an EngineId but also has some
*    parameters that need to be maintained such as the EngineBoots
*    and EngineTime parameters. Some of these parameters might even
*    be actively learned from peer Engines when receiving REPORT
*    messages.
*
*    If an SNMP Engine entry is used for the local Engine the
*    EngineTime of this entry needs to be periodically updated
*    by the application. The EngineTime should be updated once every
*    second or at least before the previous
*    "EngineTime + IP_SNMP_AGENT_SM_USM_CONFIG.Timeout" expires.
*
*    The SNMPv3 USM "msgAuthoritativeEngineBoots" and "msgAuthoritativeEngineTime"
*    values do not necessarily reflect an actual 1:1 time of the system
*    since booting. Their value is only exchanged between two SNMP
*    entities once AUTH(entication) has succeeded. If the time of
*    an SNMP engine is unknown or outside the time window, the time
*    might need to be retrieved in a separate request. The initiator/client
*    can maintain the discovered values on its own and try to directly
*    send more messages using its own maintained values without having
*    to discover the engine time to use again and again.
*
*    The parameters EngineBoots and EngineTime are meant to be stored in
*    non-volatile memory when the SNMP Agent is shut down and be restored
*    when starting again. This procedure does not even need to increase
*    the EngineBoots necessarily. This only makes sense if SNMP is started
*    again before the timeout expires in which another entity might be
*    sending further mesages.
*
*    The EngineBoots and EngineTime values do not have to be strictly
*    maintained by the application. Their purpose is to prevent
*    replay attacks of messages by limiting the time window in which
*    they can be utilized. It should also be perfectly fine to use
*    randomized start values for these parameters each time as this
*    will typically only lead to having to discover the engine time
*    again with an additional message while also needing to successfully
*    AUTH(enticate) again for these values to be included in the response.
*/
typedef struct {
  const U8* pEngineId;    // Pointer to an EngineId.
        I32 EngineBoots;  // Number of how often the SNMP engine has been
                          // "booted". Ideally this value is increased for each
                          // time the hardware boots or SNMP is started. However,
                          // EngineBoots shall also be incremented once EngineTime
                          // reaches its I32 maximum of 2147483647 seconds, then
                          // resetting EngineTime back to 0 again.
        I32 EngineTime;   // Seconds since the SNMP engine has "booted". Once
                          // the I32 maximum of 2147483647 seconds is reached,
                          // EngineBoots shall be incremented and EngineTime
                          // starts from 0 again.
        U8  EngineIdLen;  // Length of the EngineId.
} IP_SNMP_SM_USM_ENGINE_ENTRY;

/*********************************************************************
*
*       IP_SNMP_AGENT_SM_USM_CONFIG
*
*  Function description
*    Used to configure the User-basedSecurityModel (USM).
*/
typedef struct {
  const IP_SNMP_SM_USM_ENGINE_ENTRY* pLocalEngine;  // Pointer to the local SNMP Engine to use.
        unsigned                     Timeout;       // Timeout in seconds, relative to the EngineTime in which
                                                    // a received message is valid. The default according
                                                    // to RFC 3414 is 150 seconds, which means that messages
                                                    // are in the time window if they use NOW +- Timeout .
} IP_SNMP_AGENT_SM_USM_CONFIG;

/*********************************************************************
*
*       IP_SNMP_SM_USM_USER_TABLE_ENTRY
*
*  Function description
*    The SNMPv3 user access is managed by a table/array of
*    IP_SNMP_SM_USM_USER_TABLE_ENTRY items that allow to set different
*    combinations of "noAuthNoPriv", "authNoPriv" and "authPriv"
*    along with the different hash and encryption algorithms.
*
*  Additional information
*    The user table can be constructed directly by using either
*    the fields in order as they are or by using "Designated Initializers"
*    (initialization with a structs member name in form of
*    ".<StructMember>=<Value>"). Members of this structure might
*    change their order of appearance in the future to allow for
*    memory efficient extension of the structure and user table. For
*    this reason structure members should only be initialized by either
*    using "Designated Initializers" for a ROM/const placement for
*    example or by using the IP_SNMP_SM_USM_USER_* API.
*
*    When extending this structure in the future, structure members are
*    expected to move in patterns that will be easily recognizable as
*    they lead to compile errors. In case of doubt (when directly
*    interacting based on fixed order initialization), the application
*    should check the size of the structure and compare it to a
*    previously known size of this structure and raise an error if the
*    size (and most likely order of members) has changed.
*
*    To support "view-based" permissions, based on the security level
*    achieved ("noAuthNoPriv", "authNoPriv" or "authPriv"), a username
*    can be added multiple times for the same EngineId with different
*    parameters being available/set or not set. The selection is done
*    using a perfect match, which means that no entry with a higher
*    security level is used to handle a lower security level request.
*
*    When creating the user table as array of IP_SNMP_SM_USM_USER_TABLE_ENTRY
*    placeholder entries can be used by settings these entries with
*    "pEngine = NULL". This can be used to allocate contiguous space
*    once and providing space for up to that many entries without
*    having to reallocate the memory when adding or removing entries.
*/
typedef struct {
  const IP_SNMP_SM_USM_ENGINE_ENTRY* pEngine;      // Pointer to the authorizational Engine that this user entry belongs to.
  const U8*                          pUsername;    // Pointer to the Username (without string termination).
  const IP_SNMP_AGENT_PERM*          pPerm;        // Pointer to the permissions table of type IP_SNMP_AGENT_PERM .
  const IP_SNMP_SM_USM_AUTH_PARAMS*  pAuthParams;  // Pointer to an AUTHentication configuration of type IP_SNMP_SM_USM_AUTH_PARAMS .
                                                   // Can be NULL to create a "noAuthNoPriv" type entry.
  const U8*                          pAuthKey;     // Pointer to a calculated AuthKey for the EngineId at pEngineId .
                                                   // The length of the AuthKey is determined by the digest length of pAuthParams .
                                                   // Can be NULL if pAuthParams is NULL .
  const IP_SNMP_SM_USM_PRIV_PARAMS*  pPrivParams;  // Pointer to a PRIVacy configuration of type IP_SNMP_SM_USM_PRIV_PARAMS .
  const U8*                          pPrivKey;     // Pointer to a calculated PrivKey for the EngineId at pEngineId .
                                                   // The length of the PrivKey is determined by the digest length of pAuthParams
                                                   // as the PRIV key is also calculated based on the hash algorithm used for AUTH .
                                                   // Can be NULL if pPrivParams is NULL .
        U8                           UsernameLen;  // Length of the Username at pUsername .
} IP_SNMP_SM_USM_USER_TABLE_ENTRY;

struct IP_SNMP_AGENT_CONTEXT_STRUCT {
        U8*                              pData;             // Data pointer that can be passed through multiple functions. Typically used for open/close Varbind actions.
        void*                            pUserContext;
        IP_SNMP_AGENT_COMMUNITY*         pCommunity;
  const IP_SNMP_SM_USM_USER_TABLE_ENTRY* pUser;
  const IP_SNMP_AGENT_PERM*              pPerm;
        IP_SNMP_AGENT_BUFFER_DESC*       pInBufferDesc;
        void*                            pVarbindFlags;     // Only when processing incoming messages. Contains flags that are set during process of a Varbind
                                                            // for other functions to know a specific state. Should be reset before processing each new Varbind.
  const U8*                              pEnterpriseOID;
  const U8*                              pTrapOID;
        U32                              EnterpriseOIDLen;
        U32                              TrapOIDLen;
        IP_SNMP_AGENT_BUFFER_DESC        OutBufferDesc;
        U32                              AgentAddr;
        U8                               Error;             // SNMP error code.
        U8                               AuthError;         // In case we have not enough permissions for an access we might avoid sending back to prevent brute forcing the community string.
        U8                               SendReport;        // An SNMPv3 report has been prepared.
        U8                               ReportReceived;    // The PRIV(acy) flag has been set in a request. This might mean that we can not read the "request-id".
        U8                               PrivRequested;     // The PRIV(acy) flag has been set in a request. This might mean that we can not read the "request-id".
        U8                               PrivApplied;       // When set, PRIV(acy) has been handled for a received message OR PRIV(acy) needs to be applied to a message to send.
};

typedef struct IP_SNMP_AGENT_TRAP_INFORM_CONTEXT {
  struct IP_SNMP_AGENT_TRAP_INFORM_CONTEXT* pNext;                // Pointer to next TRAP/INFORM context for INFORM wait for ACK list.
         IP_SNMP_AGENT_CONTEXT*             pVarbindListContext;  // Pointer to context holding the custom Varbinds to send.
         IP_SNMP_AGENT_COMMUNITY*           pCommunity;           // Community (string) to use (SNMP1/SNMPv2c).
         IP_SNMP_SM_USM_USER_TABLE_ENTRY*   pUser;                // User handle to use (SNMPv3).
         void*                              pSendContext;         // Send context, typically a socket.
         U8                                 IPAddr[16];           // Enough space to handle IPv4 and IPv6 Manager addresses. Address has to be stored in network order.
         U32                                Timeout;              // INFORM timeout [ms] of each message sent.
         U32                                NextTimeout;          // Next timestamp when to check for a necessary resend.
         U32                                TickCnt;              // The request timestamp (tick count) needs to be reused in case we have a resend of an INFORM.
         I32                                RequestId;            // The request ID needs to be reused in case we have a resend of an INFORM.
         U16                                Port;                 // TRAP/INFORM port in host order. Typically 162.
         U16                                DiscoverPort;         // Engine discover port in host order. Typically 161.
         U8                                 IPAddrLen;            // Length of IP address to determine if it is an IPv4(4 bytes) or IPv6(16 bytes) address.
         U8                                 Type;                 // IP_SNMP_PDU_TYPE_TRAPV1 or IP_SNMP_PDU_TYPE_TRAPV2 or IP_SNMP_PDU_TYPE_INFORMV2.
         U8                                 Retries;              // Number of INFORM/DISCOVER retries to send. Also used for DISCOVER for TRAPv2 .
         U8                                 MPFlags;              // OR-combination of IP_SNMPV3_MSG_FLAG_* flags to use.
         I8                                 Status;               // Only used for v2 INFORM messages.
} IP_SNMP_AGENT_TRAP_INFORM_CONTEXT;

typedef int  (*IP_SNMP_AGENT_pfMIB)             (IP_SNMP_AGENT_CONTEXT* pContext, void* pUserContext, const U8* pMIB, U32 MIBLen, const U8* pIndex, U32 IndexLen, U8 RequestType, U8 VarType);
typedef void (*IP_SNMP_AGENT_pfOnInformResponse)(void* pUserContext, IP_SNMP_AGENT_CONTEXT* pVarbindContext, IP_SNMP_AGENT_TRAP_INFORM_CONTEXT* pTrapInformContext, int Status);

typedef struct IP_SNMP_AGENT_HOOK_ON_INFORM_RESPONSE {
  struct IP_SNMP_AGENT_HOOK_ON_INFORM_RESPONSE* pNext;
         IP_SNMP_AGENT_pfOnInformResponse       pf;  // Callback upon a change of the wait for INFORM ACK element.
} IP_SNMP_AGENT_HOOK_ON_INFORM_RESPONSE;

typedef struct IP_SNMP_AGENT_MIB {
  struct IP_SNMP_AGENT_MIB*  pParent;      // Parent MIB of the current one.
  struct IP_SNMP_AGENT_MIB*  pFirstChild;  // First child MIB of the current one.
  struct IP_SNMP_AGENT_MIB*  pNext;        // Next one with the same parent.
         U32                 Id;           // OID part of the current MIB.
         IP_SNMP_AGENT_pfMIB pf;           // Callback for the current MIB.
         void*               pContext;     // Context specific pointer to store information like an additional API structure for internal purposes.
} IP_SNMP_AGENT_MIB;

//
// System description represented at MIB-II at oid value 1.3.6.1.2.1.1 .
// For more details please refer to http://www.alvestrand.no/objectid/1.3.6.1.2.1.1.html .
//
typedef struct {
  const char* sSysDescr;                                                    // String including full name and version of the target and other information. Up to 255 characters + termination.
  const U8*   pSysObjectID;                                                 // The vendor's authoritative identification of the network management subsystem contained in the entity.
        U32   SysObjectIDLen;                                               // Length of the oid value at pSysObjectID.
  U32 (*pfGetSysUpTime)     (void);                                         // Time in in hundredths of a second since the network management portion of the system was last re-initialized.
  int (*pfGetSetSysContact) (char* pBuffer, U32* pNumBytes, char IsWrite);  // String including information regarding the contact person for this managed node and how to contact this person. Up to 255 characters + termination.
  int (*pfGetSetSysName)    (char* pBuffer, U32* pNumBytes, char IsWrite);  // String including an administratively-assigned name for this managed node e.g. FQDN. Up to 255 characters + termination.
  int (*pfGetSetSysLocation)(char* pBuffer, U32* pNumBytes, char IsWrite);  // String including the physical location of this node. Up to 255 characters + termination.
  U8          SysServices;                                                  // Value representing the services offered.
} IP_SNMP_AGENT_MIB2_SYSTEM_API;

//
// Interfaces description represented at MIB-II at oid value 1.3.6.1.2.1.1 .
// For more details please refer to http://www.alvestrand.no/objectid/1.3.6.1.2.1.1.html .
//
typedef struct {
  int  (*pfGetIfNumber)         (void);
  int  (*pfGetIfIndex)          (unsigned IfEntry);
  void (*pfGetIfDescr)          (unsigned IfEntry, char* pBuffer, U32* pNumBytes);
  int  (*pfGetIfType)           (unsigned IfEntry);
  int  (*pfGetIfMtu)            (unsigned IfEntry);
  U32  (*pfGetIfSpeed)          (unsigned IfEntry);
  void (*pfGetIfPhysAddress)    (unsigned IfEntry, U8* pBuffer, U32* pNumBytes);
  int  (*pfGetIfAdminStatus)    (unsigned IfEntry);
  int  (*pfSetIfAdminStatus)    (unsigned IfEntry, unsigned Status);
  int  (*pfGetIfOperStatus)     (unsigned IfEntry);
  U32  (*pfGetIfLastChange)     (unsigned IfEntry);
  U32  (*pfGetIfInOctets)       (unsigned IfEntry);
  U32  (*pfGetIfInUcastPkts)    (unsigned IfEntry);
  U32  (*pfGetIfInNUcastPkts)   (unsigned IfEntry);
  U32  (*pfGetIfInDiscards)     (unsigned IfEntry);
  U32  (*pfGetIfInErrors)       (unsigned IfEntry);
  U32  (*pfGetIfInUnknownProtos)(unsigned IfEntry);
  U32  (*pfGetIfOutOctets)      (unsigned IfEntry);
  U32  (*pfGetIfOutUcastPkts)   (unsigned IfEntry);
  U32  (*pfGetIfOutNUcastPkts)  (unsigned IfEntry);
  U32  (*pfGetIfOutDiscards)    (unsigned IfEntry);
  U32  (*pfGetIfOutErrors)      (unsigned IfEntry);
  U32  (*pfGetIfOutQLen)        (unsigned IfEntry);
  void (*pfGetIfSpecific)       (unsigned IfEntry, U8* pBuffer, U32* pNumBytes);
} IP_SNMP_AGENT_MIB2_INTERFACES_API;

/*********************************************************************
*
*       IP_SNMP_USM_ENGINE_INFO
*
*  Function description
*    Provides information about a peer SNMPv3 Engine that can be used
*    to maintain the list of Engines and their parameters.
*/
typedef struct {
  IP_SNMP_SM_USM_ENGINE_ENTRY Engine;  // Pointer to Engine information of type IP_SNMP_SM_USM_ENGINE_ENTRY .
} IP_SNMP_USM_ENGINE_INFO;

/*********************************************************************
*
*       IP_SNMP_AGENT_MPV3_CONFIG
*
*  Function description
*    Used to configure the MessageProcessor (MP) for SNMPv3.
*/
typedef struct {
  I32 MaxSize;  // Value to use for the "msgMaxSize" field in SNMPv3 messages.
                // This field describes the maxmimum size of a "ScopedPDU" that
                // can be received without the SNMP headers for various layers
                // preceeding it.
                // This value can not be simply calculated as the header size
                // might differ due to fields of variable length such as the
                // "msgAuthoritativeEngineID" field.
                // The typical Ethernet limit for IPv4 UDP payload is around
                // 1472 bytes for the "ScopedPDU" plus SNMP headers.
                // A "good" value used by other implementations is 1400 bytes.
} IP_SNMP_AGENT_MPV3_CONFIG;

/*********************************************************************
*
*       IP_SNMP_AGENT_ON_INFORM_REPORT_FUNC
*
*  Function description
*    Callback executed whenever a REPORT with new information about
*    an SNMPv3 Engine is received for a pending INFORM.
*
*  Parameters
*    pContext      : Pointer to an SNMP Agent context.
*    pInformContext: Pointer to the INFORM context of type
*                    IP_SNMP_AGENT_TRAP_INFORM_CONTEXT for which new
*                    Engine information have been discovered.
*    pUserContext  : User specific context passed to the process
*                    message API.
*    pInfo         : Pointer to information received about an Engine.
*
*  Return value
*    == 0: Retry to send the INFORMs when returning from the callback.
*    <  0: Do not access pInformContext when returning from the callback (removed?).
*
*  Additional information
*    This callback gets executed when a REPORT message with (new) SNMPv3
*    Engine information for a yet to be sent INFORM is received.
*    This is typically the case when sending an INFORM while only
*    knowing the IP address of the receiving Manager but not the EngineId.
*    Another case where a REPORT is received is when the peer Engine
*    time window was missed. In this case the REPORT lets us know the
*    current time of the peer Engine and we have to send the INFORM
*    again after updating our information about the peer Engine time
*    which prevents replay attacks with old messages if the
*    AUTH(thorization) security level is used.
*
*    Once new Engine information is received the Engine table
*    maintained by the application should be updated and the INFORM
*    should either be resent immediately from within this callback
*    or is resent by the retry mechanism for INFORM messages.
*
*    Once initial EngineBoots and EngineTime values have been
*    discovered for an Engine they can be maintained locally by
*    the application to prevent the discover part being necessary
*    if the authoritative Engine is happy with what we send. In
*    the worst case we will receive a REPORT by the peer Engine
*    telling us the latest information.
*/
typedef int (IP_SNMP_AGENT_ON_INFORM_REPORT_FUNC)(IP_SNMP_AGENT_CONTEXT* pContext, IP_SNMP_AGENT_TRAP_INFORM_CONTEXT* pInformContext, void* pUserContext, IP_SNMP_USM_ENGINE_INFO* pInfo);

/*********************************************************************
*
*       Helper macros
*
**********************************************************************
*/

/*********************************************************************
*
*       IP_SNMP_AGENT_TRAP_INFORM_SetIPv4AddrPort()
*
*  Function description
*    Helper function that sets an IPv4 address in an
*    IP_SNMP_AGENT_TRAP_INFORM_CONTEXT context.
*
*  Parameters
*    pContext    : Pointer to TRAP/INFORM context of type IP_SNMP_AGENT_TRAP_INFORM_CONTEXT .
*    IPAddr      : IPv4 address where to send the TRAP/INFORM message in host order.
*    Port        : UDP port to send to in host order. Typically 162.
*    DiscoverPort: UDP port to use for SNMPv3 Engine discovery in host order.
*                  Typically 161. Can be 0 if not using Engine discovery.
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_AGENT_TRAP_INFORM_CONTEXT
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*/
#define IP_SNMP_AGENT_TRAP_INFORM_SetIPv4AddrPort(pContext_, IPAddr_, Port_, DiscoverPort_)  {  \
  SEGGER_WrU32BE(&(pContext_)->IPAddr[0], IPAddr_);                                             \
  (pContext_)->IPAddrLen    = 4u;                                                               \
  (pContext_)->Port         = (Port_);                                                          \
  (pContext_)->DiscoverPort = (DiscoverPort_);                                                  \
}

/*********************************************************************
*
*       IP_SNMP_AGENT_TRAP_INFORM_SetIPv6AddrPort()
*
*  Function description
*    Helper function that sets an IPv6 address in an
*    IP_SNMP_AGENT_TRAP_INFORM_CONTEXT context.
*
*  Parameters
*    pContext    : Pointer to TRAP/INFORM context of type IP_SNMP_AGENT_TRAP_INFORM_CONTEXT .
*    pIPAddr     : Pointer to the IPv6 address where to send the TRAP/INFORM message.
*    Port        : UDP port to send to in host order. Typically 162.
*    DiscoverPort: UDP port to use for SNMPv3 Engine discovery in host order.
*                  Typically 161. Can be 0 if not using Engine discovery.
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_AGENT_TRAP_INFORM_CONTEXT
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*/
#define IP_SNMP_AGENT_TRAP_INFORM_SetIPv6AddrPort(pContext_, pIPAddr_, Port_, DiscoverPort_)  {  \
  IP_SNMP_AGENT_MEMCPY(&(pContext_)->IPAddr[0], pIPAddr_, 16u);                              \
  (pContext_)->IPAddrLen    = 16u;                                                           \
  (pContext_)->Port         = (Port_);                                                       \
  (pContext_)->DiscoverPort = (DiscoverPort_);                                               \
}

/*********************************************************************
*
*       IP_SNMP_AGENT_TRAP_INFORM_SetType()
*
*  Function description
*    Helper function that sets the Type structure member in an
*    IP_SNMP_AGENT_TRAP_INFORM_CONTEXT context.
*
*  Parameters
*    pContext: Pointer to TRAP/INFORM context of type IP_SNMP_AGENT_TRAP_INFORM_CONTEXT .
*    Type    : TRAP/INFORM message type to send:
*                * IP_SNMP_PDU_TYPE_TRAPV1  : Send an SNMPv1 TRAP.
*                * IP_SNMP_PDU_TYPE_TRAPV2  : Send an SNMPv2c TRAP   (also SNMPv3).
*                * IP_SNMP_PDU_TYPE_INFORMV2: Send an SNMPv2c INFORM (also SNMPv3).
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_AGENT_TRAP_INFORM_CONTEXT
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*
*    SNMPv3 TRAP/INFORM messages use the same PDUs as SNMPv2c
*    TRAP/INFORM messages. To send SNMPv2 TRAP/INFORM messages a
*    community needs to be set using IP_SNMP_AGENT_TRAP_INFORM_SetCommunity() .
*    If no community is set SNMPv3 TRAP/INFORM messages are sent.
*/
#define IP_SNMP_AGENT_TRAP_INFORM_SetType(pContext_, Type_)  {  \
  (pContext_)->Type = (Type_);                                  \
}

/*********************************************************************
*
*       IP_SNMP_AGENT_TRAP_INFORM_SetCommunity()
*
*  Function description
*    Helper function that sets the Community structure member in an
*    IP_SNMP_AGENT_TRAP_INFORM_CONTEXT context.
*
*  Parameters
*    pContext  : Pointer to TRAP/INFORM context of type IP_SNMP_AGENT_TRAP_INFORM_CONTEXT .
*    pCommunity: Pointer to community handle of type IP_SNMP_AGENT_COMMUNITY .
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_AGENT_TRAP_INFORM_CONTEXT
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*
*    SNMPv3 TRAP/INFORM messages use the same PDUs as SNMPv1/SNMPv2c
*    TRAP/INFORM messages. To send SNMPv1/SNMPv2 TRAP/INFORM messages
*    a community needs to be set.
*    If no community is set this generates an SNMPv3 TRAP/INFORM
*    messages instead.
*/
#define IP_SNMP_AGENT_TRAP_INFORM_SetCommunity(pContext_, pCommunity_)  {  \
  (pContext_)->pCommunity = (pCommunity_);                                 \
}

/*********************************************************************
*
*       IP_SNMP_AGENT_TRAP_INFORM_SetUser()
*
*  Function description
*    Helper function that sets the User structure member in an
*    IP_SNMP_AGENT_TRAP_INFORM_CONTEXT context.
*
*  Parameters
*    pContext: Pointer to TRAP/INFORM context of type IP_SNMP_AGENT_TRAP_INFORM_CONTEXT .
*    pUser   : Pointer to User handle of type IP_SNMP_SM_USM_USER_TABLE_ENTRY .
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_AGENT_TRAP_INFORM_CONTEXT
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*
*    SNMPv3 TRAP/INFORM messages use the same PDUs as SNMPv1/SNMPv2c
*    TRAP/INFORM messages. To send SNMPv1/SNMPv2 TRAP/INFORM messages
*    a community needs to be set.
*    If no community is set this generates an SNMPv3 TRAP/INFORM
*    messages instead.
*/
#define IP_SNMP_AGENT_TRAP_INFORM_SetUser(pContext_, pUser_)  {  \
  (pContext_)->pUser = (pUser_);                                 \
}

/*********************************************************************
*
*       IP_SNMP_AGENT_TRAP_INFORM_SetTimeoutRetries()
*
*  Function description
*    Helper function that sets the Timeout and Retries structure
*    members in an IP_SNMP_AGENT_TRAP_INFORM_CONTEXT context.
*
*  Parameters
*    pContext: Pointer to TRAP/INFORM context of type IP_SNMP_AGENT_TRAP_INFORM_CONTEXT .
*    Timeout : INFORM timeout [ms] of each message sent.
*    Retries : Number of INFORM retries to send.
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_AGENT_TRAP_INFORM_CONTEXT
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*
*    The Timeout and Retries parameters are typically only used when
*    sending INFORM messages. When the peer EngineId shall be
*    discovered the Retries value is the combined number of retries
*    for discovering the peer EngineId and receiving a response for
*    the INFORM message.
*/
#define IP_SNMP_AGENT_TRAP_INFORM_SetTimeoutRetries(pContext_, Timeout_, Retries_)  {  \
  (pContext_)->Timeout = (Timeout_);                                                   \
  (pContext_)->Retries = (Retries_);                                                   \
}

/*********************************************************************
*
*       IP_SNMP_AGENT_TRAP_INFORM_SetMPFlags()
*
*  Function description
*    Helper function that sets the Message Processor (MP) flags to use
*    in an IP_SNMP_AGENT_TRAP_INFORM_CONTEXT context.
*
*  Parameters
*    pContext: Pointer to TRAP/INFORM context of type IP_SNMP_AGENT_TRAP_INFORM_CONTEXT .
*    MPFlags : OR-combination of IP_SNMPV3_MSG_FLAG_* to use. Valid flags are:
*                * IP_SNMPV3_MSG_FLAG_AUTH_MASK: If the user has AUTH(entication) parameters and shall use them.
*                * IP_SNMPV3_MSG_FLAG_PRIV_MASK: If the user has PRIV(acy) parameters and shall use them (automatically sets IP_SNMPV3_MSG_FLAG_AUTH_MASK as well).
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_AGENT_TRAP_INFORM_CONTEXT
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*
*    The Timeout and Retries parameters are only required for
*    sending INFORM messages.
*/
#define IP_SNMP_AGENT_TRAP_INFORM_SetMPFlags(pContext_, MPFlags_)  {  \
  (pContext_)->MPFlags = (MPFlags_);                                  \
}

/*********************************************************************
*
*       IP_SNMP_SM_USM_USER_SetEngine()
*
*  Function description
*    Helper function that sets the Engine structure member in an
*    IP_SNMP_SM_USM_USER_TABLE_ENTRY entry.
*
*  Parameters
*    pEntry : Pointer to user table entry of type IP_SNMP_SM_USM_USER_TABLE_ENTRY .
*    pEngine: Pointer to the Engine to use for this user of type IP_SNMP_SM_USM_ENGINE_ENTRY .
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_SM_USM_USER_TABLE_ENTRY
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*/
#define IP_SNMP_SM_USM_USER_SetEngine(pEntry_, pEngine_)  {  \
  (pEntry_)->pEngine = (pEngine_);                           \
}

/*********************************************************************
*
*       IP_SNMP_SM_USM_USER_SetUsername()
*
*  Function description
*    Helper function that sets the Username structure member in an
*    IP_SNMP_SM_USM_USER_TABLE_ENTRY entry.
*
*  Parameters
*    pEntry     : Pointer to user table entry of type IP_SNMP_SM_USM_USER_TABLE_ENTRY .
*    pUsername  : Pointer to the Username to set (without string termination).
*    UsernameLen: Length of the Username at pUsername .
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_SM_USM_USER_TABLE_ENTRY
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*/
#define IP_SNMP_SM_USM_USER_SetUsername(pEntry_, pUsername_, UsernameLen_)  {  \
  (pEntry_)->pUsername   = (pUsername_);                                       \
  (pEntry_)->UsernameLen = (UsernameLen_);                                     \
}

/*********************************************************************
*
*       IP_SNMP_SM_USM_USER_SetPerm()
*
*  Function description
*    Helper function that sets the permission structure member in an
*    IP_SNMP_SM_USM_USER_TABLE_ENTRY entry.
*
*  Parameters
*    pEntry: Pointer to user table entry of type IP_SNMP_SM_USM_USER_TABLE_ENTRY .
*    pPerm : Pointer to the NULL entry terminated permissions table to set.
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_SM_USM_USER_TABLE_ENTRY
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*/
#define IP_SNMP_SM_USM_USER_SetPerm(pEntry_, pPerm_)  {  \
  (pEntry_)->pPerm = (pPerm_);                           \
}

/*********************************************************************
*
*       IP_SNMP_SM_USM_USER_SetAuthParamsAndKey()
*
*  Function description
*    Helper function that sets the AUTH(entication) parameters used
*    for AUTH(entication) handling and the calculated AuthKey
*    structure member in an IP_SNMP_SM_USM_USER_TABLE_ENTRY entry.
*
*  Parameters
*    pEntry     : Pointer to user table entry of type IP_SNMP_SM_USM_USER_TABLE_ENTRY .
*    pAuthParams: Pointer to a configuration of type IP_SNMP_SM_USM_AUTH_PARAMS .
*    pAuthKey   : Pointer to the calculated AuthKey.
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_SM_USM_USER_TABLE_ENTRY
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*
*    A call to this function can be omitted if the entry created shall
*    be of type "noAuthNoPriv".
*/
#define IP_SNMP_SM_USM_USER_SetAuthParamsAndKey(pEntry_, pAuthParams_, pAuthKey_)  {  \
  (pEntry_)->pAuthParams = (pAuthParams_);                                            \
  (pEntry_)->pAuthKey    = (pAuthKey_);                                               \
}

/*********************************************************************
*
*       IP_SNMP_SM_USM_USER_SetPrivParamsAndKey()
*
*  Function description
*    Helper function that sets the PRIV(acy) parameters used for
*    PRIV(acy) handling and the calculated PrivKey structure
*    member in an IP_SNMP_SM_USM_USER_TABLE_ENTRY entry.
*
*  Parameters
*    pEntry     : Pointer to user table entry of type IP_SNMP_SM_USM_USER_TABLE_ENTRY .
*    pPrivParams: Pointer to a configuration of type IP_SNMP_SM_USM_PRIV_PARAMS .
*    pPrivKey   : Pointer to the calculated PrivKey.
*
*  Additional information
*    The purpose of this helper function is to provide a persistent
*    API while allowing the members of the IP_SNMP_SM_USM_USER_TABLE_ENTRY
*    structure to be moved around for best memory efficiency when
*    extending the structure in the future.
*
*    A call to this function can be omitted if the entry created shall
*    be of type "noAuthNoPriv".
*/
#define IP_SNMP_SM_USM_USER_SetPrivParamsAndKey(pEntry_, pPrivParams_, pPrivKey_)  {  \
  (pEntry_)->pPrivParams = (pPrivParams_);                                            \
  (pEntry_)->pPrivKey    = (pPrivKey_);                                               \
}

/*********************************************************************
*
*       API functions, internal
*
*  Internal API that is made public so glue layers like the OS layer
*  can be shipped in source without the need for internal header
*  files when using object shipments.
*
**********************************************************************
*/

const IP_SNMP_AGENT_API* IP_SNMP_AGENT_GetAPI(void);

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

#define IP_SNMP_AGENT_ProcessRequest        IP_SNMP_AGENT_ProcessMessage
#define IP_SNMP_AGENT_AddInformReponseHook  IP_SNMP_AGENT_AddInformResponseHook

void IP_SNMP_AGENT_AddCommunity           (IP_SNMP_AGENT_COMMUNITY* pCommunity, const char* sCommunity, U32 Len);
int  IP_SNMP_AGENT_AddMIB                 (const U8* pParentOID, U32 Len, IP_SNMP_AGENT_MIB* pMIB, IP_SNMP_AGENT_pfMIB pf, U32 Id);
void IP_SNMP_AGENT_AddInformResponseHook  (IP_SNMP_AGENT_HOOK_ON_INFORM_RESPONSE* pHook, IP_SNMP_AGENT_pfOnInformResponse pf);
void IP_SNMP_AGENT_CancelInform           (IP_SNMP_AGENT_TRAP_INFORM_CONTEXT* pTrapInformContext);
int  IP_SNMP_AGENT_CheckInformStatus      (IP_SNMP_AGENT_TRAP_INFORM_CONTEXT* pContext);
void IP_SNMP_AGENT_DeInit                 (void);
U32  IP_SNMP_AGENT_Exec                   (void);
int  IP_SNMP_AGENT_GetMessageType         (U8* pIn, U32 NumBytesIn, U8* pType);
void IP_SNMP_AGENT_Init                   (const IP_SNMP_AGENT_API* pAPI);
void IP_SNMP_AGENT_PrepareTrapInform      (IP_SNMP_AGENT_CONTEXT* pContext, void* pUserContext, const U8* pEnterpriseOID, U32 EnterpriseOIDLen, const U8* pTrapOID, U32 TrapOIDLen, U8* pBuffer, U32 BufferSize, U32 AgentAddr);
int  IP_SNMP_AGENT_ProcessInformResponse  (U8* pIn, U32 NumBytesIn);
int  IP_SNMP_AGENT_ProcessMessage         (U8* pIn, U32 NumBytesIn, U8* pOut, U32 NumBytesOut, void* pUserContext);
int  IP_SNMP_AGENT_SendTrapInform         (void* pContext, IP_SNMP_AGENT_CONTEXT* pVarbindContext, IP_SNMP_AGENT_TRAP_INFORM_CONTEXT* pTrapInformContext);
void IP_SNMP_AGENT_SetCommunityPerm       (IP_SNMP_AGENT_COMMUNITY* pCommunity, const IP_SNMP_AGENT_PERM* pPerm);
void IP_SNMP_AGENT_SetInformReportCallback(IP_SNMP_AGENT_ON_INFORM_REPORT_FUNC* pf);

//
// SNMPv3 specific functions.
//
void IP_SNMP_AGENT_MPV3_Add           (const IP_SNMP_AGENT_MPV3_CONFIG*   pConfig);
void IP_SNMP_AGENT_SM_USM_Add         (const IP_SNMP_AGENT_SM_USM_CONFIG* pConfig);
int  IP_SNMP_AGENT_SM_USM_CalcKey     (const IP_SNMP_SM_USM_AUTH_PARAMS* pAuthParams, U8* pBuffer, unsigned BufferSize, const U8* pEngineId, unsigned EngineIdLen, const U8* pPass, unsigned PassLen);
int  IP_SNMP_AGENT_SM_USM_SetUserTable(const IP_SNMP_SM_USM_USER_TABLE_ENTRY* pUserTable, U8 NumUsers);

//
// Functions to hook in standard MIBs into the tree.
//
int IP_SNMP_AGENT_AddMIB_IsoOrgDodInternetPrivateEnterprise (void);
int IP_SNMP_AGENT_AddMIB_IsoOrgDodInternetIetfMib2Interfaces(const IP_SNMP_AGENT_MIB2_INTERFACES_API* pAPI);
int IP_SNMP_AGENT_AddMIB_IsoOrgDodInternetIetfMib2System    (const IP_SNMP_AGENT_MIB2_SYSTEM_API* pAPI);

//
// MIB-II Interface API layer for emNet.
//
#define IP_SNMP_AGENT_MIB2_INTERFACES_embOSIP  IP_SNMP_AGENT_MIB2_INTERFACES_emNet
extern const IP_SNMP_AGENT_MIB2_INTERFACES_API IP_SNMP_AGENT_MIB2_INTERFACES_emNet;

//
// Message construct functions.
//
int IP_SNMP_AGENT_CloseVarbind              (IP_SNMP_AGENT_CONTEXT* pContext);
int IP_SNMP_AGENT_OpenVarbind               (IP_SNMP_AGENT_CONTEXT* pContext);
int IP_SNMP_AGENT_StoreCounter32            (IP_SNMP_AGENT_CONTEXT* pContext, U32 v);
int IP_SNMP_AGENT_StoreCurrentMibOidAndIndex(IP_SNMP_AGENT_CONTEXT* pDstContext, IP_SNMP_AGENT_CONTEXT* pSrcContext, U32 NumIndexes, ...);
int IP_SNMP_AGENT_StoreInstanceNA           (IP_SNMP_AGENT_CONTEXT* pContext);
int IP_SNMP_AGENT_StoreInteger              (IP_SNMP_AGENT_CONTEXT* pContext, I32 v);
int IP_SNMP_AGENT_StoreIpAddress            (IP_SNMP_AGENT_CONTEXT* pContext, U32 IpAddress);
int IP_SNMP_AGENT_StoreOctetString          (IP_SNMP_AGENT_CONTEXT* pContext, const U8* pData, U32 NumBytes);
int IP_SNMP_AGENT_StoreOID                  (IP_SNMP_AGENT_CONTEXT* pContext, const U8* pOIDBytes, U32 OIDLen, U32 MIBLen, U8 IsValue);
int IP_SNMP_AGENT_StoreOpaque               (IP_SNMP_AGENT_CONTEXT* pContext, const U8* pData, U32 NumBytes);
int IP_SNMP_AGENT_StoreTimeTicks            (IP_SNMP_AGENT_CONTEXT* pContext, U32 v);
int IP_SNMP_AGENT_StoreUnsigned32           (IP_SNMP_AGENT_CONTEXT* pContext, U32 v);

//
// Message parsing functions.
//
int IP_SNMP_AGENT_ParseCounter32  (IP_SNMP_AGENT_CONTEXT* pContext, U32* pCounter32);
int IP_SNMP_AGENT_ParseInteger    (IP_SNMP_AGENT_CONTEXT* pContext, I32* pInteger);
int IP_SNMP_AGENT_ParseIpAddress  (IP_SNMP_AGENT_CONTEXT* pContext, U32* pIpAddress);
int IP_SNMP_AGENT_ParseOctetString(IP_SNMP_AGENT_CONTEXT* pContext, const U8** ppData, U32* pLen);
int IP_SNMP_AGENT_ParseOID        (IP_SNMP_AGENT_CONTEXT* pContext, const U8** ppData, U32* pLen);
int IP_SNMP_AGENT_ParseOpaque     (IP_SNMP_AGENT_CONTEXT* pContext, const U8** ppData, U32* pLen);
int IP_SNMP_AGENT_ParseTimeTicks  (IP_SNMP_AGENT_CONTEXT* pContext, U32* pTimeTicks);
int IP_SNMP_AGENT_ParseUnsigned32 (IP_SNMP_AGENT_CONTEXT* pContext, U32* pUnsigned32);

//
// Opaque message construct/parsing functions. These might not be
// supported by every SNMP solution and are typically based on
// drafts that have been agreed by several people to use it in
// this way. However they are not officially supported and might
// be reported simply as Opaque fields with an octet string in there.
//
#if IP_SNMP_AGENT_SUPPORT_64_BIT_TYPES
int IP_SNMP_AGENT_StoreCounter64 (IP_SNMP_AGENT_CONTEXT* pContext, U64 v);
int IP_SNMP_AGENT_StoreDouble    (IP_SNMP_AGENT_CONTEXT* pContext, double v);
int IP_SNMP_AGENT_StoreInteger64 (IP_SNMP_AGENT_CONTEXT* pContext, I64 v);
int IP_SNMP_AGENT_StoreUnsigned64(IP_SNMP_AGENT_CONTEXT* pContext, U64 v);

int IP_SNMP_AGENT_ParseCounter64 (IP_SNMP_AGENT_CONTEXT* pContext, U64* pCounter64);
int IP_SNMP_AGENT_ParseDouble    (IP_SNMP_AGENT_CONTEXT* pContext, double* pDouble);
int IP_SNMP_AGENT_ParseInteger64 (IP_SNMP_AGENT_CONTEXT* pContext, I64* pInteger64);
int IP_SNMP_AGENT_ParseUnsigned64(IP_SNMP_AGENT_CONTEXT* pContext, U64* pUnsigned64);
#endif

int IP_SNMP_AGENT_StoreFloat     (IP_SNMP_AGENT_CONTEXT* pContext, float v);
int IP_SNMP_AGENT_ParseFloat     (IP_SNMP_AGENT_CONTEXT* pContext, float* pFloat);

//
// Function macros for implementing types that are already handled by other real functions.
// The SNMP standard defines several types that use the same ID tag. Therefore an application
// could as well use one of the real functions that use the same type instead of the macro.
// This works as even for an Unsigned32 type only the bytes are used that are required to
// handle the information to transport.
// Examples:
//   Unsigned32 with value         1 (0x01)       requires 1 byte to store the value.
//   Unsigned32 with value      1000 (0x03E8)     requires 2 byte to store the value.
//   Unsigned32 with value    100000 (0x0186A0)   requires 3 byte to store the value.
//   Unsigned32 with value 100000000 (0x05F5E100) requires 4 byte to store the value.
//
#define IP_SNMP_AGENT_StoreBits       IP_SNMP_AGENT_StoreOctetString
#define IP_SNMP_AGENT_StoreCounter    IP_SNMP_AGENT_StoreCounter32
#define IP_SNMP_AGENT_StoreGauge      IP_SNMP_AGENT_StoreUnsigned32
#define IP_SNMP_AGENT_StoreGauge32    IP_SNMP_AGENT_StoreUnsigned32
#define IP_SNMP_AGENT_StoreInteger32  IP_SNMP_AGENT_StoreInteger

#define IP_SNMP_AGENT_ParseBits       IP_SNMP_AGENT_ParseOctetString
#define IP_SNMP_AGENT_ParseCounter    IP_SNMP_AGENT_ParseCounter32
#define IP_SNMP_AGENT_ParseGauge      IP_SNMP_AGENT_ParseUnsigned32
#define IP_SNMP_AGENT_ParseGauge32    IP_SNMP_AGENT_ParseUnsigned32
#define IP_SNMP_AGENT_ParseInteger32  IP_SNMP_AGENT_ParseInteger

//
// Helper functions.
//
int IP_SNMP_AGENT_DecodeOIDValue(const U8* pOID, U32* pLen, U32* pValue, const U8** ppNext);
int IP_SNMP_AGENT_EncodeOIDValue(U32 Value, U8* pBuffer, U32 BufferSize, U8** ppNext, U8* pNumEncodedBytes);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
