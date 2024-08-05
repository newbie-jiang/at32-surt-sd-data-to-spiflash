/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "bsp_qspi_flash.h"
#include "my_flash.h"

/* Definitions of physical drive number for each drive */

#define DEV_CHIP_FLASH		0	

#define DEV_SPI		        1	
   
 	
#define spi_file_sys_base_address		0x00600000	         /* base address 6M */
#define chip_file_sys_base_address		0x08080000	         /* base address 512K   */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) 
	{

		case DEV_CHIP_FLASH :
			 result = 0;
	         stat = (DSTATUS)result;
	         return stat;
			
		case DEV_SPI :
			 result = 0;
			 stat = (DSTATUS)result;
			 return stat;			
       
	}
	  
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
    uint16_t i;
	switch (pdrv) {
		
		case DEV_CHIP_FLASH :

			result = 0;
			stat = (DSTATUS)result;
			return stat;
		   
			
		case DEV_SPI :
			spiflash_init();
			i=500;
			while(--i);	
			spiflash_wakeup();		
		    result = 0;
			stat = (DSTATUS)result;
			return stat;
		
	}

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	

	case DEV_CHIP_FLASH :
		
	   for(;count>0;count--)
		{
			my_flash_read(buff,(chip_file_sys_base_address+sector*2048),2048);
			sector++;
			buff+=2048;
		}	
		 result = 0;
		 res = (DRESULT)result;
		 return res;
		
	case DEV_SPI :
		for(;count>0;count--)
		{
			spiflash_read(buff,(spi_file_sys_base_address+sector*512),512);
			sector++;
			printf("spi=%#x\n",*buff);
			buff+=512;
		}				
		 result = 0;
	     res = (DRESULT)result;
		 return res;       	
	}
	
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {	

	case DEV_CHIP_FLASH :
	
		for(;count>0;count--)
		{	
			flash_sector_erase(chip_file_sys_base_address+sector*2048);
			my_flash_write((u8*)buff,(chip_file_sys_base_address+sector*2048),2048);
			sector++;
			buff+=2048;
		}
	
			result = 0;
			res = (DRESULT)result;
			return res;
		
	case DEV_SPI :
		for(;count>0;count--)
		{										    
			spiflash_write((u8*)buff,(spi_file_sys_base_address+sector*512),512);
			sector++;
			buff+=512;
		}
      
			result = 0;
			res = (DRESULT)result;
			return res;
  }	
	  

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	

	case DEV_CHIP_FLASH :

			switch (cmd) 
			{
				case GET_SECTOR_COUNT:

//					  *(DWORD*)buff = 256; //512k
				      *(DWORD*)buff = 256; //256 
					  result = 0;
				
					break;
			    case GET_SECTOR_SIZE :
					  *(WORD*)buff = 2048; 
					  result = 0;
					break;
				case GET_BLOCK_SIZE :
					  *(WORD*)buff = 1;
					  result = 0;
					break;        
          }		
          
				result = 0;
				res = (DRESULT)result;
				return res; 
		  
		 case DEV_SPI :
			switch (cmd) 
			{
				case GET_SECTOR_COUNT:
    	//         	  *(DWORD*)buff =15360;//7.86M
					  *(DWORD*)buff =3840;
					  result = 0;
				
					break;
			    case GET_SECTOR_SIZE :
					  *(WORD*)buff = 512; 
					  result = 0;
					break;
				case GET_BLOCK_SIZE :
					  *(WORD*)buff = 2;
					  result = 0;
					break;        
          }
				result = 0;
				res = (DRESULT)result;
				return res; 			
          
	}
	

	return RES_PARERR;
}

