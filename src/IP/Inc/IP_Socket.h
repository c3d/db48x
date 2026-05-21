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
File    : IP_Socket.h
Purpose : Berkeley socket compatible application programming interface,
          macros and defines.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef SOCKET_H                         // Avoid multiple inclusion
#define  SOCKET_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       #defines, configurable
*
**********************************************************************
*/

#ifndef   IP_BSD_COMPLIANCE
  #define IP_BSD_COMPLIANCE   0
#endif

#ifndef   IP_NO_BSD_SOCK_API
  #define IP_NO_BSD_SOCK_API  0
#endif

/*********************************************************************
*
*       #defines, fixed
*
**********************************************************************
*/

//
// Socket types
//
#define  SOCK_STREAM           (1)       // Stream socket
#define  SOCK_DGRAM            (2)       // Datagram socket
#define  SOCK_RAW              (3)       // RAW socket

//
// Socket options (that are kept as flags with the socket).
//
#define  IP_DONTFRAG           0x00001   // Set the "Don't fragment" flag for IPv4 UDP/RAW packets.
#define  SO_ACCEPTCONN         0x00002   // Socket has had listen()
#define  SO_REUSEADDR          0x00004   // Allow local address reuse
#define  SO_KEEPALIVE          0x00008   // Keep connections alive
#define  SO_DONTROUTE          0x00010   // Just use interface addresses
#define  MSG_DONTROUTE         SO_DONTROUTE
#define  SO_BROADCAST          0x00020   // Permit sending of broadcast msgs
#define  SO_LINGER             0x00080   // Linger on close if data present
#define  SO_TIMESTAMP          0x00800   // Set TCP timestamp option
#define  SO_NOSLOWSTART        0x04000   // Suppress slowstart on this socket

//
// Socket options (that are kept as flags with the socket).
// Unused, kept here for an overview of available options in other stacks.
//
#define  SO_DEBUG              0x00001   // Turn on debugging info recording
#define  SO_WINSCALE           0x00400   // Set scaling window option
#define  SO_BIGCWND            0x01000   // Large initial TCP Congestion window

//
// Other standard socket options.
//
#define  SO_SNDBUF             0x1001    // Send buffer size
#define  SO_RCVBUF             0x1002    // Receive buffer size
#define  SO_SNDTIMEO           0x1005    // Send timeout
#define  SO_RCVTIMEO           0x1006    // Receive timeout
#define  SO_ERROR              0x1007    // Get error status and clear
#define  SO_TYPE               0x1008    // Get socket type
#define  SO_MAXMSG             0x1010    // Get TCP_MSS (max segment size)

#define  SO_RXDATA             0x1011    // Get count of bytes in Rx SOCKET_BUFFER
#define  SO_TXDATA             0x1012    // Get count of bytes in Tx SOCKET_BUFFER
#define  SO_MYADDR             0x1013    // Return my IPv4 address
#define  SO_NBIO               0x1014    // Set socket into NON-blocking mode
#define  SO_BIO                0x1015    // Set socket into blocking mode
#define  SO_NONBLOCK           0x1016    // Set/get blocking mode
#define  SO_CALLBACK           0x1017    // Set/get zero_copy callback routine

//
// TCP socket options.
// All TCP options use the 0x2000 number space.
//
#define  TCP_ACKDELAYTIME      0x2001    // Set time for delayed acks
#define  TCP_NOACKDELAY        0x2002    // Suppress delayed ACKs
#define  TCP_MAXSEG            0x2003    // Set maximum segment size
#define  TCP_NODELAY           0x2004    // Disable Nagle Algorithm

//
// IPv6 socket options.
//
#define  IPV6_UNICAST_HOPS     0x6001    // Set default hop limit
#define  IPV6_JOIN_GROUP       0x6002    // Join an IPv6 multicast group
#define  IPV6_LEAVE_GROUP      0x6003    // Leave an IPv6 multicast group

