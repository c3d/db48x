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
File        : IP.h
Purpose     : API of the TCP/IP stack
---------------------------END-OF-HEADER------------------------------
*/

#ifndef IP_H_
#define IP_H_

#ifdef __ICCARM__  // IAR
  #pragma diag_suppress=Pa029  // No warning for unknown pragmas in earlier verions of EWARM
  #pragma diag_suppress=Pa137  // No warning for C-Style-Casts with C++
#endif

#include "SEGGER.h"            // Some segger-specific, global defines.
#include "SEGGER_UTIL.h"       // Utility functions related to multi-byte load and store.
#include "IP_ConfDefaults.h"
#include "IP_Socket.h"
#include "BSP_IP.h"            // IP_BSP_ types have been outsourced to elimate BSP_IP.c requiring IP.h .

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#define IP_VERSION  34210u  // Format: Mmmrr. Example: 34210 is 3.42.10

/*********************************************************************
*
*       IP_MTYPE
*
*  Ids to distinguish different message types
*/
#define IP_MTYPE_INIT         (1UL << 0)
#define IP_MTYPE_CORE         (1UL << 1)
#define IP_MTYPE_ALLOC        (1UL << 2)
#define IP_MTYPE_DRIVER       (1UL << 3)
#define IP_MTYPE_ARP          (1UL << 4)
#define IP_MTYPE_IP           (1UL << 5)
#define IP_MTYPE_TCP_CLOSE    (1UL << 6)
#define IP_MTYPE_TCP_OPEN     (1UL << 7)
#define IP_MTYPE_TCP_IN       (1UL << 8)
#define IP_MTYPE_TCP_OUT      (1UL << 9)
#define IP_MTYPE_TCP_RTT      (1UL << 10)
#define IP_MTYPE_TCP_RXWIN    (1UL << 11)
#define IP_MTYPE_TCP          (IP_MTYPE_TCP_OPEN | IP_MTYPE_TCP_CLOSE | IP_MTYPE_TCP_IN | IP_MTYPE_TCP_OUT | IP_MTYPE_TCP_RTT)
#define IP_MTYPE_UDP_IN       (1UL << 12)
#define IP_MTYPE_UDP_OUT      (1UL << 13)
#define IP_MTYPE_UDP          (IP_MTYPE_UDP_IN | IP_MTYPE_UDP_OUT)
#define IP_MTYPE_LINK_CHANGE  (1UL << 14)
#define IP_MTYPE_AUTOIP       (1UL << 15)
#define IP_MTYPE_DHCP         (1UL << 17)
#define IP_MTYPE_DHCP_EXT     (1UL << 18)
#define IP_MTYPE_APPLICATION  (1UL << 19)
#define IP_MTYPE_ICMP         (1UL << 20)
#define IP_MTYPE_NET_IN       (1UL << 21)
#define IP_MTYPE_NET_OUT      (1UL << 22)
#define IP_MTYPE_NET          (IP_MTYPE_NET_IN | IP_MTYPE_NET_OUT)
#define IP_MTYPE_DNS          (1UL << 24)
#define IP_MTYPE_PPP          (1UL << 25)
#define IP_MTYPE_SOCKET_STATE (1UL << 26)
#define IP_MTYPE_SOCKET_READ  (1UL << 27)
#define IP_MTYPE_SOCKET_WRITE (1UL << 28)
#define IP_MTYPE_SOCKET       (IP_MTYPE_SOCKET_STATE | IP_MTYPE_SOCKET_READ | IP_MTYPE_SOCKET_WRITE)
#define IP_MTYPE_DNSC         (1UL << 29)
#define IP_MTYPE_ACD          (1UL << 30)

#define IP_MTYPE_IPV6         (1UL << 15)  // Temporary message type. Reuse IP_MTYPE_AUTOIP, since it is normally not used.

/*********************************************************************
*
*       IP_ERR_
*
* Error codes.
* In general, these are negative numbers.
*/

//
// Positive values are used for conditions where the result depends on
// a reply which is not checked in the function which returns the value.
//
#define     IP_ERR_SEND_PENDING     1

#define     IP_ERR_MISC            (-1)    // Any error, unspecified
#define     IP_ERR_TIMEDOUT        (-2)
#define     IP_ERR_ISCONN          (-3)
#define     IP_ERR_OP_NOT_SUPP     (-4)
#define     IP_ERR_CONN_ABORTED    (-5)
#define     IP_ERR_WOULD_BLOCK     (-6)
#define     IP_ERR_CONN_REFUSED    (-7)
#define     IP_ERR_CONN_RESET      (-8)
#define     IP_ERR_NOT_CONN        (-9)
#define     IP_ERR_ALREADY         (-10)
#define     IP_ERR_IN_VAL          (-11)
#define     IP_ERR_MSG_SIZE        (-12)
#define     IP_ERR_PIPE            (-13)
#define     IP_ERR_DEST_ADDR_REQ   (-14)
#define     IP_ERR_SHUTDOWN        (-15)
#define     IP_ERR_NO_PROTO_OPT    (-16)
#define     IP_ERR_NO_MEM          (-18)
#define     IP_ERR_ADDR_NOT_AVAIL  (-19)
#define     IP_ERR_ADDR_IN_USE     (-20)
#define     IP_ERR_IN_PROGRESS     (-22)
#define     IP_ERR_NO_BUF          (-23)
#define     IP_ERR_NOT_SOCK        (-24)
#define     IP_ERR_FAULT           (-25)
#define     IP_ERR_NET_UNREACH     (-26)
#define     IP_ERR_PARAM           (-27)
#define     IP_ERR_LOGIC           (-28)
#define     IP_ERR_NOMEM           (-29)   // System error
#define     IP_ERR_NOBUFFER        (-30)   // System error
#define     IP_ERR_RESOURCE        (-31)   // System error
#define     IP_ERR_BAD_STATE       (-32)   // System error
#define     IP_ERR_TIMEOUT         (-33)   // System error
#define     IP_ERR_NO_ROUTE        (-36)   // Net error
#define     IP_ERR_QUEUE_FULL      (-37)   // Typically returned when waiting for an ARP reply and no more packets can be queued for sending once the reply comes in.

#define     IP_ERR_TRIAL_LIMIT     (-128)  // Trial library limit reached

const char * IP_Err2Str(int x);

/*********************************************************************
*
*  Convert little/big endian - these should be efficient,
*  inline code or MACROs
*/
#if IP_IS_BIG_ENDIAN
  #define htonl(l) (l)
  #define htons(s) (s)
  #define IP_HTONL_FAST(l) (l)
#else
  #define htonl(l) IP_SwapU32(l)
  #define htons(s) ((U16)((U16)(s) >> 8) | (U16)((U16)(/*lint --e(778) */(s) & 0xFFu) << 8))  // Amazingly, some compilers really need all these U16 casts and even the masking is required to avoid a warning.
  #define IP_HTONL_FAST(v) (             \
      (((U32)((v) << 0)  >> 24) << 0)  | \
      (((U32)((v) << 8)  >> 24) << 8)  | \
      (((U32)((v) << 16) >> 24) << 16) | \
      ((v) << 24)                        \
      )
#endif

#define ntohl(l) htonl(l)
#define ntohs(s) htons(s)

#define IPV6_ADDR_LEN                (16)
#define IPV4_ADDR_LEN                 (4)


/*********************************************************************
*
*  Helper MACROs
*/
#define IP_BYTES2ADDR(a, b, c, d)  (  \
          (((U32)((U8)(a))) << 24) |  \
          (((U32)((U8)(b))) << 16) |  \
          (((U32)((U8)(c))) <<  8) |  \
           ((U32)((U8)(d)))           \
        )

/*********************************************************************
*
*  Typedef forward declarations
*/
typedef struct IFACE_STRUCT IFACE;

/*********************************************************************
*
*       IP_DLIST
*
*  Some very old compilers like the NEC 78K0R seem to have a problem
*  with using structs from other files if these are included after
*  using a pointer to the structure. The typical two-step declaration
*  and using it as a pointer does not work if the struct itself comes
*  from another file. As the struct IP_DLIST_ITEM is simple enough
*  and does not include any internals we keep it here. This allows
*  us to keep the structures IP_RAW_CONNECTION and IP_UDP_CONNECTION
*  in this file as well which is the real reason for the compiler
*  problem.
*/
typedef struct IP_DLIST_ITEM_STRUCT IP_DLIST_ITEM;
struct IP_DLIST_ITEM_STRUCT {
  IP_DLIST_ITEM* pNext;
  IP_DLIST_ITEM* pPrev;
};

typedef struct IP_SLIST_ITEM_STRUCT IP_SLIST_ITEM;
struct IP_SLIST_ITEM_STRUCT {
  IP_SLIST_ITEM* pNext;
};

#if IP_ALLOW_DEINIT
/*********************************************************************
*
*       IP_ON_EXIT_CB
*
*  Typically only used by the stack internally. Made public only for
*  usage in IP_ConfigIO.c to be able to use embOWView via UDP for
*  log messages.
*/
typedef void (IP_ON_EXIT_FUNC)(void);

typedef struct IP_ON_EXIT_CB_STRUCT IP_ON_EXIT_CB;
struct IP_ON_EXIT_CB_STRUCT {
  IP_ON_EXIT_CB*   pNext;
  IP_ON_EXIT_FUNC* pf;
};

void IP_AddOnExitHandler(IP_ON_EXIT_CB* pCB, IP_ON_EXIT_FUNC* pf);
#endif

/*********************************************************************
*
*       Delayed execution
*
*  Delayed execution is an internal API workaround to be able to
*  execute chosen API from an interrupt context that otherwise could
*  not be executed due to IP_OS_LOCK() being unable to work from
*  interrupt due to the underlying semaphore used.
*  Delayed execution means that API to execute will be queued to be
*  executed the next time executing IP_Task/IP_Exec. The IP task will
*  get signaled whenever a command is added to the queue, making sure
*  that it executes right after the interrupt has finished.
*/
typedef struct IP_EXEC_DELAYED_STRUCT IP_EXEC_DELAYED;
struct IP_EXEC_DELAYED_STRUCT {
  IP_EXEC_DELAYED* pNext;
  void (*pfExec)(const void* pPara);
  void*            pPara;
  void*            pContext;
  void (*pfDoneCB)(IP_EXEC_DELAYED* pED, void* pContext);
};

//
// Use if delayed execution call has 2 parameters.
//
typedef struct {
  PTR_ADDR Para0;
  PTR_ADDR Para1;
} IP_EXEC_DELAYED_PARA_2;

//
// Use if delayed execution call has 3 parameters.
//
typedef struct {
  PTR_ADDR Para0;
  PTR_ADDR Para1;
  PTR_ADDR Para2;
} IP_EXEC_DELAYED_PARA_3;

//
// Use if delayed execution call has 4 parameters.
//
typedef struct {
  PTR_ADDR Para0;
  PTR_ADDR Para1;
  PTR_ADDR Para2;
  PTR_ADDR Para3;
} IP_EXEC_DELAYED_PARA_4;

void IP_ExecDelayed(IP_EXEC_DELAYED* pED, void (*pfExec)(const void* pPara), void* pPara, void* pContext, void (*pfDoneCB)(IP_EXEC_DELAYED* pED, void* pContext));

//
// Available delayed API calls.
//
void IP_DHCPS_ConfigDNSAddr_Delayed(const void* pPara);
void IP_DHCPS_ConfigPool_Delayed   (const void* pPara);
void IP_DHCPS_Halt_Delayed         (const void* pPara);
void IP_DHCPS_Start_Delayed        (const void* pPara);

void IP_DNS_SERVER_Start_Delayed   (const void* pPara);
void IP_DNS_SERVER_Stop_Delayed    (const void* pPara);
void IP_MDNS_SERVER_Start_Delayed  (const void* pPara);
void IP_MDNS_SERVER_Stop_Delayed   (const void* pPara);

void IP_SetAddrMaskEx_Delayed      (const void* pPara);

/*********************************************************************
*
*       Internal API
*
*  Internal API that is made public so glue layers like the OS layer
*  can be shipped in source without the need for internal header
*  files when using object shipments.
*/
void IP_SignalIfPacketFreeUsed(void);

/*********************************************************************
*
*  IP_BSP_
*/
void IP_BSP_SetAPI(unsigned IFaceId, const BSP_IP_API* pAPI);

/*********************************************************************
*
*  IP_OS_
*/
      void   IP_OS_Delay           (unsigned ms);
      void   IP_OS_DisableInterrupt(void);
      void   IP_OS_EnableInterrupt (void);
      void   IP_OS_Init            (void);
      void   IP_OS_Unlock          (void);
      void   IP_OS_AssertLock      (void);
      void   IP_OS_Lock            (void);
      U32    IP_OS_GetTime32       (void);
      void   IP_OS_AddTickHook     (void (*pfHook)(void));
const char * IP_OS_GetTaskName     (void *pTask);
//
// Wait and signal for Net task
//
      void   IP_OS_WaitNetEvent    (unsigned ms);
      void   IP_OS_SignalNetEvent  (void);
//
// Wait and signal for the optional Rx task
//
      void   IP_OS_WaitRxEvent     (void);
      void   IP_OS_SignalRxEvent   (void);
//
// Wait and signal for the optional WiFi task
//
      void   IP_OS_WaitWiFiEventTimed(unsigned Timeout);
      void   IP_OS_SignalWiFiEvent   (void);
//
// Wait and signal for application tasks
//
      void   IP_OS_WaitItem        (void *pWaitItem);
      void   IP_OS_WaitItemTimed   (void *pWaitItem, unsigned Timeout);
      void   IP_OS_SignalItem      (void *pWaitItem);

/*********************************************************************
*
*       IP_PACKET
*
*/
#define IP_PACKET_FLAG_PTP_MASK          (U8)(1u << 0)
#define IP_PACKET_FLAG_ZERO_SRC_IP_MASK  (U8)(1u << 1)  // Use a zero source IP in IP header. For the moment this only applies to IPv4. This is to fulfill the DHCP RFC 2131 4.1 .
#define IP_PACKET_FLAG_TS_UPDATED_MASK   (U8)(1u << 2)
#define IP_PACKET_FLAG_DO_NOT_FRAGMENT   (U8)(1u << 3)
#define IP_PACKET_FLAG_DO_NOT_ROUTE      (U8)(1u << 4)

//
// Use the upmost n bits of the IP_PACKET.Flags field (U8) to
// prepare the IP layer for n words of additional IP options
// being passed in the existing packet data.
//
#define IP_PACKET_FLAG_NUM_IP_OPT_BITS             (2u)
#define IP_PACKET_FLAG_NUM_IP_OPT_SHIFT(p)         (8u - IP_PACKET_FLAG_NUM_IP_OPT_BITS)
#define IP_PACKET_FLAG_SET_NUM_IP_OPT_WORDS(p, n)  ((p)->Flags = (U8)(((p)->Flags & ~(0xFFu << IP_PACKET_FLAG_NUM_IP_OPT_SHIFT(p))) | ((n) << IP_PACKET_FLAG_NUM_IP_OPT_SHIFT(p))))
#define IP_PACKET_FLAG_GET_NUM_IP_OPT_WORDS(p)     ((p)->Flags                              >> IP_PACKET_FLAG_NUM_IP_OPT_SHIFT(p))

typedef   U32 ip_addr;
typedef   U32 IP_ADDR;

typedef struct {
  U16 PortSrc;
  U16 PortDest;
  U32 SeqNo;
  U32 AckNo;
  U8 HeaderLen;
  U8 Flags;
  U16 WindowSize;
  U16 CheckSum;
} IP_TCP4;

typedef struct {
  U16 Dummy;
  U8  aMACDest[6];
  U8  aMACSrc [6];
  U16 Type;
  // IP
  U8  VersionHeaderLen;
  U8  Service;
  U16 Len;
  U16 PacketId;
  U16 FragmentOff;
  U8  TTL;
  U8  Proto;
  U16 HeaderChecksum;
  U32 IPSrc;
  U32 IPDest;
  IP_TCP4 Tcp;
} IP_ETH_IP4_TCP;

typedef struct {
  U8  DefaultTTL;
  U8  LocalMcTTL;
  U8  GlobalMcTTL;
} IP_TTL_CONFIG;

