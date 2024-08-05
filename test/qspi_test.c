#include "test.h"



#define BUF_SIZE                        4096
uint8_t write_buf[BUF_SIZE];
uint8_t read_buf[BUF_SIZE];

void qspi_test(void)
{
  uint16_t i;
	
  /* display information */
  lcd_string_show(10, 200, 200, 24, 24, (uint8_t *)"QSPI Flash Test");

  /* initialize write buffer */
  for( i = 0; i < BUF_SIZE; i++)
  {
    write_buf[i] = i % 256;
  }

  /* erase sector 0 */
  qspi_flash_erase(0);

  /* write data to quad spi flash */
  qspi_flash_data_write(0, write_buf, BUF_SIZE);

  /* read data from quad spi flash */
  qspi_flash_data_read(0, read_buf, BUF_SIZE);

  /* compare data */
  if(buffer_compare(write_buf, read_buf, BUF_SIZE) == SUCCESS)
  {
    lcd_string_show(10, 230, 310, 24, 24, (uint8_t *)"flash write/read ok");
  }
  else
  {
    lcd_string_show(10, 230, 310, 24, 24, (uint8_t *)"flash write/read error");
  }	
}