//
// Other socket options.
// Other socket options use the 0x3000 number space.
//
#define  IP_HDRINCL            0x3001    // Provide own IP header for RAW socket

//
// User hook socket options.
// User socket options that can be added as getsockopt()/setsockopt()
// hook use the 0xF300 number space.
//
#define  IP_SOCK_GETSETOPT_HOOK_NAME_BASE  0xF300

//
// IPv4 options related defines
//
#define  IP_OPTIONS           (1)
#define  IP_TOS               (3)
#define  IP_TTL               (7)
#define  IP_MULTICAST_TTL     (10)

//
// setsockopt() values for IP_TOS.
//
#define IPTOS_LOWDELAY              0x10
#define IPTOS_THROUGHPUT            0x08
#define IPTOS_RELIABILITY           0x04
#define IPTOS_MINCOST               0x02

#define IPTOS_PREC_NETCONTROL       0xE0
#define IPTOS_PREC_INTERNETCONTROL  0xC0
#define IPTOS_PREC_CRITIC_ECP       0xA0
#define IPTOS_PREC_FLASHOVERRIDE    0x80
#define IPTOS_PREC_FLASH            0x60
#define IPTOS_PREC_IMMEDIATE        0x40
#define IPTOS_PREC_PRIORITY         0x20
#define IPTOS_PREC_ROUTINE          0x00

//
// IPv4 socket interface related structures
//
struct linger {
  int l_onoff;        // Option on/off.
  int l_linger;       // Linger time [s].
};

struct in_addr {
  U32  s_addr;
};

struct sockaddr {
  U16  sa_family;     // Address family
  char sa_data[14];   // Up to 14 bytes of direct address
};

typedef struct sockaddr_in {
  U16            sin_family;
  U16            sin_port;
  struct in_addr sin_addr;
  char           sin_zero[8];
} SOCKADDR_IN;


//
// IPv6 socket interface related structures
// Refer to [1] https://www.ietf.org/rfc/rfc3493.txt for further information
//

//
// IPv6 Address Structure
//
// [1] This data structure contains an array of sixteen 8-bit elements,
// which make up one 128-bit IPv6 address.  The IPv6 address is stored
// in network byte order.
//
//  struct in6_addr {
//    U8 s6_addr[16];      // IPv6 address
//  };

typedef struct {
  union {
    U8  aU8[16];
    U16 aU16[8];
    U32 aU32[4];
  } Union;
} IPV6_ADDR;

#define in6_addr IPV6_ADDR      // Used for compatibility to standard socket interface

//
//
//
typedef struct {
  IPV6_ADDR     MulticastAddr; // IPv6 multicast address
  unsigned int  IFaceId;       // Interface index
} IPV6_MREQ;

#define ipv6_mreq IPV6_MREQ      // Used for compatibility to standard socket interface

//
// This structure is designed to be compatible with the sockaddr data structure used in the 4.3BSD release.
//
typedef struct sockaddr_in6 {
  U16       sin6_family;    // AF_INET6
  U16       sin6_port;      // Transport layer port. The port number is stored in network byte order.
  U32       sin6_flowinfo;  // IPv6 flow information
  IPV6_ADDR sin6_addr;      // IPv6 address
  U32       sin6_scope_id;  // Set of interfaces for a scope
} SOCKADDR_IN6;

//
// The union improves the readability of our code in functions,
// where we only need the protocol family or port element of
// the structures.
//
typedef struct {
  U16            sin_family;
  U16            sin_port;
} SOCKADDR_IN_GEN;

//
// Union used as a wrapper for sockaddr_in / sockaddr_in6
// The union improves the readability of our code.
//
typedef union {
  struct sockaddr *pSockAddr;
  SOCKADDR_IN     *pIPv4In;
  SOCKADDR_IN6    *pIPv6In;
  SOCKADDR_IN_GEN *pIPIn;
} IP_SOCKADDR_IN;

//
// Address families
//
#define  AF_INET               2   // IPv4
#define  AF_INET6              3   // IPv6