/*********************************************************************
*
*       IP_PTP_TIMESTAMP
*
*  Function description
*    A structure which stores the PTP timestamp. This is the time
*    passed since the EPOCH of the system.
*/
typedef struct {
  U32 Seconds;      // Time [s] passed since start of epoch of the network, typically since January 1st 1970.
  U32 Nanoseconds;  // The sub-second part of the time passed since the EPOCH expressed in nanoseconds.
} IP_PTP_TIMESTAMP;

typedef struct IP_PACKET_STRUCT IP_PACKET;
struct IP_PACKET_STRUCT {
  IP_PACKET*        pNext;
  IFACE*            pIFace;              // The interface (net) it came in on
  U8*               pBuffer;             // Beginning of raw buffer
  U8*               pData;               // Beginning of protocol/data. This is always >= pBuffer.
  IP_TTL_CONFIG*    pTTLConfig;          // Overrides the default TTL values of the system. If NULL the configured defaults are used.
#if IP_SUPPORT_ON_PACKET_FREE_CB
  void (*pfOnFreeCB)(IP_PACKET* pPacket, void* pContext);
  void*             pOnFreeContext;
#endif
  U16               NumBytes;            // Number of bytes in buffer
  U16               BufferSize;          // Length of raw buffer
  I16               UseCnt;              // Use count, for cloning buffer
  U8                ToS;                 // Type of Service (QoS) byte in IPv4 header. Reused as DSCP byte for IPv4/IPv6 in newer implementations.
  U8                Flags;               // Flags for various indications (PTP request, ...)
#if (defined(U64) && ((IP_SUPPORT_PACKET_TIMESTAMP != 0) || (IP_SUPPORT_PTP != 0)))
  U64               OSTimestamp_ns;      // Time stamp (microseconds) based on OS time.
#endif
#if IP_SUPPORT_PTP
  IP_PTP_TIMESTAMP  HWTimestamp;         // Timestamp read from PTP HW clock.
#endif
#if ((IP_SUPPORT_FRAGMENTATION_IPV4 != 0) || (IP_SUPPORT_FRAGMENTATION_IPV6 != 0))
  IP_PACKET*        pNextFrag;
#endif
#if IP_DEBUG != 0
  IP_PACKET*        pNextPacketInUse;
  U8                NumSpareBytesFront;  // Number of "invisible" spare bytes in front of IP_PACKET.pBuffer that can be used to store for example SPI overhead.
  U8                NumSpareBytesEnd;    // Number of "invisible" spare bytes after       IP_PACKET.pBuffer that can be used to store for example SPI overhead.
#endif
};

/*********************************************************************
*
*       IP_PTP_INTERNAL_API
*
*  Function description
*    A structure which stores function pointers for internal API
*    to be able to decouple the stack in library form from the
*    PTP modules in source form. This is required by drivers that
*    have been compiled into a library with IP_SUPPORT_PTP=1 but
*    the define being zeroed when shipping the libraries. The drivers
*    expect the PTP sources to be included which might not be the case.
*    Therefore "callbacks" to internal PTP API need to be encapsulated
*    by any sort of function pointer that can be NULL checked.
*/
typedef struct {
  void (*pfSignalEvent)(U8 EventMask);
} IP_PTP_INTERNAL_API;

typedef struct IP_PTP_ENDPOINT_STRUCT  IP_PTP_ENDPOINT;

/*********************************************************************
*
*       PTP driver
*/
typedef struct {
  int  (*pfInit)       (unsigned IFaceId, void* pContext, IP_PTP_ENDPOINT* pEndpoint);
  void (*pfCorrectTime)(unsigned IFaceId, U32* pFlags, U32 Sec, U32 NSec);  // Callback used by IP_PTP to give a time correction to the driver.
  int  (*pfGetTime)    (unsigned IFaceId, U32 *pSec, U32 *pNSec);           // Callback used by IP_PTP to get the precise time from the driver.
  int  (*pfControl)    (unsigned IFaceId, int Cmd, void* p);
  const IP_PTP_INTERNAL_API* pIntAPI;
} IP_PTP_DRIVER;

/*********************************************************************
*
*       NI driver
*/
typedef struct {
  int  (*pfInit)             (unsigned IFaceId);
  int  (*pfSendPacket)       (unsigned IFaceId);                                 // Sends a packet on this interface. What exactly the driver does depends very much on the hardware.
                                                                                 // Typically it does nothing if it is already sending and starts sending the first packet in the fifo if transmitter is idle.
                                                                                 // Return value: 0: OK, < 0 Error
                                                                                 // Packet is owned by driver from this point on.
  int  (*pfGetPacketSize)    (unsigned IFaceId);                                 // Return the number of bytes in next packet, <= 0 if there is no more packet.
  int  (*pfReadPacket)       (unsigned IFaceId, U8 * pDest, unsigned NumBytes);  // Read (if pDest is valid) and discard packet.
  void (*pfTimer)            (unsigned IFaceId);                                 // Routine is called periodically
  int  (*pfControl)          (unsigned IFaceId, int Cmd, void * p);              // Various control functions
  void (*pfEnDisableRxInt)   (unsigned IFaceId, unsigned OnOff);                 // Masks or unmasks Rx Interrupt. OnOff = 1: Enable, OnOff = 0: Disable
#if IP_SUPPORT_PTP
  int  (*pfGetRxTimestamp)   (unsigned IFaceId, U32* pSec, U32* pNSec);          // Callback used by the stack to get the timestamp of the received packet.
#endif
} IP_HW_DRIVER;

/*********************************************************************
*
*       IP_WIFI_...
*
*  WiFi functions for WiFi interfaces
*/
typedef struct {
  const char* sSSID;
        I32   Rssi;  // Receive Signal Strength Indicator. Typically given as -<value>. Higher (towards 0) means better.
        U16   BeaconInterval;
        U8    abBSSID[6];
        U8    Channel;
        U8    Security;
        U8    Mode;  // IP_WIFI_MODE_UNKNOWN or IP_WIFI_MODE_INFRASTRUCTURE or IP_WIFI_MODE_ADHOC .
} IP_WIFI_SCAN_RESULT;

typedef struct {
  U8 acKey[13];  // 5 (64 bit) or 13 (128 bit) byte long WEP key.
  U8 Len;        // Length of the WEP key, 5 or 13 bytes.
  U8 Index;      // Unique key index 0..3 .
} IP_WIFI_WEP_KEY;

typedef struct {
  const char*            sSSID;
  const char*            sEnterpriseUser;    // WPA/WPA2 enterprise username.
  const char*            sWPAPass;           // WPA/WPA2 enterprise/PSK passphrase.
  const IP_WIFI_WEP_KEY* paWEPKey;
  U8                     abBSSID[6];         // HW address of Access Point.
  U8                     NumWEPKeys;         // Number of WEP keys configured.
  U8                     WEPActiveKeyIndex;  // 0..3: Index of WEP key to be used for sending, typically index 0 .
  U8                     Mode;               // IP_WIFI_MODE_INFRASTRUCTURE or IP_WIFI_MODE_ADHOC .
  U8                     Security;
  U8                     Channel;
} IP_WIFI_CONNECT_PARAMS;

typedef struct {
  U8 abBSSID[6];
  U8 Channel;
} IP_WIFI_ASSOCIATE_INFO;

typedef struct {
  U8 Domain24GHz;
  U8 Domain5GHzMask;
} IP_WIFI_DOMAIN_CONFIG;

typedef struct {
  I32 Rssi;
} IP_WIFI_SIGNAL_INFO;

typedef struct {
  U8* pMacAddress;  // Pointer is not valid after the hook call.
} IP_WIFI_CLIENT_INFO;

typedef void (*IP_WIFI_pfScanResult)          (unsigned IFaceId, const IP_WIFI_SCAN_RESULT* pResult, int Status);
typedef void (*IP_WIFI_pfOnAssociateChange)   (unsigned IFaceId, const IP_WIFI_ASSOCIATE_INFO* pInfo, U8 State);
typedef void (*IP_WIFI_pfOnSignalChange)      (unsigned IFaceId, IP_WIFI_SIGNAL_INFO* pInfo);

#define IP_WIFI_CLIENT_DISCONNECT  (0u)
#define IP_WIFI_CLIENT_CONNECT     (1u)
typedef void (*IP_WIFI_pfOnClientNotification)(unsigned IFaceId, const IP_WIFI_CLIENT_INFO* pInfo, unsigned IsConnected);

typedef struct IP_HOOK_ON_WIFI_ASSOCIATE_CHANGE {
  struct IP_HOOK_ON_WIFI_ASSOCIATE_CHANGE* pNext;  // Pointer to the next hook.
         IP_WIFI_pfOnAssociateChange       pf;     // Pointer to the function to be called by the hook.
} IP_HOOK_ON_WIFI_ASSOCIATE_CHANGE;

typedef struct IP_HOOK_ON_WIFI_SIGNAL_CHANGE {
  struct IP_HOOK_ON_WIFI_SIGNAL_CHANGE* pNext;     // Pointer to the next hook.
         IP_WIFI_pfOnSignalChange       pf;        // Pointer to the function to be called by the hook.
} IP_HOOK_ON_WIFI_SIGNAL_CHANGE;

typedef struct IP_HOOK_ON_WIFI_CLIENT_NOTIFICATION_STRUCT IP_HOOK_ON_WIFI_CLIENT_NOTIFICATION;
struct IP_HOOK_ON_WIFI_CLIENT_NOTIFICATION_STRUCT {
  IP_HOOK_ON_WIFI_CLIENT_NOTIFICATION* pNext;
  IP_WIFI_pfOnClientNotification       pf;
};

#define IP_WIFI_MODE_UNKNOWN         0
#define IP_WIFI_MODE_INFRASTRUCTURE  1
#define IP_WIFI_MODE_ADHOC           2
#define IP_WIFI_MODE_ACCESS_POINT    3

#define IP_WIFI_CHANNEL_ALL          0

//
// 2.4 GHz channels by regulation (one at a time can be used) together with multiple 5 GHz channel defines.
//
#define IP_WIFI_24GHZ_FCC_WORLD  0x00  // Set to 0 to be the default.
#define IP_WIFI_24GHZ_NONE       0x01
#define IP_WIFI_24GHZ_ETSI       0x02
#define IP_WIFI_24GHZ_TELEC      0x03

//
// 5 GHz channels as defined by U-NII (can be or-ed to use a wider channel selection).
// Can be or-ed with 2.4 GHz channel setup.
//
#define IP_WIFI_5GHZ_UNII1_MASK   0x10
#define IP_WIFI_5GHZ_UNII2_MASK   0x20
#define IP_WIFI_5GHZ_UNII2E_MASK  0x40
#define IP_WIFI_5GHZ_UNII3_MASK   0x80

#define IP_WIFI_SECURITY_OPEN                       (0)
#define IP_WIFI_SECURITY_WEP_OPEN                   (1)
#define IP_WIFI_SECURITY_WEP_SHARED                 (2)
#define IP_WIFI_SECURITY_WPA_TKIP                   (3)
#define IP_WIFI_SECURITY_WPA_AES                    (4)
#define IP_WIFI_SECURITY_WPA2_AES                   (5)
#define IP_WIFI_SECURITY_WPA_WPA2_MIXED             (6)                              //lint !e621 Ignore identifier clash for 31 unique chars in name.
#define IP_WIFI_SECURITY_WPA_ENTERPRISE             (7)
#define IP_WIFI_SECURITY_WPA2_ENTERPRISE            IP_WIFI_SECURITY_WPA_ENTERPRISE  //                                                                All supported modules use WPA/WPA2 combined, so everything is the same.
#define IP_WIFI_SECURITY_WPA_WPA2_MIXED_ENTERPRISE  IP_WIFI_SECURITY_WPA_ENTERPRISE  //lint !e621 Ignore identifier clash for 31 unique chars in name. All supported modules use WPA/WPA2 combined, so everything is the same.

//
// Compatibility macros.
//
#define IP_WIFI_AddIsrExecDoneHook           IP_DTASK_AddExecDoneHook
#define IP_WIFI_IsrExec                      IP_DTASK_Exec
#define IP_WIFI_IsrTask                      IP_DTASK_Task
#define IP_WIFI_ConfigIsrTaskAlwaysSignaled  IP_DTASK_ConfigAlwaysSignaled
#define IP_WIFI_ConfigIsrTaskTimeout         IP_DTASK_ConfigTimeout
#define IP_WIFI_SignalIsrTask                IP_DTASK_Signal
#define IP_HOOK_ON_WIFI_ISR_EXEC_DONE        IP_HOOK_ON_DTASK_EXEC_DONE

/*********************************************************************
*
*       IP_ON_DTASK_EXEC_DONE_FUNC
*
*  Function description
*    Callback that gets notified after a DriverTask execution.
*
*  Parameters
*    IFaceId: Zero-based interface index.
*/
typedef void (IP_ON_DTASK_EXEC_DONE_FUNC)(unsigned IFaceId);

typedef struct IP_HOOK_ON_DTASK_EXEC_DONE_STRUCT IP_HOOK_ON_DTASK_EXEC_DONE;
struct IP_HOOK_ON_DTASK_EXEC_DONE_STRUCT {
  IP_HOOK_ON_DTASK_EXEC_DONE* pNext;    // Pointer to the next hook.
  IP_ON_DTASK_EXEC_DONE_FUNC* pf;       // Callback to execute once the DriverTask exec has finished.
  unsigned                    IFaceId;  // Zero-based interface index for which the callback will be executed.
};

      void  IP_DTASK_AddExecDoneHook            (unsigned IFaceId, IP_HOOK_ON_DTASK_EXEC_DONE* pHook, IP_ON_DTASK_EXEC_DONE_FUNC* pf);
      void  IP_DTASK_Exec                       (unsigned IFaceId);
      void  IP_DTASK_Task                       (void);
      void  IP_DTASK_ConfigAlwaysSignaled       (unsigned IFaceId, char OnOff);
      void  IP_DTASK_ConfigTimeout              (unsigned IFaceId, U32 ms);
      void  IP_DTASK_Signal                     (unsigned IFaceId);

      void  IP_WIFI_AddAssociateChangeHook     (IP_HOOK_ON_WIFI_ASSOCIATE_CHANGE* pHook, IP_WIFI_pfOnAssociateChange pf);
      void  IP_WIFI_AddClientNotificationHook  (IP_HOOK_ON_WIFI_CLIENT_NOTIFICATION* pHook, IP_WIFI_pfOnClientNotification pf);
      int   IP_WIFI_AddInterface               (const IP_HW_DRIVER* pDriver);
      void  IP_WIFI_AddSignalChangeHook        (IP_HOOK_ON_WIFI_SIGNAL_CHANGE* pHook, IP_WIFI_pfOnSignalChange pf);
      int   IP_WIFI_Connect                    (unsigned IFaceId, const IP_WIFI_CONNECT_PARAMS* pParams, U32 Timeout);
      int   IP_WIFI_ConfigAllowedChannels      (unsigned IFaceId, const U8* paChannel, U8 NumChannels);
      int   IP_WIFI_ConfigRegDomain            (unsigned IFaceId, const IP_WIFI_DOMAIN_CONFIG* pDomainConfig);
      int   IP_WIFI_Disconnect                 (unsigned IFaceId, U32 Timeout);
      int   IP_WIFI_Scan                       (unsigned IFaceId, U32 Timeout, IP_WIFI_pfScanResult pf, const char* sSSID, U8 Channel);
const char* IP_WIFI_Security2String            (U8 Security);

//
// Prototypes that make our life easier providing generic samples
// for WiFi callbacks such as connect/disconnect used in the
// configuration files.
//
int  WIFI_CB_Connect             (unsigned IFaceId);
int  WIFI_CB_Connect_StartAP     (unsigned IFaceId);
int  WIFI_CB_Disconnect          (unsigned IFaceId);
void WIFI_CB_OnAssociateChange   (unsigned IFaceId, const IP_WIFI_ASSOCIATE_INFO* pInfo, U8 State);
void WIFI_CB_OnLinkChange        (unsigned IFaceId, U32 Duplex, U32 Speed);
void WIFI_CB_OnSignalChange      (unsigned IFaceId, IP_WIFI_SIGNAL_INFO* pInfo);
void WIFI_CB_OnClientNotification(unsigned IFaceId, const IP_WIFI_CLIENT_INFO* pInfo, unsigned IsConnected);

