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

File    : IPV6_IPv6.h
Purpose : IPv6 header file.
*/

#ifndef IPV6_IPV6_H
#define IPV6_IPV6_H                // Avoid multiple/recursive inclusion

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif


/*********************************************************************
*
*       Defines
*
*  List of options which are enabled by default on an interface
*  which uses IPv6.
*/
#define IPV6_GENERATE_LINK_LOCAL_ADDR      (1u << 0)     // Generate a link-local address fe80::/64
#define IPV6_ICMPV6_MLD_ADD_DEF_ADDR       (1u << 1)     // Add multicast addresses ALL-Nodes and All-Router to the interface
#define IPV6_USE_SLAAC                     (1u << 2)     // Use Stateless Address Autoconfiguration (SLAAC)
#define IPV6_USE_ROUTER_ADVERTISMENTS      (1u << 3)     // Use configuration options supplied through Router Advertisements.
#define IPV6_ENABLE_DEFAULTS               (0xFFFFFFFFu) // Mask to keep sure that the default configuration can be easily restored.

#define IP_ICMPV6_TYPE_ECHO_REQUEST        (128u)
#define IP_ICMPV6_TYPE_ECHO_REPLY          (129u)

/*********************************************************************
*
*       Typedefs
*/
typedef struct {
// Socket info
  void*        pSocket;          // Pointer to socket structure
  U16          hSocket;          // Socket handle
  U8           Type;             // Socket type
  U8           Proto;            // Protocol number
  U16          Options;          // Socket options
  U16          BackLog;          // Backlog of listening sockets
// Addr/port info
  IPV6_ADDR    ForeignAddr;
  IPV6_ADDR    LocalAddr;
  U16          ForeignPort;
  U16          LocalPort;
// TCP Info
  U16          TcpMtu;           // MTU of the connection
  U16          TcpMss;           // MSS of the connection
  U32          TcpRetransDelay;  // TCP retransmission delay
  U32          TcpIdleTime;      // TCP idle time
  U32          RxWindowCur;      // Current Rx window size
  U32          RxWindowMax;      // Maximum Rx window size
  U32          TxWindow;         // Tx window size
  U8           TcpState;         // TCP state
// Maintenance info
  U8           AddrFam;          // Address family (4/6)
} IP_IPV6_CONNECTION;


/*********************************************************************
*
*       Public API hooks
*/

/*********************************************************************
*
*       IP_ON_ICMPV6_FUNC
*
*  Function description
*    Callback executed when an ICMPv6 packet is received.
*
*  Parameters
*    IFaceId     : Zero-based interface index.
*    pPacket     : Packet that has been received.
*    pUserContext: User context given when adding the hook.
*    p           : Reserved for future extensions of this API.
*
*  Return value
*    == IP_OK                  : Packet has been handled (freed or reused).
*    == IP_OK_TRY_OTHER_HANDLER: Packet is untouched and stack shall try another handler.
*
*  Additional information
*    The callback can remove its own hook using IP_ICMPV6_RemoveRxHook() .
*/
typedef int (IP_ON_ICMPV6_FUNC)(unsigned IFaceId, IP_PACKET* pPacket, void* pUserContext, void* p);

typedef struct IP_HOOK_ON_ICMPV6_STRUCT IP_HOOK_ON_ICMPV6;
struct IP_HOOK_ON_ICMPV6_STRUCT {
  IP_HOOK_ON_ICMPV6* pNext;         // Pointer to the next hook.
  IP_ON_ICMPV6_FUNC* pf;            // Pointer to the function to be called by the hook.
  void*              pUserContext;  // User context set when adding the hook.
};

/*********************************************************************
*
*       IPv6 API functions
*/

//
// General API functions
//
void                IP_IPV6_Add                                (unsigned IFaceId);
int                 IP_IPV6_AddUnicastAddr                     (unsigned IFaceId, const U8* pAddr);
void                IP_IPV6_ChangeDefaultConfig                (unsigned IFaceId, unsigned Option, unsigned OnOff);
void                IP_IPV6_GetIPv6Addr                        (unsigned IFaceId, U8 AddrId, IPV6_ADDR* pIPv6Addr, U8* pNumAddr);
const U8*           IP_IPV6_GetIPPacketInfo                    (IP_PACKET* pPacket);
int                 IP_IPV6_IFaceIsReady                       (unsigned IFaceId);
int                 IP_IPV6_ParseIPv6Addr                      (const char* pIn, IPV6_ADDR* pIPv6Addr);
int                 IP_IPV6_ResolveHost                        (unsigned IFaceId, const char* sHost, IPV6_ADDR* pIPv6Addr, U32 ms);
int                 IP_IPV6_SetDefHopLimit                     (unsigned IFaceId, U8 HopLimit);
int                 IP_IPV6_SetGateway                         (unsigned IFaceId, const char* sIFaceAddr,  const char* sRouterAddr);
void                IP_IPV6_SetLinkLocalUnicastAddr            (unsigned IFaceId);