//
// Protocol families
//
#define  PF_UNSPEC             AF_UNSPEC
#define  PF_UNIX               AF_UNIX
#define  PF_INET               AF_INET
#define  PF_INET6              AF_INET6

//
// Supported flags for receive functions
//
#define  MSG_PEEK              0x01     // Peek at incoming message
#define  MSG_DONTWAIT          0x04     // This message should be nonblocking
#define  MSG_TRUNC             0x10     // Data discarded before delivery

//
// IP address related defines
//
#define  INADDR_ANY            0L
#define  ADDR_ANY              0L

//
// Packet timestamp, accessible via extended socket functions.
//
typedef struct {
  U32  HW_Sec;   // PTP hardware timestamp (if hardware support is available, otherwise 0). Seconds part.
  U32  HW_NSec;  // PTP hardware timestamp (if hardware support is available, otherwise 0). Nanoseconds part.
#ifdef U64
  U64  SW_us;    // Software timestamp with microseconds precision (if provided to the stack using IP_SetMicrosecondsCallback).
  U64  SW_ns;    // Software timestamp with nanoseconds precision (if provided to the stack using IP_SetNanosecondsCallback).
#endif
} IP_PACKET_TIMESTAMP;

/*********************************************************************
*
*       Socket hooks
*/
#define IP_SOCK_HOOK_GETOPT    (0u)
#define IP_SOCK_HOOK_SETOPT    (1u)

#define IP_SOCK_HOOK_IGNORE_CB (4321)  // Magic value to be used as return value from a setsockopt()/getsockopt() hook.
                                       // Ignores the callback and uses the internal handling as usual. One example
                                       // for using this would be a case where only a get OR set without the other
                                       // shall be implemented.

/*********************************************************************
*
*       IP_SOCK_HOOK_ON_GETSETOPT_FUNC
*
*  Function description
*    Callback for custom implementations with setsockopt()/getsockopt() .
*
*  Parameters
*    Type  : Source/reason of execution:
*            * IP_SOCK_HOOK_GETOPT
*            * IP_SOCK_HOOK_SETOPT
*    hSock : Socket handle.
*    Level : Socket level such as SOL_SOCKET .
*    Name  : Option name.
*    pVal  : Pointer to the option value.
*    ValLen: Length of the option at pVal .
*
*  Return value
*    == IP_SOCK_HOOK_IGNORE_CB: Magic return value used to tell the
*                               stack that while it ended up in the
*                               callback it should still execute its
*                               regular (internal) behavior.
*    == 0                     : O.K.
*    != 0                     : Error (typically negative) that will be
*                               stored as socket error. The API call
*                               itself will still return SOCKET_ERROR .
*                               Value is limited to the size of signed char.
*
*/
typedef int (IP_SOCK_HOOK_ON_GETSETOPT_FUNC)(unsigned Type, int hSock, int Level, int Name, void* pVal, int ValLen);

/*********************************************************************
*
*       IP_SOCK_HOOK_ON_SETGETOPT
*
*  Function description
*    Memory block for a getsockopt()/setsockopt() callback added via
*    IP_SOCKET_AddGetSetOptHook() . Management is done internally.
*/
typedef struct IP_SOCK_HOOK_ON_SETGETOPT_STRUCT IP_SOCK_HOOK_ON_SETGETOPT;
struct IP_SOCK_HOOK_ON_SETGETOPT_STRUCT {
  const IP_SOCK_HOOK_ON_SETGETOPT*      pNext;  // Next item in the linked list.
        IP_SOCK_HOOK_ON_GETSETOPT_FUNC* pf;     // Callback to execute.
        int                             Name;   // Option name for which the callback gets executed.
};

