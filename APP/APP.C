#include "app.h"
#include "string.h"
#include "bsp_spi_flash.h"

typedef struct file_info{
	uint32_t file_size_byte;


}file_info;

file_info fil_info;

void  all_bsp_init(void)
{
   //usrt init
   uart_print_init(115200);
	
   printf("printf test\r\n");
	
   //lcd 初始化	
   lcd__init();
		
   //sdram初始化
//   sdram__init();

   //文件系统初始化
	if(file_system_init() == SUCCESS)
   { 
	   printf("sd card init ok\r\n");
   }else{
	   printf("sd card init err\r\n");
   }
    
   //qspi初始化
   qspi_flash_init();
   
   //spi初始化 
   spiflash_init();
   
   spi_flash_data_test();
}


#define SDRAM_BASE_ADDR 0xC0000000
#define BUFFER_SIZE 4096  // 缓冲区大小

uint32_t crc32(const void *data, size_t length, uint32_t crc);

/* 拷贝数据至sdram并校验数据一致性 */
void copy_file_to_sdram(void) {
    FIL file;        // 文件对象
    FRESULT res;     // 文件操作结果
    UINT br;         // 读取的字节数
    uint8_t* sdram_ptr; // SDRAM指针
    uint32_t file_size;
    uint8_t buffer[BUFFER_SIZE];  // 缓冲区

    // 打开文件
    res = f_open(&file, "1:resource.bin", FA_READ);
    if (res != FR_OK) {
        printf("Failed to open file: %d\n", res);
        return;
    }

    // 获取文件大小
    file_size = f_size(&file);
	fil_info.file_size_byte = file_size;
	
    printf("file_size is %d\r\n", file_size);

    // 设置SDRAM指针
    sdram_ptr = (uint8_t*)SDRAM_BASE_ADDR;

    // 读取文件内容到SDRAM
    uint32_t file_crc = 0xFFFFFFFF;  // 初始化文件CRC
    uint32_t sdram_crc = 0xFFFFFFFF; // 初始化SDRAM CRC
    uint32_t bytes_read = 0;

    while (bytes_read < file_size) {
        UINT bytes_to_read = (file_size - bytes_read) < BUFFER_SIZE ? (file_size - bytes_read) : BUFFER_SIZE;

        // 从文件读取数据到缓冲区
        res = f_read(&file, buffer, bytes_to_read, &br);
        if (res != FR_OK || br == 0) {
            printf("Failed to read file: %d\n", res);
            f_close(&file);
            return;
        }

        // 将数据从缓冲区复制到SDRAM
        memcpy(sdram_ptr + bytes_read, buffer, br);

        // 计算文件数据的CRC
        file_crc = crc32(buffer, br, file_crc);

        // 计算SDRAM数据的CRC
        sdram_crc = crc32(sdram_ptr + bytes_read, br, sdram_crc);

        bytes_read += br;
    }

    file_crc ^= 0xFFFFFFFF;
    sdram_crc ^= 0xFFFFFFFF;

    printf("File CRC: 0x%08X\n", file_crc);
    printf("SDRAM CRC: 0x%08X\n", sdram_crc);

    if (file_crc == sdram_crc) {
        printf("CRC check passed. Data is consistent success.\n");
    } else {
        printf("CRC check failed. Data is inconsistent.\n");
    }

    // 关闭文件
    f_close(&file);
}



