/**
 * @file bsp_xpt2046.c
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-11 14:25:38
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "bsp_xpt2046.h"
#include "bsp_lcd_xmc.h"
#include "string.h"
/* private define -------------------------------------------------------*/
#define  XPT2046_CHANNEL_X    0x90            //通道Y+的选择控制字
#define  XPT2046_CHANNEL_Y    0xd0            //通道X+的选择控制字
//校准触摸屏时触摸坐标的AD值相差门限 
#define XPT2046_THRESHOLD_CalDiff   2

//触屏信号有效电平
#define  XPT2046_EXTI_ActiveLevel   0

// 校验标志
#define XPT2046_TOUCH_CALIBRATE_FLAG        (0xA5)
#define XPT2046_TOUCH_CALIBRATE_FLAG_ADDR   (0x80FF800)
#define XPT2046_TOUCH_CALIBRATE_PARAM_ADDR  (0x80FF800 + 4)
// 打印校准参数
#define _XPT_PRINTF_CALIBRATE_PARAM     0
// delay
#define _XPT_DELAY_NUMB    (5) // 5us
// 
#define _XPT_SPI_PIN_NUMB  (5)

/* private typedef ------------------------------------------------------*/
typedef enum _xpt_spi_pin_e
{
    XPT_SPI_PIN_CS = 0,
    XPT_SPI_PIN_CLK,
    XPT_SPI_PIN_MOSI,
    XPT_SPI_PIN_MISO,
    XPT_SPI_PIN_INT
}_xpt_spi_pin_et;

typedef struct {        
    
    int16_t x;      //记录最新的触摸参数值
    int16_t y;

    /*用于记录连续触摸时(长按)的上一次触摸位置*/
    int16_t pre_x;
    int16_t pre_y;

} strType_XPT2046_Coordinate;

typedef struct
{
    float An;
    float Bn;
    float Cn;
    float Dn;
    float En;
    float Fn;
    float Divider;
	
} strType_XPT2046_Calibration;

typedef struct
{
    float dX_X;
    float dX_Y;
    float dX;
    float dY_X;
    float dY_Y;
    float dY;
} strType_XPT2046_TouchPara;
/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static const bsp_gpio_st _xpt_gpio_info[_XPT_SPI_PIN_NUMB] = {
    [XPT_SPI_PIN_CS] = {.port = GPIOB , .pin = GPIO_PINS_7 , .crm_clk = CRM_GPIOB_PERIPH_CLOCK , .state = TRUE},
    [XPT_SPI_PIN_CLK] = {.port = GPIOB , .pin = GPIO_PINS_3 , .crm_clk = CRM_GPIOB_PERIPH_CLOCK , .state = TRUE},
    [XPT_SPI_PIN_MOSI] = {.port = GPIOB , .pin = GPIO_PINS_5 , .crm_clk = CRM_GPIOB_PERIPH_CLOCK , .state = TRUE},
    [XPT_SPI_PIN_MISO] = {.port = GPIOB , .pin = GPIO_PINS_4 , .crm_clk = CRM_GPIOB_PERIPH_CLOCK , .state = TRUE},
    [XPT_SPI_PIN_INT] = {.port = GPIOD , .pin = GPIO_PINS_13 , .crm_clk = CRM_GPIOD_PERIPH_CLOCK , .state = TRUE},
};



static volatile uint8_t ucXPT2046_TouchFlag = 0;
static strType_XPT2046_TouchPara strXPT2046_TouchPara = {
    -0.082808,    0.000799,  325.385437,    0.008188,    0.128538,  -33.394218,
};

// 
static tp_dev_t* tp_dev_xpt2046 = NULL;

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _xpt_pin_set(uint8_t pin_numb)
{
    gpio_bits_set(_xpt_gpio_info[pin_numb].port, _xpt_gpio_info[pin_numb].pin);
}

static void _xpt_pin_reset(uint8_t pin_numb)
{
    gpio_bits_reset(_xpt_gpio_info[pin_numb].port, _xpt_gpio_info[pin_numb].pin);
}

static uint8_t _xpt_pin_read(uint8_t pin_numb)
{
    gpio_input_data_bit_read(_xpt_gpio_info[pin_numb].port, _xpt_gpio_info[pin_numb].pin);
}


