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
File    : IP_TCP.h
Purpose : Tranport Control Protocol related header file.
--------  END-OF-HEADER  ---------------------------------------------
*/


#ifndef TCPIP_H
#define  TCPIP_H 1

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#ifdef IP_TCP_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

#if IP_SUPPORT_IPV6
  #include "IPV6_Int.h"
#endif


/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define     ISN_ADD_ON_CONNECT   0x1000000UL
#define     ISN_ADD_ON_TIMER     0x0100000UL

//
// TCP flags
//
#define     TCP_FLAG_ACK_NOW     0x0001
#define     TCP_FLAG_NO_DELAY    0x0004
#define     TCP_FLAG_FIN_SENT    0x0010
#define     TCP_FLAG_TIMESTAMP   0x0100

//
// Definitions of the TCP states.
//
#define     TCP_STATE_CLOSED          0  // Closed
#define     TCP_STATE_LISTEN          1  // Server only: Waiting for connection
#define     TCP_STATE_SYN_SENT        2  // Client only: SYN sent, Waiting for SYN+ACK
#define     TCP_STATE_SYN_RECEIVED    3  // Server only: SYN received, SYN+ACK sent, Waiting for ACK
#define     TCP_STATE_ESTABLISHED     4
#define     TCP_STATE_CLOSE_WAIT      5
#define     TCP_STATE_FIN_WAIT_1      6
#define     TCP_STATE_CLOSING         7
#define     TCP_STATE_LAST_ACK        8
#define     TCP_STATE_FIN_WAIT_2      9
#define     TCP_STATE_TIME_WAIT       10
#define     TCP_STATE_NUM_STATES      11

//
// Definitions of the TCP timers.
//
#define     TCP_TIMER_RETRANSMIT    0
#define     TCP_TIMER_PERSISTENCE   1
#define     TCP_TIMER_KEEP_ALIVE    2
#define     TCP_TIMER_2MSL          3
#define     TCP_TIMER_NUM_TIMERS    4

#define     TCP_MASK_FIN   0x01
#define     TCP_MASK_SYN   0x02
#define     TCP_MASK_RST   0x04
#define     TCP_MASK_PUSH  0x08
#define     TCP_MASK_ACK   0x10
#define     TCP_MASK_URG   0x20   // Not implemented.

#define     LOOKUP_WILDCARD   1

/*********************************************************************
*
*       TCP options
*
*      Kind     Length    Meaning
*      ----     ------    -------
*       0         -       End of option list.         RFC 793
*       1         -       No-Operation.               RFC 793
*       2         4       Maximum Segment Size.       RFC 793     SYN-only
*       3         3       Window Scale Option (WSopt) RFC 1323    SYN-only
*       8        10       Timestamps Option (TSopt)   RFC 1323
*/
#define  TCP_OPT_EOL        0
#define  TCP_OPT_NOP        1
#define  TCP_OPT_MAXSEG     2
#define  TCP_OPT_WINSCALE   3
#define  TCP_OPT_RTT        8

/*********************************************************************
*
*       Time constants.
*/
#define  TCP_INITIAL_ASSUMED_RTT    1000                 // Initial assumed RTT if no info


/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/