//
// IP Network Discovery Protocol related functions
//
void                IP_ICMPV6_NDP_SetDNSSLCallback             (unsigned IFaceId, void (*pfHandleDNSSLOpt)(unsigned IFaceId, U8* pData, unsigned NumBytes, U32 Lifetime));


//
// IP DNS server address
//
IPV6_ADDR*          IP_IPV6_DNS_GetServer                      (unsigned IFaceId, unsigned DNSIndex);
int                 IP_IPV6_DNS_SetServer                      (unsigned IFaceId, const char* sDNSServerAddr);
int                 IP_IPV6_DNS_SendDynUpdate                  (unsigned IFaceId, const char* sHost, const char* sDomain, U32 IPv4Addr, U8* pIPv6Addr, int ClearPreviousRR, U32 ms);


//
// Multicast Listener related API functions
//
int                 IP_ICMPV6_MLD_AddMulticastAddr             (unsigned IFaceId, IPV6_ADDR* pMultiCAddr);
int                 IP_ICMPV6_MLD_RemoveMulticastAddr          (unsigned IFaceId, IPV6_ADDR* pMultiCAddr);


//
// Internet Control Message Protocol version 6 related API functions
//
void                IP_ICMPV6_AddRxHook                        (IP_HOOK_ON_ICMPV6* pHook, IP_ON_ICMPV6_FUNC* pf, void* pUserContext);
int                 IP_ICMPV6_SendEchoReq                      (unsigned IFaceId, IPV6_ADDR* pDestAddr, char* pData, unsigned NumBytes, U16 Id, U16 SeqNum);
void                IP_ICMPV6_RemoveRxHook                     (IP_HOOK_ON_ICMPV6* pHook);
void                IP_ICMPV6_SetRxHook                        (IP_RX_HOOK* pf);

//
// TCP related API functions
//
void                IP_IPV6_TCP_Add                            (void);

//
// UDP related API functions
//
void                IP_IPV6_UDP_Add                            (void);
IP_UDP_CONNECTION*  IP_IPV6_UDP_AddEchoServer                  (U16 LPort);
IP_PACKET*          IP_IPV6_UDP_Alloc                          (unsigned NumBytesData);
IP_PACKET*          IP_IPV6_UDP_AllocEx                        (unsigned IFaceId, unsigned NumBytesIPHeader, unsigned NumBytesData);
void                IP_IPV6_UDP_Free                           (IP_PACKET* pPacket);
void                IP_IPV6_UDP_GetDestAddr                    (const IP_PACKET* pPacket, void* pDestAddr, int AddrLen);
void                IP_IPV6_UDP_GetSrcAddr                     (const IP_PACKET* pPacket, void* pSrcAddr, int AddrLen);
IP_UDP_CONNECTION*  IP_IPV6_UDP_OpenEx                         (IPV6_ADDR* pFAddr, U16 FPort, IPV6_ADDR* pLAddr, U16 LPort, int (*handler)(IP_PACKET* pPacket, void* pContext), void* pContext);
int                 IP_IPV6_UDP_Send                           (int IFaceId, IPV6_ADDR* pFHost, U16 FPort, U16 LPort, IP_PACKET* pPacket);
int                 IP_IPV6_UDP_SendAndFree                    (int IFaceId, IPV6_ADDR* pFHost, U16 FPort, U16 LPort, IP_PACKET* pPacket);



//
// Fragmentation.
//
#if IP_SUPPORT_FRAGMENTATION_IPV6
void                IP_IPV6_FRAGMENT_Enable                    (void);
void                IP_IPV6_FRAGMENT_ConfigRx                  (U16 MaxFragments, U32 Timeout, U8 KeepOOO);
#endif


//
// Info functions.
// Normally used to simplify the visualization of a connection in applications like a Web server.
//
int                 IP_IPV6_INFO_GetConnectionInfo            (IP_CONNECTION_HANDLE hConn, IP_IPV6_CONNECTION* pConInfo);


//
// UDP function related defines
// (To avoid code doubling and present a clean API with convenient name prefix.)
//
#define IP_IPV6_UDP_Close         IP_UDP_Close
#define IP_IPV6_UDP_FindFreePort  IP_UDP_FindFreePort
#define IP_IPV6_UDP_GetFPort      IP_UDP_GetFPort
#define IP_IPV6_UDP_GetLPort      IP_UDP_GetLPort
#define IP_IPV6_UDP_GetIFIndex    IP_UDP_GetIFIndex
#define IP_IPV6_UDP_GetDataPtr    IP_UDP_GetDataPtr
#define IP_IPV6_UDP_GetDataSize   IP_UDP_GetDataSize

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif

/****** End Of File *************************************************/
