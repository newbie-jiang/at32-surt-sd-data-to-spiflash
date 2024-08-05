
#include "bsp_spi_flash.h"
#include "stdio.h"
#include "string.h"

uint8_t spiflash_sector_buf[SPIF_SECTOR_SIZE];

void spiflash_init(void)
{
  gpio_init_type gpio_initstructure;
  spi_init_type spi_init_struct;
  /* ʹ�ܶ�Ӧ�˿ڵ�ʱ�� */
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
* @briefд���ݵ�flash 
* @param pbuffer:���ݻ�������ָ��
* @param write_addr:д�����ݵĵ�ַ
* @param length:����������
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

  /* ������ַ */
  sector_pos = write_addr / SPIF_SECTOR_SIZE;

  /* ������ĵ�ַƫ�� */
  sector_offset = write_addr % SPIF_SECTOR_SIZE;

  /* ���� */
  sector_remain = SPIF_SECTOR_SIZE - sector_offset;
  if(length <= sector_remain)
  {
    /* С��������С */
    sector_remain = length;
  }
  while(1)
  {
    /* ����ȡ */
    spiflash_read(spiflash_buf, sector_pos * SPIF_SECTOR_SIZE, SPIF_SECTOR_SIZE);

    /* ��֤��ȡ���� */
    for(index = 0; index < sector_remain; index++)
    {
      if(spiflash_buf[sector_offset + index] != 0xFF)
      {
        /* �������ݲ�����0xff ���������� */
        break;
      }
    }
    if(index < sector_remain)
    {
      /* �������� */
      spiflash_sector_erase(sector_pos);

      /* ���Ʋ�д������ */
      for(index = 0; index < sector_remain; index++)
      {
        spiflash_buf[index + sector_offset] = pbuffer[index];
      }
      spiflash_write_nocheck(spiflash_buf, sector_pos * SPIF_SECTOR_SIZE, SPIF_SECTOR_SIZE); /* program the sector */
    }
    else
    {
      /* д��������� */
      spiflash_write_nocheck(pbuffer, write_addr, sector_remain);
    }
    if(length == sector_remain)
    {
      /* д���� */
      break;
    }
    else
    {
      /* ����д */
      sector_pos++;
      sector_offset = 0;

      pbuffer += sector_remain;
      write_addr += sector_remain;
      length -= sector_remain;
      if(length > SPIF_SECTOR_SIZE)
      {
        /* �޷�д����һ��������ʣ������ */
        sector_remain = SPIF_SECTOR_SIZE;
      }
      else
      {
        /* ��ʣ������д����һ������ */
        sector_remain = length;
      }
    }
  }
}

/**
  * @brief  ��ȡflash����
  * @param  pbuffer: ���ݻ�������ָ��
  * @param  read_addr: ��ȡ���ݵĵ�ַ
  * @param  length: ���峤��
  * @retval none
  */
void spiflash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t length)
{
  FLASH_CS_LOW();
  spi_byte_write(SPIF_READDATA); /* ���Ͷ�ָ�� */
  spi_byte_write((uint8_t)((read_addr) >> 16)); /* ����24λ��ַ */
  spi_byte_write((uint8_t)((read_addr) >> 8));
  spi_byte_write((uint8_t)read_addr);
  spi_bytes_read(pbuffer, length);
  FLASH_CS_HIGH();
}

/**
  * @brief  ������������
  * @param  erase_addr: ѡ��ɾ����������ַ
  * @retval none
  */
