/**
 * @file bsp_lcd_xmc.c
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2023-10-07 15:41:38
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "bsp_lcd_xmc.h"
#include "timer_manage.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _xmc_init(void)
{
		gpio_init_type  gpio_init_struct = {0};
		xmc_norsram_init_type  xmc_norsram_init_struct;
		xmc_norsram_timing_init_type rw_timing_struct, w_timing_struct;

		/* enable the gpio clock */
		crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
		crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
		crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
		crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
		crm_periph_clock_enable(CRM_GPIOE_PERIPH_CLOCK, TRUE);
		crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
		/* enable the xmc clock */
		crm_periph_clock_enable(CRM_XMC_PERIPH_CLOCK, TRUE);
		

    /*-- gpio configuration ------------------------------------------------------*/
		
		/*LEDK*/
        #if !PWM_LCD_DEBUG
		gpio_init_struct.gpio_pins = GPIO_PINS_6;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
		gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
		gpio_init(GPIOD, &gpio_init_struct);
		//gpio_bits_reset(GPIOD,GPIO_PINS_6);
		gpio_bits_set(GPIOD,GPIO_PINS_6);
		#else
		crm_periph_clock_enable(CRM_TMR13_PERIPH_CLOCK,TRUE);
        gpio_init_struct.gpio_pins = GPIO_PINS_6;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
		gpio_init(GPIOA, &gpio_init_struct);
        tmr13_init();
        #endif


		/* lcd data lines configuration */
		/*D0 - D3*/
		gpio_init_struct.gpio_pins = GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_14 | GPIO_PINS_15;
		gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init(GPIOD, &gpio_init_struct);
		
		/*D4 - D7*/
		gpio_init_struct.gpio_pins = GPIO_PINS_7 | GPIO_PINS_8 | GPIO_PINS_9 | GPIO_PINS_10;
		gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init(GPIOE, &gpio_init_struct);

		/* lcd reset lines configuration */
		gpio_init_struct.gpio_pins = GPIO_PINS_3;
		gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init(GPIOD, &gpio_init_struct);

		/* lcd rs/cs/wr/rd lines configuration */
		/*RS*/
		gpio_init_struct.gpio_pins = GPIO_PINS_3;
		gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init(GPIOC, &gpio_init_struct);
		/*CS*/
		gpio_init_struct.gpio_pins = GPIO_PINS_7 ;
		gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init(GPIOD, &gpio_init_struct);
		/*WR RD*/
		gpio_init_struct.gpio_pins = GPIO_PINS_4 | GPIO_PINS_5;
		gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
		gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init(GPIOD, &gpio_init_struct);
		
		/*-- xmc configuration ------------------------------------------------------*/
		xmc_norsram_default_para_init(&xmc_norsram_init_struct);
		xmc_norsram_init_struct.subbank = XMC_BANK1_NOR_SRAM1;
		xmc_norsram_init_struct.data_addr_multiplex = XMC_DATA_ADDR_MUX_DISABLE;
		xmc_norsram_init_struct.device = XMC_DEVICE_SRAM;
		xmc_norsram_init_struct.bus_type = XMC_BUSTYPE_8_BITS;
		xmc_norsram_init_struct.burst_mode_enable = XMC_BURST_MODE_DISABLE;
		xmc_norsram_init_struct.asynwait_enable = XMC_ASYN_WAIT_DISABLE;
		xmc_norsram_init_struct.wait_signal_lv = XMC_WAIT_SIGNAL_LEVEL_LOW;
		xmc_norsram_init_struct.wrapped_mode_enable = XMC_WRAPPED_MODE_DISABLE;
		xmc_norsram_init_struct.wait_signal_config = XMC_WAIT_SIGNAL_SYN_BEFORE;
		xmc_norsram_init_struct.write_enable = XMC_WRITE_OPERATION_ENABLE;
		xmc_norsram_init_struct.wait_signal_enable = XMC_WAIT_SIGNAL_DISABLE;
		xmc_norsram_init_struct.write_timing_enable = XMC_WRITE_TIMING_ENABLE;
		xmc_norsram_init_struct.write_burst_syn = XMC_WRITE_BURST_SYN_DISABLE;
		xmc_nor_sram_init(&xmc_norsram_init_struct);

		/* timing configuration */
		xmc_norsram_timing_default_para_init(&rw_timing_struct, &w_timing_struct);
		rw_timing_struct.subbank = XMC_BANK1_NOR_SRAM1;
		rw_timing_struct.write_timing_enable = XMC_WRITE_TIMING_ENABLE;
		rw_timing_struct.addr_setup_time = 0xf;
		rw_timing_struct.addr_hold_time = 0x0;
		rw_timing_struct.data_setup_time = 0xf;
		rw_timing_struct.bus_latency_time = 0x0;
		rw_timing_struct.clk_psc = 0x0;
		rw_timing_struct.data_latency_time = 0x0;
		rw_timing_struct.mode = XMC_ACCESS_MODE_A;
		w_timing_struct.subbank = XMC_BANK1_NOR_SRAM1;
		w_timing_struct.write_timing_enable = XMC_WRITE_TIMING_ENABLE;
		w_timing_struct.addr_setup_time = 0xf;
		w_timing_struct.addr_hold_time = 0x0;
		w_timing_struct.data_setup_time = 0xf;
		w_timing_struct.bus_latency_time = 0x0;
		w_timing_struct.clk_psc = 0x0;
		w_timing_struct.data_latency_time = 0x0;
		w_timing_struct.mode = XMC_ACCESS_MODE_A;
		xmc_nor_sram_timing_config(&rw_timing_struct, &w_timing_struct);

		/* bus turnaround phase for consecutive read duration and consecutive write duration */
		xmc_ext_timing_config(XMC_BANK1_NOR_SRAM1, 0x08, 0x08);

		/* enable xmc_bank1_nor_sram1 */
		xmc_nor_sram_enable(XMC_BANK1_NOR_SRAM1, TRUE);

    

}

