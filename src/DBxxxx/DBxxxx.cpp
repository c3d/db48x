#include "DBxxxx.h"
#include "blitter.h"
#include "dmcp.h"
#include "expression.h"
#include "font.h"
#include "program.h"
#include "recorder.h"
#include "stack.h"
#include "sysmenu.h"
#include "target.h"
#include "user_interface.h"
#include "util.h"


 LCD_Handle_t     OS_RAM       hlcd;
uint32_t      lcd_refresh_requested = 0;

extern settings Settings;


LCD_Status_t lcd_forced_refresh(void)
{
   lcd_refresh_requested++;   
   return Disp_Update(&hlcd, true, false);
}

LCD_Status_t lcd_refresh(void)
{
   lcd_refresh_requested++;
   return Disp_Update(&hlcd, false, false);
}

//undefined symbol lcd_forced_refresh referenced by symbol menu_item_run(unsigned char) (section .text.menu_item_run(unsigned char) in file Start_STM32U5G9v1f_lto.o)
// undefined symbol lcd_refresh referenced by symbol handle_menu.constprop.0 (section .text.handle_menu.constprop.0 in file Start_STM32U5G9v1f_lto.o)
LCD_Status_t lcd_refresh_dma(void)
{
   lcd_refresh_requested++;
   return Disp_Update(&hlcd, false, false);
}

LCD_Status_t lcd_refresh_wait(void)
{
//SEGGER_RTT_printf(0, "\nWait refresh requested %u drawn %u",
//           lcd_refresh_requested, ui_refresh_count());
    lcd_refresh_requested++;
   return Disp_Update(&hlcd, false, true);
}

LCD_Status_t lcd_refresh_lines(int ln, int cnt)
{
// used one time

   if (ln > 0 && ln < LCD_HEIGHT && cnt > 0)
   {
      hlcd.modified.x_min = 0;
      hlcd.modified.x_max = LCD_WIDTH-1;
      hlcd.modified.y_min = ln;
      hlcd.modified.y_max = ln + cnt < LCD_WIDTH ? ln + cnt : LCD_WIDTH-1 ;
      lcd_refresh_requested++;
      return Disp_Update(&hlcd, false, false);
     }
   return LCD_ERROR;
}





int       ui_file_selector(const char *title,
                           const char *base_dir,
                           const char *ext,
                           file_sel_fn callback,
                           void       *data,
                           int         disp_new,
                           int         overwrite_check){
                     
   RTT_vprintf_cr_time( "ui_file selector : %s, %s, %s", title, base_dir, ext);
}                    







void rtc_write(tm_t * tmarg, dt_t *dtarg)
// from rpl set date and time
// 18.1015 SETTIME (18h10min15sec)
// 20251030 SETDATE (30 octobre 2025)
// transform db48x time in time_t and call RTC_set_time_t
{
    struct tm t = {           // c++ order of declaration !
        .tm_sec  = tmarg->sec,
        .tm_min  =  tmarg->min,
        .tm_hour = tmarg->hour,
        .tm_mday = dtarg->day,
        .tm_mon  = dtarg->month - 1,
        .tm_year =  dtarg->year - 1900,
        .tm_isdst = -1
    };
    time_t t_time = mktime(&t);
    RTC_set_time_t(t_time);
}






void rtc_read(tm_t * tm, dt_t *dt)
{
// unix time
   int32_t time_msec = Get_Elapsed_Dual_Res(0);
   time_t  UnixTime = Get_Time_t(time_msec);

int i_city = find_city("Paris");
time_t local_time = utc_to_local( Settings.TMzone(), UnixTime);


    struct tm     *pTM;
pTM = gmtime(&local_time);


// result setting
    dt->year = 1900 + pTM->tm_year;
    dt->month = pTM->tm_mon + 1;
    dt->day = pTM->tm_mday;

    tm->hour = pTM->tm_hour;
    tm->min = pTM->tm_min;
    tm->sec = pTM->tm_sec;
    tm->csec = (time_msec%1000)/10;
    tm->dow = (pTM->tm_wday + 6) % 7;
}

