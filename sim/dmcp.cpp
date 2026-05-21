// ****************************************************************************
//  dmcp.cpp                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     A fake DMCP implementation with the functions we use in the simulator
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

// on n'utilise pas test


#include "dmcp.h"
#include "DBxxxx.h"


#include "datetime.h"
#include "dmcp_fonts.c"
#include "recorder.h"




#include "target.h"
#include "tests.h"
#include "types.h"

#include <iostream>
#include <stdarg.h>
#include <stdio.h>
//#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>


#include "SEGGER_RTT.h"
#include "RTOS.h"

#include "user_interface.h"

//#include "LS027B7DH01.h"

#ifndef  SIMULATOR
volatile uint test_command = 0;
#endif

#pragma GCC diagnostic ignored "-Wunused-parameter"

RECORDER(dmcp,          64, "DMCP system calls");
RECORDER(dmcp_error,    64, "DMCP errors");
RECORDER(dmcp_warning,  64, "DMCP warnings");
RECORDER(dmcp_notyet,   64, "DMCP features that are not yet implemented");
RECORDER(keys,          64, "DMCP key handling");
RECORDER(keys_empty,    64, "DMCP key_empty() call");
RECORDER(keys_warning,  64, "Warnings related to key handling");
RECORDER(lcd,           64, "DMCP lcd/display functions");
RECORDER(lcd_refresh,   64, "DMCP lcd/display refresh");
RECORDER(lcd_width,     64, "Width of strings and chars");
RECORDER(lcd_warning,   64, "Warnings from lcd/display functions");

#undef ppgm_fp

extern bool          run_tests;
extern bool          noisy_tests;
extern bool          no_beep;

extern uint32_t      Cnt_ms ; 
extern OS_MAILBOX   Mb_Keyboard;

extern LCD_Handle_t hlcd;


int                  lcd_buf_cleared_result = 0;
//pixword              lcd_buffer[LCD_SCANLINE * LCD_H * color::BPP / 32];
bool                 shift_held = false;
bool                 alt_held   = false;


// Eliminate a really cumbersome warning
#define DS_INIT                                 \
    .x          = 0,                            \
    .y          = 0,                            \
    .ln_offs    = 0,                            \
    .y_top_grd  = 0,                            \
    .ya         = 0,                            \
    .yb         = 0,                            \
    .xspc       = 0,                            \
    .xoffs      = 0,                            \
    .fixed      = 0,                            \
    .inv        = 0,                            \
    .bgfill     = 0,                            \
    .lnfill     = 0,                            \
    .newln      = 0,                            \
    .post_offs  = 0

static disp_stat_t   t20_ds     = { .f = &lib_mono_10x17, DS_INIT };
static disp_stat_t   t24_ds     = { .f = &lib_mono_12x20, DS_INIT };
static disp_stat_t   fReg_ds    = { .f = &lib_mono_17x25, DS_INIT };
static FIL           ppgm_fp_file;

sys_sdb_t sdb =
{
    // Can't even use .field notation here because all fields are #defined!
    /* calc_state         */ 0,
    /* ppgm_fp            */ &ppgm_fp_file,
    /* key_to_alpha_table */ nullptr,
    /* run_menu_item_app  */ nullptr,
    /* menu_line_str_app  */ nullptr,
    /* after_fat_format   */ nullptr,
    /* get_flag_dmy       */ nullptr,
    /* set_flag_dmy       */ nullptr,
    /* is_flag_clk24      */ nullptr,
    /* set_flag_clk24     */ nullptr,
    /* is_beep_mute       */ nullptr,
    /* set_beep_mute      */ nullptr,
    /* pds_t20            */ &t20_ds,
    /* pds_t24            */ &t24_ds,
    /* pds_fReg           */ &fReg_ds,
    /* timer2_counter     */ nullptr,
    /* timer3_counter     */ nullptr,
    /* msc_end_cb         */ nullptr
};


cstring get_wday_shortcut(int day)
{
    static cstring dow[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    return dow[day];
}

int julian_day(dt_t *dt)
{
    return julian_day_number(dt->day, dt->month, dt->year);
}


cstring get_month_shortcut(int month)
{
    static cstring name[] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    return name[month - 1];
}


void LCD_power_off(int UNUSED clear)
{
    record(dmcp, "LCD_power_off");
}


void LCD_power_on()
{
    record(dmcp, "LCD_power_on");
}



int create_screenshot(int report_error)
{
    record(dmcp_notyet,
           "create_screenshot(%d) not implemented", report_error);
 //   ui_screenshot();
    return 0;
}


void draw_power_off_image(int allow_errors)
{
       SEGGER_RTT_printf(0, "\ndraw_power_off_image to do...");


    record(dmcp_notyet,
           "draw_power_off_image(%d) not implemented", allow_errors);


}


int handle_menu(const smenu_t * menu_id, int action, int cur_line)
{
    uint menu_line = 0;
    bool done = false;
   uint32_t keybdata;
    lcd_clear_buf();
    while (!done)
    {
        t24->xoffs = 0;
        lcd_writeClr(t24);
        lcd_writeClr(t20);
        lcd_putsR(t20, menu_id->name);

        char buf[80];
        uint count = 0;
        while (menu_id->items[count])
            count++;

        for (uint i = 0; i < count; i++)
        {
            uint8_t mid = menu_id->items[i];
            cstring label = menu_line_str_app(mid, buf, sizeof(buf));
            if (!label)
            {
                label = "Unimplemented DMCP menu";
                switch(mid)
                {
                case MI_MSC:            label = "Activate USB Disk"; break;
                case MI_PGM_LOAD:       label = "Load Program"; break;
                case MI_LOAD_QSPI:      label = "Load QSPI from FAT"; break;
                case MI_SYSTEM_ENTER:   label = "System >"; break;
                case MI_SET_TIME:       label = "Set Time >"; break;
                case MI_SET_DATE:       label = "Set Date >"; break;
                case MI_BEEP_MUTE:      label = "Beep Mute"; break;
                case MI_SLOW_AUTOREP:   label = "Slow Autorepeat"; break;
                case MI_DISK_INFO:      label = "Show Disk Info"; break;
                }
            }
            t24->inv = i == menu_line;
            lcd_printAt(t24, i+1, "%u. %s", i+1, label);
        }
        lcd_refresh();

        bool redraw = false;
        while (!redraw)
        {
            int key = 0;
            StopMode2_enable = 0xff;

            while (!test_command && key_empty())  {    OS_TASK_Delay(100); } // first try
            StopMode2_enable = 0;

//               sys_sleep();

            if (test_command)
                   return 0;

            key = key_pop();

            uint wanted = 0;
            switch (key)
            {
            case KEY_UP:
                if (menu_line > 0)
                {
                    menu_line--;
                    redraw = true;
                }
                break;
            case KEY_DOWN:
                if (menu_line + 1 < count)
                {
                    menu_line++;
                    redraw = true;
                }
                break;
            case KEY_1: wanted = 1; break;
            case KEY_2: wanted = 2; break;
            case KEY_3: wanted = 3; break;
            case KEY_4: wanted = 4; break;
            case KEY_5: wanted = 5; break;
            case KEY_6: wanted = 6; break;
            case KEY_7: wanted = 7; break;
            case KEY_8: wanted = 8; break;
            case KEY_9: wanted = 9; break;

            case -1:
                // Signals that main application is exiting, leave all dialogs
            case KEY_EXIT:
                redraw = true;
                done = true;
                break;

            case KEY_ENTER:
                run_menu_item_app(menu_id->items[menu_line]);
                lcd_clear_buf();
                redraw = true;
                break;
            }
            if (wanted)
            {
                if (wanted <= count)
                {
                    menu_line = wanted - 1;
                    run_menu_item_app(menu_id->items[menu_line]);
                    lcd_clear_buf();
                    redraw = true;
                }
            }

        }
    }

    return 0;
}

volatile int8_t  keys[4] = { 0 };
volatile uint    keyrd   = 0;
volatile uint    keywr   = 0;
enum {  nkeys = sizeof(keys) / sizeof(keys[0]) };



static uint32_t last_warning = 0;

inline void lcd_set_pixel(int x, int y)
{
   Disp_SetPixel( &hlcd, x, y, 1);
/*    if (x < 0 || x > LCD_W || y < 0 || y > LCD_H)
    {
        uint now = sys_current_ms();
        if (now - last_warning > 1000)
        {
            record(lcd_warning, "Clearing pixel at (%d, %d)", x, y);
            last_warning = now;
        }
        return;
    }
    surface s(lcd_buffer, LCD_W, LCD_H, LCD_SCANLINE, LCD_W);
    s.fill(x, y, x, y, pattern::black);
   LCD_MarkLineModified( &hlcd, x);
*/
}

inline void lcd_clear_pixel(int x, int y)
{
   Disp_SetPixel( &hlcd, x, y, 0);

/*    if (x < 0 || x > LCD_W || y < 0 || y > LCD_H)
    {
        uint now = sys_current_ms();
        if (now - last_warning > 1000)
        {
            record(lcd_warning, "Setting pixel at (%d, %d)", x, y);
            last_warning = now;
        }
        return;
    }
    surface s(lcd_buffer, LCD_W, LCD_H, LCD_SCANLINE, LCD_W);
    s.fill(x, y, x, y, pattern::white);
   LCD_MarkLineModified( &hlcd, x);
*/
}

inline void lcd_pixel(int x, int y, int val)
{
    if (!val)
        lcd_set_pixel(x, y);
    else
        lcd_clear_pixel(x, y);
}

void lcd_draw_menu_keys_not_used(const char *keys[])
{
    int my = LCD_H - t20->f->height - 4;
    int mh = t20->f->height + 2;
    int mw = (LCD_W - 10) / 6;
    int sp = (LCD_W - 5) - 6 * mw;

    t20->inv = 1;
    t20->lnfill = 0;
    t20->bgfill = 1;
    t20->newln = 0;
    t20->y = my + 1;

    record(lcd, "Menu [%s][%s][%s][%s][%s][%s]",
           keys[0], keys[1], keys[2], keys[3], keys[4], keys[5]);
    for (int m = 0; m < 6; m++)
    {
        int x = (2 * m + 1) * mw / 2 + (m * sp) / 5 + 2;
        lcd_fill_rect(x - mw/2+2, my,   mw-4,   mh, 1);
        lcd_fill_rect(x - mw/2+1, my+1, mw-2, mh-2, 1);
        lcd_fill_rect(x - mw/2,   my+2, mw,   mh-4, 1);

        // Truncate the menu to fit
        // Note that DMCP is NOT robust to overflow here and can die
        int size = 11;
        int w = 0;
        char buffer[12];
        do
        {
            snprintf(buffer, sizeof(buffer), "%.*s", size, keys[m]);
            w = lcd_textWidth(t20, buffer);
            size--;
        } while (w > mw);

        if (size < (int) strlen(keys[m]))
            record(lcd_warning,
                       "Menu entry %d [%s] is too long "
                       "(%d chars lost, shows as [%s])",
                       m, keys[m], (int) strlen(keys[m]) - size + 1, buffer);

        t20->x = x - w / 2;
        lcd_puts(t20, buffer);
    }
    t20->lnfill = 1;
    t20->inv = 0;
}

void lcd_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, int val)
{
    if (val)
        record(lcd, "Fill  rectangle (%u,%u) + (%u, %u)", x, y, w, h);
    else
        record(lcd, "Clear rectangle (%u,%u) + (%u, %u)", x, y, w, h);

    if (x + w > LCD_W)
    {
        record(lcd_warning,
               "Rectangle X is outside screen (%u, %u) + (%u, %u)",
               x, y, w, h);
        w = LCD_W - x;
        if (w > LCD_W)
            x = w = 0;
    }
    if (y +h > LCD_H)
    {
        record(lcd_warning,
               "Rectangle Y is outside screen (%u, %u) + (%u, %u)",
               x, y, w, h);
        h = LCD_W - y;
        if (h > LCD_W)
            y = h = 0;
    }

    for (uint r = y; r < y + h; r++)
        for (uint c = x; c < x + w; c++)
            lcd_pixel(c, r, val);
}

