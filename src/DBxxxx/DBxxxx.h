#ifndef DBxxxx_H
#define DBxxxx_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include "stdbool.h"
#include "stdint.h"
#include <time.h>



#include "RTOS.h"
#include "FS.h"
#include "FS_OS.h"
#include "SEGGER_RTT.h" // using SEGGER_RTT_printf


#include "DBx_typedef.h"

#include "Display.h"

#if DBh7a3
   #include "stm32h7xx.h"
   #include "h7a3_usb.h"
   #include "h7a3_rtc.h"
   #include "h7a3_kbd.h"
   #include "h7a3_bkpram.h"
   #include "h7a3_low_power.h"
   #include "h7a3_Sharp_LS.h"

#endif // DBh7a3

#if DBu585
   #include "stm32u5xx.h"
   #include "WA_u585_rtc.h"
   #include "WA_u585_low_power.h"
   #include "WA_u585_kbd.h"
   #include "WA_u585_USB.h"
   #include "WA_u585_Sharp_LS.h"


#endif //DBu585

#if DBu5G9
   #include "WA_u585_rtc.h"
   #include "WA_u585_low_power.h"
   #include "WA_u585_kbd.h"
   #include "WA_u585_USB.h"

#endif //DBu5G9


#include "DBx_tsk_pw_kbd.h"


#if SHARP_27_400x240
   #include "u5_spi.h"

#elif SHARP_32_536x336
   #include "u5_spi.h"

#elif TFT_LTDC
//   #include "LCDConf.h"
//   #include "u5g_ltdc.h"

#else
   #error wrong lcd setting
#endif      // lcd


// events mask for DB48x
#define EV_DBx_KBD      (0x1<<0)
#define EV_DBx_SYS      (0x2)
#define EV_DBx_WAKEUP      (0x4)
#define EV_DBx_USB_CON      (0x8)
#define EV_DBx_USB_DIS      (0x10)
#define EV_DBx_RESET    (0x20)
#define EV_DBx_POFF     (0x40)
#define EV_DBx_PON     (0x80)
#define EV_DBx_LCD     (0x100)
#define EV_DBx_PB_PA0     (0x200)
#define EV_DBx_LPTIM1     (0x400)
#define EV_DBx_FORMAT     (0x800)


#define EV_USB_ACQ     (0x1<<0)

#define EV_LCD_REFFRESH (0x10000)
#define EV_LCD_F_REFFRESH (0x20000)



// events mask for Kbd_Power
#define EV_KPo_KBD      (0x1<<0)          // 1
#define EV_KPo_RTC      (0x1<<1)          // 2 
#define EV_KPo_USB      (0x1<<2)          // 4
#define EV_KPo_LPTIM    (0x1<<3)          // 8
#define EV_KPo_DMA      (0x1<<4)          // 16
#define EV_KPo_ReqPOff  (0x1<<10)
#define EV_KPo_ReqLCD   (0x1<<11)
#define EV_KPo_EndOfTask   (0x1<<12)

// define debug level to enable rtt_printf
#define DL_display      (1<<0)
#define DL_keyboard     (1<<1)
#define DL_low_power    (1<<2)
#define DL_timers       (1<<3)
#define DL_rtc          (1<<4)
#define DL_usb          (1<<5)

#define DL_ALL          (0xffffffff)


typedef struct {
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
} dt_t;

typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t csec;
  uint8_t dow;
} tm_t;


typedef int (*file_sel_fn)(const char *fpath, const char *fname, void *data);

int       ui_file_selector(const char *title,
                           const char *base_dir,
                           const char *ext,
                           file_sel_fn callback,
                           void       *data,
                           int         disp_new,
                           int         overwrite_check);

void rtc_read(tm_t * tm, dt_t *dt);
void rtc_write(tm_t * tm, dt_t *dt);



extern uint32_t db_calc_state;
extern    uint8_t volatile  StopMode2_enable;
extern uint32_t current_printf_level;

