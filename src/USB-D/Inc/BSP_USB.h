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
File        : BSP_USB.h
Purpose     : BSP (Board support package) for USB.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _BSP_USB_H_     // Avoid multiple/recursive inclusion.
#define _BSP_USB_H_  1

#if defined(__cplusplus)
extern "C" {  /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// In order to avoid warnings for unused parameters.
//
#ifndef BSP_USE_PARA
  #if defined(NC30) || defined(NC308)
    #define BSP_USE_PARA(para)
  #else
    #define BSP_USE_PARA(para) (void)para;
  #endif
#endif

/*********************************************************************
*
*       USBD
*
* Functions for USB device controllers (as far as present).
*/
void BSP_USB_InstallISR      (void (*pfISR)(void));
void BSP_USB_InstallISR_Ex   (int ISRIndex, void (*pfISR)(void), int Prio);
void BSP_USB_ISR_Handler     (void);
void BSP_USB_Init            (void);
void BSP_USB_EnableInterrupt (int ISRIndex);
void BSP_USB_DisableInterrupt(int ISRIndex);

/*********************************************************************
*
*       USBH
*
* Functions for USB Host controllers (as far as present).
*/
void BSP_USBH_InstallISR   (void (*pfISR)(void));
void BSP_USBH_InstallISR_Ex(int ISRIndex, void (*pfISR)(void), int Prio);
void BSP_USBH_Init         (void);

/*********************************************************************
*
*       CACHE
*
* Functions for cache handling (as far as present).
*/
void BSP_CACHE_CleanInvalidateRange(void *p, unsigned NumBytes);
void BSP_CACHE_CleanRange          (void *p, unsigned NumBytes);
void BSP_CACHE_InvalidateRange     (void *p, unsigned NumBytes);


#if defined(__cplusplus)
  }     // Make sure we have C-declarations in C++ programs
#endif

#endif  // Avoid multiple/recursive inclusion

/****** End Of File *************************************************/