int lcd_fontWidth(disp_stat_t * ds)
{
    return ds->f->width;
}
int lcd_for_calc(int what)
{
    record(dmcp_notyet, "lcd_for_calc %d not implemented", what);
    return 0;
}
int lcd_get_buf_cleared()
{
    record(lcd, "get_buf_cleared returns %d", lcd_buf_cleared_result);
    return lcd_buf_cleared_result;
}
int lcd_lineHeight(disp_stat_t * ds)
{
    return ds->f->height;
}
/*
uint8_t * lcd_line_addr(int y)
{
    if (y < 0 || y > LCD_H)
    {
        record(lcd_warning, "lcd_line_addr(%d), line is out of range", y);
        y = 0;
    }
    blitter::offset offset = y * LCD_SCANLINE * color::BPP / 32;
    return (uint8_t *) (lcd_buffer + offset);
}
*/
int lcd_toggleFontT(int nr)
{
    return nr;
}
int lcd_nextFontNr(int nr)
{
    if (nr < (int) dmcp_fonts_count - 1)
        nr++;
    else
        nr = dmcp_fonts_count - 1;
    return nr;
}
int lcd_prevFontNr(int nr)
{
    if (nr > 0)
        nr--;
    else
        nr = 0;
    return nr;
}
void lcd_prevLn(disp_stat_t * ds)
{
    ds->y -= lcd_lineHeight(ds);
    ds->x = ds->xoffs;
}
void lcd_print(disp_stat_t * ds, const char* fmt, ...)
{
    static char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    lcd_puts(ds, buffer);
}

