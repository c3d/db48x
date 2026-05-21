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

File    : IP_SNMP_AGENT_MIB2_INTERFACES_emNet.c
Purpose : SNMP agent MIB-II Interfaces implementation for emNet.

Literature:
  [1]  Please refer to IP_SNMP_AGENT.c

Notes:
*/

#include "IP_SNMP_AGENT.h"
#include "IP.h"

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _GetIfNumber()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifNumber(1)
*
*  Return value
*    Number of interfaces registered with the IP stack.
*/
static int _GetIfNumber(void) {
  return IP_INFO_GetNumInterfaces();
}

/*********************************************************************
*
*       _GetIfIndex()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifIndex(1)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Unique interface index.
*/
static int _GetIfIndex(unsigned IfEntry) {
  return (IfEntry - 1);  // For emNet the SNMP specific ifEntry index is simply one higher than the emNet interface index.
}

/*********************************************************************
*
*       _GetIfDescr()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifDescr(2)
*
*  Parameters
*    IfEntry  : SNMP specific ifEntry value (typically 1..n).
*    pBuffer  : Pointer to buffer where to store the string.
*    pNumBytes: Pointer to size of the buffer at pBuffer and where to store the length of the string (without termination).
*/
static void _GetIfDescr(unsigned IfEntry, char* pBuffer, U32* pNumBytes) {
  IP_NI_GetIFaceType(IfEntry - 1, pBuffer, pNumBytes);
}

/*********************************************************************
*
*       _GetIfType()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifType(3)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Interface type according to the following list:
*    http://www.alvestrand.no/objectid/1.3.6.1.2.1.2.2.1.3.html .
*/
static int _GetIfType(unsigned IfEntry) {
  U32  NumBytes;
  char ac[16];

  NumBytes     = sizeof(ac) - 1;                       // Preserve one byte for termination.
  IP_NI_GetIFaceType(IfEntry - 1, &ac[0], &NumBytes);  // Get the interface type as short name.
  ac[NumBytes] = '\0';                                 // Make sure the string is terminated.
  //
  // For simplicity we differentiate only between three types of interfaces:
  //   - softwareLoopback(24)
  //   - ppp(23)
  //   - Everything else = ethernet-csmacd(6)
  //
  if        (IP_STRCMP(&ac[0], "Loopback") == 0) {
    return 24;  // SNMP: softwareLoopback(24)
  } else if (IP_STRCMP(&ac[0], "PPP") == 0) {
    return 23;  // SNMP: ppp(23)
  } else {
    return 6;   // SNMP: ethernet-csmacd(6)
  }
}