/*********************************************************************
*
*       IP_SOCK_RECVFROM_INFO
*
*  Function description
*    Returns information about the received UDP packet typically
*    not available with the original BSD compatible call.
*/
typedef struct {
#if (IP_SUPPORT_PACKET_TIMESTAMP != 0) || (IP_SUPPORT_PTP != 0)
  IP_PACKET_TIMESTAMP* pTimestamp;  // Pointer where to store the packet timestamp. Can be NULL.
#endif
#if (IP_SUPPORT_IPV6 != 0)
  void*                pLAddrV6;    // Pointer to buffer where to store the local IPv6 address on which the datagram was received. Can be NULL.
  unsigned             AddrLenV6;   // Length of the buffer at pLAddrV6 .
#endif
#if (IP_SUPPORT_IPV4 != 0)
  U32                  LAddr;       // Local IPv4 address on which the datagram was received.
#endif
  U8                   IFaceId;     // Zero-based interface index the packet has been received on.
} IP_SOCK_RECVFROM_INFO;

//
// BSD compatible socket functions, typically accessed via their BSD named macros.
//
int  IP_SOCK_socket       (int Family, int Type, int Proto);
int  IP_SOCK_bind         (int hSock, struct sockaddr *pSockAddr, int AddrLen);
int  IP_SOCK_listen       (int hSock, int Backlog);
int  IP_SOCK_accept       (int hSock, struct sockaddr *pSockAddr, int *pAddrLen);
int  IP_SOCK_connect      (int hSock, struct sockaddr *pSockAddr, int AddrLen);
int  IP_SOCK_getpeername  (int hSock, struct sockaddr *pSockAddr, int *pAddrLen);
int  IP_SOCK_getsockname  (int hSock, struct sockaddr *pSockAddr, int *pAddrLen);
int  IP_SOCK_setsockopt   (int hSock, int Level, int Name, const void *pVal, int ValLen);
int  IP_SOCK_getsockopt   (int hSock, int Level, int Name, void *pVal, int ValLen);
int  IP_SOCK_recv         (int hSock, char *pData, int NumBytes, int Flag);
int  IP_SOCK_send         (int hSock, const char *pBuffer, int NumBytes, int Flags);
int  IP_SOCK_recvfrom     (int hSock, char *pData, int NumBytes, int Flags, struct sockaddr *pFrom, int *pAddrLen);
int  IP_SOCK_recvfrom_ts  (int hSock, char* pData, int NumBytes, int Flags, struct sockaddr* pFrom, int* pAddrLen, IP_PACKET_TIMESTAMP* pTimestamp);
int  IP_SOCK_recvfrom_info(int hSock, char* pData, int NumBytes, int Flags, struct sockaddr* pFrom, int* pAddrLen, IP_SOCK_RECVFROM_INFO* pInfo);
int  IP_SOCK_sendto       (int hSock, const char * pBuffer, int NumBytes, int Flags, struct sockaddr * pDestAddr, int NumBytesAddr);
int  IP_SOCK_shutdown     (int hSock, int How);
int  IP_SOCK_socketclose  (int hSock);

#if (IP_NO_BSD_SOCK_API == 0)
//
// BSD socket API.
// Map socket routine names to our IP_SOCK_ names using the preprocessor
//
#define  accept                  IP_SOCK_accept
#define  bind                    IP_SOCK_bind
#define  connect                 IP_SOCK_connect
#define  listen                  IP_SOCK_listen
#define  closesocket             IP_SOCK_socketclose
#define  getsockopt              IP_SOCK_getsockopt
#define  socket                  IP_SOCK_socket
#define  send                    IP_SOCK_send
#define  recv                    IP_SOCK_recv
#define  sendto                  IP_SOCK_sendto
#define  recvfrom                IP_SOCK_recvfrom
#define  socketclose             IP_SOCK_socketclose
#define  setsockopt              IP_SOCK_setsockopt
#define  shutdown                IP_SOCK_shutdown
#define  getpeername             IP_SOCK_getpeername
#define  getsockname             IP_SOCK_getsockname

#if IP_BSD_COMPLIANCE
  #define  select                IP_SOCK_select5
#else
  #define  select                IP_SOCK_select4
#endif
#endif  // (IP_NO_BSD_SOCK_API == 0)