/*
  RFC 791, 3.1.  Internet Header Format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version|  IHL  |Type of Service|          Total Length         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Identification        |Flags|      Fragment Offset    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Time to Live |    Protocol   |         Header Checksum       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       Source Address                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Destination Address                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  RFC 793, 3.1 TCP Header Format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          Source Port          |       Destination Port        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Sequence Number                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Acknowledgment Number                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Data |           |U|A|P|R|S|F|                               |
   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
   |       |           |G|K|H|T|N|N|                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Checksum            |         Urgent Pointer        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             data                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

typedef struct {
  U16  SrcPort;       // Source Port
  U16  DestPort;      // Destination Port
  U32  SeqNo;         // Sequence Number
  U32  AckNo;         // Acknowledgement Number
  U8   DataOff;       // Data Offset: high 4 bits, indicating the number of words in header. Typ. value is 5
  U8   Flags;
  U16  Win;           // Window size
  U16  CheckSum;
  U16  UrgentPtr;     // Urgent Pointer. Not used any more.
} TCP_HEADER;

typedef struct {
  IP_HEADER   IpHeader;
  TCP_HEADER  TcpHeader;
} TCP_IP_HEADER;

#if IP_SUPPORT_IPV6
typedef struct {
  IPV6_HEADER  IPv6Header;
  TCP_HEADER   TCPHeader;
} TCP_IPV6_HEADER;
#endif

/*********************************************************************
*
*       TCP control block
*/
typedef struct TCPCB {
  void*   pIPAddrInfo;
  U16     FPort;
  U16     LPort;
  SOCKET* pSocket;
  IFACE*  pIFace;
  //
  // TCP state info
  //
  U8            State;
  U8            RetransShift;
  U8            DupACKs;
  U8            Force;
  U16           Mss;                         // Maximum segment size. This is always MTU - 40, acc. to RFC 879: THE TCP MAXIMUM SEGMENT SIZE IS THE IP MAXIMUM DATAGRAM SIZE MINUS FORTY.
  U16           TrueMSS;                     // True MSS: Mss - OptLen
  U16           OptLen;                      // Option len. typically 0 for no options, or 12 with  RTTM
  U16           Flags;
  U32           Timer[TCP_TIMER_NUM_TIMERS]; // Timeout values in ms. 0 means timer is not running
  U32           RetransDelay;                // Retransmission delay in ms
//
//  RFC791:
//  The following diagrams may help to relate some of these variables to
//  the sequence space.
//
//  Send Sequence Space
//
//                   1         2          3          4
//              ----------|----------|----------|----------
//                     SND.UNA    SND.NXT    SND.UNA
//                                          +SND.WND
//
//        1 - old sequence numbers which have been acknowledged
//        2 - sequence numbers of unacknowledged data
//        3 - sequence numbers allowed for new data transmission
//        4 - future sequence numbers which are not yet allowed
//
   U32     snd_una;                          // Send unacknowledged
   U32     snd_nxt;                          // Send next
   U32     snd_wl1;                          // Window update seg seq number
   U32     snd_wl2;                          // Window update seg ack number
   U32     snd_wnd;                          // Send window
//
// Receive Sequence Space
//
//                       1          2          3
//                   ----------|----------|----------
//                          RCV.NXT    RCV.NXT
//                                    +RCV.WND
//
//        1 - old sequence numbers which have been acknowledged
//        2 - sequence numbers allowed for new reception
//        3 - future sequence numbers which are not yet allowed
//
   U32     rcv_adv;    /* advertised window */
   U32     rcv_wnd;    /* receive window */
   U32     rcv_nxt;    /* receive next */
   /* retransmit variables */
   /* highest sequence number sent used to recognize retransmits */
   U32   snd_max;
   /* congestion control (for slow start, source quench, retransmit after loss) */
   U32  snd_cwnd;      /* congestion-controlled window */
   U32  snd_ssthresh;   // snd_cwnd size threshhold for for slow start exponential to linear switch
   /*
    * transmit timing stuff.
    * srtt and rttvar are stored as fixed point; for convenience in smoothing,
    * srtt has 3 bits to the right of the binary point, rttvar has 2.
    * "Variance" is actually smoothed difference.
    */
   unsigned IdleCnt;       // Inactivity time counter. Incremented with every "slow tick"
   U32      TickCnt;
   U32      t_rtseq;       /* sequence number being timed */
   int      t_srtt;        /* smoothed round-trip time */
   int      t_rttvar;      /* variance in round-trip time */
   U32      max_rcvd;      /* most peer has sent into window */
   U32      max_sndwnd;    /* largest window peer has offered */
#if IP_TCP_SUPPORT_TIMESTAMP
   U32  ts_recent;           /* RFC-1323 TS.Recent (peer's timestamp) */
   U32  last_ack;            /* RFC-1323 Last.ACK (last ack to peer) */
#endif   /* IP_TCP_SUPPORT_TIMESTAMP */

#if IP_SUPPORT_TCP_DELAYED_ACK
   U16     DelayAckPeriod;        // Period for delaying ACK [ms]. 0 means no delayed acknowledges.
   U16     DelayAckRem;           // Time left for delaying this ACK [ms]
#endif
   U16         TCPIPHeaderSize;
   U8          KeepAliveSent;
} TCPCB;

