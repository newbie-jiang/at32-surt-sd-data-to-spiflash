#include "at32f435_437_clock.h"
#include "at_surf_f437_board_lcd.h"
#include "at_surf_f437_board_qspi_flash.h"
#include "at_surf_f437_board_touch.h"
#include "ff.h"
#include "test.h"
//tp_dev_t *tp_dev;
/* --------------------------------------------------------------------------*/
#include "gt.h"
#include "gt_ui.h"
#include "app.h"
#include "bsp_qspi_flash_zq.h"

uint32_t spi_wr(uint8_t * data_write, uint32_t len_write, uint8_t * data_read, uint32_t len_read)
{
    uint32_t ReadAddr;
	ReadAddr = *(data_write + 1) << 16;					//高八位地址
	ReadAddr += *(data_write + 2) << 8;				//中八位地址
	ReadAddr += *(data_write + 3);						//低八位地址

    qspi_flash_data_read(ReadAddr,data_read,len_read);
//	spiflash_read(data_read,ReadAddr,len_read);
    return 1;
}

void _flush_cb(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color)
{

	lcd_color_fill(area->x,area->y,area->w+area->x-1 ,area->h+area->y-1,(uint16_t *)color);	
   	
}

void read_cb(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data)
{
	// GT_LOGD(GT_LOG_TAG_TP , "point in: x: %d , y: %d , state = %d\r\n", data->point.x , data->point.y , data->state);

//    if(!tp_dev->read_point()){
//		data->state = GT_INDEV_STATE_RELEASED;
//        data->point.x = tp_dev->point.x;
//        data->point.y = tp_dev->point.y;
//		return;
//	}
//    data->point.x = tp_dev->point.x;
//	data->point.y = tp_dev->point.y;
//	data->state = tp_dev->point.status == TP_STATE_RELEASED ? GT_INDEV_STATE_RELEASED : GT_INDEV_STATE_PRESSED;

	// GT_LOGD(GT_LOG_TAG_TP , "point: x: %d , y: %d , state = %d\r\n", data->point.x , data->point.y , data->state);
}

void read_cb_btn(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data)
{
    uint8_t status = 0;
	//status = KEY_Scan(1);
	if (status) {
		data->btn_id = status;
		data->state = GT_INDEV_STATE_PRESSED;
	}
	else {
		data->state = GT_INDEV_STATE_RELEASED;
	}
}

//#define TOUCH_TEST
//#define QSPI_TEST
//#define FATFS_TEST
//#define SDRAM_TEST

int main(void)
{
//	/* config vector table offset */
//	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x20000);

	system_clock_config();

	nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	
	__enable_irq();
	
	delay_init();
	
	/************ unit test *****************/
	#ifdef TOUCH_TEST
	 lcd_touch_init();
	#endif
	
	
	#ifdef FATFS_TEST
	 lcd__init();
	 fatfs_test();
	#endif
	
	
	#ifdef QSPI_TEST
	 lcd__init();
	 qspi_flash_init();
	 qspi_test();
	#endif
	
	#ifdef SDRAM_TEST
	lcd__init();
	sdram__init();
	sdram_test();
	#endif 
	
	/*****************************************/
	application();
	
	GT_Font_Init();
	gt_init();
	gt_ui_init();

	while(1)
	{
		#ifdef TOUCH_TEST
	    touch_test_while();	
	    #endif
					
		gt_task_handler();
		delay_ms(1);	
	}
}



