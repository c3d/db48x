/*
#include "h7a3_rtc.h"
#include "h7a3_usb.h"

#include "h7a3_low_power.h"


#include "FS.h"
*/

#include "BSP.h"
#include "DBxxxx.h"
#include "IP.h"

#include "USB_IP.h"

/*

#ifdef __cplusplus
extern "C" {     // Make sure we have C-declarations in C++ programs 
#endif
void DBx_Task_usb(void);
#ifdef __cplusplus
}
#endif
*/

enum {
   TASK_PRIO_SNTP = 140
   ,TASK_PRIO_IP_WEBSOCKET = 150
   ,TASK_PRIO_FTPS_CHILD
   ,TASK_PRIO_FTPS_PARENT
   ,TASK_PRIO_IP_TASK           // Priority should be higher than all IP application tasks.
#if USE_RX_TASK
  ,TASK_PRIO_IP_RX_TASK        // Must be the highest priority of all IP related tasks, comment out to read packets in ISR
#endif
};

#define NTP_SERVER  "us.pool.ntp.org"
//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int Stack_IP[TASK_STACK_SIZE_IP_TASK/sizeof(int)];             // Stack of the IP_Task.
static OS_STACKPTR int Stack_pFTP[2048];                  /* Task stack */
static OS_STACKPTR int Stack_WSK[2048];                  /* Task stack */
static OS_STACKPTR int Stack_SNTP[1024];                  /* Task stack */

static OS_TASK        TCB_SNTP, TCB_IP, TCB_pFTP, TCB_WSK;                                                    // Task-Control-Block of the IP_Task.

volatile int32_t ip_connected = 0;
static IP_HOOK_ON_STATE_CHANGE _StateChangeHook;
static int  _IFaceIdCte = 0;                                           // Get the last registered interface ID as this is most likely the interface we want to use in this sample.

/*********************************************************************
*
*       _OnStateChange()
*
* Function description
*   Callback that will be notified once the state of an interface
*   changes.
*
* Parameters
*   IFaceId   : Zero-based interface index.
*   AdminState: Is this interface enabled ?
*   HWState   : Is this interface physically ready ?
*/
static void _OnStateChange(unsigned IFaceId, U8 AdminState, U8 HWState) {
  //
  // Check if this is a disconnect from the peer or a link down.
  // In this case call IP_Disconnect() to get into a known state.
  //
    IP_Logf_Application( "\n_OnStateChange AdminState:%d HWstate:%d\n", AdminState, HWState);


   if (((AdminState == IP_ADMIN_STATE_DOWN) && (HWState == 1)) ||  // Typical for dial-up connection e.g. PPP when closed from peer. Link up but app. closed.
      ((AdminState == IP_ADMIN_STATE_UP)   && (HWState == 0))) {  // Typical for any Ethernet connection e.g. PPPoE. App. opened but link down.
      IP_Disconnect(IFaceId);     
      ip_connected = -1;                                  // Disconnect the interface to a clean state.

   }
   if  ((AdminState == IP_ADMIN_STATE_UP)   && (HWState == 1)) { 
      ip_connected = 1;  
   }
}

void FormatNTPTime(U32 NTPTimestamp, char *pBuf, int BufSize) {
    time_t  UnixTime;
    struct  tm *pTM;
    
    UnixTime = (time_t)(NTPTimestamp - 2208988800UL);
    pTM      = gmtime(&UnixTime);
    strftime(pBuf, BufSize, "%Y-%m-%d %H:%M:%S", pTM);
}

/*
void UpdateRTCFromNTP(U32 NTPTimestamp) {
    time_t        UnixTime;
    struct tm    *pTM;
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    UnixTime = (time_t)(NTPTimestamp - 2208988800UL);
    pTM      = gmtime(&UnixTime);

    // Fill RTC time
    sTime.Hours          = (uint8_t)pTM->tm_hour;
    sTime.Minutes        = (uint8_t)pTM->tm_min;
    sTime.Seconds        = (uint8_t)pTM->tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    // Fill RTC date
    sDate.Year    = (uint8_t)(pTM->tm_year - 100);  // tm_year is years since 1900, RTC wants years since 2000
    sDate.Month   = (uint8_t)(pTM->tm_mon  + 1);    // tm_mon is 0-based
    sDate.Date    = (uint8_t) pTM->tm_mday;
    sDate.WeekDay = (uint8_t)(pTM->tm_wday == 0 ? RTC_WEEKDAY_SUNDAY : pTM->tm_wday);  // tm_wday: 0=Sunday, HAL: 1=Monday..7=Sunday

    // Write to RTC — date MUST be set after time on STM32
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    SEGGER_RTT_printf(0, "RTC updated: %04d-%02d-%02d %02d:%02d:%02d\n",
                      pTM->tm_year + 1900,
                      pTM->tm_mon  + 1,
                      pTM->tm_mday,
                      pTM->tm_hour,
                      pTM->tm_min,
                      pTM->tm_sec);
}
*/