void rtc_read_old(tm_t * tm, dt_t *dt)
{
    time_t           now;
    struct tm        utm;
    struct timeval      tv;
   RTC_TimeTypeDef      time_rtc;
   RTC_DateTypeDef      date_rtc;

// toujours ensemble !!!!!!
   HAL_RTC_GetTime(&hrtc, &time_rtc, RTC_FORMAT_BIN);
   HAL_RTC_GetDate(&hrtc, &date_rtc, RTC_FORMAT_BIN);
   uint32_t prescaler = hrtc.Init.SynchPrediv + 1;
   uint32_t msec = (1000000 - ((time_rtc.SubSeconds * 1000000) / prescaler))/1000;


// Fill the struct tm utm
    utm.tm_sec  = time_rtc.Seconds;
    utm.tm_min  = time_rtc.Minutes;
    utm.tm_hour = time_rtc.Hours;
    utm.tm_mday = date_rtc.Date;
    utm.tm_mon  = date_rtc.Month - 1;   // struct tm month range: 0-11
    utm.tm_year = date_rtc.Year + 100;  // struct tm year = years since 1900 (assuming 2000-based RTC)
    utm.tm_wday = date_rtc.WeekDay % 7; // tm_wday: Sunday = 0, RTC: Monday = 1

// result setting
    dt->year = 1900 + utm.tm_year;
    dt->month = utm.tm_mon + 1;
    dt->day = utm.tm_mday;

    tm->hour = utm.tm_hour;
    tm->min = utm.tm_min;
    tm->sec = utm.tm_sec;
    tm->csec = msec/10;
    tm->dow = (utm.tm_wday + 6) % 7;
}


int Check_Disk(void)
// check flash nor disk, 1 : ok, 0 erreur
{
   char         acVolumeName[20] = {"nor:0"};
   int res = FS_IsLLFormatted(acVolumeName);
   RTT_vprintf_cr_time( "FS, low level : %s", (0 == res) ?"error":"ok");
   if (res == 0) {
      RTT_vprintf_cr_time("Low Level formatting");
      FS_FormatLow(acVolumeName);          /* Erase & Low-level  format the volume */
   }
   res = FS_IsHLFormatted(acVolumeName);
   RTT_vprintf_cr_time( "FS, Fat File system : %s", (0 == res) ?"error":"ok");
   if (res == 0)
   {
      RTT_vprintf_cr_time("Low Level and Fat format");
      FS_FormatLow(acVolumeName);          /* Erase & Low-level  format the volume */
      FS_Format(acVolumeName, NULL);       /* High-level format the volume */
      return 0;
   }
   return 1;
}



int usb_powered(void)
{
    return Usb_Detect();
}

void refresh_dirty_if_needed(void);



/***************************************************************************
 * keyboard


*/



int key_tail(void)
/* Check for key in key buffer.
Returns
    Next key in key buffer or -1 if buffer is empty 
*/
{  
st_key_data drcvd;
   char result = OS_MAILBOX_Peek(&Mb_Keyboard, &drcvd);
   if ( 0 == result)
   { // an event key is here, format db48 
      if (drcvd.released == true) return 0;
      if (drcvd.released == false) return key_DB_to_DM(drcvd.key);
   }
   return -1;
}

int key_pop_last(void)
{
   key_tail();
}


void wait_for_key_press(void)
{
st_key_data drcvd;
// OS_MAILBOX_Clear(&Mb_Keyboard);
   SEGGER_RTT_printf(0, "\nWaiting for key press");
//      if (LCD_Need_Refresh( &hlcd)) refresh_dirty();

//   OS_MAILBOX_Clear(&Mb_Keyboard);
   StopMode2_enable = 0xff;
   while(1){
         OS_TASKEVENT_GetSingleTimed( 
         EV_DBx_KBD 
         | EV_DBx_LPTIM1
          
         | EV_DBx_PON
         | EV_DBx_USB_CON 
         | EV_DBx_USB_DIS
         | EV_DBx_RESET, 5000);  // eqivalent to timer0 repeat


      char result = OS_MAILBOX_GetTimed(&Mb_Keyboard, &drcvd, 5000);
      if (result==0)
      { // an envent keyboard occured...
         if (drcvd.released==false)     break;
      }
   }
   OS_MAILBOX_Clear(&Mb_Keyboard);
   StopMode2_enable = 0x00;
}
void wait_for_key_sleeping(void)
{
   st_key_data drcvd;
   SEGGER_RTT_printf(0, "\nWaiting for key press");
   StopMode2_enable = 0xff;
//   while(1){
         OS_TASKEVENT_GetSingleTimed( 
         EV_DBx_KBD 
         | EV_DBx_LPTIM1
         | EV_DBx_PON
         | EV_DBx_USB_CON 
         | EV_DBx_USB_DIS
         | EV_DBx_RESET, 5000);  // eqivalent to timer0 repeat
      char result = OS_MAILBOX_GetTimed(&Mb_Keyboard, &drcvd, 5000);
//      if (result !=0)  break;
//      { // an envent keyboard occured...
//         if (drcvd.released==false)     break;
//      }
//   }
   StopMode2_enable = 0;
}