/*********************************************************************
*
*       Driver capabilities
*/
#define IP_NI_CAPS_WRITE_IP_CHKSUM               (1uL <<  0)  // Driver capable of inserting the IP-checksum into an outgoing packet ?
#define IP_NI_CAPS_WRITE_UDP_CHKSUM              (1uL <<  1)  // Driver capable of inserting the UDP-checksum into an outgoing packet ?
#define IP_NI_CAPS_WRITE_TCP_CHKSUM              (1uL <<  2)  // Driver capable of inserting the TCP-checksum into an outgoing packet ?
#define IP_NI_CAPS_WRITE_ICMP_CHKSUM             (1uL <<  3)  // Driver capable of inserting the ICMP-checksum into an outgoing packet ?
#define IP_NI_CAPS_CHECK_IP_CHKSUM               (1uL <<  4)  // Driver capable of computing and comparing the IP-checksum of an incoming packet ?
#define IP_NI_CAPS_CHECK_UDP_CHKSUM              (1uL <<  5)  // Driver capable of computing and comparing the UDP-checksum of an incoming packet ?
#define IP_NI_CAPS_CHECK_TCP_CHKSUM              (1uL <<  6)  // Driver capable of computing and comparing the TCP-checksum of an incoming packet ?
#define IP_NI_CAPS_CHECK_ICMP_CHKSUM             (1uL <<  7)  // Driver capable of computing and comparing the ICMP-checksum of an incoming packet ?
#define IP_NI_CAPS_WRITE_ICMPV6_CHKSUM           (1uL <<  8)  // Driver capable of inserting the ICMP-checksum into an outgoing packet ?
//
// Capabilites above this point are legacy capabilities.
// Everything from here on are EXtended capabilities that can be retrieved
// from a driver together with the legacy capabilities with a new internal
// API and provide more insight for the stack on what the needs of the driver
// are and what the driver provides in enhanced features.
//
#define IP_NI_CAPS_HAS_HASH_FILTER               (1uL <<  9)  // Driver supports hash filters ?
#define IP_NI_CAPS_HAS_PROMISCUOUS_MODE          (1uL << 10)  // Driver supports promiscuous mode ?
#define IP_NI_CAPS_USE_BUFFER_SPARE_BYTES_FRONT  (1uL << 11)  // Driver asks to prepend the packet buffer with IP_NI_CMD_GET_CAPS_EX_DATA.PacketDataShiftCnt ?
                                                              // Number of spare bytes (given as shift count) the driver needs before the packet payload
                                                              // in the IP_PACKET.pBuffer for example to provide space for an SPI header that needs to be
                                                              // sent in front of the packet when using an external controller.
#define IP_NI_CAPS_PACKET_GETS_MODIFIED          (1uL << 12)  // Driver modifies the packet ? In this case some things like (re-)sending directly from the SocketBuffer can not be used.
#define IP_NI_CAPS_UNUSED_13                     (1uL << 13)  // Not used yet. Placeholder for (U16)IFACE.Caps awareness.
#define IP_NI_CAPS_UNUSED_14                     (1uL << 14)  // Not used yet. Placeholder for (U16)IFACE.Caps awareness.
#define IP_NI_CAPS_UNUSED_15                     (1uL << 15)  // Not used yet. Placeholder for (U16)IFACE.Caps awareness.
#define IP_NI_CAPS_UNUSED_16                     (1uL << 16)  // Can not be used due to (U16)IFACE.Caps !!!

/*********************************************************************
*
*       PHY configuration
*/
#define IP_PHY_MODE_MII       0u
#define IP_PHY_MODE_RMII      1u

#define IP_PHY_ADDR_ANY       0xFF                          // IP_PHY_ADDR_ANY is used as PHY addr to initiate automatic scan for PHY
#define IP_PHY_ADDR_INTERNAL  0xFE                          // IP_PHY_ADDR_INTERNAL is used as PHY addr to select internal PHY

#define IP_ADMIN_STATE_DOWN   0u
#define IP_ADMIN_STATE_UP     1u

/*********************************************************************
*
*       Ethernet PHY
*/
typedef struct IP_PHY_CONTEXT  IP_PHY_CONTEXT;

typedef struct {
  unsigned (*pfRead) (IP_PHY_CONTEXT* pContext, unsigned RegIndex);
  void     (*pfWrite)(IP_PHY_CONTEXT* pContext, unsigned RegIndex, unsigned  val);
} IP_PHY_ACCESS;

#define IP_PHY_MODE_10_HALF    ((U16)1u <<  5)  // Real bit in register.
#define IP_PHY_MODE_10_FULL    ((U16)1u <<  6)  // Real bit in register.
#define IP_PHY_MODE_100_HALF   ((U16)1u <<  7)  // Real bit in register.
#define IP_PHY_MODE_100_FULL   ((U16)1u <<  8)  // Real bit in register.
#define IP_PHY_MODE_1000_HALF  ((U16)1u <<  9)  // Pseudo bit. Bit is not consecutive to the rest as other registers and bits are used.
#define IP_PHY_MODE_1000_FULL  ((U16)1u << 10)  // Pseudo bit. Bit is not consecutive to the rest as other registers and bits are used.

struct IP_PHY_CONTEXT {
  const IP_PHY_ACCESS *pAccess;
        void          *pContext;        // Context needed for low level functions
        U32            Speed;
        U32            SpeedStableCnt;
        U16            SupportedModes;
        U16            Anar;            // Value written to ANAR (Auto-negotiation Advertisement register)
        U16            Anar1000;        // Value written to 1000BASE-T control register (Auto-negotiation Advertisement register for 1000BASE-T)
        U16            Bmcr;            // Value written to BMCR (basic mode control register)
        U8             Addr;
        U8             UseRMII;         // 0: MII, 1: RMII
        U8             LastLinkChange;  // Direction of last link change. 0: No link change yet, 1: Link changed to up, 2: Link changed to down
        U8             SupportGmii;     // On PHY init we detect if the PHY supports Gigabit Ethernet and save the result here. If 0 on PHY init disables Gigabit Ethernet support.
        U8             IFaceId;
};

typedef void (*IP_PHY_pfConfig)(unsigned IFaceId);

typedef struct {
  const U8* pAddrList;
        U8  NumAddr;
} IP_PHY_ALT_LINK_STATE_ADDR;

typedef struct {
               IP_PHY_pfConfig             pfConfig;
  const        IP_PHY_ALT_LINK_STATE_ADDR* pAltPhyAddr;
  const struct IP_PHY_HW_DRIVER*           pDriver;
  const        void*                       pAccess;
               void*                       pContext;         // Context that is used to attach the driver specific context to.
               void*                       pUserContext;     // User/internal specified context. Also used by some drivers to pass a context different callbacks in the driver.
               U32                         Speed;
               U32                         SpeedStableCnt;
               U32                         LastSavedSpeed;   // Disables periodic PHY access from the stack for link checks if set.
               U32                         LastSavedDuplex;  // Disables periodic PHY access from the stack for link checks if set.
               U16                         AfterResetDelay;
               U8                          InitDone;
               U8                          LastLinkChange;   // Direction of last link change. 0: No link change yet, 1: Link changed to up, 2: Link changed to down.
               U8                          DoNotUseStaticFilters;
               U8                          DisableChecks;
} IP_PHY_CONTEXT_EX;

typedef struct IP_PHY_HW_DRIVER {
  int (*pfAttachContext)(unsigned IFaceId, IP_PHY_CONTEXT_EX* pContext);
  int (*pfInit)         (unsigned IFaceId, IP_PHY_CONTEXT_EX* pContext);
  int (*pfGetLinkState) (unsigned IFaceId, IP_PHY_CONTEXT_EX* pContext, U32* pDuplex, U32* pSpeed);
  int (*pfControl)      (unsigned IFaceId, IP_PHY_CONTEXT_EX* pContext, unsigned Cmd, void* p);  // Control function for various jobs.
} IP_PHY_HW_DRIVER;

typedef struct {
  int (*pfInit)        (IP_PHY_CONTEXT *pContext);
  int (*pfGetLinkState)(IP_PHY_CONTEXT *pContext, U32 *pDuplex, U32 *pSpeed);
} IP_PHY_DRIVER;

typedef struct {
  unsigned (*pfRead) (void *pContext, unsigned RegIndex);
  void     (*pfWrite)(void *pContext, unsigned RegIndex, unsigned Data);
} IP_PHY_API;

typedef void (*IP_NI_pfOnPhyReset)(unsigned IFaceId, void *pContext, const IP_PHY_API *pApi);

typedef struct IP_HOOK_ON_PHY_RESET {
  struct IP_HOOK_ON_PHY_RESET *pNext;    // Pointer to the next hook function.
         IP_NI_pfOnPhyReset    pf;       // Pointer to the function to be called by the hook.
} IP_HOOK_ON_PHY_RESET;

void IP_PHY_AddDriver             (unsigned IFaceId, const IP_PHY_HW_DRIVER* pDriver, const void* pAccess, IP_PHY_pfConfig pf);
int  IP_PHY_AddPTPDriver          (unsigned IFaceId, const IP_PTP_DRIVER* pPTPDriver, U32 Clock);
void IP_PHY_AddResetHook          (IP_HOOK_ON_PHY_RESET *pHook, IP_NI_pfOnPhyReset pf);
void IP_PHY_ConfigAddr            (unsigned IFaceId, unsigned Addr);
void IP_PHY_ConfigAfterResetDelay (unsigned IFaceId, U16 ms);
void IP_PHY_ConfigAltAddr         (unsigned IFaceId, const IP_PHY_ALT_LINK_STATE_ADDR* pAltPhyAddr);
void IP_PHY_ConfigGigabitSupport  (unsigned IFaceId, unsigned OnOff);
void IP_PHY_ConfigSupportedModes  (unsigned IFaceId, unsigned Modes);
void IP_PHY_ConfigUseStaticFilters(unsigned IFaceId, unsigned OnOff);
void IP_PHY_DisableCheck          (U32 Mask);
void IP_PHY_DisableCheckEx        (unsigned IFaceId, U32 Mask);
void IP_PHY_ReInit                (unsigned IFaceId);
void IP_PHY_SetWdTimeout          (int ShiftCnt);

#define IP_PHY_GENERIC_AddResetHook  IP_PHY_AddResetHook  // Compatibility macro for old API
void IP_PHY_GENERIC_RemapAccess   (unsigned IFaceId, unsigned AccessIFaceId);

extern const IP_PHY_DRIVER    IP_PHY_Generic;         // Generic PHY driver (legacy).
extern const IP_PHY_HW_DRIVER IP_PHY_Driver_Generic;  // Generic PHY driver.
extern const IP_PHY_HW_DRIVER IP_PHY_Driver_MARVELL_88E1111_Fiber;

#define PHY_DISABLE_CHECK_ID                   (1uL << 0)
#define PHY_DISABLE_CHECK_LINK_STATE_AFTER_UP  (1uL << 1)
#define PHY_DISABLE_WATCHDOG                   (1uL << 2)

/*********************************************************************
*
*       Network interface configuration and handling
*/
void IP_NI_ClrBPressure             (unsigned IFaceId);
int  IP_NI_ConfigLinkCheckMultiplier(unsigned IFaceId, unsigned Multiplier);
void IP_NI_ConfigPHYAddr            (unsigned IFaceId, U8 Addr);                                // Configure PHY Addr (5-bit)
void IP_NI_ConfigPHYMode            (unsigned IFaceId, U8 Mode);                                // Configure PHY Mode: 0: MII, 1: RMII
void IP_NI_ConfigUsePromiscuousMode (unsigned IFaceId, unsigned OnOff);
void IP_NI_ConfigPoll               (unsigned IFaceId);
void IP_NI_ForceCaps                (unsigned IFaceId, U8 CapsForcedMask, U8 CapsForcedValue);  // Allows to forcibly set the checksum capabilities of the hardware
int  IP_NI_GetAdminState            (unsigned IFaceId);
int  IP_NI_GetIFaceType             (unsigned IFaceId, char* pBuffer, U32* pNumBytes);
int  IP_NI_GetState                 (unsigned IFaceId);
int  IP_NI_GetTxQueueLen            (unsigned IFaceId);
void IP_NI_SetAdminState            (unsigned IFaceId, int AdminState);
void IP_NI_SetBPressure             (unsigned IFaceId);
int  IP_NI_SetTxBufferSize          (unsigned IFaceId, unsigned NumBytes);
int  IP_NI_AddPTPDriver             (unsigned IFaceId, const IP_PTP_DRIVER* pPTPDriver, U32 CoreClock);
int  IP_NI_PauseRx                  (unsigned IFaceId, U32 Pause);
int  IP_NI_PauseRxInt               (unsigned IFaceId, U32 Pause);

/*********************************************************************
*
*       IP stack tasks
*/
void IP_Task       (void);
void IP_RxTask     (void);
void IP_ShellServer(void);

typedef int  (IP_RX_HOOK)     (IP_PACKET *pPacket);
typedef void IP_ON_RX_FUNC    (IP_PACKET *pPacket);

#define IP_TASK_REQ_SHUTDOWN_MASK  (1u << 0)
#define IP_TASK_LEAVE_LOOP_MASK    (1u << 1)
#define IP_TASK_IP_MASK            (1u << 2)
#define IP_TASK_RX_MASK            (1u << 3)
#define IP_TASK_DTASK_MASK         (1u << 4)
#define IP_TASK_WIFI_ISR_MASK      (IP_TASK_DTASK_MASK)  // Compatibility macro. WiFi ISR task is now Driver Task.

/*********************************************************************
*
*       Public API hooks
*/
typedef struct IP_HOOK_AFTER_CONFIG {
  struct IP_HOOK_AFTER_CONFIG *pNext;                         // Pointer to the next hook function.
  int    (*pf)(void);                                         // Pointer to the function to be called by the hook.
} IP_HOOK_AFTER_CONFIG;

typedef struct IP_HOOK_AFTER_INIT {
  struct IP_HOOK_AFTER_INIT *pNext;                           // Pointer to the next hook function.
  void   (*pf)(void);                                         // Pointer to the function to be called by the hook.
} IP_HOOK_AFTER_INIT;

typedef struct IP_HOOK_ON_STATE_CHANGE {
  struct IP_HOOK_ON_STATE_CHANGE *pNext;                      // Pointer to the next hook function.
  void   (*pf)(unsigned IFaceId, U8 AdminState, U8 HWState);  // Pointer to the function to be called by the hook.
} IP_HOOK_ON_STATE_CHANGE;

typedef struct IP_HOOK_ON_ETH_TYPE {
  struct IP_HOOK_ON_ETH_TYPE* pNext;                                             // Pointer to the next hook.
  int (*pf)(unsigned IFaceId, IP_PACKET* pPacket, void* pBuffer, U32 NumBytes);  // Pointer to the function to be called by the hook.
  U16 Type;                                                                      // Ethernet type that triggers the callback.
} IP_HOOK_ON_ETH_TYPE;

typedef struct IP_HOOK_ON_PACKET_FREE {
  struct IP_HOOK_ON_PACKET_FREE* pNext;  // Pointer to the next hook.
  void (*pf)(IP_PACKET* pPacket);        // Pointer to the function to be called by the hook.
} IP_HOOK_ON_PACKET_FREE;

typedef struct IP_HOOK_ON_LINK_CHANGE_STRUCT IP_HOOK_ON_LINK_CHANGE;
struct IP_HOOK_ON_LINK_CHANGE_STRUCT {
  IP_HOOK_ON_LINK_CHANGE* pNext;                        // Pointer to the next hook.
  void (*pf)(unsigned IFaceId, U32 Duplex, U32 Speed);  // Pointer to the function to be called by the hook.
};

typedef struct IP_HOOK_ON_RX_FIFO_ADD_STRUCT IP_HOOK_ON_RX_FIFO_ADD;
struct IP_HOOK_ON_RX_FIFO_ADD_STRUCT {
  IP_HOOK_ON_RX_FIFO_ADD *pNext;                                        // Pointer to the next hook.
  int (*pf)(IP_PACKET* pPacket, unsigned AllowNoLock, unsigned InInt);  // Pointer to the function to be called by the hook.
};