static uint8_t XPT2046_EXTI_Read(void)
{
    gpio_input_data_bit_read(_xpt_gpio_info[XPT_SPI_PIN_INT].port, _xpt_gpio_info[XPT_SPI_PIN_INT].pin);
}


static void _xpt_write_cmd(uint8_t ucCmd )
{
    uint8_t i;

    _xpt_pin_reset(XPT_SPI_PIN_CS);

    _xpt_pin_reset(XPT_SPI_PIN_MOSI);
    _xpt_pin_reset(XPT_SPI_PIN_CLK);

    for ( i = 0; i < 8; i ++ ) {
    ( ( ucCmd >> ( 7 - i ) ) & 0x01 ) ? _xpt_pin_set(XPT_SPI_PIN_MOSI) : _xpt_pin_reset(XPT_SPI_PIN_MOSI);
        delay_us(_XPT_DELAY_NUMB);
        _xpt_pin_set(XPT_SPI_PIN_CLK);
        delay_us(_XPT_DELAY_NUMB);
        _xpt_pin_reset(XPT_SPI_PIN_CLK);
    }

}

static uint16_t _xpt_read_cmd(void)
{
    uint8_t i;
    uint16_t usBuf=0, usTemp;

    _xpt_pin_reset(XPT_SPI_PIN_CS);

    _xpt_pin_reset(XPT_SPI_PIN_MOSI);
    _xpt_pin_set(XPT_SPI_PIN_CLK);

    for ( i=0; i<12; i++ ) {
        _xpt_pin_reset(XPT_SPI_PIN_CLK);
        usTemp = _xpt_pin_read(XPT_SPI_PIN_MISO);
        usBuf |= usTemp << ( 11 - i );
        _xpt_pin_set(XPT_SPI_PIN_CLK);
    }

    _xpt_pin_set(XPT_SPI_PIN_CS);

    return usBuf;
}


static uint16_t _xpt_read_adc(uint16_t ucChannel)
{
    _xpt_write_cmd(ucChannel);
    return _xpt_read_cmd();
}


static void _xpt_read_adc_xy(int16_t *sX_Ad , int16_t *sY_Ad)
{
    int16_t sX_Ad_Temp, sY_Ad_Temp;

    sX_Ad_Temp = _xpt_read_adc ( XPT2046_CHANNEL_X );
	
    delay_us(1);
    sY_Ad_Temp =  _xpt_read_adc ( XPT2046_CHANNEL_Y );

    * sX_Ad = sX_Ad_Temp;
    * sY_Ad = sY_Ad_Temp;
}