static void _lcd_write_cmd(uint8_t cmd)
{
    *(__IO uint8_t *) LCD_XMC_COMMAND = cmd; 
}

static void _lcd_write_data(uint8_t dat)
{
    *(__IO uint8_t *) LCD_XMC_DATA = dat; 
}

static void _lcd_init(void)
{
    /* reset lcd */
    LCD_RESET_HIGH; 
    delay_ms(1); 
    LCD_RESET_LOW; 
    delay_ms(10); 
    LCD_RESET_HIGH;
    delay_ms(120); 

    _lcd_write_cmd(0xE0); 
    _lcd_write_data(0x00); 
    _lcd_write_data(0x01); 
    _lcd_write_data(0x02); 
    _lcd_write_data(0x04); 
    _lcd_write_data(0x14); 
    _lcd_write_data(0x09); 
    _lcd_write_data(0x3F); 
    _lcd_write_data(0x57); 
    _lcd_write_data(0x4D); 
    _lcd_write_data(0x05); 
    _lcd_write_data(0x0B); 
    _lcd_write_data(0x09); 
    _lcd_write_data(0x1A); 
    _lcd_write_data(0x1D); 
    _lcd_write_data(0x0F);  
    
    _lcd_write_cmd(0xE1); 
    _lcd_write_data(0x00); 
    _lcd_write_data(0x1D); 
    _lcd_write_data(0x20); 
    _lcd_write_data(0x02); 
    _lcd_write_data(0x0E); 
    _lcd_write_data(0x03); 
    _lcd_write_data(0x35); 
    _lcd_write_data(0x12); 
    _lcd_write_data(0x47); 
    _lcd_write_data(0x02); 
    _lcd_write_data(0x0D); 
    _lcd_write_data(0x0C); 
    _lcd_write_data(0x38); 
    _lcd_write_data(0x39); 
    _lcd_write_data(0x0F); 

    _lcd_write_cmd(0xC0); 
    _lcd_write_data(0x18); 
    _lcd_write_data(0x16); 
    
    _lcd_write_cmd(0xC1); 
    _lcd_write_data(0x41); 

    _lcd_write_cmd(0xC5); 
    _lcd_write_data(0x00); 
    _lcd_write_data(0x21); 
    _lcd_write_data(0x80); 

    _lcd_write_cmd(0x36); 
    _lcd_write_data((1<<3)|(1<<7)|(1<<5)); 

    _lcd_write_cmd(0x3A); //Interface Mode Control
    _lcd_write_data(0x55); //16BIT      0x66   18bit


    _lcd_write_cmd(0XB0);  //Interface Mode Control  
    _lcd_write_data(0x00); 
    _lcd_write_cmd(0xB1);   //Frame rate 70HZ  
    _lcd_write_data(0xB0); 

    _lcd_write_cmd(0xB4); 
    _lcd_write_data(0x02);   

    _lcd_write_cmd(0xB6); //RGB/MCU Interface Control
    _lcd_write_data(0x02);  //0x32 RGB      0x02  MCU
    _lcd_write_data(0x22); 

    _lcd_write_cmd(0xE9); 
    _lcd_write_data(0x00);
    
    _lcd_write_cmd(0XF7);    
    _lcd_write_data(0xA9); 
    _lcd_write_data(0x51); 
    _lcd_write_data(0x2C); 
    _lcd_write_data(0x82);

    _lcd_write_cmd(0x11); 
    delay_ms(120); 
    _lcd_write_cmd(0x29); 
}