typedef struct IP_HOOK_ON_HW_ADDR_CHANGE_STRUCT IP_HOOK_ON_HW_ADDR_CHANGE;
struct IP_HOOK_ON_HW_ADDR_CHANGE_STRUCT {
  IP_HOOK_ON_HW_ADDR_CHANGE* pNext;                           // Pointer to the next hook.
  void (*pf)(unsigned IFace, const U8* pAddr, unsigned Len);  // Pointer to the function to be called by the hook.
};

/*********************************************************************
*
*       IP_ON_ICMPV4_FUNC
*
*  Function description
*    Callback executed when an ICMPv4 packet is received.
*
*  Parameters
*    IFaceId     : Zero-based interface index.
*    pPacket     : Packet that has been received.
*                  pPacket->pData points to the IPv4 header.
*    pUserContext: User context given when adding the hook.
*    p           : Reserved for future extensions of this API.
*
*  Return value
*    == IP_OK                  : Packet has been handled (freed or reused).
*    == IP_OK_TRY_OTHER_HANDLER: Packet is untouched and stack shall try another handler.
*
*  Additional information
*    The callback can remove its own hook using IP_ICMP_RemoveRxHook() .
*/
typedef int (IP_ON_ICMPV4_FUNC)(unsigned IFaceId, IP_PACKET* pPacket, void* pUserContext, void* p);

typedef struct IP_HOOK_ON_ICMPV4_STRUCT IP_HOOK_ON_ICMPV4;
struct IP_HOOK_ON_ICMPV4_STRUCT {
  IP_HOOK_ON_ICMPV4* pNext;         // Pointer to the next hook.
  IP_ON_ICMPV4_FUNC* pf;            // Pointer to the function to be called by the hook.
  void*              pUserContext;  // User context set when adding the hook.
};

/*********************************************************************
*
*       Public API callbacks
*/

#define IP_ON_IFACE_SELECT_FLAG_BROADCAST  (1u << 0)
#define IP_ON_IFACE_SELECT_FLAG_MULTICAST  (1u << 1)

/*********************************************************************
*
*       IP_ON_IFACE_SELECT_INFO
*
*  Function description
*    Provides information about an internal interface selection for
*    an operation (typically sending without previous receive), as
*    well as to propose an interface.
*
*  Additional information
*    Most parameters are presented as pointers to the actual internal
*    value. If a parameter/pointer is NULL, this means that this
*    parameter was not involved in selecting the proposed interface.
*
*    If IFaceId is -1, this means no interface has been selected
*    by the internal procedure.
*/
typedef struct {
  int              IFaceId;   // Interface as proposed by internal selection. -1 if no suitable interface was found.
  const U32*       pLAddrV4;  // Pointer to local   IPv4 address. NULL if not used. Value is in network endianness (big endian).
  const U32*       pFAddrV4;  // Pointer to foreign IPv4 address. NULL if not used. Value is in network endianness (big endian).
  const IPV6_ADDR* pLAddrV6;  // Pointer to local   IPv6 address. NULL if not used.
  const IPV6_ADDR* pFAddrV6;  // Pointer to foreign IPv6 address. NULL if not used.
  U8               Flags;     // ORR-ed combination of IP_IFACE_SELECT_FLAG_* :
                              //   * None                             : Looking for a unicast interface.
                              //   * IP_ON_IFACE_SELECT_FLAG_BROADCAST: Looking for an interface that is capable of broadcasting.
                              //   * IP_ON_IFACE_SELECT_FLAG_MULTICAST: Looking for an interface that is capable of multicast.
} IP_ON_IFACE_SELECT_INFO;

/*********************************************************************
*
*       IP_ON_IFACE_SELECT_FUNC
*
*  Function description
*    Callback executed for an internal interface selection. The
*    proposed interface selected internally can be overridden.
*
*  Parameters
*    PFamily: Protocol family (at the moment only PF_INET or PF_INET6).
*    pInfo  : Further information of type IP_ON_IFACE_SELECT_INFO
*             about the interface selection parameters as well as
*             the proposed interface, selected internally based upon
*             these parameters.
*
*  Return value
*    == -1: No suitable interface.
*    >=  0: Interface index to use.
*/
typedef int (IP_ON_IFACE_SELECT_FUNC)(int PFamily, IP_ON_IFACE_SELECT_INFO* pInfo);

/*********************************************************************
*
*       IP_CACHE_...
*
*  Cache related functions
*/
void IP_CACHE_SetConfig(const SEGGER_CACHE_CONFIG *pConfig, unsigned ConfSize);

/*********************************************************************
*
*       IP_MEM_
*/

/*********************************************************************
*
*       IP_MEM_POOL_INFO
*
*  Function description
*    Provides information about a memory pool managed by the stack.
*
*  Additional information
*    Numbers such as free space and maximum chunk size that can be
*    allocated vary by a couple of bytes with values returned being
*    higher than what can be successfully allocated. This is due to
*    internal overhead and alignments that are calculated during
*    allocation processes that are not calculated when retrieving
*    this information about a pool and its free resources.
*/
typedef struct {
  PTR_ADDR BaseAddr;      // Base address of the memory pool.
  U32      Size;          // Total number of bytes managed for this pool.
  U32      Free;          // Number of bytes available for allocation from this pool.
  U32      MaxFreeChunk;  // Biggest chunk available for allocation.
} IP_MEM_POOL_INFO;

/*********************************************************************
*
*       Core functions
*/
#define IP_ConfTCPSpace  IP_ConfigTCPSpace  // Compatibility macro for old API

      void       IP_AddAfterConfigHook              (IP_HOOK_AFTER_CONFIG* pHook, int  (*pf)(void));
      void       IP_AddAfterInitHook                (IP_HOOK_AFTER_INIT*   pHook, void (*pf)(void));
      void       IP_AddBuffers                      (int NumBuffers, int BytesPerBuffer);
      int        IP_AddEtherInterface               (const IP_HW_DRIVER *pDriver);
      void       IP_AddHWAddrChangeHook             (IP_HOOK_ON_HW_ADDR_CHANGE* pHook, void (*pf)(unsigned IFace, const U8* pAddr, unsigned Len));
      void       IP_AddLinkChangeHook               (IP_HOOK_ON_LINK_CHANGE* pHook, void (*pf)(unsigned IFaceId, U32 Duplex, U32 Speed));
      int        IP_AddLoopbackInterface            (void);
      void       IP_AddMemory                       (U32* pMem, U32 NumBytes);
      void       IP_AddStateChangeHook              (IP_HOOK_ON_STATE_CHANGE *pHook, void (*pf)(unsigned IFaceId, U8 AdminState, U8 HWState));
      int        IP_AddVirtEtherInterface           (unsigned HWIFaceId);
      void*      IP_Alloc                           (U32 NumBytesReq);
      void*      IP_AllocEx                         (U32* pBaseAddr, U32 NumBytesReq);
      void       IP_Free                            (void *p);
      void       IP_AllowBackPressure               (char v);
      void       IP_AssignMemory                    (U32 *pMem, U32 NumBytes);
      void       IP_ConfigDoNotAddLowLevelChecks    (void);  //lint !e621 Ignore identifier clash for 31 unique chars in name.
      void       IP_ConfigDoNotAddLowLevelChecks_ARP(void);  //lint !e621 Ignore identifier clash for 31 unique chars in name.
      void       IP_ConfigDoNotAddLowLevelChecks_UDP(void);  //lint !e621 Ignore identifier clash for 31 unique chars in name.
      void       IP_ConfigMaxIFaces                 (unsigned NumIFaces);
      void       IP_ConfigNumLinkDownProbes         (U8 IFaceId, U8 NumProbes);
      void       IP_ConfigNumLinkUpProbes           (U8 IFaceId, U8 NumProbes);
      void       IP_ConfigOffCached2Uncached        (I32 Off);
      void       IP_ConfigReportSameMacOnNet        (unsigned OnOff, void* p);
      void       IP_ConfigTCPSpace                  (unsigned SendSpace, unsigned RecvSpace);  // Set window sizes
      int        IP_Connect                         (unsigned IFaceId);
      void       IP_DeInit                          (void);
      void       IP_DisableIPRxChecksum             (U8 IFace);
      void       IP_DisableIPv4                     (void);
      int        IP_Disconnect                      (unsigned IFaceId);
      void       IP_EnableIPRxChecksum              (U8 IFace);
      U32        IP_Exec                            (void);
      int        IP_FindIFaceByIP                   (void* pAddr, unsigned Len);
      void       IP_GetAddrMask                     (U8 IFace, U32 *pAddr, U32 *pMask);
      int        IP_GetCurrentLinkSpeed             (void);
      int        IP_GetCurrentLinkSpeedEx           (unsigned IFaceId);
      U32        IP_GetFreePacketCnt                (U32 NumBytes);
      U32        IP_GetGWAddr                       (U8 IFace);
      U32        IP_GetIFaceHeaderSize              (unsigned IFaceId);
      void       IP_GetHWAddr                       (unsigned IFaceId, U8 *pDest, unsigned Len);
      U32        IP_GetIPAddr                       (unsigned IFaceId);
const U8*        IP_GetIPPacketInfo                 (IP_PACKET* pPacket);
      U32        IP_GetMaxAvailPacketSize           (int IFaceId);
      int        IP_GetMemPoolInfo                  (void* pPoolAddr, IP_MEM_POOL_INFO* pInfo);
      int        IP_GetPrimaryIFace                 (void);
const char *     IP_GetRawPacketInfo                (const IP_PACKET *pPacket, U16 *pNumBytes);
      int        IP_GetVersion                      (void);                   // Format: Mmmrr. Sample 10201 is 1.02a
      int        IP_IFaceIsReady                    (void);
      int        IP_IFaceIsReadyEx                  (unsigned IFaceId);
      int        IP_Init                            (void);
      unsigned   IP_IsAllZero                       (const U8* p, unsigned NumBytes);
      unsigned   IP_IsExpired                       (U32 Time);
      unsigned   IP_IsInitialized                   (void);
      void       IP_Panic                           (const char *sError);
      void       IP_PrintStatus                     (void);
      void       IP_SetAddrMask                     (U32 Addr, U32 Mask);
      void       IP_SetAddrMaskEx                   (U8 IFace, U32 Addr, U32 Mask);
      void       IP_SetPacketToS                    (IP_PACKET *pPacket, U8 ToS);
      void       IP_SetGlobalMcTTL                  (int v);
      void       IP_SetLocalMcTTL                   (int v);
      void       IP_SetOnIFaceSelectCallback        (IP_ON_IFACE_SELECT_FUNC* pf);
      void       IP_SetTTL                          (int v);
      void       IP_SetGWAddr                       (U8 IFace, U32 GWAddr);
      void       IP_SetHWAddr                       (const U8 *pHWAddr);
      void       IP_SetHWAddrEx                     (unsigned IFaceId, const U8* pHWAddr, unsigned NumBytes);
      void       IP_SetIFaceConnectHook             (unsigned IFaceId, int (*pf)(unsigned IFaceId));
      void       IP_SetIFaceDisconnectHook          (unsigned IFaceId, int (*pf)(unsigned IFaceId));
      int        IP_SetPrimaryIFace                 (int IFaceId);
      void       IP_SetRandCallback                 (void (*pfGetRand)(U8* pBuffer, unsigned NumBytes));
      int        IP_SendPacket                      (unsigned IFace, void* pData, unsigned NumBytes);
      int        IP_SendPing                        (U32 FHost, char *pData, unsigned NumBytes, U16 SeqNum);
      int        IP_SendPingEx                      (U32 IFaceId, U32 FHost, char *pData, unsigned NumBytes, U16 SeqNum);
      int        IP_SendPingCheckReply              (U32 IFaceId, U32 FHost, char* pData, unsigned NumBytes, unsigned ms);
#if IP_SUPPORT_ON_PACKET_FREE_CB
      void       IP_SetOnPacketFreeCallback         (IP_PACKET* pPacket, void (*pfOnFreeCB)(IP_PACKET* pPacketCB, void* pContextCB), void* pContext);
#endif
      void       IP_SetRxHook                       (IP_RX_HOOK *pfRxHook);
      int        IP_SetSupportedDuplexModes         (unsigned IFace, unsigned DuplexMode);
      void       IP_SetUseRxTask                    (void);
      unsigned   IP_Shutdown                        (unsigned LeaveTaskLoop, U32 Timeout);
      IP_PACKET* IP_AllocEtherPacket                (unsigned IFaceId, U32 NumBytes, U8** ppBuffer);
      void       IP_FreePacket                      (IP_PACKET* pPacket);
      int        IP_SendEtherPacket                 (unsigned IFaceId, IP_PACKET* pPacket, U32 NumBytes);
      void       IP_AddEtherTypeHook                (IP_HOOK_ON_ETH_TYPE* pHook, int (*pf)(unsigned IFaceId, IP_PACKET* pPacket, void* pBuffer, U32 NumBytes), U16 Type);
      void       IP_RemoveEtherTypeHook             (IP_HOOK_ON_ETH_TYPE* pHook);
      void       IP_AddOnPacketFreeHook             (IP_HOOK_ON_PACKET_FREE *pHook, void (*pf)(IP_PACKET* pPacket));
      void       IP_X_Config                        (void);
      U32        IP_GetMTU                          (unsigned IFaceId);
      void       IP_SetMTU                          (unsigned IFaceId, U32 Mtu);

/*********************************************************************
*
*       Fragmentation related functions
*/
#if IP_SUPPORT_FRAGMENTATION_IPV4
void IP_FRAGMENT_Enable  (void);
void IP_FRAGMENT_ConfigRx(U16 MaxFragments, U32 Timeout, U8 KeepOOO);
#endif

/*********************************************************************
*
*       IP_MICREL_TAIL_TAGGING_
*/
int IP_MICREL_TAIL_TAGGING_AddInterface(unsigned HWIFaceId, U8 InTag, U8 OutTag);

/*********************************************************************
*
*       IP_ARP_
*/
void IP_ARP_CleanCache              (void);
void IP_ARP_CleanCacheByInterface   (unsigned IFaceId);
void IP_ARP_ConfigAgeout            (U32 Ageout);
void IP_ARP_ConfigAgeoutSniff       (U32 Ageout);
void IP_ARP_ConfigAgeoutNoReply     (U32 Ageout);
void IP_ARP_ConfigAllowGratuitousARP(U8 OnOff);
void IP_ARP_ConfigAnnounceStaticIP  (unsigned IFaceId, U8 NumAnnouncements);
int  IP_ARP_ConfigNumEntries        (unsigned NumEntries);
void IP_ARP_ConfigMaxRetries        (unsigned Retries);
void IP_ARP_ConfigMaxPending        (unsigned NumPackets);

/*********************************************************************
*
*       IP_AutoIP_
*/

#define  AUTOIP_STATE_UNUSED      0
#define  AUTOIP_STATE_INITREBOOT  1
#define  AUTOIP_STATE_INIT        2
#define  AUTOIP_STATE_SENDPROBE   3
#define  AUTOIP_STATE_CHECKREPLY  4
#define  AUTOIP_STATE_BOUND       5

typedef void (IP_AUTOIP_INFORM_USER_FUNC)(U32 IFace, U32 Stat);

void IP_AutoIP_Activate       (unsigned IFaceId);
int  IP_AutoIP_Halt           (unsigned IFaceId, char KeepIP);
void IP_AutoIP_SetUserCallback(unsigned IFaceId, IP_AUTOIP_INFORM_USER_FUNC * pfInformUser);
void IP_AutoIP_SetStartIP     (unsigned IFaceId, U32 IPAddr);


/*********************************************************************
*
*       IP_SOCKET_
*  Typically located in IP_Socket.h but kept in here for dependencies
*  like IP_PACKET or other public structures.
*/
int IP_SOCKET_SetCallback(int hSock, int (*pfCallback)(int hSock, IP_PACKET* pPacket, int MsgCode));

/*********************************************************************
*
*       IP_TCP_
*/
#define IPV4_TCP_HEADER_LEN  (72u)  // Helper define, typical  worst-case header assumed by the stack for IPv4 & TCP. 40 IP/TCP, 12 RTTM, 14+2 Ether, 4 VLAN.

typedef struct {
  U16 FPort;
  U16 LPort;
} IP_TCP_ACCEPT_INFO;