/*
I32 RTC_GetDeltaFromNTP(U32 NTPTimestamp) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    struct tm       TM    = {0};
    time_t          RTCUnixTime;
    time_t          NTPUnixTime;

    // Read RTC — date MUST be read after time
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // Fill struct tm
    TM.tm_year  = sDate.Year + 100;  // years since 1900
    TM.tm_mon   = sDate.Month - 1;   // 0-based
    TM.tm_mday  = sDate.Date;
    TM.tm_hour  = sTime.Hours;
    TM.tm_min   = sTime.Minutes;
    TM.tm_sec   = sTime.Seconds;
    TM.tm_isdst = 0;

    // Convert UTC struct tm to Unix time
    // timegm() is the UTC equivalent of mktime() — no TZ adjustment
//    RTCUnixTime = timegm(&TM);
RTCUnixTime = mktime(&TM) ;

    NTPUnixTime = (time_t)(NTPTimestamp - 2208988800UL);

    // Positive = RTC ahead, negative = RTC behind
    return (I32)(RTCUnixTime - NTPUnixTime);
}
*/


void _SNTP_Task(void) {
   IP_NTP_TIMESTAMP Time;
   char buff[64];
   int first = 0;
   int delta = 0;
      BSP_ClrLED(0);
   while (1){

      if (ip_connected>0)
      {
         RTT_vprintf_cr_time("Requesting time stamp from %s: ", NTP_SERVER);
         int  Status = -11;
         Status = IP_SNTPC_GetTimeStampFromServer(_IFaceIdCte, NTP_SERVER, &Time);
         switch (Status) 
         {
               case IP_SNTPC_STATE_NO_ANSWER:
                  RTT_vprintf("Timeout.\n");
                  break;
               case IP_SNTPC_STATE_UPDATED:
//                  Adjust_Time( &Time);             // lptim3
                  if ( 0 == first) 
                  {
//                     UpdateRTCFromNTP(Time.Seconds);
                     
                     Adjust_Time( &Time);
                     Init_Rtc(true);
                     first = 1;
                  }
                  else
                  {
                     delta = Adjust_Time( &Time);
                  }
//                  FormatNTPTime(Time.Seconds, buff, sizeof(buff));

                  time_t  UnixTime = Get_Time_t(Get_Elapsed_Dual_Res(0));
                  struct  tm *pTM;
                  pTM = gmtime(&UnixTime);
                  strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", pTM);
                  uint64_t fsec64 =((uint64_t) Time.Fractions * 1000)>>32; 
                  BSP_SetLED(0);
                  RTT_vprintf("SNTP: %s, %dmsec, Delta %d,   \n", buff, fsec64, delta);
                  break;
               case IP_SNTPC_STATE_KOD:
                  RTT_vprintf("Kiss-Of-Death received. Use another server.\n");
                  break;
               default:
                  RTT_vprintf("Communication error : %d\n", Status);
                  break;
         }
      }
       OS_TASK_Delay_ms( 60 * 1000 * 5 );
   }
// never
}


void _FTPServerParentTask(void);

