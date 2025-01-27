/**
  **************************************************************************
  * @file     at32f403a_407_board.h
  * @brief    header file for at-start board. set of firmware functions to
  *           manage leds and push-button. initialize delay function.
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#ifndef __AT32F403A_407_BOARD_H
#define __AT32F403A_407_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "at32f403a_407.h"
#include "define.h"
/** @addtogroup AT32F403A_407_board
  * @{
  */

/** @addtogroup BOARD
  * @{
  */

/** @defgroup BOARD_pins_definition
  * @{
  */

/**
  * this header include define support list:
  * 1. at-start-f403a v1.x board
  * 2. at-start-f407 v1.x board
  * if define AT_START_F403A_V1, the header file support at-start-f403a v1.x board
  * if define AT_START_F407_V1, the header file support at-start-f407 v1.x board
  */

#if !defined (AT_START_F403A_V1)&& !defined (AT_START_F407_V1)
#error "please select first the board at-start device used in your application (in at32f403a_407_board.h file)"
#endif

/******************** define led ********************/
typedef enum
{
  ARM_RLED,                                   
  ARM_YLED,                                   
  M4_RLED ,                                 
	AC_LED,
	BAT_LED
} led_type;

#define LED_NUM                          5

#if defined (AT_START_F403A_V1) || defined (AT_START_F407_V1)
//MCU RLED
#define LED1_PIN                         GPIO_PINS_1
#define LED1_GPIO                        GPIOA
#define LED1_GPIO_CRM_CLK                CRM_GPIOA_PERIPH_CLOCK
//MCU YLED
#define LED2_PIN                         GPIO_PINS_2
#define LED2_GPIO                        GPIOA
#define LED2_GPIO_CRM_CLK                CRM_GPIOA_PERIPH_CLOCK
//M4 RLED
#define LED3_PIN                         GPIO_PINS_3
#define LED3_GPIO                        GPIOA
#define LED3_GPIO_CRM_CLK                CRM_GPIOA_PERIPH_CLOCK
//AC LED
#define LED4_PIN                         GPIO_PINS_15
#define LED4_GPIO                        GPIOA
#define LED4_GPIO_CRM_CLK                CRM_GPIOA_PERIPH_CLOCK
//BAT LED
#define LED5_PIN                         GPIO_PINS_8
#define LED5_GPIO                        GPIOC
#define LED5_GPIO_CRM_CLK                CRM_GPIOC_PERIPH_CLOCK
#endif

/**************** define print uart ******************/
#define PRINT_UART                       USART1
#define PRINT_UART_CRM_CLK               CRM_USART1_PERIPH_CLOCK
#define PRINT_UART_TX_PIN                GPIO_PINS_9
#define PRINT_UART_TX_GPIO               GPIOA
#define PRINT_UART_TX_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK

/******************* define button *******************/
typedef enum
{
  USER_BUTTON                            = 0,
  NO_BUTTON                              = 1
} button_type;

#define USER_BUTTON_PIN                  GPIO_PINS_0
#define USER_BUTTON_PORT                 GPIOA
#define USER_BUTTON_CRM_CLK              CRM_GPIOA_PERIPH_CLOCK


typedef struct bsp_gpio {
    gpio_type *port;
    uint16_t pin;
    confirm_state state;
    crm_periph_clock_type crm_clk;
}bsp_gpio_st;

 
/* 光耦检测状态 */
typedef enum optocoupler_state{	 
	 optocoupler_motor_run = 0x55,                //光耦检测电机到达其他叶片
	 optocoupler_motor_arrive_motor_point = 0xCC, //光耦检测电机到达首个叶片
	 optocoupler_motor_stop = 0xAA,               //光耦检测电机为停止状态
}optocoupler_state;

optocoupler_state get_optocoupler_signal(void);
/**
  * @} 
  */

/** @defgroup BOARD_exported_functions
  * @{
  */

/******************** functions ********************/
void at32_board_init(void);

/* led operation function */
void led_on(led_type led);
void led_off(led_type led);
void led_turn(led_type led);

/* button operation function */
void at32_button_init(void);
button_type at32_button_press(void);
uint8_t at32_button_state(void);

/* delay function */
void delay_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
void delay_sec(uint16_t sec);


/* printf uart init function */
void uart_print_init(uint32_t baudrate);


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif

