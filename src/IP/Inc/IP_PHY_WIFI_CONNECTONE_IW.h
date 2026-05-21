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

File    : IP_PHY_WIFI_CONNECTONE_IW.h
Purpose : Header file for PHY driver for ConnectOne WiReach LAN to WiFi bridges.
*/

#ifndef IP_PHY_WIFI_CONNECTONE_IW_H  // Avoid multiple inclusion.
#define IP_PHY_WIFI_CONNECTONE_IW_H

#include "IP_Int.h"

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/

typedef struct {
  void (*pfHWReset)      (unsigned IFaceId);
  int  (*pfSendATCommand)(unsigned IFaceId, U32 Timeout, const char* sCmd);
  int  (*pfClrBuf)       (unsigned IFaceId, U32 Timeout, char IgnoreSpiInt);
  int  (*pfLoadLine)     (unsigned IFaceId, U32 Timeout);
  int  (*pfReadLine)     (unsigned IFaceId, U32 Timeout, char* pBuffer, unsigned BufferSize);
} IP_PHY_WIFI_CONNECTONE_IW_ACCESS;

extern const IP_PHY_HW_DRIVER IP_PHY_Driver_ConnectOne_iW;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

void IP_PHY_WIFI_CONNECTONE_IW_ConfigSPI(unsigned IFaceId, char Port);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