// a améliorer pour éviter de taper deux fois pour sortir des menus



void wait_for_key_release(int tout)
{
st_key_data drcvd;
   SEGGER_RTT_printf(0, "\nWaiting for key release");
//   refresh_dirty_if_needed();
   StopMode2_enable = 0xff;
   while(1){
            OS_TASKEVENT_GetSingleTimed( 
         EV_DBx_KBD 
         | EV_DBx_LPTIM1
          
         | EV_DBx_PON
         | EV_DBx_USB_CON 
         | EV_DBx_USB_DIS
         | EV_DBx_RESET, 5000);  // eqivalent to timer0 repeat

      char result = OS_MAILBOX_GetTimed(&Mb_Keyboard, &drcvd, 5000);
      if (result==0)
      { // an envent keyboard occured...
         if (drcvd.released == true)      break;
      }
   }
   OS_MAILBOX_Clear(&Mb_Keyboard);
   StopMode2_enable = 0;
}






int key_pop()
/* Remove and return next key from key buffer.
Returns
    Next key in key buffer or -1 if buffer is empty 
Fetch the key (<0: no key event, >0: key pressed, 0: key released)

*/
{
st_key_data drcvd;

   char result = OS_MAILBOX_Get(&Mb_Keyboard, &drcvd);
   if ( 0 == result)
   { // an event key is here, format db48 
      if (drcvd.released == true) return 0;
      if (drcvd.released == false) return key_DB_to_DM(drcvd.key);
   }
   return -1;
}


void key_pop_all()
/* Remove all keys from key buffer. */
{
   OS_MAILBOX_Clear(&Mb_Keyboard);
}



int runner_get_key(int *repeat)
{

    return repeat ? key_pop_last() :  key_pop();
}


//int key_pop_last();
//int key_pop();


int key_remaining(void)
/* key number in the buffer */
{
   return (int)OS_MAILBOX_GetMessageCnt(&Mb_Keyboard);
}

int runner_get_key_(int *repeat)
{
// ?????? runner_get_key_ ?????
    return repeat ? key_pop_last() :  key_pop();
}



// test avec OS_MAILBOX_GetMessageCnt()
int key_empty(void)
/* Check whether key buffer is empty.
Returns
    (0/1) 1 = Key buffer empty 
*/
{
// gérer le mode stop ici ???
//   StopMode2_disable = true;


   OS_UINT nb_mess = OS_MAILBOX_GetMessageCnt(&Mb_Keyboard);
   if ( nb_mess != 0)
   { // a key is here
      return 0;
   } 
   else 
   { // empty     
      // avoid key release ?
      if (key_tail() == 0) 
      {
         key_pop();
         return 0;
      }
   return 1;
   }
}



uint32_t read_power_voltage(void)
{
    const uint32_t vmax = 3000;
    const uint32_t vmin = 2600;
//    if (tests::running)
    //    return 3098;
return        vbat;
//    return ui_battery() * (vmax - vmin) / 1000 + vmin;
}

int get_vbat(void)
{
    return read_power_voltage();
}

int get_lowbat_state(void)
{
    const uint32_t vlow = 2450;
    return read_power_voltage() < vlow;
}



void reset_auto_off(void)
{
   if (keybd.sleeping_soon >61000)
      keybd.sleeping_soon = 60000;
}



