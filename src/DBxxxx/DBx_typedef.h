#ifndef DBxTYPEDEF_H
#define DBxTYPEDEF_H



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <time.h>
#include <stdbool.h>
#include <stdint.h>

   #include "stm32u5xx.h"


/* Driver Status */
#define LCD_MESS_STATUS_LENGTH 32
typedef enum {
    LCD_OK = 0,
    LCD_ERROR,
    LCD_BUSY,
    LCD_TIMEOUT,
   LCD_DMA1,
   LCD_DMA2,
   LCD_LTDC,
   LCD_NOT_INIT,
   LCD_LAST,
} LCD_Status_t;



typedef enum {
   POWER_ON = 1,
   POWER_OFF,
   LUMI_STD,
   LUMI_MIN,
   INIT
} Disp_Power_t;

typedef enum {
    BL1,
    BL2,
    BL5,
    BL10,
    BL20,
    BL30,
    BL40,
    BL50,
    BL60,
    BL70,
    BL80,
    BL100,
    BLMAX
} LCD_BL_t;


typedef struct {
    const char *city;
    const char *region;
    int         std_offset_min;
    int         dst_offset_min;
    bool      (*is_dst_func)(time_t);
} CityTimeZone;


typedef struct {

    SPI_HandleTypeDef *hspi;
    bool use_dma;
    uint32_t timeout_ms;
} LCD_Config_t;

/* Modified line tracking */
typedef struct {
    uint16_t x_min;    // First modified line index
    uint16_t x_max;    // First modified line index
    uint16_t y_min;    // First modified line index
    uint16_t y_max;    // First modified line index

} LCD_ModifiedLines_t;


/* Driver Handle */
typedef struct {
   uint8_t* p_lcd_buf;
   uint8_t* p_dma_buf;
   uint8_t* p_RamDb48x;
   uint8_t* p_Vram;

    volatile bool transfer_complete;
    uint16_t need_refresh;
    bool initialized;
    bool vcom_state;
    bool spi_off;
    LCD_Config_t config;
    LCD_ModifiedLines_t modified;
    LCD_Status_t status;
    bool use_ltsc;
    uint32_t BL_level;
    Disp_Power_t Power;
} LCD_Handle_t;




#ifdef __cplusplus
}
#endif // __cplusplus

#endif //DBxTYPEDEF_H