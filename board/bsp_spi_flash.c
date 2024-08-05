
#include "bsp_spi_flash.h"
#include "stdio.h"
#include "string.h"

uint8_t spiflash_sector_buf[SPIF_SECTOR_SIZE];

void spiflash_init(void)
{
  gpio_init_type gpio_initstructure;
  spi_init_type spi_init_struct;
  /* 使能对应端口的时钟 */
  crm_periph_clock_enable(FLASH_SPI_CS_CLK, TRUE);
  crm_periph_clock_enable(FLASH_SPI_SCK_CLK, TRUE);
  crm_periph_clock_enable(FLASH_SPI_MISO_CLK, TRUE);
  crm_periph_clock_enable(FLASH_SPI_MOSI_CLK, TRUE);
	
	/* cs */
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins           = FLASH_SPI_CS_PIN;
  gpio_init(FLASH_SPI_CS_PORT, &gpio_initstructure);

  /* sck */
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
  gpio_initstructure.gpio_pins           = FLASH_SPI_SCK_PIN;
  gpio_init(FLASH_SPI_SCK_PORT, &gpio_initstructure);
  gpio_pin_mux_config(FLASH_SPI_SCK_PORT, FLASH_SPI_SCK_SOURCE, FLASH_SPI_SCK_MUX);

  /* miso */
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_pins           = FLASH_SPI_MISO_PIN;
  gpio_init(FLASH_SPI_MISO_PORT, &gpio_initstructure);
  gpio_pin_mux_config(FLASH_SPI_MISO_PORT, FLASH_SPI_MISO_SOURCE, FLASH_SPI_MISO_MUX);

  /* mosi */
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_pins           = FLASH_SPI_MOSI_PIN;
  gpio_init(FLASH_SPI_MOSI_PORT, &gpio_initstructure);
  gpio_pin_mux_config(FLASH_SPI_MOSI_PORT, FLASH_SPI_MOSI_SOURCE, FLASH_SPI_MOSI_MUX);

  FLASH_CS_HIGH();
  crm_periph_clock_enable(FLASH_SPI_CLK, TRUE);
  spi_default_para_init(&spi_init_struct);
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_8;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(FLASH_SPIx, &spi_init_struct);
  spi_enable(FLASH_SPIx, TRUE);
}

/*
* @brief写数据到flash 
* @param pbuffer:数据缓冲区的指针
* @param write_addr:写入数据的地址
* @param length:缓冲区长度
* @retval none 
*/
void spiflash_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  uint32_t sector_pos;
  uint16_t sector_offset;
  uint16_t sector_remain;
  uint16_t index;
  uint8_t *spiflash_buf;
  spiflash_buf = spiflash_sector_buf;

  /* 扇区地址 */
  sector_pos = write_addr / SPIF_SECTOR_SIZE;

  /* 扇区里的地址偏移 */
  sector_offset = write_addr % SPIF_SECTOR_SIZE;

  /* 保留 */
  sector_remain = SPIF_SECTOR_SIZE - sector_offset;
  if(length <= sector_remain)
  {
    /* 小于扇区大小 */
    sector_remain = length;
  }
  while(1)
  {
    /* 板块读取 */
    spiflash_read(spiflash_buf, sector_pos * SPIF_SECTOR_SIZE, SPIF_SECTOR_SIZE);

    /* 验证读取区域 */
    for(index = 0; index < sector_remain; index++)
    {
      if(spiflash_buf[sector_offset + index] != 0xFF)
      {
        /* 部分数据不等于0xff 扇区擦除掉 */
        break;
      }
    }
    if(index < sector_remain)
    {
      /* 擦除扇区 */
      spiflash_sector_erase(sector_pos);

      /* 复制并写入数据 */
      for(index = 0; index < sector_remain; index++)
      {
        spiflash_buf[index + sector_offset] = pbuffer[index];
      }
      spiflash_write_nocheck(spiflash_buf, sector_pos * SPIF_SECTOR_SIZE, SPIF_SECTOR_SIZE); /* program the sector */
    }
    else
    {
      /* 写入擦除区域 */
      spiflash_write_nocheck(pbuffer, write_addr, sector_remain);
    }
    if(length == sector_remain)
    {
      /* 写结束 */
      break;
    }
    else
    {
      /* 继续写 */
      sector_pos++;
      sector_offset = 0;

      pbuffer += sector_remain;
      write_addr += sector_remain;
      length -= sector_remain;
      if(length > SPIF_SECTOR_SIZE)
      {
        /* 无法写入下一个扇区的剩余数据 */
        sector_remain = SPIF_SECTOR_SIZE;
      }
      else
      {
        /* 将剩余数据写入下一个扇区 */
        sector_remain = length;
      }
    }
  }
}