void lcd_setLine(disp_stat_t * ds, int ln_nr)
{
    ds->x = ds->xoffs;;
    ds->y = ln_nr * lcd_lineHeight(ds);
//SEGGER_RTT_printf(0, "\nset line %u coord (%d, %d)", ln_nr, ds->x, ds->y);
}

void lcd_setXY_not_used(disp_stat_t * ds, int x, int y)
{
//SEGGER_RTT_printf(0, "\nset XY (%d, %d)", x, y);
    ds->x = x;
    ds->y = y;
}
void lcd_set_buf_cleared(int val)
{
//SEGGER_RTT_printf(0, "\nSet buffer cleared %d", val);
    lcd_buf_cleared_result = val;
}
void lcd_switchFont(disp_stat_t * ds, int nr)
{
//SEGGER_RTT_printf(0, "\nSelected font %d", nr);
    if (nr >= 0 && nr <= (int) dmcp_fonts_count)
        ds->f = dmcp_fonts[nr];
}

int lcd_charWidth(disp_stat_t * ds, int c)
{
    int                width = 0;
    const line_font_t *f     = ds->f;
    byte               first = f->first_char;
    byte               count = f->char_cnt;
    const uint16_t    *offs  = f->offs;
    const uint8_t     *data  = f->data;
    uint               xspc  = ds->xspc;

    c -= first;
    if (c >= 0 && c < count)
    {
        uint off = offs[c];
        width += data[off + 0] + data[off + 2] + xspc;
        record(lcd_width,
               "Character width of %c (%d=0x%x) is %d",
               c + first, c + first, c + first, width);
    }
    else
    {
//SEGGER_RTT_printf(0, "\nCharacter width of nonexistent %d is %d", c, width);
    }

    return width;
}