typedef struct IP_TCP_ACCEPT_HOOK_STRUCT IP_TCP_ACCEPT_HOOK;
struct IP_TCP_ACCEPT_HOOK_STRUCT {
  IP_TCP_ACCEPT_HOOK* pNext;
  void (*pfAccept)(int hSock, IP_TCP_ACCEPT_INFO* pInfo, void* pContext);
  void*               pContext;
  int                 hSock;
};

int  IP_TCP_Accept                (IP_TCP_ACCEPT_HOOK* pHook, void (*pfAccept)(int hSock, IP_TCP_ACCEPT_INFO* pInfo, void* pContext), int hSock, void* pContext);
void IP_TCP_Add                   (void);
void IP_TCP_DisableRxChecksum     (U8 IFace);
void IP_TCP_EnableRxChecksum      (U8 IFace);
void IP_TCP_Set2MSLDelay          (unsigned v);
void IP_TCP_SetConnKeepaliveOpt   (U32 Init, U32 Idle, U32 Period, U32 Cnt);
void IP_TCP_SetRetransDelayRange  (unsigned RetransDelayMin, unsigned RetransDelayMax);

/*********************************************************************
*
*       TCP Zero copy
*/
IP_PACKET*  IP_TCP_Alloc      (unsigned NumBytes);
IP_PACKET*  IP_TCP_AllocEx    (unsigned NumBytes, unsigned NumBytesHeader);
void        IP_TCP_Free       (           IP_PACKET * pPacket);
int         IP_TCP_Send       (int hSock, IP_PACKET * pPacket);
int         IP_TCP_SendAndFree(int hSock, IP_PACKET * pPacket);

/*********************************************************************
*
*       UDP & RAW zero copy return values
*/
#define IP_RX_ERROR             (-1)
#define IP_OK                     0
#define IP_OK_KEEP_PACKET         1
#define IP_OK_KEEP_IN_SOCKET      2
#define IP_OK_TRY_OTHER_HANDLER   3

/*********************************************************************
*
*       UDP
*/
typedef struct {
  IP_DLIST_ITEM Link;
  U16           LPort;
  U16           FPort;
  void         *pIPAddrInfo;
  int           (*pfOnRx)(IP_PACKET* pPacket, void* pContext);
  void         *pContext;
  U8            IsIPv6;
} IP_UDP_CONNECTION;

typedef IP_UDP_CONNECTION*  IP_UDP_CONN_HANDLE;  // Used as handle by the application.

void                 IP_UDP_Add              (void);
IP_UDP_CONNECTION *  IP_UDP_AddEchoServer    (U16 LPort);
IP_UDP_CONNECTION *  IP_UDP_Open             (IP_ADDR FAddr, U16 FPort,                U16 LPort, int (*handler)(IP_PACKET *pPacket, void *pContext), void *pContext);
IP_UDP_CONNECTION *  IP_UDP_OpenEx           (IP_ADDR FAddr, U16 FPort, IP_ADDR LAddr, U16 LPort, int (*handler)(IP_PACKET *pPacket, void *pContext), void *pContext);
void                 IP_UDP_Close            (IP_UDP_CONNECTION* pCon);
IP_PACKET          * IP_UDP_Alloc            (unsigned NumBytes);
IP_PACKET          * IP_UDP_AllocEx          (unsigned IFaceId, unsigned NumBytes);
int                  IP_UDP_Send             (int IFace, IP_ADDR FHost, U16 fport, U16 lport, IP_PACKET *pPacket);
int                  IP_UDP_SendAndFree      (int IFace, IP_ADDR FHost, U16 fport, U16 lport, IP_PACKET *pPacket);
void                 IP_UDP_Free             (IP_PACKET *pPacket);
U16                  IP_UDP_FindFreePort     (void);
U16                  IP_UDP_GetFPort         (const IP_PACKET *pPacket);
U16                  IP_UDP_GetLPort         (const IP_PACKET *pPacket);
unsigned             IP_UDP_GetIFIndex       (const IP_PACKET *pPacket);
void               * IP_UDP_GetDataPtr       (const IP_PACKET *pPacket);
U16                  IP_UDP_GetDataSize      (const IP_PACKET *pPacket);
void                 IP_UDP_GetDestAddr      (const IP_PACKET *pPacket, void *pDestAddr, int AddrLen);
void                 IP_UDP_GetSrcAddr       (const IP_PACKET *pPacket, void *pSrcAddr , int AddrLen);
int                  IP_UDP_ReducePayloadLen (IP_PACKET *pPacket, int NumBytes);

void                 IP_UDP_EnableRxChecksum (void);
void                 IP_UDP_DisableRxChecksum(void);
void                 IP_UDP_EnableTxChecksum (void);
void                 IP_UDP_DisableTxChecksum(void);

/*********************************************************************
*
*       RAW
*/
typedef struct {
  IP_DLIST_ITEM Link;
  U32           LHost;
  U32           FHost;
  int           (*pfOnRx)(IP_PACKET* pPacket, void* pContext);
  void *        pContext;
  U8            Protocol;
  U8            IpHdrIncl;
} IP_RAW_CONNECTION;

typedef IP_RAW_CONNECTION*  IP_RAW_CONN_HANDLE;  // Used as handle by the application.

#define IPPROTO_IP      (0)  // Dummy value for TCP.
#define IPPROTO_ICMP    (1)  // Internet Control Message Protocol.
#define IPPROTO_TCP     (6)  // Transmission Control Protocol.
#define IPPROTO_UDP    (17)  // User Datagram Protocol.
#define IPPROTO_RAW   (255)  // RAW IP packets.

void                 IP_RAW_Add              (void);
int                  IP_RAW_AddPacketToSocket(int hSock, IP_PACKET* pPacket);
IP_RAW_CONNECTION*   IP_RAW_Open             (IP_ADDR FAddr, IP_ADDR LAddr, U8 Protocol, int (*handler)(IP_PACKET* pPacket, void* pContext), void * pContext);
void                 IP_RAW_Close            (IP_RAW_CONNECTION* pCon);
IP_PACKET          * IP_RAW_Alloc            (unsigned IFaceId, unsigned NumBytesData, int IpHdrIncl);
int                  IP_RAW_Send             (int IFace, IP_ADDR FHost, U8 Protocol, IP_PACKET * pPacket);
int                  IP_RAW_SendAndFree      (int IFace, IP_ADDR FHost, U8 Protocol, IP_PACKET * pPacket);
void                 IP_RAW_Free             (IP_PACKET * pPacket);
unsigned             IP_RAW_GetIFIndex       (const IP_PACKET *pPacket);
void               * IP_RAW_GetDataPtr       (const IP_PACKET *pPacket);
U16                  IP_RAW_GetDataSize      (const IP_PACKET *pPacket);
void                 IP_RAW_GetDestAddr      (const IP_PACKET *pPacket, void *pDestAddr, int AddrLen);
void                 IP_RAW_GetSrcAddr       (const IP_PACKET *pPacket, void *pSrcAddr , int AddrLen);
int                  IP_RAW_ReducePayloadLen (IP_PACKET *pPacket, int NumBytes);

/*********************************************************************
*
*       IGMP_
*/
#define IP_IGMP_MCAST_ALLHOSTS_GROUP    0xE0000001uL  // 224.0.0.1
#define IP_IGMP_MCAST_ALLROUTERS_GROUP  0xE0000002uL  // 224.0.0.2
#define IP_IGMP_MCAST_ALLRPTS_GROUP     0xE0000016uL  // 224.0.0.22, IGMPv3

int  IP_IGMP_Add                 (void);
int  IP_IGMP_AddEx               (unsigned IFaceId);
void IP_IGMP_ConfigV2AlwaysReport(unsigned IFaceId, U8 OnOff);
int  IP_IGMP_JoinGroup           (unsigned IFaceId, IP_ADDR GroupIP);
int  IP_IGMP_JoinGroup_AutoRejoin(unsigned IFaceId, IP_ADDR GroupIP);
void IP_IGMP_LeaveGroup          (unsigned IFaceId, IP_ADDR GroupIP);

/*********************************************************************
*
*       UPNP_
*/
int IP_UPNP_Activate(unsigned IFace, const char * acUDN);

/*********************************************************************
*
*       IP_PPPOE_
*/
typedef void (IP_PPPOE_INFORM_USER_FUNC)(U32 IFaceId, U32 Stat);

int        IP_PPPOE_AddInterface   (unsigned IFaceId);
void       IP_PPPOE_ConfigRetries  (unsigned IFaceId, U32 NumTries, U32 Timeout);
void       IP_PPPOE_Reset          (unsigned IFaceId);
void       IP_PPPOE_SetAuthInfo    (unsigned IFaceId, const char * sUser, const char * sPass);
void       IP_PPPOE_SetUserCallback(U32 IFaceId, IP_PPPOE_INFORM_USER_FUNC * pfInformUser);

/*********************************************************************
*
*       IP_VLAN_
*/
int IP_VLAN_AddInterface(unsigned HWIFace, U16 VLANId);

/*********************************************************************
*
*       IP_SNTPC_
*/
typedef struct IP_NTP_TIMESTAMP {
  U32 Seconds;
  U32 Fractions;
} IP_NTP_TIMESTAMP;

#define IP_SNTPC_STATE_NO_ANSWER  0  // Request sent but no answer received from NTP server within timeout
#define IP_SNTPC_STATE_UPDATED    1  // Timestamp received from NTP server
#define IP_SNTPC_STATE_KOD        2  // Kiss-Of-Death received from server. This means the server wants us to use another server.

void IP_SNTPC_ConfigAcceptNoSyncSource(U8 OnOff);
void IP_SNTPC_ConfigTimeout           (unsigned ms);
int  IP_SNTPC_GetTimeStampFromServer  (unsigned IFaceId, const char * sServer, IP_NTP_TIMESTAMP * pTimestamp);

/*********************************************************************
*
*       IP_MODEM_
*/
struct IP_PPP_CONTEXT;
typedef struct {
  void (*pfInit)               (struct IP_PPP_CONTEXT * pPPPContext); //
  void (*pfSend)               (U8 Data);                             // Send the first data byte
  void (*pfSendNext)           (U8 Data);                             // Send the next data byte
  void (*pfTerminate)          (U8 IFaceId);                          // Terminate connection
  void (*pfOnPacketCompletion) (void);                                // Optional. Called when packet is complete. Normally used for packet oriented PPP interfaces GPRS or USB modems.
} IP_PPP_LINE_DRIVER;

extern const IP_PPP_LINE_DRIVER MODEM_Driver;

      int    IP_MODEM_Connect            (const char * sATCommand);
      void   IP_MODEM_Disconnect         (unsigned IFaceId);
const char * IP_MODEM_GetResponse        (unsigned IFaceId, char * pBuffer, unsigned NumBytes, unsigned * pNumBytesInBuffer);
      void   IP_MODEM_SendString         (unsigned IFaceId, const char * sCmd);
      int    IP_MODEM_SendStringEx       (unsigned IFaceId, const char * sCmd, const char * sResponse, unsigned Timeout, unsigned RecvBufOffs);
      void   IP_MODEM_SetAuthInfo        (unsigned IFaceId, const char * sUser, const char * sPass);
      void   IP_MODEM_SetConnectTimeout  (unsigned IFaceId, unsigned ms);
      void   IP_MODEM_SetInitCallback    (void (*pfInit)(void));
      void   IP_MODEM_SetInitString      (const char * sInit);
      void   IP_MODEM_SetSwitchToCmdDelay(unsigned IFaceId, unsigned ms);
      void   IP_MODEM_SetUartConfig      (unsigned int Unit, unsigned long Baudrate, unsigned char NumDataBits, unsigned char Parity, unsigned char NumStopBits);

/*********************************************************************
*
*       IP_PPP_
*/
typedef void IP_PPP_INFORM_USER_FUNC(U32 IFaceId, U32 Stat);

typedef void* IP_PPP_MD5_INIT (void);
typedef void  IP_PPP_MD5_ADD  (void* pContext, const U8* pInput, unsigned InputLen);
typedef void  IP_PPP_MD5_FINAL(void* pContext, U8* pDigest, unsigned DigestLen);

typedef struct {
  IP_PPP_MD5_INIT*  pfInit;
  IP_PPP_MD5_ADD*   pfAdd;
  IP_PPP_MD5_FINAL* pfFinal;
} IP_PPP_MD5_API;

int  IP_PPP_AddInterface   (const IP_PPP_LINE_DRIVER * pLineDriver, int ModemIndex);
void IP_PPP_OnRx           (struct IP_PPP_CONTEXT * pContext, U8 * pData, int NumBytes);
void IP_PPP_OnRxChar       (struct IP_PPP_CONTEXT * pContext, U8 Data);
int  IP_PPP_OnTxChar       (unsigned Unit);
void IP_PPP_SetUserCallback(U32 IFaceId, IP_PPP_INFORM_USER_FUNC * pfInformUser);

void IP_PPP_CHAP_AddWithMD5(const IP_PPP_MD5_API* pAPI);

/*********************************************************************
*
*       IP_ICMP_
*/
void IP_ICMP_Add              (void);
void IP_ICMP_AddRxHook        (IP_HOOK_ON_ICMPV4* pHook, IP_ON_ICMPV4_FUNC* pf, void* pUserContext);
void IP_ICMP_DisableRxChecksum(U8 IFace);
void IP_ICMP_EnableRxChecksum (U8 IFace);
void IP_ICMP_RemoveRxHook     (IP_HOOK_ON_ICMPV4* pHook);
void IP_ICMP_SetRxHook        (IP_RX_HOOK *pfRxHook);

//
// Echo or Echo Reply Message
//
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |     Type      |     Code      |          Checksum             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |           Identifier          |        Sequence Number        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |     Data ...
//   +-+-+-+-+-
//
typedef struct {
  U8  Type;
  U8  Code;
  U16 Checksum;
  U16 Id;
  U16 Seq;
} IP_ICMP_HEADER;

#define IP_ICMP_TYPE_ECHO_REPLY        (0u)
#define IP_ICMP_TYPE_DEST_UNREACHABLE  (3u)
#define IP_ICMP_TYPE_ECHO_REQUEST      (8u)

/*********************************************************************
*
*       Log/Warn functions (public)
*/
void IP_AddLogFilter     (U32 TypeMask);
void IP_AddWarnFilter    (U32 TypeMask);
void IP_RemoveLogFilter  (U32 TypeMask);
void IP_RemoveWarnFilter (U32 TypeMask);
void IP_SetLogFilter     (U32 TypeMask);
void IP_SetWarnFilter    (U32 TypeMask);
void IP_Log              (const char* s);
void IP_Logf_Application (const char* sFormat, ...);
void IP_Warn             (const char* s);
void IP_Warnf_Application(const char* sFormat, ...);

/*********************************************************************
*
*       Log/Warn functions (internal)
*
*  Typically only used by the stack internally. Made public only for
*  usage in IP_Config callbacks for driver init log messages.
*/
void IP_Logf (U32 TypeMask, const char * sFormat, ...);
void IP_Warnf(U32 TypeMask, const char * sFormat, ...);

/*********************************************************************
*
*       Client: DNS-SD (DNS - service discovery) / mDNS / LLMNR
*
*/

//
//  Reply structure.
//
typedef struct {
        void*     pContext;
        U16       Type;
  const char*     sName;
  const char*     sRoot;
        U32       TTL;
  union {
    struct {
      IP_ADDR     IPAddr;  // In network endianness.
    } A;
    //
#if IP_SUPPORT_IPV6
    struct {
      U8*         pIPAddrV6;
    } AAAA;
#endif
    //
    struct {
      char*       sDomainName;
      char*       sRoot;
    } PTR;
    //
    struct {
      U16         Priority;
      U16         Weight;
      U16         Port;
      char*       sTarget;
      char*       sRoot;
    } SRV;
    //
    struct {
      char*       sTXT;  // This string is not null-terminated.
      U8          Length;
    } TXT;
  } Config;
} IP_MDNS_REPLY;

//
// Request structure
//
typedef struct {
        void* pContext;        // Context that will be set for each reply in IP_MDNS_REPLY as well.
  const char* sHost;
        void (*pfResult)(IP_MDNS_REPLY* pReply, unsigned IsLast);
        U16   Type;
        U8    FirstReplyOnly;  // Wait for the first reply and ignore potential other replies.
} IP_DNSSD_REQUEST;

int IP_MDNS_ResolveHost        (unsigned IFaceId, const IP_DNSSD_REQUEST* pRequest, unsigned ms);
int IP_MDNS_ResolveHostSingleIP(unsigned IFaceId, void* pIP, const char* sHost, U16 Type, unsigned ms);