/*********************************************************************
*
*       _GetIfMtu()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifMtu(4)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    MTU value of the interface.
*/
static int _GetIfMtu(unsigned IfEntry) {
  return IP_GetMTU(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfSpeed()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifSpeed(5)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Interface speed in bits/s.
*/
static U32 _GetIfSpeed(unsigned IfEntry) {
  return (U32)IP_GetCurrentLinkSpeedEx(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfPhysAddress()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifPhysAddress(6)
*
*  Parameters
*    IfEntry  : SNMP specific ifEntry value (typically 1..n).
*    pBuffer  : Pointer to buffer where to store the physical address..
*    pNumBytes: Pointer to size of the buffer at pBuffer and where to store the length of the address.
*/
static void _GetIfPhysAddress(unsigned IfEntry, U8* pBuffer, U32* pNumBytes) {
  IP_GetHWAddr(IfEntry - 1, pBuffer, *pNumBytes);
  *pNumBytes = SEGGER_MIN(*pNumBytes, 6);  // A physical addr. typically contains 6 bytes. Limit to buffer size.
}

/*********************************************************************
*
*       _GetIfAdminStatus()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifAdminStatus(7)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Up                          : 1
*    Down                        : 2
*    Testing (typically not used): 3
*/
static int _GetIfAdminStatus(unsigned IfEntry) {
  int Status;

  Status = IP_NI_GetAdminState(IfEntry - 1);
  if (Status != 1) {  // Not enabled/up ?
    Status = 2;       // Then we use down.
  }
  return Status;
}

/*********************************************************************
*
*       _SetIfAdminStatus()
*
*  Function description
*    Sets the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifAdminStatus(7)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*    Status : Admin status to set:
*               Up                          : 1
*               Down                        : 2
*               Testing (typically not used): 3
*
*  Return value
*    O.K.           : IP_SNMP_OK
*    Invalid value  : IP_SNMP_ERR_BAD_VALUE
*    Any other error: IP_SNMP_ERR_*
*/
static int _SetIfAdminStatus(unsigned IfEntry, unsigned Status) {
  int r;

  r = IP_SNMP_ERR_BAD_VALUE;

  //
  // Check for valid value to set and change it to our format.
  // The MIB-II value range for this entity should be already
  // checked by the MIB callback.
  //
  if (Status != 3) {  // Testing(3) is not supported.
    //
    // Change Down(2) to our Down(0).
    // Up(1) can be used as is.
    //
    if (Status == 2) {
      Status = 0;
    }
    IP_NI_SetAdminState(IfEntry - 1, Status);
    r = IP_SNMP_OK;
  }
  return r;
}

/*********************************************************************
*
*       _GetIfOperStatus()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifOperStatus(8)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Up                          : 1
*    Down                        : 2
*    Testing (typically not used): 3
*/
static int _GetIfOperStatus(unsigned IfEntry) {
  int Status;

  Status = IP_NI_GetState(IfEntry - 1);
  if (Status != 1) {  // Not enabled/up ?
    Status = 2;       // Then we use down.
  }
  return Status;
}

/*********************************************************************
*
*       _GetIfLastChange()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifLastChange(9)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Tick count in 1/100 seconds when the interface entered its
*    current state.
*/
static U32 _GetIfLastChange(unsigned IfEntry) {
  const IP_SNMP_AGENT_API* pAPI;

  pAPI = IP_SNMP_AGENT_GetAPI();
  return pAPI->pfSysTicks2SnmpTime(IP_STATS_GetLastLinkStateChange(IfEntry - 1));
}

/*********************************************************************
*
*       _GetIfInOctets()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifInOctets(10)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of bytes received on this interface.
*/
static U32 _GetIfInOctets(unsigned IfEntry) {
  return IP_STATS_GetRxBytesCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfInUcastPkts()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifInUcastPkts(11)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of unicast packets received on this interface.
*/
static U32 _GetIfInUcastPkts(unsigned IfEntry) {
  return IP_STATS_GetRxUnicastCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfInNUcastPkts()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifInNUcastPkts(12)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of packets received on this interface that were not unicasts.
*/
static U32 _GetIfInNUcastPkts(unsigned IfEntry) {
  return IP_STATS_GetRxNotUnicastCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfInDiscards()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifInDiscards(13)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of packets received but discarded although they were O.K. .
*/
static U32 _GetIfInDiscards(unsigned IfEntry) {
  return IP_STATS_GetRxDiscardCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfInErrors()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifInErrors(14)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of receive errors.
*/
static U32 _GetIfInErrors(unsigned IfEntry) {
  return IP_STATS_GetRxErrCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfInUnknownProtos()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifInUnknownProtos(15)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of packets received with unknown protocols.
*/
static U32 _GetIfInUnknownProtos(unsigned IfEntry) {
  return IP_STATS_GetRxUnknownProtoCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfOutOctets()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifOutOctets(16)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of bytes sent on this interface.
*/
static U32 _GetIfOutOctets(unsigned IfEntry) {
  return IP_STATS_GetTxBytesCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfOutUcastPkts()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifOutUcastPkts(17)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of unicast packets sent on this interface.
*/
static U32 _GetIfOutUcastPkts(unsigned IfEntry) {
  return IP_STATS_GetTxUnicastCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfOutNUcastPkts()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifOutNUcastPkts(18)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of packets sent on this interface that were not unicasts.
*/
static U32 _GetIfOutNUcastPkts(unsigned IfEntry) {
  return IP_STATS_GetTxNotUnicastCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfOutDiscards()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifOutDiscards(19)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of packets to send but discarded although they were O.K. .
*/
static U32 _GetIfOutDiscards(unsigned IfEntry) {
  return IP_STATS_GetTxDiscardCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfOutErrors()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifOutErrors(20)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of send errors.
*/
static U32 _GetIfOutErrors(unsigned IfEntry) {
  return IP_STATS_GetTxErrCnt(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfOutQLen()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifOutQLen(21)
*
*  Parameters
*    IfEntry: SNMP specific ifEntry value (typically 1..n).
*
*  Return value
*    Number of packets queued for sending on this interface.
*/
static U32 _GetIfOutQLen(unsigned IfEntry) {
  return IP_NI_GetTxQueueLen(IfEntry - 1);
}

/*********************************************************************
*
*       _GetIfSpecific()
*
*  Function description
*    Serves the value for the following SNMP OID:
*    iso(1).org(3).dod(6).internet(1).ietf(2).mib2(1).interfaces(2).ifTable(2).ifEntry(IfEntry).ifSpecific(22)
*
*  Parameters
*    IfEntry  : SNMP specific ifEntry value (typically 1..n).
*    pBuffer  : Pointer to buffer where to store the physical address..
*    pNumBytes: Pointer to size of the buffer at pBuffer and where to store the length of the address.
*/
static void _GetIfSpecific(unsigned IfEntry, U8* pBuffer, U32* pNumBytes) {
  (void)IfEntry;

  //
  // We just assume that we have at least a 2 byte buffer to write to.
  // No specific data to store, therefore we just store the smallest
  // valid empty OID value 0.0 .
  // For more information please refer to:
  // http://www.alvestrand.no/objectid/1.3.6.1.2.1.2.2.1.22.html
  //
  *pBuffer++ = 0;
  *pBuffer   = 0;
  *pNumBytes = 2;
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/

const IP_SNMP_AGENT_MIB2_INTERFACES_API IP_SNMP_AGENT_MIB2_INTERFACES_emNet = {
  _GetIfNumber,           // pfGetIfNumber
  _GetIfIndex,            // pfGetIfIndex
  _GetIfDescr,            // pfGetIfDescr
  _GetIfType,             // pfGetIfType
  _GetIfMtu,              // pfGetIfMtu
  _GetIfSpeed,            // pfGetIfSpeed
  _GetIfPhysAddress,      // pfGetIfPhysAddress
  _GetIfAdminStatus,      // pfGetIfAdminStatus
  _SetIfAdminStatus,      // pfSetIfAdminStatus
  _GetIfOperStatus,       // pfGetIfOperStatus
  _GetIfLastChange,       // pfGetIfLastChange
  _GetIfInOctets,         // pfGetIfInOctets
  _GetIfInUcastPkts,      // pfGetIfInUcastPkts
  _GetIfInNUcastPkts,     // pfGetIfInNUcastPkts
  _GetIfInDiscards,       // pfGetIfInDiscards
  _GetIfInErrors,         // pfGetIfInErrors
  _GetIfInUnknownProtos,  // pfGetIfInUnknownProtos
  _GetIfOutOctets,        // pfGetIfOutOctets
  _GetIfOutUcastPkts,     // pfGetIfOutUcastPkts
  _GetIfOutNUcastPkts,    // pfGetIfOutNUcastPkts
  _GetIfOutDiscards,      // pfGetIfOutDiscards
  _GetIfOutErrors,        // pfGetIfOutErrors
  _GetIfOutQLen,          // pfGetIfOutQLen
  _GetIfSpecific          // pfGetIfSpecific
};

/*************************** End of file ****************************/