static uint8_t _xpt_read_adc_smooth_xy(strType_XPT2046_Coordinate *pScreenCoordinate)
{
    uint8_t ucCount = 0;
	
	int16_t sAD_X, sAD_Y;
	int16_t sBufferArray [ 2 ] [ 9 ] = { { 0 }, { 0 } };  //坐标X和Y进行9次采样

	int32_t lAverage  [ 3 ], lDifference [ 3 ];

    do
	{		   
		_xpt_read_adc_xy ( & sAD_X, & sAD_Y );
		
		sBufferArray [ 0 ] [ ucCount ] = sAD_X;  
		sBufferArray [ 1 ] [ ucCount ] = sAD_Y;
		
		ucCount ++; 
    
	} while ( ( XPT2046_EXTI_Read() == XPT2046_EXTI_ActiveLevel ) && ( ucCount < 9 ) ); 	//用户点击触摸屏时即TP_INT_IN信号为低 并且 ucCount<9*/
    
    /*如果触笔弹起*/
	if ( XPT2046_EXTI_Read() != XPT2046_EXTI_ActiveLevel ){
		ucXPT2046_TouchFlag = 0;			//触摸中断标志复位		
    }
    
    /* 如果成功采样9次,进行滤波 */ 
	if ( ucCount == 9 )   								
	{  
		/* 为减少运算量,分别分3组取平均值 */
		lAverage  [ 0 ] = ( sBufferArray [ 0 ] [ 0 ] + sBufferArray [ 0 ] [ 1 ] + sBufferArray [ 0 ] [ 2 ] ) / 3;
		lAverage  [ 1 ] = ( sBufferArray [ 0 ] [ 3 ] + sBufferArray [ 0 ] [ 4 ] + sBufferArray [ 0 ] [ 5 ] ) / 3;
		lAverage  [ 2 ] = ( sBufferArray [ 0 ] [ 6 ] + sBufferArray [ 0 ] [ 7 ] + sBufferArray [ 0 ] [ 8 ] ) / 3;
		
		/* 计算3组数据的差值 */
		lDifference [ 0 ] = lAverage  [ 0 ]-lAverage  [ 1 ];
		lDifference [ 1 ] = lAverage  [ 1 ]-lAverage  [ 2 ];
		lDifference [ 2 ] = lAverage  [ 2 ]-lAverage  [ 0 ];
		
		/* 对上述差值取绝对值 */
		lDifference [ 0 ] = lDifference [ 0 ]>0?lDifference [ 0 ]: ( -lDifference [ 0 ] );
		lDifference [ 1 ] = lDifference [ 1 ]>0?lDifference [ 1 ]: ( -lDifference [ 1 ] );
		lDifference [ 2 ] = lDifference [ 2 ]>0?lDifference [ 2 ]: ( -lDifference [ 2 ] );
		
		
		/* 判断绝对差值是否都超过差值门限，如果这3个绝对差值都超过门限值，则判定这次采样点为野点,抛弃采样点，差值门限取为2 */
		if (  lDifference [ 0 ] > XPT2046_THRESHOLD_CalDiff  &&  lDifference [ 1 ] > XPT2046_THRESHOLD_CalDiff  &&  lDifference [ 2 ] > XPT2046_THRESHOLD_CalDiff  ) 
			return 0;
		
		
		/* 计算它们的平均值，同时赋值给strScreenCoordinate */ 
		if ( lDifference [ 0 ] < lDifference [ 1 ] )
		{
			if ( lDifference [ 2 ] < lDifference [ 0 ] ) 
				pScreenCoordinate ->x = ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
			else 
				pScreenCoordinate ->x = ( lAverage  [ 0 ] + lAverage  [ 1 ] ) / 2;	
		}
		
		else if ( lDifference [ 2 ] < lDifference [ 1 ] ) 
			pScreenCoordinate -> x = ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
		
		else 
			pScreenCoordinate ->x = ( lAverage  [ 1 ] + lAverage  [ 2 ] ) / 2;
		
		
		/* 同上，计算Y的平均值 */
		lAverage  [ 0 ] = ( sBufferArray [ 1 ] [ 0 ] + sBufferArray [ 1 ] [ 1 ] + sBufferArray [ 1 ] [ 2 ] ) / 3;
		lAverage  [ 1 ] = ( sBufferArray [ 1 ] [ 3 ] + sBufferArray [ 1 ] [ 4 ] + sBufferArray [ 1 ] [ 5 ] ) / 3;
		lAverage  [ 2 ] = ( sBufferArray [ 1 ] [ 6 ] + sBufferArray [ 1 ] [ 7 ] + sBufferArray [ 1 ] [ 8 ] ) / 3;
		
		lDifference [ 0 ] = lAverage  [ 0 ] - lAverage  [ 1 ];
		lDifference [ 1 ] = lAverage  [ 1 ] - lAverage  [ 2 ];
		lDifference [ 2 ] = lAverage  [ 2 ] - lAverage  [ 0 ];
		
		/* 取绝对值 */
		lDifference [ 0 ] = lDifference [ 0 ] > 0 ? lDifference [ 0 ] : ( - lDifference [ 0 ] );
		lDifference [ 1 ] = lDifference [ 1 ] > 0 ? lDifference [ 1 ] : ( - lDifference [ 1 ] );
		lDifference [ 2 ] = lDifference [ 2 ] > 0 ? lDifference [ 2 ] : ( - lDifference [ 2 ] );
		
		
		if ( lDifference [ 0 ] > XPT2046_THRESHOLD_CalDiff && lDifference [ 1 ] > XPT2046_THRESHOLD_CalDiff && lDifference [ 2 ] > XPT2046_THRESHOLD_CalDiff ) 
			return 0;
		
		if ( lDifference [ 0 ] < lDifference [ 1 ] )
		{
			if ( lDifference [ 2 ] < lDifference [ 0 ] ) 
				pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
			else 
				pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 1 ] ) / 2;	
		}
		else if ( lDifference [ 2 ] < lDifference [ 1 ] ) 
			pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
		else
			pScreenCoordinate ->y =  ( lAverage  [ 1 ] + lAverage  [ 2 ] ) / 2;
				
		return 1;		
	}
	else if ( ucCount > 1 )
	{
		pScreenCoordinate ->x = sBufferArray [ 0 ] [ 0 ];
		pScreenCoordinate ->y = sBufferArray [ 1 ] [ 0 ];
		return 0;
	}  	
	return 0;
}


