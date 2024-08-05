/**
 * @file touch.c
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-21 17:22:16
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "touch.h"
#include "at32f403a_407_exint.h"
#include "at32f403a_407_misc.h"
#include "at32f403a_407_board.h"

/* private define -------------------------------------------------------*/
#include "bsp_xpt2046.h"
/* private typedef ------------------------------------------------------*/



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/
static tp_dev_t _tp_dev;


/* static functions -----------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static bool _touch_write_reg(uint8_t ic_add , uint8_t reg, uint8_t *buf , uint32_t len)
{
    return true;

}

static bool _touch_read_reg(uint8_t ic_add , uint8_t reg, uint8_t *buf , uint32_t len)
{
    return true;
}




/* global functions / API interface -------------------------------------*/
tp_dev_t* touch_init(void)
{ 
	
    gpio_init_type gpio_init_struct;
	
    crm_periph_clock_enable( CRM_GPIOD_PERIPH_CLOCK , TRUE);

    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_pins = GPIO_PINS_13 ; 
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init(GPIOD, &gpio_init_struct);	
	
    _tp_dev.write_reg = _touch_write_reg;
    _tp_dev.read_reg = _touch_read_reg;
    bsp_xpt2046_init(&_tp_dev);
    _tp_dev.point.status = TP_STATE_RELEASED;
    return &_tp_dev;
}


/* end of file ----------------------------------------------------------*/


