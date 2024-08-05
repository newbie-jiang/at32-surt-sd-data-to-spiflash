/**
 * @file bsp_qspi_flash.h
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2023-09-07 16:05:38
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
#ifndef _BSP_QSPI_FLASH_H_
#define _BSP_QSPI_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "at32f403a_407_board.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"





 /* use dma transfer spi data */
#define SPI_TRANS_DMA  


#define FLASH_CS_HIGH()                  gpio_bits_set(GPIOE, GPIO_PINS_4)
#define FLASH_CS_LOW()                   gpio_bits_reset(GPIOE, GPIO_PINS_4)

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
//#define FLASH_SPI_DUMMY_BYTE             0xA5
#define FLASH_SPI_DUMMY_BYTE             0x00


/* define ---------------------------------------------------------------*/
#define QSPI_FLASH_SECTOR_SIZE           4096
#define QSPI_FLASH_PAGE_SIZE             256
#define QSPI_FLASH_FIFO_DEPTH            (32*4)

#define QSPI_FLASH_QSPIx                 SPI4
#define QSPI_FLASH_QSPI_CLK              CRM_SPI4_PERIPH_CLOCK

#define QSPI_FLASH_CS_GPIO_CLK           CRM_GPIOE_PERIPH_CLOCK
#define QSPI_FLASH_CS_GPIO_PIN           GPIO_PINS_4
#define QSPI_FLASH_CS_GPIO_PINS_SOURCE   GPIO_PINS_SOURCE4
#define QSPI_FLASH_CS_GPIO_PORT          GPIOE
#define QSPI_FLASH_CS_GPIO_MUX           GPIO_MODE_MUX

#define QSPI_FLASH_CLK_GPIO_CLK          CRM_GPIOE_PERIPH_CLOCK
#define QSPI_FLASH_CLK_GPIO_PIN          GPIO_PINS_2
#define QSPI_FLASH_CLK_GPIO_PINS_SOURCE  GPIO_PINS_SOURCE2
#define QSPI_FLASH_CLK_GPIO_PORT         GPIOE
#define QSPI_FLASH_CLK_GPIO_MUX          GPIO_MODE_MUX

#define QSPI_FLASH_D0_GPIO_CLK           CRM_GPIOE_PERIPH_CLOCK
#define QSPI_FLASH_D0_GPIO_PIN           GPIO_PINS_6
#define QSPI_FLASH_D0_GPIO_PINS_SOURCE   GPIO_PINS_SOURCE6
#define QSPI_FLASH_D0_GPIO_PORT          GPIOE
#define QSPI_FLASH_D0_GPIO_MUX           GPIO_MODE_MUX

#define QSPI_FLASH_D1_GPIO_CLK           CRM_GPIOE_PERIPH_CLOCK
#define QSPI_FLASH_D1_GPIO_PIN           GPIO_PINS_5
#define QSPI_FLASH_D1_GPIO_PINS_SOURCE   GPIO_PINS_SOURCE5
#define QSPI_FLASH_D1_GPIO_PORT          GPIOE
#define QSPI_FLASH_D1_GPIO_MUX           GPIO_MODE_MUX



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
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
void spiflash_wakeup(void);
//
unsigned long r_dat_bat(unsigned long address,unsigned long DataLen,unsigned char *pBuff);
unsigned char gt_read_data(unsigned char* sendbuf , unsigned char sendlen , unsigned char* receivebuf, unsigned int receivelen);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_BSP_QSPI_FLASH_H_

/* end of file ----------------------------------------------------------*/