// 更新后的CRC计算函数，支持累积计算
uint32_t crc32(const void *data, size_t length, uint32_t crc) {
    const uint8_t *buf = (const uint8_t *)data;
    for (size_t i = 0; i < length; ++i) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/* 获取文件大小，单位 byte */
uint32_t get_file_size(void)
{	
  return fil_info.file_size_byte;
}

#define PAGE_SIZE   4096


//拷贝sdram到qspi flash
void copy_sdram_to_qspi_flash(uint32_t sdram_addr, uint32_t flash_addr, uint32_t data_length) {
	
    uint8_t buffer[BUFFER_SIZE];
    uint8_t verify_buffer[BUFFER_SIZE];
    uint32_t bytes_processed = 0;
    uint32_t chunk_size;
    uint32_t sdram_crc = 0xFFFFFFFF;
    uint32_t flash_crc = 0xFFFFFFFF;
    uint8_t* sdram_ptr = (uint8_t*)sdram_addr;
		
	//获取文件大小并擦除对应扇区
    uint32_t erasure_byte = get_file_size();
	
	//将获取的字节数适配对应的擦除算法	
	for(uint32_t address = 0;address <= erasure_byte; address += PAGE_SIZE)
	{
	  qspi_flash_erase(address);
	  printf("erase adress:%08x\r\n",address);
	}
	
    // 从SDRAM读取并写入到QSPI Flash
    while (bytes_processed < data_length) {
        chunk_size = (data_length - bytes_processed) < BUFFER_SIZE ? (data_length - bytes_processed) : BUFFER_SIZE;

        // 从SDRAM读取数据到缓冲区
        memcpy(buffer, sdram_ptr + bytes_processed, chunk_size);

        // 写入QSPI Flash
        qspi_flash_data_write(flash_addr + bytes_processed, buffer, chunk_size);

        // 计算SDRAM数据的CRC
        sdram_crc = crc32(buffer, chunk_size, sdram_crc);

        bytes_processed += chunk_size;
    }

    sdram_crc ^= 0xFFFFFFFF;

    // 验证QSPI Flash中的数据
    bytes_processed = 0;
    while (bytes_processed < data_length) {
        chunk_size = (data_length - bytes_processed) < BUFFER_SIZE ? (data_length - bytes_processed) : BUFFER_SIZE;

        // 从QSPI Flash读取数据到验证缓冲区
        qspi_flash_data_read(flash_addr + bytes_processed, verify_buffer, chunk_size);

        // 计算QSPI Flash数据的CRC
        flash_crc = crc32(verify_buffer, chunk_size, flash_crc);

        bytes_processed += chunk_size;
    }

    flash_crc ^= 0xFFFFFFFF;

    printf("SDRAM CRC: 0x%08X\n", sdram_crc);
    printf("Flash CRC: 0x%08X\n", flash_crc);

    if (sdram_crc == flash_crc) {
        printf("CRC check passed. Data is consistent success.\n");
    } else {
        printf("CRC check failed. Data is inconsistent.\n");
    }
}




void copy_file_to_qspi_flash(uint8_t use_flash_flag) {
    FIL file;        // 文件对象
    FRESULT res;     // 文件操作结果
    UINT br;         // 读取的字节数
    uint32_t file_size;
    uint8_t buffer[BUFFER_SIZE];  // 缓冲区
    uint8_t verify_buffer[BUFFER_SIZE];  // 校验缓冲区
    uint32_t bytes_processed = 0;
    uint32_t chunk_size;
    uint32_t file_crc = 0xFFFFFFFF;
    uint32_t flash_crc = 0xFFFFFFFF;
    uint32_t flash_addr = 0;  // QSPI Flash的起始地址

    // 打开文件
    res = f_open(&file, "1:resource.bin", FA_READ);
    if (res != FR_OK) {
        printf("Failed to open file: %d\r\n", res);
        return;
    }

    // 获取文件大小
    file_size = f_size(&file);
    printf("file_size is %d\r\n", file_size);

    // 擦除对应的QSPI Flash扇区
    for (uint32_t address = 0; address <= file_size; address += PAGE_SIZE) {
		if(use_flash_flag==1)
		{
		  qspi_flash_erase(flash_addr + address);
		}else{
		  spiflash_sector_erase(flash_addr + address);
		}	
        printf("erase address: %08x\r\n", flash_addr + address);
    }

    // 从文件系统读取并写入到QSPI Flash
    while (bytes_processed < file_size) {
        chunk_size = (file_size - bytes_processed) < BUFFER_SIZE ? (file_size - bytes_processed) : BUFFER_SIZE;

        // 从文件读取数据到缓冲区
        res = f_read(&file, buffer, chunk_size, &br);
        if (res != FR_OK || br == 0) {
            printf("Failed to read file: %d\r\n", res);
            f_close(&file);
            return;
        }

		
		if(use_flash_flag==1){
		
		  // 写入QSPI Flash
        qspi_flash_data_write(flash_addr + bytes_processed, buffer, br);
		}else{
		 spiflash_write(buffer,flash_addr + bytes_processed,br);
		}
      
		
//         
        // 计算文件数据的CRC
        file_crc = crc32(buffer, br, file_crc);

        bytes_processed += br;
    }
	

    file_crc ^= 0xFFFFFFFF;

    // 验证QSPI Flash中的数据
    bytes_processed = 0;
    while (bytes_processed < file_size) {
        chunk_size = (file_size - bytes_processed) < BUFFER_SIZE ? (file_size - bytes_processed) : BUFFER_SIZE;

        // 从QSPI Flash读取数据到验证缓冲区
		
		if(use_flash_flag==1){
		  qspi_flash_data_read(flash_addr + bytes_processed, verify_buffer, chunk_size);
		}else{
		  spiflash_read(verify_buffer,flash_addr + bytes_processed,chunk_size);
		}	  
        // 计算QSPI Flash数据的CRC
        flash_crc = crc32(verify_buffer, chunk_size, flash_crc);

        bytes_processed += chunk_size;
    }

    flash_crc ^= 0xFFFFFFFF;

    printf("File CRC: 0x%08X\r\n", file_crc);
    printf("Flash CRC: 0x%08X\r\n", flash_crc);

    if (file_crc == flash_crc) {
        printf("CRC check passed. Data is consistent success.\r\n");
    } else {
        printf("CRC check failed. Data is inconsistent.\r\n");
    }

    // 关闭文件
    f_close(&file);
}






void application(void)
{
   all_bsp_init();
	
   /************* 两种方式做文件写入qspi flash ****************/
	
   /* 拷贝文件至qspi falsh */
	copy_file_to_qspi_flash(2);  //1 qspi   2:spi
		
   /* 拷贝文件至sdram 再拷贝到 qspi falsh */
// copy_file_to_sdram();
	
// copy_sdram_to_qspi_flash(0xC0000000,0,get_file_size());
  /************************************************************/	
	
	
}