#if IP_DEBUG != 0
  #define IP_TCP_START_RETRANS_TIMER(pTCP) IP_TCP_StartRetransTimer(pTCP)
#else
  #define IP_TCP_START_RETRANS_TIMER(pTCP) { pTCP->Timer[TCP_TIMER_RETRANSMIT] = (U16)(pTCP->RetransDelay); }
#endif

typedef int IP_TCP_FUNC  (struct TCPCB *pTCPCB);
typedef struct {
  IP_TCP_FUNC * pfTCPOutput;
  IP_TCP_FUNC * pfTCPKeepAlive;
} TCP_FUNCTIONS;

//
// TCP control block related functions
//
int      IP_TCP_PCB_bind       (TCPCB* pTCPCB, struct sockaddr * pSockAddr);
int      IP_TCP_PCB_connect    (TCPCB* pTCPCB, struct sockaddr * pSockAddr);
TCPCB *  IP_TCP_PCB_lookup     (unsigned PFamily, void * pFAddr, U16 FPort, void * pLAddr, U16 LPort, int Flags);

//
// TCP timer
//
void     IP_TCP_Timer             (void* pContext);
void     IP_TCP_StartKEEPTimer    (TCPCB* pTCPCB, U32 Start);
void     IP_TCP_Start2MSLTimer    (TCPCB* pTCPCB);
void     IP_TCP_SetRetransDelay   (TCPCB* pTCPCB, unsigned Delay);
void     IP_TCP_StartRetransTimer (TCPCB* pTCPCB);
void     IP_TCP_CancelTimers      (TCPCB* pTCPCB);
void     IP_TCP_StartPersistTimer (TCPCB* pTCPCB);

//
// TCP socket related functions
//
int  IP_TCP_SOCK_Abort       (SOCKET* pSock);
int  IP_TCP_SOCK_Accept      (SOCKET* pSock, struct sockaddr* pSockAddr);
int  IP_TCP_SOCK_Attach      (SOCKET* pSock, int Proto, IFACE* pIFace);
int  IP_TCP_SOCK_Bind        (SOCKET* pSock, struct sockaddr* pSockAddr, int (*pfBind)(TCPCB* pTCPCB, struct sockaddr* pSockAddr));
int  IP_TCP_SOCK_Connect     (SOCKET* pSock, struct sockaddr* pSockAddr, int (*pfBind)(TCPCB* pTCPCB, struct sockaddr* pSockAddr), int (*pfConnect)(TCPCB* pTCPCB, struct sockaddr* pSockAddr), int (*pfOutput)(TCPCB* pTCPCB));
void IP_TCP_SOCK_Detach      (SOCKET* pSock, int (*pfOutput)(TCPCB* pTCPCB));
int  IP_TCP_SOCK_Disconnect  (SOCKET* pSock, int (*pfOutput)(TCPCB* pTCPCB));
U16  IP_TCP_SOCK_FindFreePort(SOCKET* pSock, TCPCB* (*pfPCBLookup)(unsigned PFamily, void* pFAddr, U16 FPort, void* pLAddr, U16 LPort, int Flags));
int  IP_TCP_SOCK_GetPeerName (SOCKET* pSock, struct sockaddr_in* pSockAddrIn);
int  IP_TCP_SOCK_GetSockName (SOCKET* pSock, struct sockaddr_in* pSockAddrIn);
int  IP_TCP_SOCK_Listen      (SOCKET* pSock, int (*pfBind)(TCPCB* pTCPCB, struct sockaddr* pSockAddr));
int  IP_TCP_SOCK_Recv        (SOCKET* pSock, char* pBuffer, unsigned NumBytes, int Flags, int (*pfOutput)(TCPCB* pTCPCB));
int  IP_TCP_SOCK_Send        (SOCKET* pSock, const U8* pData, unsigned NumBytes, int (*pfOutput)(TCPCB* pTCPCB));
int  IP_TCP_SOCK_Shutdown    (SOCKET* pSock, int (*pfOutput)(TCPCB* pTCPCB));


