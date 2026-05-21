
//#include "Global.h"

#include "BSP.h"
#include "DBxxxx.h"

#include "leitz5006.h"
#include "leitz5006_1.h"
#include "leitz5006_2.h"

#include "leitz5006_400_240.h"



keyboard OS_RAM keybd;
uint64_t scrut_last = 0;

uint8_t     volatile  StopMode2_enable = 0x00;


extern uint8_t OS_RAM wakeup_from;

extern OS_MAILBOX  Mb_Keyboard;
extern   OS_TASK     TKBD, TDB48X, TUSB;


#ifdef DM42
#  pragma GCC push_options
#  pragma GCC optimize("-O0")
#endif // DM42


extern U8 _IsInitialized;
void     _HW_DeInit(U8 Unit);
int      _HW_Init(U8 Unit);
extern   bool wake_from_lptim3;


void DBx_Task_Kbd(void) 
/* keyboard and scheduler task
 * key repetition by db48x 
 * send struct st_key_data.
 * active key : bool released, int key, allready pressed : key1, key2, key3 
 * sending events to other tasks : EV_DBx_RESET

 * using db_power_state
 */
{
   uint32_t tmpu32;

   uint32_t running_time;
//   uint32_t new_sleeping_time_sec = 0;
   OS_TASKEVENT MyEvents;
   bool lcd_force_update = true;
//   char toggle='s';
   char buff[50];
int32_t delta_t = 0;
   KeyboardInit( &keybd);

   HAL_LPTIM_Counter_Start(&hlptim1);

   // Set auto-reload value to 1mn
   __HAL_LPTIM_AUTORELOAD_SET(&hlptim3, 1024*60-1);
   HAL_LPTIM_Counter_Start_IT(&hlptim3);

   // Clear any pending flags.
//   __HAL_LPTIM_CLEAR_FLAG(&hlptim1, LPTIM_FLAG_ARRM);
   uint32_t isr_status1 = LPTIM1->ISR;
   LPTIM1->ICR = isr_status1;
   uint32_t isr_status3 = LPTIM3->ISR;
   LPTIM3->ICR = isr_status3;


   // Start the timer.
   __HAL_RCC_LPTIM1_CLKAM_ENABLE();
   __HAL_RCC_LPTIM3_CLKAM_ENABLE();

      Disp_Init(&hlcd);

   vbat = ReadBatVoltage();
   vbat_rd_time = sys_current_ms();
Init_Time(false);
Init_Power_On_Stamp();


//   LCD_Status_t lcd_res = LCD_Sharp_Init(&hlcd, true);
   RTT_vprintf_cr_time( "LCD status : %s, add : %08x", LCD_Status_Desc[hlcd.status], LCD_GetFramebuffer());
   snprintf(buff, sizeof(buff), "%s v%s %luMhz %dko Lcd %dx%d 49keys H",HARD_NAME, HARD_VERSION, SystemCoreClock/1000000, DB_MEM_SIZE, LCD_WIDTH, LCD_HEIGHT);
   display_text_12x24(&hlcd, 2, 4, buff,1);
   int32_t st_refresh = Get_Elapsed_Dual_Res(0);
   Disp_Update(&hlcd, true, false);
   Disp_WaitForTransfer(&hlcd);
   uint32_t rfr_t =  Get_Elapsed_Dual_Res(st_refresh);

   RTT_vprintf_cr_time( "LCD refresh time :%dmsec, add : %08x", rfr_t, LCD_GetFramebuffer());

//   LCD_UpdateModifiedLines( &hlcd, true);

   FS_Init();
   FS_FAT_SupportLFN();
   OS_TASK_Delay_ms(500);     // 

   OS_EVENT_Set(&EV_db_lcd_kbd_ready);
   keybd.sleeping_soon = 0;
   db_power_state = PW_power_on;
   db_previous_power_state = db_power_state;

   while(1) 
   {   
   // moving lcd refresh here
      MyEvents = OS_TASKEVENT_Clear(NULL);

      if (( EV_LCD_F_REFFRESH & MyEvents) || lcd_force_update)
      {
//         LCD_UpdateModifiedLines( &hlcd, true);
         lcd_force_update = false;
         OS_TASKEVENT_Set( &TDB48X, EV_DBx_LCD);
      }
      else if ( EV_LCD_REFFRESH & MyEvents)
      {
//         LCD_UpdateModifiedLines( &hlcd, false);
         lcd_force_update = false;
         OS_TASKEVENT_Set( &TDB48X, EV_DBx_LCD);
      }
      else if (( EV_KPo_ReqPOff & MyEvents) && (!Usb_Detect()))
      {
         db_previous_power_state = db_power_state;
         db_power_state = PW_request_Poff;
      }

      switch (db_power_state){
         case PW_power_on:
            OS_EVENT_GetBlocked(&EV_db_app_ready);
StopMode2_enable = 0x00;
            db_previous_power_state = db_power_state;
            db_power_state = PW_power_on_waiting;
            break;

         case PW_power_on_waiting:
            Kbd_Scrut_Set( &keybd,kb_scrut_std);
            bool wait_no_key = true;
            bool waiting_key = false;
// debug connection : after power off, keep pressed [on]

            while (wait_no_key)
            {
               scrut_last = Scrutation( &keybd, false);
               if (scrut_last == 0)
               {
                  wait_no_key = false;
               }
               if ( 0x0060 == scrut_last)       
               {  // [F3] + [F4] = Format

                  _FSTest(true);
                  OS_TASKEVENT_Set( &TDB48X, EV_DBx_RESET);
                  OS_TASK_Delay_ms(100);     // 
                  Disp_Update(&hlcd, true, true);
                  OS_TASK_Delay_ms(3200);
                  NVIC_SystemReset();
               }

               if ( wait_no_key && !waiting_key)
               {
                  HAL_DBGMCU_EnableDBGStopMode();
                  waiting_key = true;
               }
               uint32_t raw_ll = keybd.raw &  ((uint64_t)0xffffffff);
               uint32_t raw_hh = keybd.raw >> ((uint64_t) 32);
               snprintf(buff, sizeof(buff), "%08lx %08lx ", raw_hh, raw_ll);
//               LCD_FillRect( &hlcd, 10, 120, 216, 24, 0);
//               display_text_12x24(&hlcd, 10, 120, buff,1);
//               LCD_UpdateModifiedLines(&hlcd, true);
               OS_TASK_Delay_ms(50);
            }
            if ( PW_deep_sleep == db_previous_power_state)
            {
//               OS_TASKEVENT_Set( &TDB48X, EV_DBx_WAKEUP);
               OS_TASKEVENT_Set( &TDB48X, EV_DBx_PON);

            }
            else
            {
               OS_TASKEVENT_Set( &TDB48X, EV_DBx_PON);
            }

            keybd.sleeping_soon=0;

// Origin of time is here, 31bits in msec : 596h more than 24days
Init_Power_On_Stamp();

            db_previous_power_state = db_power_state;
            db_power_state = PW_wake_up;
            StopMode2_enable = 0xff;
            break;
            

         case PW_wake_up:
            Kbd_Scrut_Set( &keybd,kb_scrut_std);
            _HW_Init(0);                         // file system interface init

if ((StopMode2_enable > 0) && ( StopMode2_enable < 16))     StopMode2_enable -=1;

            keybd.sleeping_soon = 0;
            db_previous_power_state = db_power_state;
            db_power_state = PW_running;
            running_time = sys_current_ms();
            break;

         case PW_running:
            if (Usb_Detect())
            {
               keybd.sleeping_soon = 0;
               StopMode2_enable = 0x00;
               if ( false == usb_connected )
               {   // missed exti interrupt !!!!!
                  RTT_vprintf_cr_T_F_L( DBx_Task_Kbd, DL_low_power |DL_usb,"Missed usb connecte PA0 exti");
                  OS_TASKEVENT_Set( &TDB48X, EV_DBx_USB_CON);
                  OS_EVENT_Set(&EV_USB_Vbus);
               }
            }

            scrut_last = Scrutation( &keybd, true);
            if ( 0x8108 == scrut_last)       
            {  // [F1] + [F6] + [Exit] = reset
               OS_TASKEVENT_Set( &TDB48X, EV_DBx_RESET);
               OS_TASK_Delay_ms(100);     // 
               Disp_Update(&hlcd, true, true);
               OS_TASK_Delay_ms(2500);
               NVIC_SystemReset();
            }
            OS_TASK_Delay( TIME_CORRECTION(KB_SCRUT_PERIOD) );
            if (  Usb_Detect() | 
                  (StopMode2_enable==0) | 
                  (scrut_last != 0) | 
                  (key_remaining() != 0) )
               keybd.sleeping_soon = 0;
            else 
               keybd.sleeping_soon += KB_SCRUT_PERIOD ;

            if ((keybd.sleeping_soon > TIME_BEFORE_STOP2_msec)
               &&(  sys_elapsed_ms(running_time) > TIME_BEFORE_STOP2_msec))
            {
               db_previous_power_state = db_power_state;
               db_power_state =  PW_sleeping;
               _HW_DeInit(0);
               SHARP_SPI_DeInit(&hlcd);
               Kbd_Scrut_Set( &keybd,kb_scrut_int); // 60s, 10s, 1s, or wake up following timer1 setting
            }


            break;

         case PW_sleeping:
            tmpu32 =  sys_timer_period(1)>60000 ? 60000 :  sys_timer_period(1);
                           RTT_vprintf_cr_T_F_L( DBx_Task_Kbd, DL_low_power, "Entering sleep for %d.%03ds", tmpu32/1000, tmpu32%1000);
            //deinit FS in nor driver 
            int32_t sleep_time = sys_current_ms();

            // give some time to rtt, but not blocking
            // check a last time if a key is pressed
            MyEvents = OS_TASKEVENT_GetTimed( 0xffffff , 2);     
            if ( 0 == MyEvents)
            {
               wakeup_from = 0;
/*********************************************************************************/
               // from EnterSTOP2
               init_unused_pins();
               // stop systick
               SysTick->CTRL = 0;                     // Stops counter and clock
               SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;    // Clear pending

                // Clear all EXTI pending bits, STM32U5 uses RPRx/FPRx, not PRx
                EXTI->RPR1 = 0xFFFFFFFFu;          // EXTI rising edge pending register (EXTI_RPR1)
                EXTI->FPR1 = 0xFFFFFFFFu;          // EXTI falling edge pending register (EXTI_FPR1)
#if STOP2_ENABLE_DEBUG
                  // keeping debug enabled cost 900µA
                  HAL_DBGMCU_EnableDBGStopMode();
#else 
                  HAL_DBGMCU_DisableDBGStopMode(); 
#endif // STOP2_ENABLE_DEBUG
                  // keeping Icache enabled cost 12µA
                  HAL_ICACHE_Disable();              
                  // enter stop mode 2
                  do
                  {
                     wake_from_lptim3 =  false;
                     HAL_PWREx_EnterSTOP2Mode(  PWR_STOPENTRY_WFI);  //
                  }
                  while ( wake_from_lptim3);


                  SystemClock_Config_P160();
                  HAL_ICACHE_Enable();
                  // get wake up source
                  MyEvents = OS_TASKEVENT_Clear(NULL);
            }
     
           keybd.sleeping_soon += sys_elapsed_ms(sleep_time); 
if ( StopMode2_enable < 16)   keybd.sleeping_soon = 0;
//            keybd.sleeping_soon = StopMode2_enable ? keybd.sleeping_soon + sys_elapsed_ms(sleep_time): 0; 

            if ( EV_KPo_LPTIM & MyEvents)
            { 
               int32_t delta_lptim3  = 0;
               RTT_vprintf_cr_T_F_L( DBx_Task_Kbd, DL_low_power, "d_lpt3(%+d.%03ds) Wake up from lptim1, sleeping soon %d.%03ds ", 
                                    delta_lptim3 / 1000, (abs(delta_lptim3)) % 1000, 
                                    keybd.sleeping_soon/1000, keybd.sleeping_soon%1000);
               if (keybd.sleeping_soon >T_POWER_OFF_sec*1000) 
               { // auto power off
                  OS_TASKEVENT_Set( &TDB48X, EV_DBx_POFF);
                  OS_TASK_Delay( TIME_CORRECTION(100) );
                  db_previous_power_state = db_power_state;
                  db_power_state = PW_request_Poff;
                  break;
               } 
               else
               {
                  OS_TASKEVENT_Set( &TDB48X, EV_DBx_LPTIM1);
                  db_previous_power_state = db_power_state;
                  db_power_state = PW_wake_up;
                  break;
               } 
            }
            if ( EV_KPo_KBD & MyEvents)
            { 
               RTT_vprintf_cr_T_F( DBx_Task_Kbd, "(%d) Wake up from kbd ", keybd.sleeping_soon);
               db_previous_power_state = db_power_state;
               db_power_state = PW_wake_up;
               break;
            } 
            if ( EV_KPo_USB & MyEvents)
            { 
//               usb_connected = Usb_Detect();
               OS_TASKEVENT_Set( &TDB48X, EV_DBx_USB_CON);

               OS_EVENT_Set(&EV_USB_Vbus);                     // wake-up usb task

               RTT_vprintf_cr_T_F_L( DBx_Task_Kbd, DL_low_power, "(%d) Wake up from usb bus : %s ", keybd.sleeping_soon, usb_connected ? "connected":"disconnected");
               db_previous_power_state = db_power_state;
               db_power_state = PW_wake_up;
               break;
            } 
            if ( Usb_Detect() && (!usb_connected))
            {  // nécessaire ???
                  RTT_vprintf_cr_T_F_L( DBx_Task_Kbd, DL_low_power | DL_usb, "(%d) Missed exti from usb bus : %s ", keybd.sleeping_soon, usb_connected ? "connected":"disconnected");
                  usb_connected = true;
                  keybd.sleeping_soon = 0;
                  db_previous_power_state = db_power_state;
                  db_power_state = PW_wake_up;
                  OS_EVENT_Set(&EV_USB_Vbus);
                  break;
            }
            break;

         case PW_request_Poff:         // from sysmenu.cc, EV_KPo_ReqPOff, fct power_off()
            OS_TASK_EnterRegion();
            Kbd_Scrut_Set( &keybd, kb_scrut_int); 
            db_previous_power_state = db_power_state;
            db_power_state = PW_deep_sleep;
            OS_TASK_LeaveRegion();
            break;

         case PW_near_deep_sleep:
            if (Usb_Detect())
            {
               db_previous_power_state = PW_near_deep_sleep;
               db_power_state = PW_wake_up;
            }
            else
            {
               OS_TASK_EnterRegion();
               // deinit FS in nor driver 
               _HW_DeInit(0);
               HAL_LPTIM_MspDeInit(&hlptim1);
               Kbd_Scrut_Set( &keybd, kb_scrut_int); 
               db_previous_power_state = db_power_state;
               db_power_state = PW_deep_sleep;
               OS_TASK_LeaveRegion();
            }
            break;

         case PW_deep_sleep:
            OS_TASK_EnterRegion();
            Kbd_Scrut_Set( &keybd, kb_scrut_int_exit);
#if      SHARP_32_536x336
            memcpy( LCD_GetFramebuffer(), &leitz5006_1[0], LCD_GetFramebuffer_size());
#elif    SHARP_27_400x240
            memcpy( LCD_GetFramebuffer(), &leitz5006_400_240[0], LCD_GetFramebuffer_size());
#endif
               Disp_Update(&hlcd, true, true);
//            OS_TASK_Delay_ms(60);
            RTT_vprintf_cr_T_F_L( DBx_Task_Kbd, DL_low_power, "(%d) PowerOff", keybd.sleeping_soon);
            MyEvents = OS_TASKEVENT_Clear(NULL)<<16;
            uint32_t poweoff_time = rtc_read_ticks();

            EnterSTOP2();

            HAL_LPTIM_MspInit(&hlptim1);        
// check if file system is working better without timer 1 interrupts ????       
   __HAL_RCC_LPTIM1_CLKAM_ENABLE();


            MyEvents |= OS_TASKEVENT_Clear(NULL);

            RTT_vprintf_cr_T_F_L( DBx_Task_Kbd, DL_low_power, "PW : Leaving P Off : %X, after %d", MyEvents, rtc_elapsed_ticks(poweoff_time));
            db_previous_power_state = db_power_state;
            db_power_state = PW_power_on_waiting;

            OS_TASK_LeaveRegion();
            break;
         default:
            break;
      }
#ifdef DEBUG
      // receiving keys from segger j-link 
      int key = SEGGER_RTT_GetKey();
      if (key >= 0) {
         keybd.sleeping_soon = 0;
       // Key was pressed, 'key' contains the ASCII value          
         uint32_t db_key_from_RTT = RTT_Key_Decode(key);
         if  (db_key_from_RTT != 0) {
            Send_key(db_key_from_RTT);
         } else  {
            SEGGER_RTT_printf(0, "\nYou pressed: %d (0x%02X)", key, key);
         }
      } 
#endif // DEBUG
   }
}

#ifdef DM42
#  pragma GCC pop_options
#endif // DM42


