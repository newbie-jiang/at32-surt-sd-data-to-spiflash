/**
 * @file bsp_lcd_xmc.h
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2023-10-07 15:41:54
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
#ifndef _BSP_LCD_XMC_H_
#define _BSP_LCD_XMC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include <stdint.h>
#include "at32f403a_407_board.h"


/* define ---------------------------------------------------------------*/
/* the address of write data & command (xmc_a0) */
#define LCD_XMC_COMMAND                 0x60000000
#define LCD_XMC_DATA                    0x60000001

// 
#define LCD_RESET_HIGH                  gpio_bits_set(GPIOD,GPIO_PINS_3)
#define LCD_RESET_LOW                   gpio_bits_reset(GPIOD,GPIO_PINS_3)
#define LCD_BL_HIGH                     gpio_bits_set(GPIOD,GPIO_PINS_6)
#define LCD_BL_LOW                      gpio_bits_reset(GPIOD,GPIO_PINS_6)

#define LCD_XMC_OFFSET_X    (0)
#define LCD_XMC_OFFSET_Y    (0)
#define LCD_XMC_WIDTH       (480) //(480) 
#define LCD_XMC_HEIGHT      (320) //(320)

/* typedef --------------------------------------------------------------*/

#define WHITE                            0xFFFF
#define YELLOW                           0xFFE0
#define GREEN                            0x07E0


#define BLACK                            0x0000
#define BLUE                             0x001F
#define BRED                             0xF81F
#define GRED                             0xFFE0
#define GBLUE                            0x07FF
#define RED                              0xF800
#define MAGENTA                          0xF81F


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void bsp_lcd_xmc_init(void);
void bsp_lcd_set_block(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend);
void bsp_lcd_write_2_data(uint16_t dat);
void bsp_lcd_clear(uint16_t color);
void bsp_lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void bsp_lcd_frame_fill(uint16_t x, uint16_t y, uint16_t w , uint16_t h , uint16_t color);
void bsp_lcd_draw_line(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t color);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_BSP_LCD_XMC_H_

/* end of file ----------------------------------------------------------*/