/*********************************************************************
*
*       DNS (Domain name system)
*
*  Name resolution
*/
// Description of data base entry for a single host.
struct hostent {
  char *  h_name;        // Official name of host.
  char ** h_aliases;     // Alias list.
  int     h_addrtype;    // Host address type.
  int     h_length;      // Length of address.
  char ** h_addr_list;   // List of addresses from name server.
#define h_addr h_addr_list[0] /* Address, for backward compatibility.  */
#ifdef DNS_CLIENT_UPDT
  // Extra variables passed in to Dynamic DNS updates.
  char *  h_z_name;      // IN- zone name for UPDATE packet.
  ip_addr h_add_ipaddr;  // IN- add this ip address for host name in zone.
  U32     h_ttl;         // IN- time-to-live field for UPDATE packet.
#endif
};

int  IP_ResolveHost       (const char *sHost, U32 *pIPAddr, U32 ms);
int  IP_DNS_ResolveHostEx (unsigned IFaceId, const IP_DNSSD_REQUEST* pRequest, unsigned ms);
void IP_DNS_SetServer     (U32 DNSServerAddr);
U32  IP_DNS_GetServer     (void);
int  IP_DNS_SetServerEx   (unsigned IFaceId, U8 DNSServer, const U8 *pDNSAddr, int AddrLen);
void IP_DNS_GetServerEx   (unsigned IFaceId, U8 DNSServer, U8 *pDNSAddr, int *pAddrLen);
void IP_DNS_SetMaxTTL     (U32 TTL);
int  IP_DNS_SendDynUpdate (unsigned IFaceId, const char* sHost, const char* sDomain, U32 IPv4Addr, int ClearPreviousRR, U32 ms);
void IP_DNS_SetTSIGContext(char *KeyName, char *KeyAlgoName, int (*pfSign)(U8* pData, U16 DataLength, U8* pDigest, int DigestMaxSize), int (*pfGetTime)(U32* pSeconds)); // The 2 strings must be static.

/*********************************************************************
*
*       Server: DNS-SD (DNS - service discovery) / mDNS / LLMNR
*
*  Discovery.
*/
#define IP_DNS_SERVER_TYPE_A       (1u)       // IPv4 address.
#define IP_DNS_SERVER_TYPE_PTR     (12u)      // PTR record.
#define IP_DNS_SERVER_TYPE_TXT     (16u)      // TXT record.
#define IP_DNS_SERVER_TYPE_SRV     (33u)      // SRV record.
#define IP_DNS_SERVER_TYPE_AAAA    (28u)      // IPv6 address.
#define IP_DNS_SERVER_TYPE_ALL     (255u)     // All records '*'.

#define IP_DNS_SERVER_FLAG_FLUSH   (1u << 0)  // Tell the host that is asking to flush this entry in any case.
                                              // Typically at least A and AAAA records should use the FLUSH bit
                                              // as they should always be unique names.

//
// Configurations for the different DNS-SD types: A, AAAA, PTR, SRV, TXT.
//
typedef struct {
  char*                    sName;        // No need to have a A record for the hostname. The IP address of the interface is used in this case.
  IP_ADDR                  IPAddr;
} IP_DNS_SERVER_A;

#if IP_SUPPORT_IPV6
typedef struct {
  char*                    sName;        // No need to have a AAAA record for the hostname. The IP address of the interface is used in this case.
  U8                       aIPAddrV6[IPV6_ADDR_LEN];
} IP_DNS_SERVER_AAAA;
#endif

typedef struct {
  char*                    sName;        // Either a real name _service._udp.local or a formated IP address 01.00.168.192.in-addr.arpa
  char*                    sDomainName;  // If NULL, hostname will be used.
} IP_DNS_SERVER_PTR;

typedef struct {
  char*                    sName;
  U16                      Priority;
  U16                      Weight;
  U16                      Port;
  char*                    sTarget;      // If NULL, hostname will be used.
} IP_DNS_SERVER_SRV;

typedef struct {
  char*                    sName;        // If NULL, hostname will be used.
  char*                    sTXT;
} IP_DNS_SERVER_TXT;

//
// DNS-SD configuration.
//
typedef struct {
  U32                        TTL;
  union {
    IP_DNS_SERVER_A          A;
#if IP_SUPPORT_IPV6
    IP_DNS_SERVER_AAAA       AAAA;
#endif
    IP_DNS_SERVER_PTR        PTR;
    IP_DNS_SERVER_SRV        SRV;
    IP_DNS_SERVER_TXT        TXT;          // Note: All TXT records for the same name shall be contiguous in the configuration table.
  } Config;
  U8                         Type;         // A, AAAA, PTR, SRV, TXT
  U8                         Flags;        // IP_DNS_SERVER_FLAG_FLUSH; If no flush flag is used for override, future releases might try to figure out the flush flag by probing.
                                           // See RFC 6762 section 10.2. "Announcements to Flush Outdated Cache Entries" and section 8.1 "Probing".
} IP_DNS_SERVER_SD_CONFIG;

//
// Discovery configuration. The apSDConfig array is a list of IP_DNS_SERVER_SD_CONFIG.
//
typedef struct {
  const char*                      sHostname;
        U32                        TTL;          // If set to 0, default value DNS_TTL_INIT will be used.
        unsigned                   NumConfig;    // Number of configuration in the array pointed by apSDConfig. 0 if apSDConfig is NULL.
  const IP_DNS_SERVER_SD_CONFIG*   apSDConfig;   // Configuration array. Could be NULL.
} IP_DNS_SERVER_CONFIG;

int IP_MDNS_SERVER_Start(const IP_DNS_SERVER_CONFIG* pConfig);
int IP_MDNS_SERVER_Stop (void);
int IP_DNS_SERVER_Start (const IP_DNS_SERVER_CONFIG* pConfig);
int IP_DNS_SERVER_Stop  (void);

/*********************************************************************
*
*       Netbios Name Service (Add-on)
*/
typedef struct {
  char* sName;
  U8    NumBytes;
} IP_NETBIOS_NAME;

int  IP_NETBIOS_Init (U32 IFaceId, const IP_NETBIOS_NAME * paHostnames, U16 LPort);
int  IP_NETBIOS_Start(U32 IFaceId);
void IP_NETBIOS_Stop (U32 IFaceId);

/*********************************************************************
*
*       Utility functions
*/
#define IP_LoadU32BE  SEGGER_RdU32BE
#define IP_LoadU32LE  SEGGER_RdU32LE
#define IP_LoadU16BE  SEGGER_RdU16BE
#define IP_LoadU16LE  SEGGER_RdU16LE
#define IP_StoreU32BE SEGGER_WrU32BE
#define IP_StoreU32LE SEGGER_WrU32LE
#define IP_tolower    SEGGER_tolower
#define IP_isalpha    SEGGER_isalpha
#define IP_isalnum    SEGGER_isalnum

U32  IP_LoadU32TE    (const U8* pData);
U32  IP_BringInBounds(U32 v, U32 Min, U32 Max);
int  IP_PrintIPAddr  (char* pBuffer, U32 IPAddr, int BufferSize);
int  IP_PrintIPAddrEx(char* pBuffer, int BufferSize, const U8* pIPAddr, int AddrLen);
U32  IP_SwapU32      (U32 v);

/*********************************************************************
*
*       IP_DHCPC_...
*
*  DHCP (Dynamic host configuration protocol) client functions.
*/
#define  DHCPC_RESET_CONFIG       0
#define  DHCPC_USE_STATIC_CONFIG  1
#define  DHCPC_USE_DHCP_CONFIG    2

//
// DHCP client states
//
#define  DHCPC_STATE_UNUSED          0u
#define  DHCPC_STATE_INIT            1u
#define  DHCPC_STATE_INITREBOOT      2u
#define  DHCPC_STATE_REBOOTING       3u
#define  DHCPC_STATE_SELECTING       4u
#define  DHCPC_STATE_REQUESTING      5u
#define  DHCPC_STATE_BOUND           6u
#define  DHCPC_STATE_RENEWING        7u
#define  DHCPC_STATE_REBINDING       8u
#define  DHCPC_STATE_RESTARTING      9u
#define  DHCPC_STATE_CHECK_IP       10u  // Not part of the diagram: This state is entered before "BOUND", when waiting for answer to ARP-packet to make sure IP-addr is not in use
#define  DHCPC_STATE_WAIT_INIT      11u  // Not part of the diagram: This state is entered if CHECK_IP fails. RFC2131, 3.1.5 says: The client SHOULD wait a minimum of ten seconds before restarting the configuration process to avoid excessive network traffic in case of looping.
#define  DHCPC_STATE_REQUEST_NAK    12u  // We tried to request a specific address but were actively rejected by the server. In an enum this would be after DHCPC_STATE_REQUESTING .
#define  DHCPC_STATE_LEASE_EXPIRED  13u  // A previously acquired configuration timed out as the server did not repsond.

typedef struct {
  char* sDomain;
  U32   IPAddr;
  U32   Gateway;
  U32   SubnetMask;
  U8    DomainLen;
} IP_DHCPC_STATE_INFO;

typedef struct IP_DHCPC_HOOK_ON_STATE_CHANGE {
  struct IP_DHCPC_HOOK_ON_STATE_CHANGE* pNext;                               // Pointer to the next hook.
  void (*pf)(unsigned IFaceId, unsigned State, IP_DHCPC_STATE_INFO* pInfo);  // Pointer to the function to be called by the hook.
} IP_DHCPC_HOOK_ON_STATE_CHANGE;

/*********************************************************************
*
*       IP_DHCPC_ON_OPTION_INFO
*
*  Function description
*    Returns information about the next DHCP option to be processed.
*/
typedef struct {
  const U8* pVal;    // Value of the DHCP option.
        int Status;  // * == 0: O.K.
                     // * <  0: Parse error, abort of parser.
        U8  Type;    // DHCP option type. Please refer to RFC 1533
                     // for further information.
        U8  Len;     // Length of the option value.
} IP_DHCPC_ON_OPTION_INFO;

/*********************************************************************
*
*       IP_DHCPC_ON_OPTION_FUNC
*
*  Function description
*    Callback executed for every DHCP option received.
*
*  Parameters
*    IFaceId: Zero-based interface index.
*    pInfo  : Further information of type IP_DHCPC_ON_OPTION_INFO
*             about the DHCP option parsed.
*
*  Additional information
*    Once all options are parsed, the end marker (option type 0xFF) is
*    reported as well for an easy to detect end of the list from
*    within the callback. No end is signaled if there was an abort
*    that can be detected by looking at pInfo->Status .
*/
typedef void (IP_DHCPC_ON_OPTION_FUNC)(unsigned IFaceId, IP_DHCPC_ON_OPTION_INFO* pInfo);

int      IP_DHCPC_Activate                  (int IFaceId, const char * sHost, const char * sDomain, const char * sVendor);
void     IP_DHCPC_AddStateChangeHook        (IP_DHCPC_HOOK_ON_STATE_CHANGE* pHook, void (*pf)(unsigned IFaceId, unsigned State, IP_DHCPC_STATE_INFO* pInfo));
int      IP_DHCPC_AssignCurrentConfig       (int IFaceId);
int      IP_DHCPC_ConfigAlwaysStartInit     (int IFaceId, U8 OnOff);
int      IP_DHCPC_ConfigAssignConfigManually(int IFaceId, U8 OnOff);
int      IP_DHCPC_ConfigDisableARPCheck     (int IFaceId, U8 OnOff);
int      IP_DHCPC_ConfigDNSManually         (int IFaceId, U8 OnOff);
int      IP_DHCPC_ConfigRequestLeaseTime    (int IFaceId, U32 LeaseTime);
int      IP_DHCPC_ConfigOnActivate          (int IFaceId, U8 Mode);
int      IP_DHCPC_ConfigOnFail              (int IFaceId, U8 Mode);
int      IP_DHCPC_ConfigOnLinkDown          (int IFaceId, U32 Timeout, U8 Mode);
int      IP_DHCPC_ConfigUniBcStartMode      (int IFaceId, U8 Mode);
int      IP_DHCPC_SendDeclineAndHalt        (int IFaceId);
int      IP_DHCPC_SendDeclineAndResetIP     (int IFaceId);
int      IP_DHCPC_GetOptionRequestList      (int IFaceId, U8* pBuffer, unsigned BufferSize);
U32      IP_DHCPC_GetServer                 (int IFaceId);
unsigned IP_DHCPC_GetState                  (int IFaceId);
int      IP_DHCPC_Halt                      (int IFaceId);
int      IP_DHCPC_Renew                     (int IFaceId);
int      IP_DHCPC_SetCallback               (int IFaceId, int (*routine)(int IFaceId, int State) );
int      IP_DHCPC_SetClientId               (int IFaceId, const U8* pClientId, unsigned ClientIdLen);
int      IP_DHCPC_SetOptionRequestList      (int IFaceId, const U8* pOptions, unsigned NumOptions);
void     IP_DHCPC_SetOnOptionCallback       (IP_DHCPC_ON_OPTION_FUNC* pf);
void     IP_DHCPC_SetTimeout                (int IFaceId, U32 Timeout, U32 MaxTries, unsigned Exponential);

/*********************************************************************
*
*       IP_DHCPS_...
*
*  DHCP (Dynamic host configuration protocol) server functions.
*/

/*********************************************************************
*
*       IP_DHCPS_RESERVE_ADDR
*
*  Function description
*    Reserves a DHCP IPv4 address via HW address, hostname or both.
*/
typedef struct {
  const U8*   pHWAddr;    // Client HW/MAC address to reserve to. Can be NULL.
        U32   IPAddr;     // IPv4 address to reserve in host endianness.
                          // Does not need to be from the DHCP pool itself.
  const char* sHostName;  // Client hostname to reserve to. Can be NULL.
} IP_DHCPS_RESERVE_ADDR;

/*********************************************************************
*
*       IP_DHCPS_GET_VENDOR_OPTION_INFO
*
*  Function description
*    Returns information about the vendor specific identifier
*    received with DHCP option 60.
*/
typedef struct {
  U8* pVendorClassId;  // Pointer to the DHCP option 60 field received from
                       // a client including type and length bytes. A
                       // typical example would be Type: 60, Len: 8 and
                       // Value: 'M' 'S' 'F' 'T' ' ' '5' '.' '0' for a
                       // Microsoft client that supports vendor specific
                       // DHCP option 43 commands.
} IP_DHCPS_GET_VENDOR_OPTION_INFO;

/*********************************************************************
*
*       IP_DHCPS_GET_VENDOR_OPTION_FUNC
*
*  Function description
*    Inserts a vendor specific configuration for DHCP option 43.
*
*  Parameters
*    IFaceId : Zero-based interface index.
*    pInfo   : Further information of type IP_DHCPS_GET_VENDOR_OPTION_INFO
*              about the vendor of the client.
*    ppOption: Pointer to the pointer where to add further options.
*              The dereferenced pointer needs to be incremented
*              by the number of bytes added. Type and length bytes
*              need to be added by the callback as well.
*    NumBytes: Number of free bytes that can be used to store
*              options from the callback.
*/
typedef void (IP_DHCPS_GET_VENDOR_OPTION_FUNC)(unsigned IFaceId, IP_DHCPS_GET_VENDOR_OPTION_INFO* pInfo, U8** ppOption, unsigned NumBytes);

int  IP_DHCPS_ConfigDNSAddr           (unsigned IFIndex, U32 *paDNSAddr, U8 NumServers);
int  IP_DHCPS_ConfigGWAddr            (unsigned IFIndex, U32 GWAddr);
int  IP_DHCPS_ConfigMaxLeaseTime      (unsigned IFIndex, U32 Seconds);
int  IP_DHCPS_ConfigPool              (unsigned IFIndex, U32 StartIPAddr, U32 SNMask, U32 PoolSize);
void IP_DHCPS_Halt                    (unsigned IFIndex);
int  IP_DHCPS_Init                    (unsigned IFIndex);
int  IP_DHCPS_SetReservedAddresses    (unsigned IFIndex, const IP_DHCPS_RESERVE_ADDR* paAddr, unsigned NumAddr);
void IP_DHCPS_SetVendorOptionsCallback(IP_DHCPS_GET_VENDOR_OPTION_FUNC* pf);
int  IP_DHCPS_Start                   (unsigned IFIndex);