/**
  * @brief  计算 XPT2046 触摸坐标校正系数（注意：只有在LCD和触摸屏间的误差角度非常小时,才能运用下面公式）
  * @param  pDisplayCoordinate ：屏幕人为显示的已知坐标
  * @param  pstrScreenSample ：对已知坐标点触摸时 XPT2046 产生的坐标
  * @param  pCalibrationFactor ：根据人为设定坐标和采样回来的坐标计算出来的屏幕触摸校正系数
  * @retval 计算状态
	*   该返回值为以下值之一：
  *     @arg 1 :计算成功
  *     @arg 0 :计算失败
  */
static uint8_t XPT2046_Calculate_CalibrationFactor ( strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_Coordinate * pScreenSample, strType_XPT2046_Calibration * pCalibrationFactor )
{
	uint8_t ucRet = 1;

	/* K＝ ( X0－X2 )  ( Y1－Y2 )－ ( X1－X2 )  ( Y0－Y2 ) */
	pCalibrationFactor -> Divider =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
									                 ( ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) ) ;
	
	
	if (  pCalibrationFactor -> Divider == 0  )
		ucRet = 0;

	else
	{
		/* A＝ (  ( XD0－XD2 )  ( Y1－Y2 )－ ( XD1－XD2 )  ( Y0－Y2 ) )／K	*/
		pCalibrationFactor -> An =  ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) );
		
		/* B＝ (  ( X0－X2 )  ( XD1－XD2 )－ ( XD0－XD2 )  ( X1－X2 ) )／K	*/
		pCalibrationFactor -> Bn =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) ) - 
								                ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) );
		
		/* C＝ ( Y0 ( X2XD1－X1XD2 )+Y1 ( X0XD2－X2XD0 )+Y2 ( X1XD0－X0XD1 ) )／K */
		pCalibrationFactor -> Cn =  ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .x - pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .x ) * pScreenSample [ 0 ] .y +
								                ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .x - pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .x ) * pScreenSample [ 1 ] .y +
								                ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .x - pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .x ) * pScreenSample [ 2 ] .y ;
		
		/* D＝ (  ( YD0－YD2 )  ( Y1－Y2 )－ ( YD1－YD2 )  ( Y0－Y2 ) )／K	*/
		pCalibrationFactor -> Dn =  ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) ) ;
		
		/* E＝ (  ( X0－X2 )  ( YD1－YD2 )－ ( YD0－YD2 )  ( X1－X2 ) )／K	*/
		pCalibrationFactor -> En =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) ) ;
		
		
		/* F＝ ( Y0 ( X2YD1－X1YD2 )+Y1 ( X0YD2－X2YD0 )+Y2 ( X1YD0－X0YD1 ) )／K */
		pCalibrationFactor -> Fn =  ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .y - pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .y ) * pScreenSample [ 0 ] .y +
								                ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .y - pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .y ) * pScreenSample [ 1 ] .y +
								                ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .y - pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .y ) * pScreenSample [ 2 ] .y;
			
	}
	
	
	return ucRet;
	
	
}

static void _lcd_draw_cross(uint16_t usX, uint16_t usY, uint16_t color)
{
    bsp_lcd_draw_line(usX-10,usY,usX+10,usY ,color);
	bsp_lcd_draw_line(usX, usY - 10, usX, usY+10 ,color);
}