void DBx_Task_Usb_ip(void) 
{
   usb_connected = false;
   OS_TASKEVENT MyEvents;
   bool first_run = true;
   
   OS_EVENT_GetBlocked(&USB_Start);
   OS_TASK_Delay_ms(50);
   RTT_vprintf_cr_time(  "Usb : waiting");
   HAL_NVIC_EnableIRQ(USB_DETECT_EXTI_IRQn);
   while(1){
      char ref_w =0;
      while (!Usb_Detect())    
      {  
         if (OS_EVENT_GetTimed(&EV_USB_Vbus, 100) == 0) break;
      }

//      usb_connected = true;
// same interrupt than keyboard...
      HAL_NVIC_DisableIRQ(USB_DETECT_EXTI_IRQn);
      RTT_vprintf_cr_time( "USB : connecting\n");
      OS_TASK_Delay_ms(250);

//      USBD_Init();
//      _FSTest();

      if (first_run )
      {
         IP_Init();
         #if USE_SSL
            SSL_Init();
         #endif
         IP_AddStateChangeHook(&_StateChangeHook, _OnStateChange);                            // Register hook to be notified on disconnects.

         OS_CREATETASK(&TCB_IP  , "IP_Task"  , IP_Task  , TASK_PRIO_IP_TASK   , Stack_IP);    // Start the IP_Task.
      }
      IP_Connect(_IFaceIdCte);    
   // Wait until link is up and interface is configured.
      while (IP_IFaceIsReadyEx(_IFaceIdCte) == 0) {
         OS_Delay(50);
         BSP_ToggleLED(0);
      }
      ip_connected = 1;

      IP_SetGWAddr(_IFaceIdCte, IP_BYTES2ADDR(100, 127, 137, 2));  // two params: iface + GW
      IP_DNS_SetServer(IP_BYTES2ADDR(8, 8, 8, 8));             // one param only, no index
      SEGGER_RTT_printf(0, "GW and DNS configured\n");

      if (first_run ){
         OS_Delay(2500);
         OS_CREATETASK(&TCB_pFTP  , "pFTP_Task"  , _FTPServerParentTask  , TASK_PRIO_FTPS_PARENT   , Stack_pFTP);  
         OS_CREATETASK(&TCB_SNTP  , "SNTP_Task"  , _SNTP_Task  , TASK_PRIO_SNTP   , Stack_SNTP);  
      } 
      first_run = false;        
      OS_TASKEVENT_Set( &TDB48X, EV_DBx_USB_CON);
      MyEvents = OS_TASKEVENT_GetSingleTimed(EV_USB_ACQ, 50);

      while (Usb_Detect()) 
      {
         OS_Delay(500);
         if (ip_connected <0) 
         {
               int NewState = USBD_GetState();
               SEGGER_RTT_printf(0, "USB status %02x\n", NewState);
               OS_Delay(250);
   //            USBD_Stop();
               OS_Delay(100);
   //            USBD_Start();
               ip_connected = 0;
         }    
      }
// deconnexion
      // kill tasks sntp & ftp ????
      OS_TASK_EnterRegion();
      //OS_TASK_Terminate(&TCB_SNTP);
      //OS_TASK_Terminate(&TCB_pFTP);
      //OS_TASK_Terminate(&TCB_IP);
      //USB_OS_DeInit();
      //IP_DeInit();
      OS_TASK_LeaveRegion();
      FS_Sync("nor:0:");
      //    usb_connected = false;
      RTT_vprintf_cr_time( "USB : disconnect");
      //    USBD_DeInit();
      //     Init_Usb_Detect();
      OS_EVENT_Reset(&EV_USB_Vbus);
      HAL_NVIC_ClearPendingIRQ(USB_DETECT_EXTI_IRQn);
      HAL_NVIC_EnableIRQ(USB_DETECT_EXTI_IRQn);
      OS_TASK_Delay_ms(1);

// 135µA, 105µA higher than before usb connection 
// sd detect !!!
// 60µA instead of 30µA
//      HAL_PWREx_DisableUSBVoltageDetector();       
// back to 30µA

//      __HAL_RCC_USB_OTG_HS_CLK_DISABLE();    // needed ????

// can't modify pll when running on pll, pll => HSI, pll mod => pll
//      SystemClock_Config_MSIS24_MSIK24();       // no pll
//      SystemClock_Config_P160();                // fmax
//      SystemCoreClockUpdate();

      __HAL_RCC_USB_CLK_DISABLE();
      __HAL_RCC_HSI48_DISABLE();

// 4. Power down USB PHY
      USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_PWRDWN;
      HAL_PWREx_DisableVddUSB(); // 86µA instead of 100µA, still 26µA off !!!
 
// 7. Disable USB interrupts
      HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
      HAL_NVIC_ClearPendingIRQ(OTG_FS_IRQn); 

       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);         //cs    
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
       GPIO_InitTypeDef GPIO_InitStruct = {0};
       GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
       GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
       GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
       HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      __HAL_RCC_GPIOC_CLK_DISABLE();

//      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);         //cs
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
      GPIO_InitStruct.Pin = GPIO_PIN_13;
//   GPIO_InitStruct.Pin = GPIO_PIN_ALL & (!( GPIO_PIN_14 | GPIO_PIN_15));
//   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      OS_TASKEVENT_Set( &TDB48X, EV_DBx_USB_DIS);
// this task if higher priority than DB48X, giving some time to work after sending USB_DIS
      MyEvents = OS_TASKEVENT_GetSingleTimed(EV_USB_ACQ, 250);

// a déplacer ???
//      usb_connected = false;        // ==> power stop enabled
   }     // end while(1)
}


