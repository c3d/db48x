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

File    : IP_PHY_MICREL_SWITCH.h
Purpose : Header file for PHY driver for Micrel switch PHYs.
*/

#ifndef IP_PHY_MICREL_SWITCH_H  // Avoid multiple inclusion.
#define IP_PHY_MICREL_SWITCH_H

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
  unsigned (*pfReadReg) (IP_PHY_CONTEXT_EX* pContext, unsigned Reg);
  void     (*pfWriteReg)(IP_PHY_CONTEXT_EX* pContext, unsigned Reg, unsigned v);
} IP_PHY_MICREL_SWITCH_ACCESS;

extern const IP_PHY_HW_DRIVER IP_PHY_Driver_Micrel_Switch_KSZ8863;           //lint !e621 Ignore identifier clash for 31 unique chars in name.
extern const IP_PHY_HW_DRIVER IP_PHY_Driver_Micrel_Switch_KSZ8863_HostPort;  //lint !e621 Ignore identifier clash for 31 unique chars in name.
extern const IP_PHY_HW_DRIVER IP_PHY_Driver_Micrel_Switch_KSZ8895;           //lint !e621 Ignore identifier clash for 31 unique chars in name.
extern const IP_PHY_HW_DRIVER IP_PHY_Driver_Micrel_Switch_KSZ8895_HostPort;  //lint !e621 Ignore identifier clash for 31 unique chars in name.

//
// Macros for compatible devices.
//
#define IP_PHY_Driver_Micrel_Switch_HostPort          IP_PHY_Driver_Micrel_Switch_KSZ8895_HostPort  //lint !e621 Ignore identifier clash for 31 unique chars in name.
#define IP_PHY_Driver_Micrel_Switch_KSZ8794_HostPort  IP_PHY_Driver_Micrel_Switch_KSZ8895_HostPort  //lint !e621 Ignore identifier clash for 31 unique chars in name.
#define IP_PHY_Driver_Micrel_Switch_KSZ8794           IP_PHY_Driver_Micrel_Switch_KSZ8895           //lint !e621 Ignore identifier clash for 31 unique chars in name.

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

void IP_PHY_MICREL_SWITCH_AssignPortNumber          (unsigned IFaceId, unsigned Port);
void IP_PHY_MICREL_SWITCH_ConfigLearnDisable        (unsigned IFaceId, unsigned OnOff);
void IP_PHY_MICREL_SWITCH_ConfigTailTagging         (unsigned IFaceId, unsigned OnOff);
void IP_PHY_MICREL_SWITCH_ConfigTxEnable            (unsigned IFaceId, unsigned OnOff);
void IP_PHY_MICREL_SWITCH_ConfigRxEnable            (unsigned IFaceId, unsigned OnOff);
void IP_PHY_MICREL_SWITCH_ConfigUseInternalRmiiClock(unsigned IFaceId, unsigned OnOff);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
