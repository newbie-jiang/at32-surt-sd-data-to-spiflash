/**
 * @file bsp_xpt2046.h
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-11 14:25:55
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
#ifndef _BSP_XPT2046_H_
#define _BSP_XPT2046_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "at32f403a_407_board.h"
#include "stdbool.h"
#include "stdint.h"
#include "touch.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void bsp_xpt2046_init(tp_dev_t *dev);
void bsp_xpt2046_calibrate_or_get_touch_para_with_flash(uint8_t forceCal);





#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_BSP_XPT2046_H_

/* end of file ----------------------------------------------------------*/


