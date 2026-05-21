#ifndef DISPLAY_H
#define DISPLAY_H

#include "stdbool.h"
#include "stdint.h"
#include <time.h>

//#include "DBxxxx.h"


#include "RTOS.h"
#include "FS.h"
#include "FS_OS.h"
#include "SEGGER_RTT.h" // using SEGGER_RTT_printf


#include "DBx_typedef.h"





extern const uint16_t BL_levels[];
extern const int BL_count;


/* Driver Configuration */





LCD_Status_t Disp_WaitForTransfer(LCD_Handle_t *p_hlcd);


void LCD_DrawLine(LCD_Handle_t *hlcd,uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, int8_t pixel);
LCD_Status_t LCD_FillRect(LCD_Handle_t *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int8_t pixel);
void LCD_DrawRect(LCD_Handle_t *hlcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int8_t pixel);


// New display interface
LCD_Status_t DisplayPower(LCD_Handle_t *p_hlcd, Disp_Power_t pow_cmd);
LCD_Status_t Disp_Update(LCD_Handle_t *p_hlcd, bool force_full_redraw, bool wait_for_dma_end);
LCD_Status_t Disp_Init(LCD_Handle_t *p_hlcd);
LCD_Status_t Disp_DeInit(LCD_Handle_t *p_hlcd, bool ltdc_off);
LCD_Status_t Disp_SetPixel(LCD_Handle_t *p_hlcd, uint16_t x, uint16_t y, int32_t pixel);


void LCD_ClearFramebuffer(LCD_Handle_t *p_hlcd);
void Disp_Set_BL_Level( uint16_t bl_lev);


#endif // DISPLAY_H