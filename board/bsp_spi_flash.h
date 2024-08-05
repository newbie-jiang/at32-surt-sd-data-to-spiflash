
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "at32f435_437.h"


/*SPI接口定义-开头****************************/
#define      FLASH_SPIx                        SPI4
#define      FLASH_SPI_CLK                     CRM_SPI4_PERIPH_CLOCK

//CS(NSS)引脚 片选
#define      FLASH_SPI_CS_CLK                  CRM_GPIOE_PERIPH_CLOCK    
#define      FLASH_SPI_CS_PORT                 GPIOE
#define      FLASH_SPI_CS_PIN                  GPIO_PINS_4

//SCK引脚
#define      FLASH_SPI_SCK_CLK                 CRM_GPIOE_PERIPH_CLOCK 
#define      FLASH_SPI_SCK_PORT                GPIOE   
#define      FLASH_SPI_SCK_PIN                 GPIO_PINS_2
#define      FLASH_SPI_SCK_SOURCE              GPIO_PINS_SOURCE2
#define      FLASH_SPI_SCK_MUX                 GPIO_MUX_5

//MISO引脚

#define      FLASH_SPI_MISO_CLK                CRM_GPIOE_PERIPH_CLOCK   
#define      FLASH_SPI_MISO_PORT               GPIOE
#define      FLASH_SPI_MISO_PIN                GPIO_PINS_5
#define      FLASH_SPI_MISO_SOURCE             GPIO_PINS_SOURCE5
#define      FLASH_SPI_MISO_MUX                GPIO_MUX_5
//MOSI引脚

#define      FLASH_SPI_MOSI_CLK                CRM_GPIOE_PERIPH_CLOCK   
#define      FLASH_SPI_MOSI_PORT               GPIOE
#define      FLASH_SPI_MOSI_PIN                GPIO_PINS_6
#define      FLASH_SPI_MOSI_SOURCE             GPIO_PINS_SOURCE6
#define      FLASH_SPI_MOSI_MUX                GPIO_MUX_5

#define FLASH_CS_HIGH()                  gpio_bits_set(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN) 
#define FLASH_CS_LOW()                   gpio_bits_reset(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN)

/**
  * @}
  */

/** @defgroup SPI_flash_id_definition
  * @{
  */

/*
 * flash define
 */
#define W25Q80                           0xEF13
#define W25Q16                           0xEF14
#define W25Q32                           0xEF15
#define W25Q64                           0xEF16
/* 16mb, the range of address:0~0xFFFFFF */
#define W25Q128                          0xEF17

/**
  * @}
  */

/** @defgroup SPI_flash_operation_definition
  * @{
  */

#define SPIF_CHIP_SIZE                   0x1000000
#define SPIF_SECTOR_SIZE                 4096
#define SPIF_PAGE_SIZE                   256

#define SPIF_WRITEENABLE                 0x06
#define SPIF_WRITEDISABLE                0x04
/* s7-s0 */
#define SPIF_READSTATUSREG1              0x05
#define SPIF_WRITESTATUSREG1             0x01
/* s15-s8 */
#define SPIF_READSTATUSREG2              0x35
#define SPIF_WRITESTATUSREG2             0x31
/* s23-s16 */
#define SPIF_READSTATUSREG3              0x15
#define SPIF_WRITESTATUSREG3             0x11
#define SPIF_READDATA                    0x03
#define SPIF_FASTREADDATA                0x0B
#define SPIF_FASTREADDUAL                0x3B
#define SPIF_PAGEPROGRAM                 0x02
/* block size:64kb */
#define SPIF_BLOCKERASE                  0xD8
#define SPIF_SECTORERASE                 0x20
#define SPIF_CHIPERASE                   0xC7
#define SPIF_POWERDOWN                   0xB9
#define SPIF_RELEASEPOWERDOWN            0xAB
#define SPIF_DEVICEID                    0xAB
#define SPIF_MANUFACTDEVICEID            0x90
#define SPIF_JEDECDEVICEID               0x9F
#define FLASH_SPI_DUMMY_BYTE             0xA5

/**
  * @}
  */

/** @defgroup SPI_flash_exported_functions
  * @{
  */

void spiflash_init(void);
void spiflash_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);
void spiflash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t length);
void spiflash_sector_erase(uint32_t erase_addr);
void spiflash_write_nocheck(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);
void spiflash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);
void spi_bytes_write(uint8_t *pbuffer, uint32_t length);
void spi_bytes_read(uint8_t *pbuffer, uint32_t length);
void spiflash_wait_busy(void);
uint8_t spiflash_read_sr1(void);
void spiflash_write_enable(void);
uint16_t spiflash_read_id(void);
uint8_t spi_byte_write(uint8_t data);
uint8_t spi_byte_read(void);

unsigned long r_dat_bat(unsigned long address,unsigned long DataLen,unsigned char *pBuff);
unsigned char gt_read_data(unsigned char* sendbuf , unsigned char sendlen , unsigned char* receivebuf, unsigned int receivelen);

void spi_flash_data_test(void);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif

