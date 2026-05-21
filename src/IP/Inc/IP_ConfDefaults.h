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
File    : IP_ConfDefaults.h
Purpose : Defines defaults for most configurable defines used in the stack.
          If you want to change a value, please do so in IP_Conf.h, do NOT modify this file.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef IP_CONFDEFAULTS_H
#define IP_CONFDEFAULTS_H

#include <string.h>  // Required for memset
#include "IP_Conf.h"

//
// Operating system interface.
//
// IP_OS_DISABLE_INTERRUPT and IP_OS_ENABLE_INTERRUPT
// were previously using OS_DI() and OS_RestoreI().
// Disabling interrupts hard and restoring the previous
// state works as long as no further OS_DI() call gets
// used. In this case the previous interrupt state gets
// lost.
// As the IP_Conf.h typically does not get updated and
// should not be necessary to change by the customer on
// an update, we simply have to undef the macros from
// IP_Conf.h here and use our IP_OS layer functions for
// old configurations.
//
#ifndef   IP_OS_INC_DI
  #define IP_OS_INC_DI                    IP_OS_DisableInterrupt
#endif
#ifdef    IP_OS_DISABLE_INTERRUPT
  //lint -e(9021) allow #undef
  #undef  IP_OS_DISABLE_INTERRUPT
#endif
#define   IP_OS_DISABLE_INTERRUPT         IP_OS_INC_DI  // Remap old macro. Will be changed in source later.

#ifndef   IP_OS_DEC_RI
  #define IP_OS_DEC_RI                    IP_OS_EnableInterrupt
#endif
#ifdef    IP_OS_ENABLE_INTERRUPT
  //lint -e(9021) allow #undef
  #undef  IP_OS_ENABLE_INTERRUPT
#endif
#define   IP_OS_ENABLE_INTERRUPT          IP_OS_DEC_RI  // Remap old macro. Will be changed in source later.

#ifndef   IP_OS_GET_TIME
  #define IP_OS_GET_TIME                  IP_OS_GetTime32
#endif

//
// IP_OS_UNLOCK might be defined in IP_Conf.h that is
// meant to be kept by the customer when updating the
// stack. In a release build typically the OS functions
// shall be inlined directly if possible. IP_OS_UNLOCK()
// we no longer want to inline as we want to signal the
// IP_Task() from an unlock if a packet has been freed
// while the locking was active.
// To avoid changes to the customers IP_Conf.h we simply
// undefine IP_OS_UNLOCK() here again and define it with
// our default OS layer function.
//
#ifdef    IP_OS_UNLOCK
  //lint -e(9021) allow #undef
  #undef  IP_OS_UNLOCK
#endif
#ifndef   IP_OS_UNLOCK
  #define IP_OS_UNLOCK()                  IP_OS_Unlock()
#endif

#ifndef   IP_OS_LOCK
  #define IP_OS_LOCK()                    IP_OS_Lock()
#endif

#ifndef   IP_OS_WAIT_RX_EVENT
  #define IP_OS_WAIT_RX_EVENT()           IP_OS_WaitRxEvent()
#endif

#ifndef   IP_OS_SIGNAL_RX_EVENT
  #define IP_OS_SIGNAL_RX_EVENT()         IP_OS_SignalRxEvent()
#endif

#ifndef   IP_DEBUG
  #define IP_DEBUG                        0       // Debug level: 0: Release, 1: Support "Panic" checks, 2: Support warn & log
#endif

#ifndef   IP_MEMCPY
  #define IP_MEMCPY                       memcpy
#endif

#ifndef   IP_MEMSET
  #define IP_MEMSET                       memset
#endif

#ifndef   IP_MEMMOVE
  #define IP_MEMMOVE                      memmove
#endif

#ifndef   IP_MEMCMP
  #define IP_MEMCMP                       memcmp
#endif

#ifndef   IP_SNPRINTF
  #define IP_SNPRINTF                     SEGGER_snprintf
#endif

#ifndef   IP_VSNPRINTF
  #define IP_VSNPRINTF                    SEGGER_vsnprintf
#endif

#ifndef   IP_STRLEN
  #define IP_STRLEN                       SEGGER_strlen
#endif

#ifndef   IP_STRCMP
  #define IP_STRCMP                       strcmp
#endif

#ifndef   IP_CKSUM
  #define IP_CKSUM(p, NumHWords, Sum)     IP_cksum(p, NumHWords, Sum)
#endif

#ifndef   IP_OPTIMIZE
  #define IP_OPTIMIZE
#endif

#ifndef   IP_IS_BIG_ENDIAN
  #define IP_IS_BIG_ENDIAN                0       // Little endian is default
#endif

#ifndef   IP_USE_PARA                             // Some compiler complain about unused parameters.
  #define IP_USE_PARA(Para) (void)Para            // This works for most compilers.
#endif

#ifndef   IP_INCLUDE_STAT
  #define IP_INCLUDE_STAT (IP_DEBUG > 0)          // Can be set to 0 to disable statistics for extremly small release builds
#endif

#ifndef   IP_DEBUG_FIFO                           // Allow override in IP_Conf.h
  #define IP_DEBUG_FIFO IP_DEBUG
#endif

#ifndef   IP_DEBUG_MEM                            // Allow override in IP_Conf.h
  #define IP_DEBUG_MEM (IP_DEBUG > 1)
#endif

#ifndef   IP_MAX_ADD_ETH_TYPES
  #define IP_MAX_ADD_ETH_TYPES            2
#endif

//
// TCP retransmission range defaults
//
#ifndef   IP_TCP_RETRANS_MIN
  #define IP_TCP_RETRANS_MIN              210     // Min. delay for retransmit. Real delay is computed, this minimum applies only if computed delay is shorter. Min should be > 200, since 200 ms is a typ. value for delayed ACKs
#endif

#ifndef   IP_TCP_RETRANS_MAX
  #define IP_TCP_RETRANS_MAX              3000    // Max. delay for retransmit. Real delay is computed, this maximum applies only if computed delay is longer.
#endif

#ifndef   IP_TCP_RETRANS_NUM
  #define IP_TCP_RETRANS_NUM              6       // Number of retransmits before the connection gets dropped.
#endif

//
// TCP keep-alive defaults
//
#ifndef   IP_TCP_KEEPALIVE_INIT
  #define IP_TCP_KEEPALIVE_INIT           10000   // Initial connect keep alive [ms]
#endif

#ifndef   IP_TCP_KEEPALIVE_IDLE
  #define IP_TCP_KEEPALIVE_IDLE           10000   // Default time before probing [ms]
#endif

#ifndef   IP_TCP_KEEPALIVE_PERIOD
  #define IP_TCP_KEEPALIVE_PERIOD         10000   // Default probe interval [ms]
#endif

#ifndef   IP_TCP_KEEPALIVE_MAX_REPS
  #define IP_TCP_KEEPALIVE_MAX_REPS       8       // Max probes before drop
#endif

#ifndef   IP_TCP_MSL
  #define IP_TCP_MSL                      100     // Max segment lifetime. Used primarily for the TCP TIME_WAIT state. Large value wastes a lot of time!
#endif

#ifdef    IP_SOCKET_MAX_CONN                      // Makes no sense to limit the amount of TCP listen backlog on top of the value given by the application.
  #error For further information please refer to the release notes about the removal of this define.
#endif

#ifndef   IP_SOCKET_ID_LIMIT
  #define IP_SOCKET_ID_LIMIT              0xFFFF  // Socket ID limit on which the list wraps back to the lowest ID (typically 1) when trying to allocate a socket. The limit ID itself is not used.
#endif

#ifndef   IP_TCP_PERIOD
  #define IP_TCP_PERIOD                   10u
#endif

#ifndef   TCP_PERS_MIN_TIME
  #define TCP_PERS_MIN_TIME               500u    // Min. persistence time
#endif

#ifndef   TCP_PERS_MAX_TIME
  #define TCP_PERS_MAX_TIME               6000u   // Max. persistence time
#endif

#ifndef   IP_TCP_PORT_MIN
  #define IP_TCP_PORT_MIN                 1224
#endif

#ifndef   IP_TCP_PORT_MAX
  #define IP_TCP_PORT_MAX                 5000
#endif

#ifndef   IP_UDP_PORT_MIN
  #define IP_UDP_PORT_MIN                 1200
#endif

#ifndef   IP_UDP_PORT_MAX
  #define IP_UDP_PORT_MAX                 5000
#endif

#ifndef   IP_TCP_SUPPORT_TIMESTAMP
  #define IP_TCP_SUPPORT_TIMESTAMP        1       // Do we support RFC-1323 TCP timestamp feature to compute RTT ?