void DBx_Task_Usb(void) 
{
   usb_connected = false;
   OS_TASKEVENT MyEvents;
   
   OS_EVENT_GetBlocked(&USB_Start);
//   Init_Usb_Detect();                   // moved in main()
//   HAL_NVIC_EnableIRQ(USB_DETECT_EXTI_IRQn);
   OS_TASK_Delay_ms(50);

//   OS_EVENT_GetBlocked(&USB_Start);
   RTT_vprintf_cr_time(  "Usb : waiting");

   while(1){
      if (!usb_connected)      OS_EVENT_GetBlocked(&EV_USB_Vbus);
//      usb_connected = true;

// same interrupt than keyboard...
     HAL_NVIC_DisableIRQ(USB_DETECT_EXTI_IRQn);
      RTT_vprintf_cr_time( "USB : connecting");

//      SystemClock_Config_MSIS24_MSIK24();       // no pll
//      SystemClock_Config_P160();                // fmax

      USBD_Init();

//      _FSTest();

      _AddMSD();
      USBD_Start();

      OS_TASKEVENT_Set( &TDB48X, EV_DBx_USB_CON);
      MyEvents = OS_TASKEVENT_GetSingleTimed(EV_USB_ACQ, 50);

      while (Usb_Detect()) {
       //
       // Wait for configuration
       int usb_conting = 0;
         while (((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED)&&(Usb_Detect())) {
            usb_conting +=1;
            if (usb_conting > 500) usb_connected = false;
//            BSP_ToggleLED(0);
            OS_TASK_Delay_ms(50);

         }
//         BSP_SetLED(0);
         USBD_MSD_Task();
         OS_TASK_Delay_ms(50);
         if ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) break;
      }
      // deconnexion

      FS_Sync("nor:0:");
  //    usb_connected = false;

      RTT_vprintf_cr_time( "USB : disconnect");
      USBD_DeInit();

 //     Init_Usb_Detect();

      OS_EVENT_Reset(&EV_USB_Vbus);
      HAL_NVIC_ClearPendingIRQ(USB_DETECT_EXTI_IRQn);
      HAL_NVIC_EnableIRQ(USB_DETECT_EXTI_IRQn);
      OS_TASK_Delay_ms(1);

// 135µA, 105µA higher than before usb connection 
// sd detect !!!
// 60µA instead of 30µA
//      HAL_PWREx_DisableUSBVoltageDetector();       
// back to 30µA

//      __HAL_RCC_USB_OTG_HS_CLK_DISABLE();    // needed ????

// can't modify pll when running on pll, pll => HSI, pll mod => pll
//      SystemClock_Config_MSIS24_MSIK24();       // no pll
//      SystemClock_Config_P160();                // fmax
//      SystemCoreClockUpdate();


__HAL_RCC_USB_CLK_DISABLE();
__HAL_RCC_HSI48_DISABLE();

// 4. Power down USB PHY
USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_PWRDWN;

HAL_PWREx_DisableVddUSB(); // 86µA instead of 100µA, still 26µA off !!!
 
// 7. Disable USB interrupts
HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
HAL_NVIC_ClearPendingIRQ(OTG_FS_IRQn); 

      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);         //cs
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   __HAL_RCC_GPIOC_CLK_DISABLE();

//      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);         //cs
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
   GPIO_InitStruct.Pin = GPIO_PIN_13;
//   GPIO_InitStruct.Pin = GPIO_PIN_ALL & (!( GPIO_PIN_14 | GPIO_PIN_15));
//   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);








      OS_TASKEVENT_Set( &TDB48X, EV_DBx_USB_DIS);
// this task if higher priority than DB48X, giving some time to work after sending USB_DIS
      MyEvents = OS_TASKEVENT_GetSingleTimed(EV_USB_ACQ, 50);

// a déplacer ???
//      usb_connected = false;        // ==> power stop enabled
   }
}