// 
static uint8_t _xpt2046_touch_calibrate(void)
{
    uint8_t i;
    uint16_t usTest_x = 0, usTest_y = 0, usGap_x = 0, usGap_y = 0;
    char * pStr = 0;

    strType_XPT2046_Coordinate strCrossCoordinate[4], strScreenSample[4];
    strType_XPT2046_Calibration CalibrationFactor;

    /* 设定“十”字交叉点的坐标 */ 
    strCrossCoordinate [0].x = LCD_XMC_WIDTH >> 2;
    strCrossCoordinate[0].y = LCD_XMC_HEIGHT >> 2;
    
    strCrossCoordinate[1].x = strCrossCoordinate[0].x;
    strCrossCoordinate[1].y = ( LCD_XMC_HEIGHT * 3 ) >> 2;
    
    strCrossCoordinate[2].x = ( LCD_XMC_WIDTH * 3 ) >> 2;
    strCrossCoordinate[2].y = strCrossCoordinate[1].y;
    
    strCrossCoordinate[3].x = strCrossCoordinate[2].x;
    strCrossCoordinate[3].y = strCrossCoordinate[0].y;
    
    for ( i = 0; i < 4; i ++ )
    {
        bsp_lcd_clear(0xFFFF);
        delay_ms(1500);

        //printf("Touch Calibrate ......  x: %d y: %d\r\n" , strCrossCoordinate[i] .x, strCrossCoordinate[i].y);
        
        _lcd_draw_cross(strCrossCoordinate[i].x, strCrossCoordinate[i].y , 0xF800);  //显示校正用的“十”字

        //读取XPT2046数据到变量pCoordinate，当ptr为空时表示没有触点被按下
        while ( ! _xpt_read_adc_smooth_xy ( & strScreenSample [i] ) ); 
    }

    //用原始参数计算出 原始参数与坐标的转换系数
    XPT2046_Calculate_CalibrationFactor ( strCrossCoordinate, strScreenSample, & CalibrationFactor ) ;

    if ( CalibrationFactor.Divider == 0 ) goto Failure;

    //取一个点计算X值	 
    usTest_x = ( ( CalibrationFactor.An * strScreenSample[3].x ) + ( CalibrationFactor.Bn * strScreenSample[3].y ) + CalibrationFactor.Cn ) / CalibrationFactor.Divider;
    //取一个点计算Y值		
    usTest_y = ( ( CalibrationFactor.Dn * strScreenSample[3].x ) + ( CalibrationFactor.En * strScreenSample[3].y ) + CalibrationFactor.Fn ) / CalibrationFactor.Divider;
    //实际X坐标与计算坐标的绝对差
    usGap_x = ( usTest_x > strCrossCoordinate[3].x ) ? ( usTest_x - strCrossCoordinate[3].x ) : ( strCrossCoordinate[3].x - usTest_x );
    //实际Y坐标与计算坐标的绝对差
    usGap_y = ( usTest_y > strCrossCoordinate[3].y ) ? ( usTest_y - strCrossCoordinate[3].y ) : ( strCrossCoordinate[3].y - usTest_y );
    
    if ( ( usGap_x > 15 ) || ( usGap_y > 15 ) ) goto Failure;       //可以通过修改这两个值的大小来调整精度

    /* 校准系数为全局变量 */ 
    strXPT2046_TouchPara.dX_X = ( CalibrationFactor.An * 1.0 ) / CalibrationFactor.Divider;
    strXPT2046_TouchPara.dX_Y = ( CalibrationFactor.Bn * 1.0 ) / CalibrationFactor.Divider;
    strXPT2046_TouchPara.dX   = ( CalibrationFactor.Cn * 1.0 ) / CalibrationFactor.Divider;
    strXPT2046_TouchPara.dY_X = ( CalibrationFactor.Dn * 1.0 ) / CalibrationFactor.Divider;
    strXPT2046_TouchPara.dY_Y = ( CalibrationFactor.En * 1.0 ) / CalibrationFactor.Divider;
    strXPT2046_TouchPara.dY   = ( CalibrationFactor.Fn * 1.0 ) / CalibrationFactor.Divider;

#if _XPT_PRINTF_CALIBRATE_PARAM
    float * ulHeadAddres ;
    /* 打印校校准系数 */ 
    printf ( "显示模式【x】校准系数如下: \r\n");
    ulHeadAddres = ( float* ) ( & strXPT2046_TouchPara);
    
    for ( i = 0; i < 6; i ++ )
    {					
        printf ( "%12f,", *ulHeadAddres++  );			
    }	
    printf("\r\n");
#endif

    bsp_lcd_clear(0xFFFF);
    //printf("Calibrate Succed!!!\r\n");
	return 1;    

Failure:
    bsp_lcd_clear(0xFFFF);
    //printf("Calibrate fail!!!\r\n");
    //printf("try again!!!\r\n");
    delay_ms(100);
    return 0;
}