#endif

#ifndef   IP_TCP_ADD_ACK_TO_RST_PACKETS
  #define IP_TCP_ADD_ACK_TO_RST_PACKETS   0       // Adds the ACK flag to reset packets. MS Windows hosts send RST packets always with ACK flag and proper acknowledgement number.
#endif

#ifndef   IP_SUPPORT_MULTICAST
  #define IP_SUPPORT_MULTICAST            1
#endif

#ifndef   IP_UPNP_FULFIL_SPEC
  #define IP_UPNP_FULFIL_SPEC             0       // Fulfill the UPnP spec (more messages). This has been tested under windows and does not seem to be needed.
#endif

#ifndef   IP_UPNP_NUM_REQUESTS
  #define IP_UPNP_NUM_REQUESTS            5u      // Limit the number of UPnP requests that we can handle at the same time (expecially limits bytes allocated for required contexts).
#endif

#ifndef   IP_MAX_DNS_SERVERS
  #define IP_MAX_DNS_SERVERS              2
#endif

#ifndef   IP_LOCAL_MC_TTL_TIME
  #define IP_LOCAL_MC_TTL_TIME            1       // Multicast TTL (basically in seconds but more often counted in router hops)
#endif                                            // for local networks inside the ranges 224.0.0.x and 239.x.x.x .

#ifndef   IP_GLOBAL_MC_TTL_TIME
  #define IP_GLOBAL_MC_TTL_TIME           64      // Multicast TTL (basically in seconds but more often counted in router hops)
#endif                                            // for local networks outside the ranges 224.0.0.x and 239.x.x.x .

#ifndef   IP_TTL_TIME
  #define IP_TTL_TIME                     64      // TTL (basically in seconds but more often counted in router hops) for non
#endif                                            // multicast packets.

#ifndef IP_SUPPORT_LOG
  #if   (IP_DEBUG > 1)
    #define IP_SUPPORT_LOG                1
  #else
    #define IP_SUPPORT_LOG                0
  #endif
#endif

#ifndef IP_SUPPORT_WARN
  #if   (IP_DEBUG > 1)
    #define IP_SUPPORT_WARN               1
  #else
    #define IP_SUPPORT_WARN               0
  #endif
#endif

#if IP_SUPPORT_LOG
  #define IP_LOG(p) IP_Logf p
#else
  #define IP_LOG(p)
#endif

#if IP_SUPPORT_WARN
  #define IP_WARN(p) IP_Warnf p
#else
  #define IP_WARN(p)
#endif

#if IP_DEBUG >= 3
  #define IP_WARN_INTERNAL(p) IP_Warnf p
#else
  #define IP_WARN_INTERNAL(p)
#endif

#ifndef IP_PANIC
  #if   IP_DEBUG != 0
    #define IP_PANIC(s)                   IP_Panic(s)
  #else
    #define IP_PANIC(s)
  #endif
#endif

#if IP_INCLUDE_STAT
  #define IP_STAT_DEC(Cnt)                (Cnt)--
  #define IP_STAT_INC(Cnt)                (Cnt)++
  #define IP_STAT_ADD(Cnt, v)             { (Cnt) += (v); }
#else
  #define IP_STAT_DEC(Cnt)
  #define IP_STAT_INC(Cnt)
  #define IP_STAT_ADD(Cnt, v)
#endif

#ifndef   IP_MAX_IFACES
  #define IP_MAX_IFACES                   1       // Maximum number of interfaces to support at one time
#endif

#ifndef   IP_TIMERS_USE_PERIOD_SYNC
  #define IP_TIMERS_USE_PERIOD_SYNC       1       // Sync timers with equal periods to execute together.
#endif

#ifndef   IP_TIMERS_MAX_PERIOD_SYNC
  #define IP_TIMERS_MAX_PERIOD_SYNC       1000u   // Maximum period [ms] that will be synced to each other.
#endif

#ifndef   IP_TCP_DELAY_ACK_DEFAULT
  #define IP_TCP_DELAY_ACK_DEFAULT        200     // [ms]
#endif

#ifndef   DO_DELAY_ACKS
  #define DO_DELAY_ACKS                   1       // Enables delayed ACKs. Old define, kept for compatibility, use IP_SUPPORT_TCP_DELAYED_ACK in code.
#endif

#ifndef   IP_SUPPORT_TCP_DELAYED_ACK
  #define IP_SUPPORT_TCP_DELAYED_ACK      DO_DELAY_ACKS
#endif

#ifndef   IP_PTR_OP_IS_ATOMIC
  #define IP_PTR_OP_IS_ATOMIC             1
#endif

#ifdef    IP_ALLOW_NOLOCK
  #error "IP_ALLOW_NOLOCK is obsolete as it can not securely determine a value for all configurations."
#endif

#ifndef   IP_ALLOW_DEINIT
  #define IP_ALLOW_DEINIT                 0       // IP_DeInit() can be used to de-initialize the stack
#endif

#ifndef   IP_PPP_RESEND_TIMEOUT
  #define IP_PPP_RESEND_TIMEOUT           2000u
#endif

#ifndef   IP_SUPPORT_VLAN
  #define IP_SUPPORT_VLAN                 1
#endif

#ifndef   IP_SUPPORT_MICREL_TAIL_TAGGING
  #define IP_SUPPORT_MICREL_TAIL_TAGGING  1
#endif

#ifndef   IP_IFACE_REROUTE
  #define IP_IFACE_REROUTE                ((IP_SUPPORT_VLAN != 0) || (IP_SUPPORT_MICREL_TAIL_TAGGING != 0))
#endif

#ifndef   IP_ARP_SNIFF_ON_RX
  #define IP_ARP_SNIFF_ON_RX              1       // Create a short time ARP entry on Rx to avoid sending ARPs if we could already know the destination
#endif

#ifndef   IP_NUM_LINK_UP_PROBES
  #define IP_NUM_LINK_UP_PROBES           1       // Link probes before a link up is used by the stack. 1 means direct, 0 disables the code.
#endif

#ifndef   IP_NUM_LINK_DOWN_PROBES
  #define IP_NUM_LINK_DOWN_PROBES         1       // Link probes before a link down is used by the stack. 1 means direct, 0 disables the code.
#endif

#ifndef   IP_SUPPORT_ON_PACKET_FREE_CB
  #define IP_SUPPORT_ON_PACKET_FREE_CB    0       // Support adding a callback to be executed once a specific packet gets freed.
#endif

#ifndef   IP_TX_FIFO_TIMEOUT                      // Tx FIFO stuck check is done from 1s interface (link check) timer right now.
  #define IP_TX_FIFO_TIMEOUT              100u    // For the moment this is not the definitive timeout but more like a placeholder to define a timeout
#endif                                            // at all and might be used with its real value if implementing a dedicated timer in the future.

//
// Allow ICMP to answer for a packet that has been sent to a broadcast or multicast destination.
// Default is OFF as this might instrument us for a DDOS attack in case someone sends an ICMP
// packet to a broadcast or multicast address with a spoofed address of a target to attack.
//
#ifndef   IP_ICMP_RX_ALLOW_BC_MC
  #define IP_ICMP_RX_ALLOW_BC_MC          0
#endif

//
// Fragmentation related defines
//
#ifndef   IP_SUPPORT_FRAGMENTATION_IPV4
  #define IP_SUPPORT_FRAGMENTATION_IPV4   0
#endif

#ifndef   IP_FRAGMENT_CHECKS_PER_SEC
  #define IP_FRAGMENT_CHECKS_PER_SEC      10      // Number of checks per second.
#endif

#ifndef   IP_FRAGMENT_MAX_CNT
  #define IP_FRAGMENT_MAX_CNT             0xFF    // Number of allowed fragments. 0xFF means no limit.
#endif

#ifndef   IP_FRAGMENT_TIMEOUT
  #define IP_FRAGMENT_TIMEOUT             500     // Timeout to discard fragment queues. If not all fragments could be received until reaching the timeout, timer will discard the queue and the related packets.
#endif

#ifndef   IP_FRAGMENT_USE_SINGLE_LOCK
  #define IP_FRAGMENT_USE_SINGLE_LOCK     0       // 0: Locks (disables/enables) interrupts for each packet to free from a queue, shorter interrupt disable time.
#endif                                            // 1: Locks (disables/enables) interrupts once for freeing the whole queue, more effective regarding interrupt disable/enable times.