/*********************************************************************
*
*       IP_BOOTPC_...
*
*  BOOTP - bootstrap Protocol
*/
int IP_BOOTPC_Activate(int IFaceId);
//
// We do not need extra functions, since we have the compatible functions with IP_DHCPC_ prefix
//
#define IP_BOOTPC_Halt(IFIndex)                                     IP_DHCPC_Halt(IFIndex)
#define IP_BOOTPC_SetTimeout(IFIndex,Timeout,MaxTries,Exponential)  IP_DHCPC_SetTimeout(IFIndex,Timeout,MaxTries,1)

/*********************************************************************
*
*       IP_ACD_...
*
*       Address conflict detection (ACD)
*/
typedef enum {
  IP_ACD_STATE_DISABLED = 0u,               // Disabled, ACD not used on this interface.
  //
  // Initial probing.
  //
  IP_ACD_STATE_MODE_INIT_BEGIN,             // --- Separator for easier state checking, not used as state. Start of a section.
  IP_ACD_STATE_INIT_WAIT_BEFORE_PROBE,      // Delay before probing to avoid that multiple hosts start at the same time.
  IP_ACD_STATE_INIT_WAIT_FOR_COLLISION,     // A probe has been sent. Waiting for the probe timeout to expire without collision.
  IP_ACD_STATE_INIT_WAIT_BEFORE_ANNOUNCE,   // Probing told us that the address is free to use. Start delay before announcing it to the network.
                                            // This is actually a pretty confusing state to be in as the time to wait for a collision
                                            // on the last probe sent is no longer the same time as between all other probes but is
                                            // now a different delay that does not stack with the delay previously used.
  IP_ACD_STATE_MODE_INIT_END,               // --- Separator for easier state checking, not used as state. End of a section.
  //
  // Announcing the new IP.
  //
  IP_ACD_STATE_MODE_ANNOUNCE_BEGIN,         // --- Separator for easier state checking, not used as state. Start of a section.
  IP_ACD_STATE_ANNOUNCE_SEND_GARP,          // Announce to the network that we are now using the address via a gratuituous ARP.
  IP_ACD_STATE_MODE_ANNOUNCE_END,           // --- Separator for easier state checking, not used as state. End of a section.
  //
  // Active (continuous background) probing.
  //
  IP_ACD_STATE_MODE_ACTIVE_BEGIN,           // --- Separator for easier state checking, not used as state. Start of a section.
  IP_ACD_STATE_ACTIVE_SEND_BG_PROBES,       // Active background probing, sending periodic probe to stimulate/test the network.
  IP_ACD_STATE_MODE_ACTIVE_END,             // --- Separator for easier state checking, not used as state. End of a section.
  //
  // Passive, waiting for collision.
  //
  IP_ACD_STATE_MODE_PASSIVE_BEGIN,          // --- Separator for easier state checking, not used as state. Start of a section.
  IP_ACD_STATE_PASSIVE_WAIT_FOR_COLLISION,  // Passively waiting for a collision reported via the ARP->ACD callback. No active background probing.
  IP_ACD_STATE_MODE_PASSIVE_END,            // --- Separator for easier state checking, not used as state. End of a section.
  //
  // Events.
  //
  IP_ACD_STATE_MODE_EVENT_BEGIN,            // --- Separator for easier state checking, not used as state. Start of a section.
  IP_ACD_STATE_COLLISION,                   // A collision was detected. Check pInfo of the callback for more information.
  IP_ACD_STATE_MODE_EVENT_END               // --- Separator for easier state checking, not used as state. End of a section.
} IP_ACD_STATE;

typedef enum {
  IP_ACD_KEEP_PACKET = 0u,
  IP_ACD_DISCARD_PACKET
} IP_ACD_KEEP_DISCARD_PACKET;

typedef enum {
  IP_ACD_LOSE_ADDRESS = 0u,  // Typically suggested to the OnInfo callback for collisions during INIT and AFTER-INIT collisions if they are not the first (see RFC 5227 section 2.4c).
  IP_ACD_DEFEND_ADDRESS      // Typically suggested to the OnInfo callback for the first AFTER-INIT collision with a host.
} IP_ACD_LOSE_DEFEND_ADDRESS;

/*********************************************************************
*
*       IP_ACD_ANNOUNCE_INFO
*
*  Function description
*    Returns information about the latest ACD announce about using
*    a free and previously probed address.
*/
typedef struct {
  unsigned AnnouncementsLeft;  // Number of announements left to send.
} IP_ACD_ANNOUNCE_INFO;

/*********************************************************************
*
*       IP_ACD_COLLISION_INFO
*
*  Function description
*    Returns information about the latest ACD collision.
*/
typedef struct {
  IP_PACKET* pPacket;        // Pointer to the packet that caused the collision (pPacket->pData points to the ARP header).
  U32        DefendTimeout;  // System timestamp of when the defend window ends.
  unsigned   ProbesLeft;     // Number of INIT probes left to send.
} IP_ACD_COLLISION_INFO;

/*********************************************************************
*
*       IP_ACD_WAIT_INFO
*
*  Function description
*    Returns information about a delay/wait before the next step.
*    This can be a delay before sending the very first probe for INIT
*    or a delay between each probe sent during the INIT phase.
*
*  Additional information
*    The stack makes suggestions using the structure members as well
*    as presenting the actual value that will be used in the \c{WaitTime}
*    member. You can overwrite the \c{WaitTime} member as it is then
*    evaluated after returning from the callback and its new value
*    value is then used.
*/
typedef struct {
  unsigned WaitMin;   // Suggested minimum wait time [ms].
  unsigned WaitTime;  // Wait time before the next state that is used (does not have to obey min./max. suggestion).
                      // This value can be overwritten and is evaluated after returning from the callback.
  unsigned WaitMax;   // Suggested maximum wait time [ms]
} IP_ACD_WAIT_INFO;

/*********************************************************************
*
*       IP_ACD_INFO
*
*  Function description
*    Returns information about the current ACD status. The ACD info
*    callback parameter \c{State} has to be evaluated for further
*    information if there is more info about the new state and what
*    part of the union is the information to look at.
*/
typedef struct {
  U32                        IPAddr;                // IPv4 address (in host endianness) that gets assigned to the interface or would be assigned to the interface if IP_ACD_EX_CONFIG.AssignAddressManually is NOT used.
                                                    // Currently only valid with the IP_ACD_STATE_INIT_WAIT_BEFORE_ANNOUNCE state.
  IP_ACD_STATE               State;                 // Type of information and what part of the union to look at. The \c{State} member
                                                    // is followed by a union that might not be correctly displayed or completely missing
                                                    // in the manual. The following information describes this union part:
                                                    // * IP_ACD_STATE_EVENT_COLLISION: Information about the latest ACD collision can be found in pInfo->Data.Collision in form of IP_ACD_COLLISION_INFO .
  IP_ACD_STATE               OldState;              // Previous state. Might be the same as new state depending on actions executed in callbacks. If filtering is needed, this needs to be implemented in the application.
  IP_ACD_LOSE_DEFEND_ADDRESS Defend;                // Suggestion from the stack whether to defend the IP address on a collision after INIT or not.
                                                    // This value is evaluated after returning from the callback.
                                                    // * == IP_ACD_LOSE_ADDRESS  : Lose the address (typically if this is not the first conflict with a host and is within the defend window.
                                                    // * == IP_ACD_DEFEND_ADDRESS: Defend the address (anyhow).
  IP_ACD_KEEP_DISCARD_PACKET DiscardPacket;         // Suggestion from the stack whether to keep or discard a packet contained in the state specific information structure (e.g. IP_ACD_COLLISION_INFO).
                                                    // This value is evaluated after returning from the callback.
                                                    // * == IP_ACD_KEEP_PACKET   : Packet is kept and forwarded to the ARP module for further handling.
                                                    // * == IP_ACD_DISCARD_PACKET: Packet is discarded (e.g. to avoid ARP cache poisoning).
  union {
    IP_ACD_ANNOUNCE_INFO  Announce;
    IP_ACD_COLLISION_INFO Collision;
    IP_ACD_WAIT_INFO      Wait;
  } Data;
} IP_ACD_INFO;

/*********************************************************************
*
*       IP_ACD_ON_INFO_FUNC
*
*  Function description
*    Callback executed whenever updated ACD information is available.
*
*  Parameters
*    IFaceId : Zero-based interface index.
*    pInfo   : Further information of type IP_ACD_INFO about the actual
*              information available.
*
*  Additional information
*    Calling API like an ACTIVATE from the callback might produce
*    another callback message. It is the responsibility of the application
*    to avoid infinite recursion. Typically this is no problem as calling
*    ACTIVATE again from the callback reporting the ACTIVATE state makes
*    no sense.
*/
typedef void (IP_ACD_ON_INFO_FUNC)(unsigned IFaceId, IP_ACD_INFO* pInfo);

typedef struct {
  U32 (*pfRenewIPAddr)(unsigned IFaceId);  // Used to renew the IP address if a conflict has been detected during startup. Return value: New IP addr. to try.
  int (*pfDefend)     (unsigned IFaceId);  // Used to defend the IP address against a conflicting host on the network.
  int (*pfRestart)    (unsigned IFaceId);  // Used to restart the address conflict detection.
} ACD_FUNC;

/*********************************************************************
*
*       IP_ACD_EX_CONFIG
*
*  Function description
*    Used to configure the extended ACD functionality.
*/
typedef struct {
  U32      IPAddr;                 // IPv4 start address to use in host endianness.
  unsigned BackgroundPeriod;       // Period [ms] in which ACD will send probes running in the background.
  unsigned NumProbes;              // Number of ARP probes to send upon activating ACD before declaring the actual used IP address to be free to be used. 0 to use default.
  unsigned DefendInterval;         // Interval [ms] in which the currently active IP address is being known as defended after taking action. 0 to use default.
  unsigned NumAnnouncements;       // Number of announcements to send when using a free address. The address can already be used at this point. 0 to use default.
  unsigned AnnounceInterval;       // Time [ms] between announcements to send. 0 to use default.
  U8       AssignAddressManually;  // Configures if probed address is assigned automatically to the interface if free.
                                   // * 0: Off (default), address is automatically to the interface, using the existing subnet mask.
                                   // * 1: On, address is only reported via the IP_ACD_INFO.IPAddr member in the IP_ACD_STATE_INIT_WAIT_BEFORE_ANNOUNCE state.
                                   //      The user needs to manually assign it to the interface along with the desired subnet mask.
                                   //      Assigning an address manually might affect ACD effectiveness on virtual interfaces such as being used for multiple
                                   //      addresses on one single physical interface. ARP/ACD might not be able to correctly select the virtual interface
                                   //      for some operations until the address has finally been assigned to the interface.
} IP_ACD_EX_CONFIG;

int  IP_ACD_Activate  (unsigned IFaceId);
int  IP_ACD_ActivateEx(unsigned IFaceId, IP_ACD_ON_INFO_FUNC* pfOnInfo, const IP_ACD_EX_CONFIG* pConfig, unsigned NonBlocking);
int  IP_ACD_Config    (unsigned IFaceId, unsigned NumProbes, unsigned DefendInterval, const ACD_FUNC* pAPI);
void IP_ACD_Halt      (unsigned IFaceId);

/*********************************************************************
*
*       IP_TIMER_...
*
*       Software timers used within the stack.
*/
typedef enum {
  IP_TIMER_MODE_SYNC = 0,  // Try to keep the timer in sync by adding up period until the timeout is in the future.
  IP_TIMER_MODE_NEXT,      // Reload the timer old timeout + period. If this has already been passed, period is added once.
  IP_TIMER_MODE_PERIOD,    // Reload the timer with NOW + period value for a known execution time.
} IP_TIMER_MODE;

typedef struct IP_TIMER_STRUCT IP_TIMER;
struct IP_TIMER_STRUCT {
  IP_TIMER*     pNext;
  void (*pfHandler)(void* pContext);
  void*         pContext;
  U32           Period;
  U32           NextTime;
  IP_TIMER_MODE Mode;
};

/*********************************************************************
*
*       IP_PTP_
*/
#define IP_PTP_FLAGS_SIGNED_MASK             (1uL << 0)  // Negative correction.
#define IP_PTP_FLAGS_COARSE_CORRECTION_MASK  (1uL << 1)  // (Coarse) time correction with calculated offset.
#define IP_PTP_FLAGS_FINE_CORRECTION_MASK    (1uL << 2)  // Fine/drift frequency correction. Not supported by all drivers.
#define IP_PTP_FLAGS_FINE_RESET_MASK         (1uL << 3)  // Fine correction history values used for average calculation are reset.
#define IP_PTP_FLAGS_SET_TIME_MASK           (1uL << 4)  // Set the time (if necessary with a positive coarse correction).

typedef enum {
  PTP_STATE_INITIALIZING   = 1,
  PTP_STATE_FAULTY,
  PTP_STATE_DISABLED,
  PTP_STATE_LISTENING,
  PTP_STATE_PRE_MASTER,
  PTP_STATE_MASTER,
  PTP_STATE_PASSIVE,
  PTP_STATE_UNCALIBRATED,
  PTP_STATE_SLAVE
} PTP_STATE;

typedef struct IP_PTP_CONTEXT_STRUCT     IP_PTP_CONTEXT;
typedef struct IP_PTP_INT_CONTEXT_STRUCT IP_PTP_INT_CONTEXT;
typedef struct PTP_HEADER_STRUCT         PTP_HEADER;

#define IP_PTP_INFO_TYPE_CORRECTION       1u  // Typically provided from (simple internal PTP hardware mixed with) our PTP software algorithm where we have full control.
#define IP_PTP_INFO_TYPE_OFFSET           2u  // Typically provided from external PTP hardware where we are only able to receive a subset of information.
#define IP_PTP_INFO_TYPE_MASTER_CHANGED   3u
#define IP_PTP_INFO_TYPE_MASTER_UPDATED   4u
#define IP_PTP_INFO_TYPE_MASTER_RESET     5u  // Now serving as our own master, either by initial state or by fallback as no better master is available.

/*********************************************************************
*
*       IP_PTP_CORRECTION_INFO
*
*  Function description
*    Returns information about the latest local time correction.
*/
typedef struct {
  U32 Seconds;      // Seconds
  U32 Nanoseconds;  // Nanoseconds
  U32 Flags;        // ORR-ed combination of IP_PTP_FLAGS_*
  U8  State;        // State before applying the correction.
} IP_PTP_CORRECTION_INFO;

/*********************************************************************
*
*       IP_PTP_OFFSET_INFO
*
*  Function description
*    Returns information about the offset between slave and master.
*/
typedef struct {
  U32 Seconds;      // Seconds
  U32 Nanoseconds;  // Nanoseconds
  U32 Flags;        // ORR-ed combination of IP_PTP_FLAGS_*
} IP_PTP_OFFSET_INFO;

/*********************************************************************
*
*       IP_PTP_MASTER_INFO
*
*  Function description
*    Returns information about a newly selected master or updated
*    parameters with the currently selected one.
*/
typedef struct {
  U8  abGrandmasterIdentity[8];  // Master clock ID.
  U32 UtcOffset;                 // Offset between TAI and UTC in seconds.
  U8  IsUtcOffsetValid;          // Is the offset valid ? Basically this means, has this offset been updated/set by a master ?
} IP_PTP_MASTER_INFO;