static uint8_t _xpt2046_read_point_data(strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_TouchPara * pTouchPara)
{
    uint8_t ucRet = 1;           //若正常，则返回0
	
	strType_XPT2046_Coordinate strScreenCoordinate; 

    if ( _xpt_read_adc_smooth_xy ( & strScreenCoordinate ) )
    {    
        pDisplayCoordinate ->x = ( ( pTouchPara->dX_X * strScreenCoordinate.x ) + ( pTouchPara->dX_Y * strScreenCoordinate.y ) + pTouchPara->dX );        
        pDisplayCoordinate ->y = ( ( pTouchPara->dY_X * strScreenCoordinate.x ) + ( pTouchPara->dY_Y * strScreenCoordinate.y ) + pTouchPara->dY );

    }
        
    else ucRet = 0;            //如果获取的触点信息有误，则返回0
        
    return ucRet;
}

static bool _xpt2046_read_point(void)
{
    if(NULL == tp_dev_xpt2046){
        return false;
    }

    strType_XPT2046_Coordinate pDisplayCoordinate;
    uint8_t ret = _xpt2046_read_point_data(&pDisplayCoordinate , &strXPT2046_TouchPara);
    uint8_t state = XPT2046_EXTI_Read();

    if(XPT2046_EXTI_ActiveLevel != state){
        tp_dev_xpt2046->point.status = TP_STATE_RELEASED;
        return false;
    }

    if(0 == ret && tp_dev_xpt2046->point.status != TP_STATE_RELEASED){
        tp_dev_xpt2046->point.status = TP_STATE_HOLDE;
        return true;
    }
    tp_dev_xpt2046->point.x = pDisplayCoordinate.x;
    tp_dev_xpt2046->point.y = pDisplayCoordinate.y;
    //tp_dev_xpt2046->point.x = 480 - pDisplayCoordinate.y;
    //tp_dev_xpt2046->point.y = pDisplayCoordinate.x;
    tp_dev_xpt2046->point.status = tp_dev_xpt2046->point.status != TP_STATE_RELEASED ? TP_STATE_HOLDE : XPT2046_EXTI_Read();
#if TOUCH_LOGD_EN
    printf("touch: x = %d y = %d status = %d\r\n" , tp_dev_xpt2046->point.x , tp_dev_xpt2046->point.y , tp_dev_xpt2046->point.status);
#endif 

#if TOUCH_DRAW_POINT_EN 
    bsp_lcd_draw_point( tp_dev_xpt2046->point.x , tp_dev_xpt2046->point.y , 0xF800);
#endif

    return true;
}

/* global functions / API interface -------------------------------------*/
void bsp_xpt2046_init(tp_dev_t *dev)
{
    gpio_init_type gpio_init_struct;

    // cs
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins = _xpt_gpio_info[XPT_SPI_PIN_CS].pin;
    gpio_init(_xpt_gpio_info[XPT_SPI_PIN_CS].port, &gpio_init_struct);
    crm_periph_clock_enable( _xpt_gpio_info[XPT_SPI_PIN_CS].crm_clk , TRUE);
    _xpt_pin_set(XPT_SPI_PIN_CS);

    // clk
    gpio_init_struct.gpio_pins = _xpt_gpio_info[XPT_SPI_PIN_CLK].pin;
    gpio_init(_xpt_gpio_info[XPT_SPI_PIN_CLK].port, &gpio_init_struct);
    crm_periph_clock_enable( _xpt_gpio_info[XPT_SPI_PIN_CLK].crm_clk , TRUE);
    _xpt_pin_set(XPT_SPI_PIN_CLK);

    // MOSI
    gpio_init_struct.gpio_pins = _xpt_gpio_info[XPT_SPI_PIN_MOSI].pin;
    gpio_init(_xpt_gpio_info[XPT_SPI_PIN_MOSI].port, &gpio_init_struct);
    crm_periph_clock_enable( _xpt_gpio_info[XPT_SPI_PIN_MOSI].crm_clk , TRUE);
    _xpt_pin_set(XPT_SPI_PIN_MOSI);

    // MISO
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = _xpt_gpio_info[XPT_SPI_PIN_MISO].pin;
    gpio_init(_xpt_gpio_info[XPT_SPI_PIN_MISO].port, &gpio_init_struct);
    crm_periph_clock_enable( _xpt_gpio_info[XPT_SPI_PIN_MISO].crm_clk , TRUE);

		
    bsp_xpt2046_calibrate_or_get_touch_para_with_flash(0);

    if(NULL == dev){
        return ;
    }

    tp_dev_xpt2046 = dev;
    dev->read_point = _xpt2046_read_point;//函数指针指向read point函数
}