/**
  * @brief  读取flash数据
  * @param  pbuffer: 数据缓冲区的指针
  * @param  read_addr: 读取数据的地址
  * @param  length: 缓冲长度
  * @retval none
  */
void spiflash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t length)
{
  FLASH_CS_LOW();
  spi_byte_write(SPIF_READDATA); /* 发送读指令 */
  spi_byte_write((uint8_t)((read_addr) >> 16)); /* 发送24位地址 */
  spi_byte_write((uint8_t)((read_addr) >> 8));
  spi_byte_write((uint8_t)read_addr);
  spi_bytes_read(pbuffer, length);
  FLASH_CS_HIGH();
}

/**
  * @brief  擦除扇区数据
  * @param  erase_addr: 选择删除的扇区地址
  * @retval none
  */
void spiflash_sector_erase(uint32_t erase_addr)
{
  erase_addr *= SPIF_SECTOR_SIZE; /* 将扇区地址转换为字节地址 */
  spiflash_write_enable();
  spiflash_wait_busy();
  FLASH_CS_LOW();
  spi_byte_write(SPIF_SECTORERASE);
  spi_byte_write((uint8_t)((erase_addr) >> 16));
  spi_byte_write((uint8_t)((erase_addr) >> 8));
  spi_byte_write((uint8_t)erase_addr);
  FLASH_CS_HIGH();
  spiflash_wait_busy();
}

/**
  * @brief  无校验写入数据
  * @param  pbuffer: 数据缓冲区的指针
  * @param  write_addr: 写入数据的地址
  * @param  length: 缓冲长度
  * @retval none
  */
void spiflash_write_nocheck(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  uint16_t page_remain;

  /* 在页面中保留字节 */
  page_remain = SPIF_PAGE_SIZE - write_addr % SPIF_PAGE_SIZE;
  if(length <= page_remain)
  {
    /* 小于页面大小 */
    page_remain = length;
  }
  while(1)
  {
    spiflash_page_write(pbuffer, write_addr, page_remain);
    if(length == page_remain)
    {
      /* 所有数据都经过校验 */
      break;
    }
    else
    {
      /* length > page_remain */
      pbuffer += page_remain;
      write_addr += page_remain;

      /* 剩余的字节将被阻塞 */
      length -= page_remain;
      if(length > SPIF_PAGE_SIZE)
      {
        /* 可以一次写一页 */
        page_remain = SPIF_PAGE_SIZE;
      }
      else
      {
        /* 小于页面大小 */
        page_remain = length;
      }
    }
  }
}

/**
  * @brief  写一页数据
  * @param  pbuffer: 数据缓冲区的指针
  * @param  write_addr: 写入数据的地址
  * @param  length: 缓冲长度
  * @retval none
  */
void spiflash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  if((0 < length) && (length <= SPIF_PAGE_SIZE))
  {
    /* 写使能 */
    spiflash_write_enable();

    FLASH_CS_LOW();

    /* 发送指令 */
    spi_byte_write(SPIF_PAGEPROGRAM);

    /* 发送24位地址 */
    spi_byte_write((uint8_t)((write_addr) >> 16));
    spi_byte_write((uint8_t)((write_addr) >> 8));
    spi_byte_write((uint8_t)write_addr);
    spi_bytes_write(pbuffer,length);

    FLASH_CS_HIGH();

    /* 等待结束 */
    spiflash_wait_busy();
  }
}

/**
  * @brief  连续写入数据
  * @param  pbuffer: 数据缓冲区的指针
  * @param  length: 缓冲长度
  * @retval none
  */