extern const char LCD_Status_Desc[LCD_LAST][LCD_MESS_STATUS_LENGTH];


#define calc_state      db_calc_state



extern OS_EVENT   EV_LCD_DMA_END, WAKE_UP_EVENT, KBD_Event,  DBu_Start, USB_Start, LED_Start, EV_db_lcd_kbd_ready, EV_db_app_ready, FOR_EVER, RTC_Event, PB_Event;

extern OS_MAILBOX    Mb_Keyboard;
extern OS_TASK       TKBD, TDB48X;
extern OS_MUTEX      Mut_SPIFS;
extern LCD_Handle_t     OS_RAM       hlcd;


void RTT_vprintf( const char * sFormat, ...);

void RTT_vprintf_cr_time( const char * sFormat, ...);
void RTT_vprintf_cr_time_fct( const char * sFctName, const char * sFormat, ...);
#define RTT_vprintf_cr_T_F(...)    RTT_vprintf_cr_time_f(__VA_ARGS__)

#define RTT_vprintf_cr_time_f(rec, fmt, ...)    do {RTT_vprintf_cr_time_fct(#rec, fmt, ##__VA_ARGS__);} while(0)


void Lcd_Update(bool Force);
int Check_Disk(void);

int usb_powered(void);

// ---------------------------
//  Key buffer functions
// ---------------------------
int key_empty(void);

void wait_for_key_release(int tout);
void wait_for_key_press(void);
void wait_for_key_sleeping(void);

int key_remaining(void);


// ==== VBAT
uint32_t read_power_voltage(void);
int get_lowbat_state(void);
int get_vbat(void);


void reset_auto_off(void);

// System timers
void sys_timer_disable(int timer_ix);
void sys_timer_start(int timer_ix, uint32_t ms_value);
int sys_timer_active(int timer_ix);
int sys_timer_timeout(int timer_ix);
uint32_t sys_timer_period(int timer_ix);


// Millisecond delay
void sys_delay(uint32_t ms_delay);

int32_t sys_current_ms(void);
int32_t sys_elapsed_ms( int32_t start);
int64_t sys_current_ms_i64(void);

void    Error_Handler(int err_no);
//LCD_Status_t LCD_ClearFramebuffer(LCD_Handle_t *p_hlcd);
LCD_Status_t LCD_Clear(LCD_Handle_t *p_hlcd);
void display_text_12x24(LCD_Handle_t *hlcd, uint16_t x, uint16_t y, const char* text, int8_t pixel);
//LCD_Status_t LCD_UpdateDisplay(LCD_Handle_t *p_hlcd);
uint8_t* LCD_GetFramebuffer(void);
uint32_t LCD_GetFramebuffer_size(void);

LCD_Status_t LCD_Init(LCD_Handle_t *p_hlcd, bool use_dma);
//LCD_Status_t LCD_UpdateModifiedLines(LCD_Handle_t *p_hlcd, bool Force);
void LCD_MarkLineModified(LCD_Handle_t *p_hlcd, uint16_t x_min, uint16_t y_min, uint16_t x_max, uint16_t y_max);



// Compatibility with DMCP

LCD_Status_t lcd_clear_buf(void);
LCD_Status_t lcd_refresh(void);
LCD_Status_t lcd_refresh_dma(void);
LCD_Status_t lcd_refresh_wait(void);
LCD_Status_t lcd_forced_refresh(void);
LCD_Status_t lcd_refresh_lines(int ln, int cnt);
void refresh_dirty(void);


// ---------------------------
//  Key buffer functions
// ---------------------------
int key_push(int k1);
int key_tail(void);
int key_pop(void);
int key_pop_last(void);
void key_pop_all(void);
// Key functions
//int key_to_nr(int key);
int runner_get_key(int *repeat);
//int runner_get_key_delay(int *repeat, uint timeout, uint rep0, uint rep1, uint rep1tout);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif      // DBxxxx_H