/* global functions / API interface -------------------------------------*/
void bsp_lcd_xmc_init(void)
{
     _xmc_init();
	 LCD_BL_LOW;
     _lcd_init();
     bsp_lcd_clear(0xFFFF);
     LCD_BL_HIGH;
}

void bsp_lcd_write_2_data(uint16_t dat)
{
    _lcd_write_data(dat >> 8); 
    _lcd_write_data(dat); 
}

void bsp_lcd_set_block(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend)
{
    /* set row coordinates */
    _lcd_write_cmd(0x2a); 
    _lcd_write_data(xstart >> 8); 
    _lcd_write_data(xstart); 
    _lcd_write_data(xend >> 8); 
    _lcd_write_data(xend); 

    /* set column coordinates */
    _lcd_write_cmd(0x2b); 
    _lcd_write_data(ystart >> 8); 
    _lcd_write_data(ystart); 
    _lcd_write_data(yend >> 8); 
    _lcd_write_data(yend); 
    /* enable write menory */
    _lcd_write_cmd(0x2c); 
}

void bsp_lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    bsp_lcd_set_block(LCD_XMC_OFFSET_X + x, LCD_XMC_OFFSET_Y + y, LCD_XMC_OFFSET_X + x, LCD_XMC_OFFSET_Y + y); 
    bsp_lcd_write_2_data(color);
}

void bsp_lcd_clear(uint16_t color)
{   
    uint32_t i , len = LCD_XMC_WIDTH * LCD_XMC_HEIGHT; 

    bsp_lcd_set_block(LCD_XMC_OFFSET_X, LCD_XMC_OFFSET_Y, LCD_XMC_OFFSET_X+LCD_XMC_WIDTH-1, LCD_XMC_OFFSET_Y+LCD_XMC_HEIGHT-1); 
    for(i = 0; i < len; i++)
    {
        bsp_lcd_write_2_data(color);
    }
}

void bsp_lcd_frame_fill(uint16_t x, uint16_t y, uint16_t w , uint16_t h , uint16_t color)
{
    uint32_t i , len = w * h; 

    bsp_lcd_set_block(LCD_XMC_OFFSET_X + x, LCD_XMC_OFFSET_Y + y, LCD_XMC_OFFSET_X + x + w - 1, LCD_XMC_OFFSET_Y + y + h - 1); 

    for(i = 0; i < len ; i++)
    {
        bsp_lcd_write_2_data(color);
    }
}


void bsp_lcd_draw_line(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t color)
{
    uint16_t us; 
	uint16_t usX_Current, usY_Current;
	
	int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int32_t lIncrease_X, lIncrease_Y;

    lDelta_X = usX2 - usX1; //������������ 
	lDelta_Y = usY2 - usY1; 

    usX_Current = usX1; 
	usY_Current = usY1; 

    if ( lDelta_X > 0 ) {
		lIncrease_X = 1; //���õ������� 
    }
	else if ( lDelta_X == 0 ) {
		lIncrease_X = 0;//��ֱ�� 
    }
	else 
    { 
        lIncrease_X = -1;
        lDelta_X = - lDelta_X;
    } 


    if ( lDelta_Y > 0 ){
        lIncrease_Y = 1; 
    }
    else if ( lDelta_Y == 0 ){
        lIncrease_Y = 0;//ˮƽ�� 
    }
    else 
    {
        lIncrease_Y = -1;
        lDelta_Y = - lDelta_Y;
    } 

    if (  lDelta_X > lDelta_Y ){
        lDistance = lDelta_X; //ѡȡ�������������� 
    }
    else {
        lDistance = lDelta_Y; 
    }

    for ( us = 0; us <= lDistance + 1; us ++ )//������� 
	{  
		bsp_lcd_draw_point ( usX_Current, usY_Current , color);//���� 
		
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 
		
		if ( lError_X > lDistance ) 
		{ 
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  
		
		if ( lError_Y > lDistance ) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 
		
	}  
}


/* end of file ----------------------------------------------------------*/