struct hostent * gethostbyname(const char *name);

/*********************************************************************
*
*       IP_SOCKET_
*/
#define CLOSE_ALL_KEEP_LISTEN  (1uL << 0)

void IP_SOCKET_AddGetSetOptHook         (IP_SOCK_HOOK_ON_SETGETOPT* pHook, IP_SOCK_HOOK_ON_GETSETOPT_FUNC* pf, int Name);
void IP_SOCKET_CloseAll                 (U32 ConfMask);
void IP_SOCKET_ConfigSelectMultiplicator(U32 v);
void IP_SOCKET_SetDefaultOptions        (U16 v);
void IP_SOCKET_SetLimit                 (unsigned Limit);
int  IP_SOCKET_SetLinger                (int hSock, int Linger);
int  IP_SOCKET_GetNumRxBytes            (int hSock);
int  IP_SOCKET_SetRxTimeout             (int hSock, int Timeout);
U16  IP_SOCKET_GetLocalPort             (int hSock);
U16  IP_SOCKET_GetAddrFam               (int hSock);
int  IP_SOCKET_GetErrorCode             (int hSock);

#define IP_SOCK_errno  IP_SOCKET_GetErrorCode  // Compatibility macro for previously most likely unused function name.

//
// Socket errors
//
#define  SOCKET_ERROR   -1    // General socket error returned from send(), sendto(), ...
#define  SOL_SOCKET     -1    // Compatability parameter for set/get sockopt

//
// BSD compliance defines to ensure API compatibility.
//
#if IP_BSD_COMPLIANCE
typedef struct timeval {
  long tv_sec;
  long tv_usec;
} timeval;
#endif

//
// Max. number of sockets which can be passed to select().
//
#ifndef   IP_FD_SETSIZE
  #define IP_FD_SETSIZE  12
#endif

typedef struct {
  unsigned fd_count;
  U32      fd_array[IP_FD_SETSIZE];
} IP_fd_set;

//
// Select-related functions are calls (not macros) to save space
//
void  IP_FD_CLR  (int hSock, IP_fd_set *set);
void  IP_FD_SET  (int hSock, IP_fd_set *set);
int   IP_FD_ISSET(int hSock, IP_fd_set *set);
// and one actual macro:
#define  IP_FD_ZERO(set)   (((IP_fd_set *)(set))->fd_count=0)

//
// shutdown() related values.
//
#define IP_SHUT_RD    0
#define IP_SHUT_WR    1
#define IP_SHUT_RDWR  2

#if (IP_NO_BSD_SOCK_API == 0)
//
// BSD compatible macros.
//
#ifndef   fd_set
  #define fd_set     IP_fd_set
#endif

#ifndef   FD_CLR
  #define FD_CLR     IP_FD_CLR
#endif

#ifndef   FD_SET
  #define FD_SET     IP_FD_SET
#endif

#ifndef   FD_ISSET
  #define FD_ISSET   IP_FD_ISSET
#endif

#ifndef   FD_ZERO
  #define FD_ZERO    IP_FD_ZERO
#endif

#ifndef   SHUT_RD
  #define SHUT_RD    IP_SHUT_RD
#endif

#ifndef   SHUT_WR
  #define SHUT_WR    IP_SHUT_WR
#endif

#ifndef   SHUT_RDWR
  #define SHUT_RDWR  IP_SHUT_RDWR
#endif
#endif  // (IP_NO_BSD_SOCK_API == 0)

//
// Stack specific select call - The traditional "width" parameter is absent
//
int IP_SOCK_select4(IP_fd_set* in, IP_fd_set* out,  IP_fd_set* ex, I32 tv);
#if IP_BSD_COMPLIANCE
int IP_SOCK_select5(int nfds, IP_fd_set *in, IP_fd_set *out, IP_fd_set *ev, struct timeval *timeout);
#endif

#if defined(__cplusplus)
  }
#endif

#endif   // SOCKET_H

/*************************** End of file ****************************/