/*********************************************************************
*
*       IP_PTP_INFO
*
*  Function description
*    Returns information about the current PTP status. The \c{Type}
*    field has to be evaluated for further information about what part
*    of the union is the information to look at.
*/
typedef struct {
  IP_PTP_CONTEXT*  pContext;            // PTP context that this information originates from.
  U8               Type;                // Type of information and what part of the union to look at. The \c{Type} member
                                        // is followed by a union that might not be correctly displayed or completely missing
                                        // in the manual. The following information describes this union part:
                                        // * IP_PTP_INFO_TYPE_CORRECTION    : Information about the latest local time correction can be found in pInfo->Correction in form of IP_PTP_CORRECTION_INFO .
                                        // * IP_PTP_INFO_TYPE_OFFSET        : Information about the offset between slave and master can be found in pInfo->Offset in form of IP_PTP_OFFSET_INFO .
                                        // * IP_PTP_INFO_TYPE_MASTER_CHANGED: Information about a newly selected master can be found in pInfo->Master or pInfo->MasterNewOld.New in form of IP_PTP_MASTER_INFO .
                                        //                                    Information about the previous selected master can be found in pInfo->MasterNewOld.Old in form of IP_PTP_MASTER_INFO .
                                        // * IP_PTP_INFO_TYPE_MASTER_UPDATED: Information about parameter updates for the currently selected master can be found in pInfo->Master or pInfo->MasterNewOld.New in form of IP_PTP_MASTER_INFO .
                                        // * IP_PTP_INFO_TYPE_MASTER_RESET  : Reset to or initial start being our own master. Information can be retrieved in the same way as for IP_PTP_INFO_TYPE_MASTER_CHANGED .
                                        //                                    pInfo->Master or pInfo->MasterNewOld.New typically contains our own DefaultDS ClockIdentity. pInfo->MasterNewOld.Old contains either the
                                        //                                    ClockIdentity of the previously selected master (the old master and no other suitable is available, so we fall back to being our own master)
                                        //                                    or is all zero bytes if the first setup is applied during the init/start phase.
  union {
    IP_PTP_CORRECTION_INFO Correction;
    IP_PTP_OFFSET_INFO     Offset;
    IP_PTP_MASTER_INFO     Master;
    struct {
      IP_PTP_MASTER_INFO   New;
      IP_PTP_MASTER_INFO   Old;
    } MasterNewOld;
  } Data;
} IP_PTP_INFO;

/*********************************************************************
*
*       IP_PTP_ON_INFO_FUNC
*
*  Function description
*    Callback executed whenever updated PTP information is available.
*
*  Parameters
*    IFaceId : Zero-based interface index.
*    pInfo   : Further information of type IP_PTP_INFO about the actual
*              information available.
*/
typedef void (IP_PTP_ON_INFO_FUNC)(unsigned IFaceId, IP_PTP_INFO* pInfo);

typedef struct {
  U8 Dummy;  // Placeholder for now. Required to be able to add further PTP profiles based on this API in the future.
} IP_PTP_PROFILE;

extern const IP_PTP_PROFILE IP_PTP_Profile_1588_2008;

typedef enum {
  IP_PTP_ENDPOINT_TYPE_MASTER = 0u,
  IP_PTP_ENDPOINT_TYPE_SLAVE,
  IP_PTP_ENDPOINT_TYPE_MAX
} IP_PTP_ENDPOINT_TYPE;

/*********************************************************************
*
*       IP_PTP_PROT_TYPE
*
*  Function description
*    The PTP protocol type can be used to override the default of the
*    system which is typically IPv4/UDP .
*
*  Additional information
*    The protocol type is typically set automatically based on the
*    last PTP message received. For a PTP Slave this means that upon
*    receiving a SYNC for example, the DELAY-REQ that is then sent
*    by the Slave will use the same type.
*
*    For a Master the protocol type can be set as there will be no
*    previous message being received when sending its SYNC or ANNOUNCE
*    message.
*/
typedef enum {
  IP_PTP_PROT_TYPE_DEFAULT = (0u),  // Use the system default which is typically IP_PTP_PROT_TYPE_UDP_IPV4 .
  IP_PTP_PROT_TYPE_UDP_IPV4,        // Use PTP over Layer 3 IPv4/UDP protocol.
  IP_PTP_PROT_TYPE_UDP_IPV6,        // Use PTP over Layer 3 IPv6/UDP protocol.
  IP_PTP_PROT_TYPE_ETHERNET         // Use PTP over Layer 2 Ethernet protocol.
} IP_PTP_PROT_TYPE;

/*********************************************************************
*
*       IP_PTP_MASTER_PARAMS
*
*  Function description
*    Configuration parameters for a PTP Master.
*
*  Additional information
*    Logarithmic time parameters are given as exponent of a two's
*    base of a second (log2) with a range between -128 and +127.
*    Examples:
*      * LogSyncInterval ==  0: Send a SYNC every second.
*      * LogSyncInterval ==  1: Send a SYNC every two seconds.
*      * LogSyncInterval == -2: Send a SYNC every 250 milliseconds.
*/
typedef struct {
  IP_PTP_PROT_TYPE Prot;      // Primary protocol to use when sending ANNOUNCE/SYNC messages.
                              // Default: IP_PTP_PROT_TYPE_UDP_IPV4 .
  I8 LogAnnounceInterval;     // Log2 interval in which ANNOUNCE messages are sent.
                              // For the moment the smaller value of the parameters
                              // LogAnnounceInterval and LogSyncInterval is used for
                              // both, ANNOUNCE and SYNC messages.
                              // Default: 0 (1 second).
  I8 LogSyncInterval;         // Log2 interval in which SYNC messages are sent.
                              // For the moment the smaller value of the parameters
                              // LogAnnounceInterval and LogSyncInterval is used for
                              // both, ANNOUNCE and SYNC messages.
                              // Default: 0 (1 second).
  I8 LogMinDelayReqInterval;  // Log2 interval to report to the SLAVE in a DELAY-RESP
                              // message when to check the DELAY between Slave and Master.
                              // Default: 0 (1 second).
} IP_PTP_MASTER_PARAMS;

typedef void (IP_PTP_ENDPOINT_HANDLE_MSG_FUNC)  (IP_PTP_INT_CONTEXT* pContext, IP_PACKET* pPacket, PTP_HEADER* pPTPHeader, IP_PTP_ENDPOINT* pEndpoint);
typedef void (IP_PTP_ENDPOINT_ON_TS_UPDATE_FUNC)(IP_PTP_ENDPOINT* pEndpoint  , IP_PACKET* pPacket);

struct IP_PTP_ENDPOINT_STRUCT {
  IP_PTP_ENDPOINT*                   pNext;         // Used for linked wait for Tx timestamp list.
  IP_PACKET*                         pPacket;       // Tx packet for which we are waiting to get a timestamp.
  IP_PTP_CONTEXT*                    pContext;      // PTP context this endpoint has been assigned to.
  IP_PTP_ENDPOINT_ON_TS_UPDATE_FUNC* pfOnTsUpdate;  // Callback to notify about the timestamp in pPacket has been updated.
  IP_PTP_ENDPOINT_HANDLE_MSG_FUNC*   pfHandleMsg;   // Non NULL means that the endpoint is available.
  IP_TIMER                           Timer;
  IP_PTP_MASTER_PARAMS               Params;
  IP_PTP_ENDPOINT_TYPE               Type;
  U8                                 InitDone;
};

typedef IP_PTP_ENDPOINT IP_PTP_MASTER;
typedef IP_PTP_ENDPOINT IP_PTP_SLAVE;

struct IP_PTP_CONTEXT_STRUCT {
  IP_PTP_ENDPOINT* apEndpoint[IP_PTP_ENDPOINT_TYPE_MAX];  // For the moment only an OrdinaryClock is supported which means
                                                          // a context has to manage a maximum of 1 master and 1 slave.
                                                          // Once more complex configurations like 1 slave + n master (for
                                                          // separate networks) are required, this needs to become a pFirst
                                                          // linked list.
  unsigned         IFaceId;                               // To be moved into the endpoint once we support multiple configurations/interfaces.
};

int  IP_PTP_GetTime                  (IP_PTP_TIMESTAMP* pPTPTimestamp);
U8*  IP_PTP_GetDefaultDsClockIdentity(IP_PTP_CONTEXT* pContext, U8* pBuffer, unsigned NumBytes);
int  IP_PTP_Halt                     (IP_PTP_CONTEXT* pContext);
int  IP_PTP_Init                     (IP_PTP_CONTEXT* pContext, const IP_PTP_PROFILE* pProfile);
int  IP_PTP_SetTime                  (IP_PTP_TIMESTAMP* pPTPTimestamp);
int  IP_PTP_Start                    (IP_PTP_CONTEXT* pContext);
void IP_PTP_OC_AddMasterFallbackLogic(IP_PTP_MASTER* pMaster);
void IP_PTP_OC_AddSlaveFallbackLogic (IP_PTP_SLAVE* pSlave);
int  IP_PTP_OC_Start                 (unsigned IFaceId);
int  IP_PTP_OC_Halt                  (void);
void IP_PTP_OC_SetProductDescription (const char* pDesc);
void IP_PTP_OC_SetUserDescription    (const char* pDesc);
void IP_PTP_OC_SetRevision           (const char* pDesc);
void IP_PTP_OC_SetInfoCallback       (IP_PTP_ON_INFO_FUNC* pf);
void IP_PTP_MASTER_Add               (IP_PTP_CONTEXT* pContext, IP_PTP_MASTER* pMaster, unsigned IFaceId);
void IP_PTP_MASTER_Config            (IP_PTP_MASTER* pMaster, IP_PTP_MASTER_PARAMS* pParams);
void IP_PTP_MASTER_Remove            (IP_PTP_MASTER* pMaster);
void IP_PTP_SLAVE_Add                (IP_PTP_CONTEXT* pContext, IP_PTP_SLAVE*  pSlave , unsigned IFaceId);

extern const IP_PTP_DRIVER IP_PTP_Driver_Software;

/*********************************************************************
*
*       IP_NTP_
*/
int  IP_NTP_CLIENT_Start             (void);
int  IP_NTP_CLIENT_Halt              (unsigned ClearUserDefined);
int  IP_NTP_CLIENT_Run               (void);
int  IP_NTP_CLIENT_AddServerPool     (unsigned IFaceId, const char* sPool);
void IP_NTP_CLIENT_FavorLocalClock   (unsigned OnOff);
void IP_NTP_CLIENT_AddServerClock    (unsigned IFaceId, U32 IPAddr);
void IP_NTP_CLIENT_AddServerClockIPv6(unsigned IFaceId, U8* pIPAddr);
int  IP_NTP_CLIENT_ResetAll          (void);
int  IP_NTP_GetTimestamp             (IP_NTP_TIMESTAMP* pTimestamp);
#ifdef U64
U64  IP_NTP_GetTime                  (int* pStatus);
#endif

/*********************************************************************
*
*       IP_Show_...
*
*  Text output functions informing about the state of various components of the software
*/
int IP_ShowARP       (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowICMP      (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowIGMP      (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowTCP       (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowSocketList(void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowStat      (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowUDP       (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowUDPSockets(void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowDHCPClient(void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowDNS       (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowDNS1      (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowRAW       (void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);
int IP_ShowRAWSockets(void (*pfSendf)(void * pContext, const char * sFormat, ...), void * pContext);

typedef struct {
  int hSock;
  char * pBuffer;
  int BufferSize;
} IP_SENDF_CONTEXT;

void IP_Sendf(void * pContext, const char * sFormat, ...);

/*********************************************************************
*
*       IP_INFO
*/
//
// Buffer information
//
enum {
  IP_INFO_SMALL_BUFFERS_CONFIG = 0,
  IP_INFO_SMALL_BUFFERS_AVAIL,
  IP_INFO_SMALL_BUFFERS_SIZE,
  IP_INFO_SMALL_BUFFERS_USED_MAX,
  IP_INFO_BIG_BUFFERS_CONFIG,
  IP_INFO_BIG_BUFFERS_AVAIL,
  IP_INFO_BIG_BUFFERS_SIZE,
  IP_INFO_BIG_BUFFERS_USED_MAX
};

int  IP_INFO_GetBufferInfo(int InfoIndex);
void IP_INFO_ResetBufferStats(void);

//
// Connection information
//

typedef void * IP_CONNECTION_HANDLE;
typedef struct {
// Socket info
  void *pSock;
  U16   hSock;
  U8    Type;
  U8    Proto;
  U16   Options;
  U16   BackLog;
// Addr/port info
  U32   ForeignAddr;
  U32   LocalAddr;
  U16   ForeignPort;
  U16   LocalPort;
// TCP Info
  U8    TcpState;
  U16   TcpMtu;
  U16   TcpMss;
  U32   TcpRetransDelay;
  U32   TcpIdleTime;
  U32   RxWindowCur;
  U32   RxWindowMax;
  U32   TxWindow;
} IP_CONNECTION;

typedef struct {
  const char * sTypeName;
  U8  AdminState;
  U8  HWState;
  U8  TypeIndex;
  U32 Speed;
} IP_INFO_INTERFACE;


      int    IP_INFO_GetConnectionInfo     (IP_CONNECTION_HANDLE h, IP_CONNECTION * p);
      int    IP_INFO_GetConnectionList     (IP_CONNECTION_HANDLE *pDest, int MaxItems);
const char * IP_INFO_ConnectionState2String(U8 State);
      int    IP_INFO_GetSocketInfo         (int hSock, IP_CONNECTION * p);

      int    IP_INFO_GetNumInterfaces      (void);
      void   IP_INFO_GetInterfaceInfo      (unsigned IFaceId, IP_INFO_INTERFACE * pInterfaceInfo);

/*********************************************************************
*
*       IP_STATS
*/
typedef struct {
  U32 LastLinkStateChange;  // SNMP: ifLastChange [TimeTicks]. Needs to be converted into in 1/100 seconds since SNMP epoch.
  U32 RxBytesCnt;           // SNMP: ifInOctets [Counter].
  U32 RxUnicastCnt;         // SNMP: ifInUcastPkts [Counter].
  U32 RxNotUnicastCnt;      // SNMP: ifInNUcastPkts [Counter].
  U32 RxDiscardCnt;         // SNMP: ifInDiscards [Counter].
  U32 RxErrCnt;             // SNMP: ifInErrors [Counter].
  U32 RxUnknownProtoCnt;    // SNMP: ifInUnknownProtos [Counter].
  U32 TxBytesCnt;           // SNMP: ifOutOctets [Counter].
  U32 TxUnicastCnt;         // SNMP: ifOutUcastPkts [Counter].
  U32 TxNotUnicastCnt;      // SNMP: ifOutNUcastPkts [Counter].
  U32 TxDiscardCnt;         // SNMP: ifOutDiscards [Counter].
  U32 TxErrCnt;             // SNMP: ifOutErrors [Counter].
} IP_STATS_IFACE;

void            IP_STATS_EnableIFaceCounters   (unsigned IFaceId);
IP_STATS_IFACE* IP_STATS_GetIFaceCounters      (unsigned IFaceId);
U32             IP_STATS_GetLastLinkStateChange(unsigned IFaceId);
U32             IP_STATS_GetRxBytesCnt         (unsigned IFaceId);
U32             IP_STATS_GetRxDiscardCnt       (unsigned IFaceId);
U32             IP_STATS_GetRxErrCnt           (unsigned IFaceId);
U32             IP_STATS_GetRxNotUnicastCnt    (unsigned IFaceId);
U32             IP_STATS_GetRxUnicastCnt       (unsigned IFaceId);
U32             IP_STATS_GetRxUnknownProtoCnt  (unsigned IFaceId);
U32             IP_STATS_GetTxBytesCnt         (unsigned IFaceId);
U32             IP_STATS_GetTxDiscardCnt       (unsigned IFaceId);
U32             IP_STATS_GetTxErrCnt           (unsigned IFaceId);
U32             IP_STATS_GetTxNotUnicastCnt    (unsigned IFaceId);
U32             IP_STATS_GetTxUnicastCnt       (unsigned IFaceId);

/*********************************************************************
*
*       Profiling instrumentation functions
*/
void IP_SYSVIEW_Init(void);

/*********************************************************************
*
*       High precision timing functions
*/
#ifdef U64
void IP_SetMicrosecondsCallback(U64 (*pfGetTime_us)(void));
void IP_SetNanosecondsCallback (U64 (*pfGetTime_ns)(void));
#endif

/*********************************************************************
*
*       Compatibility
*
*  Various defines to map obsolete function names to new ones
*/
#define IP_TCP_GetMTU      IP_GetMTU
#define IP_SetDefaultTTL   IP_SetTTL
#define IP_TCP_SetMTU      IP_SetMTU
#define IP_UDP_CONN        IP_UDP_CONN_HANDLE
#define IP_DNSC_SetMaxTTL  IP_DNS_SetMaxTTL


#if defined(__cplusplus)
  }              // Make sure we have C-declarations in C++ programs
#endif

#if IP_SUPPORT_IPV6
#include "IPV6_IPv6.h"
#endif

#endif   // Avoid multiple inclusion

/*************************** End of file ****************************/