void sys_delay(uint32_t ms_delay)         // used
{
    OS_TASK_Delay_ms(ms_delay);
}


int32_t sys_current_ms(void)
// using rtc
{
//   return rtc_read_ticks();

   return Get_Elapsed_Dual_Res(0);
}

int64_t sys_current_ms_i64(void){
   int64_t tmp = Get_Elapsed_Dual_Res(0);
   return (tmp>0 ? tmp : -tmp*1000);
}

int32_t sys_elapsed_ms( int32_t start){
   return Get_Elapsed_Dual_Res(start);
}


struct timer
{
    uint32_t deadline;
    uint32_t period;
    bool     enabled;
} timers[4];


void sys_timer_disable(int timer_ix)
{
    timers[timer_ix].enabled = false;
}

void sys_timer_start(int timer_ix, uint32_t ms_value)
{
   uint32_t now = sys_current_ms();
   uint32_t then = now + ms_value;
   timers[timer_ix].deadline = then;
   timers[timer_ix].enabled = true;
   timers[timer_ix].period = ms_value;
}


uint32_t sys_timer_period(int timer_ix)
{
    return timers[timer_ix].period;
}


int sys_timer_active(int timer_ix)
{
    return timers[timer_ix].enabled;
}

int sys_timer_timeout(int timer_ix)
{
    if (timers[timer_ix].enabled)
    {
        uint32_t now = sys_current_ms();
        return timers[timer_ix].deadline < now ? 1:0;
    }
    return false;
}

// HAL error
void    Error_Handler(int err_no){
   uint32_t tmp = sys_current_ms();
   SEGGER_RTT_printf(0, "\n%04d.%03d : Err Hal : %d", (tmp/1000)%10000,tmp%1000, err_no);
   while (1){
   
   }
}


void RTT_vprintf( const char * sFormat, ...)
// debug message, deleted in release mode
{
#if DEBUG
   va_list args;
   va_start(args, sFormat);
   SEGGER_RTT_vprintf(0, sFormat, &args);
#endif // DEBUG
}

void RTT_vprintf_cr_time( const char * sFormat, ...)
// debug message, deleted in release mode
{
#if DEBUG
   va_list args;
   va_start(args, sFormat);
   uint32_t tmp = sys_current_ms();

   SEGGER_RTT_printf(0, "\n%04d.%03d : ", (tmp/1000)%10000,tmp%1000);
   SEGGER_RTT_vprintf(0, sFormat, &args);

#endif // DEBUG
}
void RTT_vprintf_L( uint32_t level, const char * sFormat, ...)
// debug message, deleted in release mode
{
#if DEBUG
   va_list args;
   va_start(args, sFormat);
   if ( current_printf_level & level)
   {
      SEGGER_RTT_vprintf(0, sFormat, &args);
   }
#endif // DEBUG
}


void RTT_vprintf_cr_time_fct( const char * sFctName, const char * sFormat, ...)
// debug message, deleted in release mode
{
#if DEBUG
   va_list args;
   va_start(args, sFormat);
   uint32_t tmp = sys_current_ms();

   SEGGER_RTT_printf(0, "\n%03d.%03d [%s] ", (tmp/1000)%10000,tmp%1000, sFctName);
   SEGGER_RTT_vprintf(0, sFormat, &args);
#endif // DEBUG
}

LCD_Status_t lcd_clear_buf(void)
{
   LCD_ClearFramebuffer(&hlcd);
//    record(lcd, "Clearing buffer");
//    for (unsigned i = 0; i < sizeof(lcd_buffer) / sizeof(*lcd_buffer); i++)
//        lcd_buffer[i] = pattern::white.bits;
   return LCD_OK;
}

void RTT_vprintf_cr_time_fct_level( const char * sFctName, uint32_t level, const char * sFormat, ...)
// debug message, deleted in release mode
{
#if DEBUG
   va_list args;
   va_start(args, sFormat);
   uint32_t tmp = sys_current_ms();
   if ( current_printf_level & level)
   {
      SEGGER_RTT_printf(0, "\n%04d.%03d [%s] ", (tmp/1000)%10000,tmp%1000, sFctName);
      SEGGER_RTT_vprintf(0, sFormat, &args);
   }
#endif // DEBUG
}

