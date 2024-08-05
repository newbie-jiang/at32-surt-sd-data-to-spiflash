#ifndef __TEST_H
#define __TEST_H

#include "stdio.h"
#include "stdint.h"
#include "at32f435_437_clock.h"
#include "at_surf_f437_board_lcd.h"
#include "at_surf_f437_board_qspi_flash.h"
#include "at_surf_f437_board_touch.h"
#include "ff.h"
#include "at_surf_f437_board_sdram.h"

error_status buffer_compare(uint8_t* buffer1, uint8_t* buffer2, uint32_t len);

void lcd__init(void);

void fatfs_test(void);
error_status file_system_init(void);

extern void uart_print_init(uint32_t baudrate);

void lcd_touch_init(void);
void touch_test_while(void);

extern void qspi_flash_init(void);
void qspi_test(void);

void sdram__init(void);
void sdram_test(void);
 
 
#endif 