// 从FLASH中获取 或 重新校正触摸参数（校正后会写入到SPI FLASH中）
void bsp_xpt2046_calibrate_or_get_touch_para_with_flash(uint8_t forceCal)
{

    uint8_t para_flag=0;

    //memcpy(&para_flag , (volatile uint32_t*)XPT2046_TOUCH_CALIBRATE_FLAG_ADDR , 1);
		memcpy(&para_flag , (uint32_t*)XPT2046_TOUCH_CALIBRATE_FLAG_ADDR , 1);
   // printf("para_flag = 0x%X\r\n" , para_flag);
    if(para_flag != XPT2046_TOUCH_CALIBRATE_FLAG || forceCal){

        while (!_xpt2046_touch_calibrate());
        
        __disable_irq();
        flash_unlock();
        flash_status_type status = FLASH_OPERATE_DONE;
        status = flash_operation_wait_for(ERASE_TIMEOUT);
        if((status == FLASH_PROGRAM_ERROR) || (status == FLASH_EPP_ERROR)){
	
            flash_flag_clear(FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
        }
        else if(status == FLASH_OPERATE_TIMEOUT){
          //  printf("xpt2046 Calibrate fail!!! FLASH_OPERATE_TIMEOUT\r\n");
            goto ret_handle;
        }

        // erase
        status = flash_sector_erase(XPT2046_TOUCH_CALIBRATE_FLAG_ADDR);
        if(status != FLASH_OPERATE_DONE){
            //printf("xpt2046 Calibrate fail!!! erase FLASH_OPERATE_DONE\r\n");
            goto ret_handle;
        }

        //写入最新的触摸参数
        uint8_t* param = (uint8_t*)&strXPT2046_TouchPara;
        uint32_t addr = XPT2046_TOUCH_CALIBRATE_PARAM_ADDR;
        for(int k = 0 ; k < sizeof(strType_XPT2046_TouchPara) ; k++)
        {
            status = flash_byte_program(addr, param[k]);
            if(status != FLASH_OPERATE_DONE){
                //printf("xpt2046 Calibrate fail!!! write FLASH_OPERATE_DONE\r\n");
                goto ret_handle;
            }
            ++addr;
        }

        //写入触摸参数标志
        status = flash_byte_program(XPT2046_TOUCH_CALIBRATE_FLAG_ADDR, XPT2046_TOUCH_CALIBRATE_FLAG);
        if(status != FLASH_OPERATE_DONE){
           // printf("xpt2046 Calibrate fail!!! write FLASH_OPERATE_DONE\r\n");
			goto ret_handle;
		}

ret_handle:
        flash_lock();
        __enable_irq();

    }
    else{
        //memcpy(&strXPT2046_TouchPara , (volatile uint32_t*)XPT2046_TOUCH_CALIBRATE_PARAM_ADDR , sizeof(strType_XPT2046_TouchPara));
        memcpy(&strXPT2046_TouchPara , (uint32_t*)XPT2046_TOUCH_CALIBRATE_PARAM_ADDR , sizeof(strType_XPT2046_TouchPara));
    #if _XPT_PRINTF_CALIBRATE_PARAM
        float * ulHeadAddres ;
        /* 打印校校准系数 */ 
        printf ( "从FLASH里读取得的校准系数如下: para_flag = 0x%X  forceCal = %d \r\n" , para_flag , forceCal);
        ulHeadAddres = ( float* ) ( & strXPT2046_TouchPara);
        
        for ( int i = 0; i < 6; i ++ )
        {					
            printf ( "%12f,", *ulHeadAddres++  );			
        }	
        printf("\r\n");
    #endif
    }

}

/* end of file ----------------------------------------------------------*/


