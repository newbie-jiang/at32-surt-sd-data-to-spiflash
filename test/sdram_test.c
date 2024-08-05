#include "test.h"

void  sdram__init(void)
{
  sdram_init();
}

#define BUF_SIZE                         4096
static uint16_t write_buf[BUF_SIZE];
static uint16_t read_buf[BUF_SIZE];
 


void sdram_test(void)
{
  uint16_t i;
	
  /* display information */
  lcd_string_show(10, 20, 200, 24, 24, (uint8_t *)"SDRAM Test");

  /* initialize write buffer */
  for(i = 0; i < BUF_SIZE; i++)
  {
    write_buf[i] = i;
  }

  /* write data to sdram */
  sdram_data_write(0, write_buf, BUF_SIZE);

  /* read data from sdram */
  sdram_data_read(0, read_buf, BUF_SIZE);

  /* compare data */
  if(buffer_compare((uint8_t *)write_buf, (uint8_t *)read_buf, BUF_SIZE * 2) == SUCCESS)
  {
    lcd_string_show(10, 60, 310, 24, 24, (uint8_t *)"sdram write/read ok");
  }
  else
  {
    lcd_string_show(10, 60, 310, 24, 24, (uint8_t *)"sdram write/read error");
  }
}