#ifndef   IP_SUPPORT_IPV4
  #define IP_SUPPORT_IPV4                 1       // Switch for the IPv4 base component. Disables IPv4 features. IPv4 code removal will be improved with future releases.
#endif

#ifndef   IP_DHCPC_CHECK_IP_BEFORE_BOUND
  #define IP_DHCPC_CHECK_IP_BEFORE_BOUND  1       // Check that an address offered via DHCP is really free by sending ARP probes.
#endif

//
// IPv6 add-on related configuration defaults
//
#ifndef   IP_SUPPORT_IPV6
  #define IP_SUPPORT_IPV6                 0       // Switch for the IPv6 add-on.
#endif

#ifndef   IP_SUPPORT_FRAGMENTATION_IPV6
  #define IP_SUPPORT_FRAGMENTATION_IPV6   0
#endif

//
// IPv6 DNS related configuration defaults
//
#ifndef   IP_IPV6_DNS_MAX_IPV6_SERVER
  #define IP_IPV6_DNS_MAX_IPV6_SERVER     1
#endif

#ifndef   IP_IPV6_DNS_MAX_NUM_IPV6_ADDR
  #define IP_IPV6_DNS_MAX_NUM_IPV6_ADDR   1
#endif

//
// PHY related configuration defaults
//
#ifndef   IP_PHY_AFTER_RESET_DELAY
  #define IP_PHY_AFTER_RESET_DELAY        3u      // Delay [ms] between (soft) resetting the PHY and further communication with it.
#endif

//
// For backwards compatibility reasons NI drivers might call the
// legacy initialization routines of the generic PHY driver.
// This init routine will add the default PHY driver if no other
// driver has been added before.
// The IP_PHY_DEFAULT_DRIVER define can be set to NULL to not link
// in the complete generic PHY driver (when usinga  smart linker).
// The (small) legacy wrapper routines however will remain linked
// in and accessed by the NI driver for its link check.
//
#ifndef   IP_PHY_DEFAULT_DRIVER
  #define IP_PHY_DEFAULT_DRIVER           &IP_PHY_Driver_Generic
#endif

//
// Log buffer related configuration defaults.
//
#ifndef     IP_LOG_BUFFER_SIZE
  #if IP_SUPPORT_IPV6
    #define IP_LOG_BUFFER_SIZE            160
  #else
    #define IP_LOG_BUFFER_SIZE            100
  #endif
#endif

//
// Profiling/SystemView related configuration defaults.
//
#ifndef   IP_SUPPORT_PROFILE
  #define IP_SUPPORT_PROFILE              0
#endif

#ifndef   IP_SUPPORT_PROFILE_END_CALL
  #define IP_SUPPORT_PROFILE_END_CALL     0
#endif

#ifndef   IP_SUPPORT_PROFILE_FIFO
  #define IP_SUPPORT_PROFILE_FIFO         0
#endif

#ifndef   IP_SUPPORT_PROFILE_PACKET
  #define IP_SUPPORT_PROFILE_PACKET       0
#endif

//
// Statistics related configuration defaults.
// By default the global IP_SUPPORT_STATS enables
// all specific stats defines.
//
#ifndef   IP_SUPPORT_STATS
  #define IP_SUPPORT_STATS                0
#endif

#ifndef   IP_SUPPORT_STATS_IFACE
  #define IP_SUPPORT_STATS_IFACE          IP_SUPPORT_STATS
#endif

//
// Statistics for FIFO current and min./max.
// By default the global IP_SUPPORT_STATS enables
// all specific stats defines.
//
#ifndef   IP_SUPPORT_STATS_FIFO
  #define IP_SUPPORT_STATS_FIFO           IP_SUPPORT_STATS
#endif

#ifndef   IP_SUPPORT_PTP
  #define IP_SUPPORT_PTP                  0       // Precision Time Protocol add-on.
#endif

#ifndef   IP_SUPPORT_PACKET_TIMESTAMP
  #define IP_SUPPORT_PACKET_TIMESTAMP     0
#endif

//
// Default first 4 bytes to use for generating a hardware address.
// The default is a SEGGER specific range. You need to change it
// to your own if used in production.
//
#ifndef   IP_GEN_HWADDR_VENDOR
  #define IP_GEN_HWADDR_VENDOR            "\x00\x22\xC7\xFF"
#endif


#endif // Avoid multiple inclusion

/*************************** End of file ****************************/