void spi_bytes_write(uint8_t *pbuffer, uint32_t length)
{
  volatile uint8_t dummy_data;

  while(length--)
  {
    while(spi_i2s_flag_get(FLASH_SPIx, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(FLASH_SPIx, *pbuffer);
    while(spi_i2s_flag_get(FLASH_SPIx, SPI_I2S_RDBF_FLAG) == RESET);
    dummy_data = spi_i2s_data_receive(FLASH_SPIx);
    pbuffer++;
  }
}

/**
  * @brief  连续读取数据
  * @param  pbuffer: 用于保存数据的缓冲区
  * @param  length: 缓冲长度
  * @retval none
  */
void spi_bytes_read(uint8_t *pbuffer, uint32_t length)
{
  uint8_t write_value = FLASH_SPI_DUMMY_BYTE;

  while(length--)
  {
    while(spi_i2s_flag_get(FLASH_SPIx, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(FLASH_SPIx, write_value);
    while(spi_i2s_flag_get(FLASH_SPIx, SPI_I2S_RDBF_FLAG) == RESET);
    *pbuffer = spi_i2s_data_receive(FLASH_SPIx);
    pbuffer++;
  }

}

/**
  * @brief  等待完成
  * @param  none
  * @retval none
  */
void spiflash_wait_busy(void)
{
  while((spiflash_read_sr1() & 0x01) == 0x01);
}

/**
  * @brief  读取sr1寄存器
  * @param  none
  * @retval none
  */
uint8_t spiflash_read_sr1(void)
{
  uint8_t breadbyte = 0;
  FLASH_CS_LOW();
  spi_byte_write(SPIF_READSTATUSREG1);
  breadbyte = (uint8_t)spi_byte_read();
  FLASH_CS_HIGH();
  return (breadbyte);
}

/**
  * @brief  使能写入
  * @param  none
  * @retval none
  */
void spiflash_write_enable(void)
{
  FLASH_CS_LOW();
  spi_byte_write(SPIF_WRITEENABLE);
  FLASH_CS_HIGH();
}

/**
  * @brief  读取flash芯片
  * @param  none
  * @retval 芯片id
  */
uint16_t spiflash_read_id(void)
{
  uint16_t wreceivedata = 0;
  FLASH_CS_LOW();
  spi_byte_write(SPIF_MANUFACTDEVICEID);
  spi_byte_write(0x00);
  spi_byte_write(0x00);
  spi_byte_write(0x00);
  wreceivedata |= spi_byte_read() << 8;
  wreceivedata |= spi_byte_read();
  FLASH_CS_HIGH();
  return wreceivedata;
}

/**
  * @brief  写入一个字节到flash
  * @param  data: 写入的数据
  * @retval flash返回的数据
  */
uint8_t spi_byte_write(uint8_t data)
{
  uint8_t brxbuff;
  spi_i2s_dma_transmitter_enable(FLASH_SPIx, FALSE);
  spi_i2s_dma_receiver_enable(FLASH_SPIx, FALSE);
  spi_i2s_data_transmit(FLASH_SPIx, data);
  while(spi_i2s_flag_get(FLASH_SPIx, SPI_I2S_RDBF_FLAG) == RESET);
  brxbuff = spi_i2s_data_receive(FLASH_SPIx);
  while(spi_i2s_flag_get(FLASH_SPIx, SPI_I2S_BF_FLAG) != RESET);
  return brxbuff;
}

/**
  * @brief  读取字节到flash
  * @param  none
  * @retval flash返回的数据
  */
uint8_t spi_byte_read(void)
{
  return (spi_byte_write(FLASH_SPI_DUMMY_BYTE));
}


unsigned long r_dat_bat(unsigned long address,unsigned long DataLen,unsigned char *pBuff)
{
	spiflash_read(pBuff, address , DataLen );
	return 1;
}

unsigned char gt_read_data(unsigned char* sendbuf , unsigned char sendlen , unsigned char* receivebuf, unsigned int receivelen)
{

    uint8_t i;
	
		
	
    if(sendlen>5)
		{
        return 0;
    }
		FLASH_CS_LOW();
		
    for(i=0;i<sendlen ;i++) //-1
    {
			spi_byte_write(sendbuf[i]);
    }

    for(i = 0; i < receivelen; ++i)
    {
        receivebuf[i] = spi_byte_read();
    }
		FLASH_CS_HIGH();

    return 1;
}


#define SPI_FLASH_SECTOR_SIZE 4096 // 假设每个扇区大小为4KB
#define TEST_ADDRESS 0x00000000    // 测试地址


void print_buffer(uint8_t *buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}


void spi_flash_data_test(void)
{
    uint8_t write_data[SPI_FLASH_SECTOR_SIZE];
    uint8_t read_data[SPI_FLASH_SECTOR_SIZE];
    uint32_t i;

    // 初始化 SPI Flash
    spiflash_init();

    // 填充写入数据
    for (i = 0; i < SPI_FLASH_SECTOR_SIZE; i++) {
        write_data[i] = (uint8_t)i;
    }

    printf("Original data to write:\r\n");
//    print_buffer(write_data, SPI_FLASH_SECTOR_SIZE);

    // 擦除扇区
    spiflash_sector_erase(TEST_ADDRESS);
	printf("spiflash_sector_erase:\r\n");

    // 写入数据到 SPI Flash
    spiflash_write(write_data, TEST_ADDRESS, SPI_FLASH_SECTOR_SIZE);
	printf("spiflash_write:\r\n");

    // 读取数据从 SPI Flash
    memset(read_data, 0, SPI_FLASH_SECTOR_SIZE); // 清空读缓冲区
    spiflash_read(read_data, TEST_ADDRESS, SPI_FLASH_SECTOR_SIZE);

    printf("Data read from SPI Flash:\n");
//    print_buffer(read_data, SPI_FLASH_SECTOR_SIZE);

    // 检查数据一致性
    if (memcmp(write_data, read_data, SPI_FLASH_SECTOR_SIZE) == 0) {
        printf("SPI Flash read/write test success.\n");
    } else {
        printf("SPI Flash read/write test failed.\n");
    }

}