int lcd_textWidth(disp_stat_t * ds, const char* text)
{
    int                width = 0;
    byte               c;
    const line_font_t *f     = ds->f;
    byte               first = f->first_char;
    byte               count = f->char_cnt;
    const uint16_t    *offs  = f->offs;
    const uint8_t     *data  = f->data;
    uint               xspc  = ds->xspc;
    const byte         *p    = (const byte *) text;

    while ((c = *p++))
    {
        c -= first;
        if (c < count)
        {
            uint off = offs[c];
            width += data[off + 0] + data[off + 2] + xspc;
        }
        else
        {
//SEGGER_RTT_printf(0, "\nNonexistent character %d at offset %d in [%s]",
//                   c + first, p - (const byte *) text, text);
        }
    }
    return width;
}

void lcd_writeClr(disp_stat_t *ds)
{
//SEGGER_RTT_printf(0, "\nClearing display state"); // Not sure this is what it does
    ds->x      = 0; // ds->xoffs;
    ds->y      = 0;
    ds->inv    = 0;
    ds->bgfill = 1;
    ds->lnfill = 1;
    ds->newln  = 1;
    ds->xspc   = 1;
}

void lcd_writeNl(disp_stat_t *ds)
{
    ds->x = ds->xoffs;
    ds->y += lcd_lineHeight(ds);
//SEGGER_RTT_printf(0, "\nNew line, now at (%d, %d)", ds->x, ds->y);
}

inline void lcd_writeTextInternal(disp_stat_t *ds, const char *text, int write)
{
    uint               c;
    const line_font_t *f        = ds->f;
    uint               first    = f->first_char;
    uint               count    = f->char_cnt;
    uint               height   = f->height;
    const uint8_t     *data     = f->data;
    const uint16_t    *offs     = f->offs;
    int                xspc     = ds->xspc;
    int                x        = ds->x + xspc;
    int                y        = ds->y + ds->ln_offs;
    int                color    = ds->inv == 0;
    const byte        *p        = (const byte *) text;

//    if (write)
//SEGGER_RTT_printf(0, "\nWrite text [%s] at (%d, %d)", text, x, y);
//    else
//SEGGER_RTT_printf(0, "\nSkip text [%s] at (%d, %d)", text, x, y);

    if (ds->lnfill)
        lcd_fill_rect(ds->xoffs, y, LCD_W, height, color);

    while ((c = *p++))
    {
        c -= first;
        if (c < count)
        {
            int            off  = offs[c];
            const uint8_t *dp   = data + off;
            int            cx   = *dp++;
            int            cy   = *dp++;
            int            cols = *dp++;
            int            rows = *dp++;

            if (!write)
            {
                x += cx + cols;
                continue;
            }

            for (int r = 0; r < cy; r++)
                for (int c = 0; c < cx + cols; c++)
                    lcd_pixel(x+c, y+r, color);

            for (int r = 0; r < rows; r++)
            {
                int data = 0;
                for (int c = 0; c < cols; c += 8)
                    data |= *dp++ << c;

                for (int c = 0; c < cx; c++)
                    lcd_pixel(x+c, y+r, color);

                for (int c = 0; c < cols; c++)
                {
                    int val = (data >> (cols - c - 1)) & 1;
                    if (val || ds->bgfill)
                        lcd_pixel(x + c + cx, y + r + cy, val != color);
                }
            }

            for (uint r = cy + rows; r < height; r++)
                for (int c = 0; c < cx + cols; c++)
                    lcd_pixel(x+c, y+r, color);


            x += cx + cols + xspc;
        }
        else
        {
//SEGGER_RTT_printf(0, "\nNonexistent character [%d] in [%s] at %d, max=%d",
//                   c + first, text, p - (byte_p) text, count + first);
        }

    }
    ds->x = x;
    if (ds->newln)
    {
        ds->x = ds->xoffs;
        ds->y += height;
    }
}

void lcd_writeText(disp_stat_t * ds, const char* text)
{
    lcd_writeTextInternal(ds, text, 1);
}
void lcd_writeTextWidth(disp_stat_t * ds, const char* text)
{
    lcd_writeTextInternal(ds, text, 0);
}
void rtc_wakeup_delay()
{
   SEGGER_RTT_printf(0, "\nrtc_wakeup_delay not implemented");
}
void run_help_file(const char * help_file)
{
   SEGGER_RTT_printf(0, "\nrun_help_file not implemented");
}
void run_help_file_style(const char * help_file, user_style_fn_t *user_style_fn)
{
   SEGGER_RTT_printf(0, "\nrun_help_file_style not implemented");
}
void start_buzzer_freq(uint32_t freq)
{
   SEGGER_RTT_printf(0, "\nBuzzer not implemented");
    record(dmcp, "start_buzzer %u.%03uHz", freq / 1000, freq % 1000);
//    if (!no_beep && (!tests::running || noisy_tests))
//        ui_start_buzzer(freq);

}
void stop_buzzer()
{
   SEGGER_RTT_printf(0, "\nBuzzer not implemented");
    record(dmcp, "stop_buzzer");
//    if (!no_beep && (!tests::running || noisy_tests))
//        ui_stop_buzzer();
}

int sys_free_mem()
{
    // On the simulator, we have real memory, only 400k in u858
    return 64 * 1024;
}


extern struct timer timers[4];

void sys_sleep()
{
/*
    uint32_t entry = sys_current_ms();
    while (!test_command && key_empty())
    {
        uint32_t now = sys_current_ms();
        for (int i = 0; i < 4; i++)
            if (timers[i].enabled &&
                int(timers[i].deadline - now) < 0 &&
                int(timers[i].deadline - entry) >= 0)
                goto done;
//        ui_ms_sleep(tests::running ? 1 : 20);
        OS_TASK_Delay_ms( 10 );
    }
done:
    CLR_ST(STAT_SUSPENDED | STAT_OFF | STAT_PGM_END);
    */
}

void sys_critical_start()
{
   OS_TASK_EnterRegion();
}

void sys_critical_end()
{
   OS_TASK_LeaveRegion();
}



int file_selection_screen(const char   *title,
                          const char   *base_dir,
                          const char   *ext,
                          file_sel_fn_t sel_fn,
                          int           disp_new,
                          int           overwrite_check,
                          void         *data)
{
    int ret = 0;

    // Make things relative to the working directory
    if (*base_dir == '/' || *base_dir == '\\')
        base_dir++;

    ret = ui_file_selector(title, base_dir, ext,
                           sel_fn, data,
                           disp_new, overwrite_check);

    return ret;
}

int power_check_screen()
{
    record(dmcp, "file_selection_screen not imlemented");
    return 0;
}


int sys_disk_write_enable(int val)
{
    return 0;
}





void disp_disk_info(const char *hdr)
{
   char buff[60];
   snprintf(buff, sizeof(buff), "\n disp_disk_info : %s ", hdr);
   SEGGER_RTT_WriteString(0,  buff);

 //   ui_draw_message(hdr);
}

void set_reset_state_file(const char * str)
{
//    ui_save_setting("state", str);
//    record(dmcp, "Setting saved state: %s", str);
}


char *get_reset_state_file()
{
    static char result[256] = { "state\\State1.48s"};
//    result[0] = 0;
//    ui_read_setting("state", result, sizeof(result));
    record(dmcp, "Saved state: %+s", result);
    return result;
}

uint32_t reset_magic = 0;
void set_reset_magic(uint32_t value)
{
    reset_magic = value;
}




void sys_reset()
{

}

int is_menu_auto_off()
{
    return false;
}




int check_create_dir(const char * dir)
{
/*
    struct stat st;
    if (stat(dir, &st) == 0)
        if (st.st_mode & S_IFDIR)
            return 0;
    return mkdir(dir, 0777);
*/
return 0;
}


void bitblt24(uint32_t x,
              uint32_t dx,
              uint32_t y,
              uint32_t val,
              int      blt_op,
              int      fill)
{
}