void spiflash_sector_erase(uint32_t erase_addr)
{
  erase_addr *= SPIF_SECTOR_SIZE; /* ��������ַת��Ϊ�ֽڵ�ַ */
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
  * @brief  ��У��д������
  * @param  pbuffer: ���ݻ�������ָ��
  * @param  write_addr: д�����ݵĵ�ַ
  * @param  length: ���峤��
  * @retval none
  */
void spiflash_write_nocheck(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  uint16_t page_remain;

  /* ��ҳ���б����ֽ� */
  page_remain = SPIF_PAGE_SIZE - write_addr % SPIF_PAGE_SIZE;
  if(length <= page_remain)
  {
    /* С��ҳ���С */
    page_remain = length;
  }
  while(1)
  {
    spiflash_page_write(pbuffer, write_addr, page_remain);
    if(length == page_remain)
    {
      /* �������ݶ�����У�� */
      break;
    }
    else
    {
      /* length > page_remain */
      pbuffer += page_remain;
      write_addr += page_remain;

      /* ʣ����ֽڽ������� */
      length -= page_remain;
      if(length > SPIF_PAGE_SIZE)
      {
        /* ����һ��дһҳ */
        page_remain = SPIF_PAGE_SIZE;
      }
      else
      {
        /* С��ҳ���С */
        page_remain = length;
      }
    }
  }
}

/**
  * @brief  дһҳ����
  * @param  pbuffer: ���ݻ�������ָ��
  * @param  write_addr: д�����ݵĵ�ַ
  * @param  length: ���峤��
  * @retval none
  */
void spiflash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  if((0 < length) && (length <= SPIF_PAGE_SIZE))
  {
    /* дʹ�� */
    spiflash_write_enable();

    FLASH_CS_LOW();

    /* ����ָ�� */
    spi_byte_write(SPIF_PAGEPROGRAM);

    /* ����24λ��ַ */
    spi_byte_write((uint8_t)((write_addr) >> 16));
    spi_byte_write((uint8_t)((write_addr) >> 8));
    spi_byte_write((uint8_t)write_addr);
    spi_bytes_write(pbuffer,length);

    FLASH_CS_HIGH();

    /* �ȴ����� */
    spiflash_wait_busy();
  }
}

/**
  * @brief  ����д������
  * @param  pbuffer: ���ݻ�������ָ��
  * @param  length: ���峤��
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
  * @brief  ������ȡ����
  * @param  pbuffer: ���ڱ������ݵĻ�����
  * @param  length: ���峤��
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
  * @brief  �ȴ����
  * @param  none
  * @retval none
  */
void spiflash_wait_busy(void)
{
  while((spiflash_read_sr1() & 0x01) == 0x01);
}

/**
  * @brief  ��ȡsr1�Ĵ���
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
  * @brief  ʹ��д��
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
  * @brief  ��ȡflashоƬ
  * @param  none
  * @retval оƬid
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
  * @brief  д��һ���ֽڵ�flash
  * @param  data: д�������
  * @retval flash���ص�����
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
  * @brief  ��ȡ�ֽڵ�flash
  * @param  none
  * @retval flash���ص�����
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


#define SPI_FLASH_SECTOR_SIZE 4096 // ����ÿ��������СΪ4KB
#define TEST_ADDRESS 0x00000000    // ���Ե�ַ


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

    // ��ʼ�� SPI Flash
    spiflash_init();

    // ���д������
    for (i = 0; i < SPI_FLASH_SECTOR_SIZE; i++) {
        write_data[i] = (uint8_t)i;
    }

    printf("Original data to write:\r\n");
//    print_buffer(write_data, SPI_FLASH_SECTOR_SIZE);

    // ��������
    spiflash_sector_erase(TEST_ADDRESS);
	printf("spiflash_sector_erase:\r\n");

    // д�����ݵ� SPI Flash
    spiflash_write(write_data, TEST_ADDRESS, SPI_FLASH_SECTOR_SIZE);
	printf("spiflash_write:\r\n");

    // ��ȡ���ݴ� SPI Flash
    memset(read_data, 0, SPI_FLASH_SECTOR_SIZE); // ��ն�������
    spiflash_read(read_data, TEST_ADDRESS, SPI_FLASH_SECTOR_SIZE);

    printf("Data read from SPI Flash:\n");
//    print_buffer(read_data, SPI_FLASH_SECTOR_SIZE);

    // �������һ����
    if (memcmp(write_data, read_data, SPI_FLASH_SECTOR_SIZE) == 0) {
        printf("SPI Flash read/write test success.\n");
    } else {
        printf("SPI Flash read/write test failed.\n");
    }

}