//
// TCP generic functions.
//
void IP_TCP_AddGeneric(const IP_TRANSPORT_PROTO* pProto, IP_ON_RX_FUNC* pf, const TCP_FUNCTIONS* pTCPFuncs, int AllowReplace);
void IP_TCP_OnAccept  (TCPCB* pTCPCB, SOCKET* pSock, SOCKET* pOwner);

//
// TCP functions
//
void     IP_TCP_AppendPacket      (SOCKET_BUFFER * sb, IP_PACKET * pPacket);
U16      IP_TCP_CalcChecksum      (TCP_IP_HEADER * pTCPIPHeader, unsigned TCPLen, U32 Sum);
void     IP_TCP_Close             (TCPCB * pTCPCB);
void     IP_TCP_Drop              (TCPCB * pTCPCB, int err);
int      IP_TCP_Output            (TCPCB * pTCPCB);
void     IP_TCP_RespondWithRST    (IP_PACKET * pPacket);
int      IP_TCP_SendKeepAlive     (TCPCB * pTCPCB);

#if IP_SUPPORT_IPV6
//
// IPv6 related routines
//
int      IPV6_TCP_Output         (TCPCB * pTCPCB);
TCPCB  * IPV6_TCP_PCB_lookup     (unsigned PFamily, void *pFAddr, U16 FPort, void *pLAddr, U16 LPort, int Flags);
void     IPV6_TCP_RespondWithRST (IP_PACKET * pPacket);
U16      IPV6_TCP_CalcChecksum   (TCP_IPV6_HEADER *pHeader, unsigned TCPLen, U32 Sum);
int      IPV6_TCP_SendKeepAlive  (TCPCB * pTCPCB);
int      IPV6_TCP_PCB_bind       (TCPCB * pTCPCB, struct sockaddr * pSockAddr);
int      IPV6_TCP_PCB_connect    (TCPCB * pTCPCB, struct sockaddr * pSockAddr);

#endif

/*********************************************************************
*
*       IP_TCP_GLOBAL
*/
typedef struct {
  const TCP_FUNCTIONS* pTCPFunc;
  IP_TCP_ACCEPT_HOOK*  pFirstAcceptHook;
  IP_TIMER             Timer;
  U32                  RetransDelayMin;
  U32                  RetransDelayMax;
  U32                  KeepInit;
  U32                  KeepIdle;
  U32                  KeepPeriod;
  U32                  KeepMaxReps;      // Max. number of repetitions of "Keep alive" packets
  U32                  Msl;
  U32                  Iss;              // TCP initial send sequence #
  U32                  TxWindowSize;
  U32                  RxWindowSize;
  U16                  NextFreePort;
} IP_TCP_GLOBAL;

EXTERN IP_TCP_GLOBAL IP_TCP_Global;

/*********************************************************************
*
*       IP_TCP_STAT
*/
typedef struct {
  U32 DelayedAckSendCnt;     // How many times has a delayed Ack actually been sent by the TCP timer
  U32 KeepAliveSentCnt;
  U32 RetransCnt;
  U32 ResetCnt;
} IP_TCP_STAT;

EXTERN IP_TCP_STAT IP_TCP_Stat;

//lint -e(9021)
#undef EXTERN

#if defined(__cplusplus)
  }
#endif


#endif // Avoid multiple inclusion

/*************************** End of file ****************************/


