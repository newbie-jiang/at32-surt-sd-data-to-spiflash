/**
  **************************************************************************
  * @file     at32f403a_407_board.c
  * @brief    set of firmware functions to manage leds and push-button.
  *           initialize delay function.
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f403a_407_board.h"
#include "HX711.h"
#include "infusion.h"
/** @addtogroup AT32F403A_407_board
  * @{
  */

/** @defgroup BOARD
  * @brief onboard periph driver
  * @{
  */

/* delay macros */
#define STEP_DELAY_MS                    50

extern void vPortSetupTimerInterrupt( void );

/* at-start led resouce array */
gpio_type *led_gpio_port[LED_NUM]        = {LED1_GPIO, LED2_GPIO, LED3_GPIO,LED4_GPIO,LED5_GPIO};
uint16_t led_gpio_pin[LED_NUM]           = {LED1_PIN, LED2_PIN, LED3_PIN,LED4_PIN,LED5_PIN};
crm_periph_clock_type led_gpio_crm_clk[LED_NUM] = {LED1_GPIO_CRM_CLK, LED2_GPIO_CRM_CLK, LED3_GPIO_CRM_CLK,LED4_GPIO_CRM_CLK,LED5_GPIO_CRM_CLK};

/* delay variable */
static __IO uint32_t fac_us;
static __IO uint32_t fac_ms;

/* support printf function, usemicrolib is unnecessary */
#if (__ARMCC_VERSION > 6000000)
  __asm (".global __use_no_semihosting\n\t");
  void _sys_exit(int x)
  {
    x = x;
  }
  /* __use_no_semihosting was requested, but _ttywrch was */
  void _ttywrch(int ch)
  {
    ch = ch;
  }
  FILE __stdout;
#else
 #ifdef __CC_ARM
#if 0
  #pragma import(__use_no_semihosting)
  struct __FILE
  {
    int handle;
  };
  FILE __stdout;
  void _sys_exit(int x)
  {
    x = x;
  }
  /* __use_no_semihosting was requested, but _ttywrch was */
  void _ttywrch(int ch)
  {
    ch = ch;
  }
	#endif
 #endif
 
#endif

#if defined (__GNUC__) && !defined (__clang__)
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

	
/**
  * @brief  retargets the c library printf function to the usart.
  * @param  none
  * @retval none
  */
PUTCHAR_PROTOTYPE
{
	
	#if 0
	usart_data_transmit(PRINT_UART, ch);
  while(usart_flag_get(PRINT_UART, USART_TDC_FLAG) == SET);
	#else
	while(usart_flag_get(PRINT_UART, USART_TDBE_FLAG) == RESET); //��usart��������Ϊ��
    usart_data_transmit(PRINT_UART, ch);
	#endif
	
	
  return ch;
}

#if defined (__GNUC__) && !defined (__clang__)
int _write(int fd, char *pbuffer, int size)
{
  for(int i = 0; i < size; i ++)
  {
    __io_putchar(*pbuffer++);
  }

  return size;
}
#endif

/**
  * @brief  initialize uart
  * @param  baudrate: uart baudrate
  * @retval none
  */
void uart_print_init(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

#if defined (__GNUC__) && !defined (__clang__)
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

  /* enable the uart and gpio clock */
  crm_periph_clock_enable(PRINT_UART_CRM_CLK, TRUE);
  crm_periph_clock_enable(PRINT_UART_TX_GPIO_CRM_CLK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the uart tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = PRINT_UART_TX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(PRINT_UART_TX_GPIO, &gpio_init_struct);


  /* configure uart param */
  usart_init(PRINT_UART, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(PRINT_UART, TRUE);
  usart_enable(PRINT_UART, TRUE);
  
}





/**
  * @brief  configure button gpio
  * @param  button: specifies the button to be configured.
  * @retval none
  */
void at32_button_init(void)
{
  gpio_init_type gpio_init_struct;

  /* enable the button clock */
  crm_periph_clock_enable(USER_BUTTON_CRM_CLK, TRUE);

  /* set default parameter */
  gpio_default_para_init(&gpio_init_struct);

  /* configure button pin as input with pull-up/pull-down */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = USER_BUTTON_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
  gpio_init(USER_BUTTON_PORT, &gpio_init_struct);
}

/**
  * @brief  returns the selected button state
  * @param  none
  * @retval the button gpio pin value
  */
uint8_t at32_button_state(void)
{
  return gpio_input_data_bit_read(USER_BUTTON_PORT, USER_BUTTON_PIN);
}

/**
  * @brief  returns which button have press down
  * @param  none
  * @retval the button have press down
  */
button_type at32_button_press()
{
  static uint8_t pressed = 1;
  /* get button state in at_start board */
  if((pressed == 1) && (at32_button_state() != RESET))
  {
    /* debounce */
    pressed = 0;
    delay_ms(10);
    if(at32_button_state() != RESET)
      return USER_BUTTON;
  }
  else if(at32_button_state() == RESET)
  {
    pressed = 1;
  }
  return NO_BUTTON;
}


/**
  * @brief  turns selected led on.
  * @param  led: specifies the led to be set on.
  *   this parameter can be one of following parameters:
  *     @arg LED2
  *     @arg LED3
  *     @arg LED1
  * @retval none
  */
void led_on(led_type led)
{
  if(led > (LED_NUM - 1))
    return;
	if(led!=ARM_RLED && led!=AC_LED && led!=BAT_LED )
	{
		if(~led_gpio_pin[led])
		led_gpio_port[led]->scr = led_gpio_pin[led];
	}
	else
	{
		if(led_gpio_pin[led])
		led_gpio_port[led]->clr = led_gpio_pin[led];
	}
 
}

/**
  * @brief  turns selected led off.
  * @param  led: specifies the led to be set off.
  *   this parameter can be one of following parameters:
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4
  * @retval none
  */
void led_off(led_type led)
{
  if(led > (LED_NUM - 1))
    return;
	
	if(led!=ARM_RLED && led!=AC_LED && led!=BAT_LED)
	{
		if(led_gpio_pin[led])
		led_gpio_port[led]->clr = led_gpio_pin[led];
	}
	else
	{
		if(~led_gpio_pin[led])
		led_gpio_port[led]->scr = led_gpio_pin[led];
	}
}

/**
  * @brief  turns selected led toggle.
  * @param  led: specifies the led to be set off.
  *   this parameter can be one of following parameters:
  *     @arg LED2
  *     @arg LED3
  *     @arg LED1
  * @retval none
  */
void led_turn(led_type led)
{
  if(led > (LED_NUM - 1))
    return;
	if(led!=ARM_RLED && led!=AC_LED && led!=BAT_LED)
	{
		if(led_gpio_pin[led])
			led_gpio_port[led]->odt ^= led_gpio_pin[led];
	}
	else
	{
		if(~led_gpio_pin[led])
			led_gpio_port[led]->odt ^= led_gpio_pin[led];
	}
}



/**
  * @brief  initialize delay function
  * @param  none
  * @retval none
  */
void delay_init()
{
  /* configure systick */
  systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
  fac_us = system_core_clock / (1000000U);
  fac_ms = fac_us * (1000U);
}




/**
  * @brief  inserts a delay time.
  * @param  nus: specifies the delay time length, in microsecond.
  * @retval none
  */
void delay_us(uint32_t nus)
{
	#if 0
  uint32_t temp = 0;
  SysTick->LOAD = (uint32_t)(nus * fac_us);
  SysTick->VAL = 0x00;
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;
  do
  {
    temp = SysTick->CTRL;
  }while((temp & 0x01) && !(temp & (1 << 16)));

  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
  SysTick->VAL = 0x00;
	#else
	u32 ticks;
	u32 told,tnow,reload,tcnt=0;
	if((0x0001&(SysTick->CTRL)) == 0)//��ʱ��δ����
		vPortSetupTimerInterrupt(); //��ʼ����ʱ��

	reload = SysTick->LOAD;//����ʱ��ֵ
	ticks = nus * fac_us;
	//��ȡ��װ�ؼĴ���ֵ
	vTaskSuspendAll();//��IoS���ȣ���ֹ���us��ʱ
	told=SysTick->VAL;//��ȡ��ǰ��ֵ�Ĵ���ֵ (��ʼʱ��ֵ)
	
	while(1)
	{
 			tnow=SysTick->VAL; //��ȡ��ǰ��ֵ�Ĵ���ֵ
		
			if(tnow!=told) //��ǰֵ�����ڿ�ʼֵ˵�����ڼ���
			{
					if(tnow<told) //��ǰֵС�ڿ�ʼ��ֵ��˵��δ�Ƶ�
						tcnt+=told-tnow; //����ֵ=��ʼֵ-��ǰֵ

					else //��ǰֵ���ӿ�ʼ��ֵ��˵���ѼƵ�6�����¼ƽ�
						tcnt+=reload-tnow+told;//����ֵ-��װ��ֵ- ��ǰֵ+��ʼֵ//�Ѵӿ�ʼֵ�Ƶ�
					 
					told=tnow;//���¿�ʼֵ
					if(tcnt>=ticks)break; //ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�
			}
	}
			xTaskResumeAll();//�ָ�0S����
	
	
	
	
	
	#endif
}


/**
  * @brief  inserts a delay time.
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval none
  */
void delay_ms(uint16_t nms)
{
	/*20240126
	�ر�˵��:
		1.��#if 1 ʱ,����Ļ�йص���������main�����²��Ῠ����
			���ǻ���vtaskdelay��ͻ�������������ʱ������
		2.��#if 0 ʱ������Ļ�йص�����������main�����»Ῠ����
			��������vtaskdelay��ͻ����
	*/
	#if 0
  uint32_t temp = 0;
  while(nms)
  {
    if(nms > STEP_DELAY_MS)  //�������50ms
    {
      SysTick->LOAD = (uint32_t)(STEP_DELAY_MS * fac_ms); //�Զ���װ��ֵ
      nms -= STEP_DELAY_MS;
    }
    else
    {
      SysTick->LOAD = (uint32_t)(nms * fac_ms);
      nms = 0;
    }
    SysTick->VAL = 0x00; // �����ʱ����ֵ
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //SysTickʹ�ܣ�ʹ�ܺ�ʱ����ʼ����
    do
    {
      temp = SysTick->CTRL;
    }while((temp & 0x01) && !(temp & (1 << 16))); //�ж��Ƿ����0������0ʱCTRL�ĵ�16λ����1����ȡ����Զ���0

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; //�رյδ�ʱ��
    SysTick->VAL = 0x00; //�����������ֵ
  }
	#else
	u32 ticks;
	u32 told,tnow,reload,tcnt=0;
	
	if((0x0001&(SysTick->CTRL)) == 0)//��ʱ��δ����
		vPortSetupTimerInterrupt(); //��ʼ����ʱ��

	reload = SysTick->LOAD;//����ʱ��ֵ
	ticks = nms * fac_ms;
	//��ȡ��װ�ؼĴ���ֵ
	vTaskSuspendAll();//��IoS���ȣ���ֹ���us��ʱ
	told=SysTick->VAL;//��ȡ��ǰ��ֵ�Ĵ���ֵ (��ʼʱ��ֵ)
	
	while(1)
	{
 			tnow=SysTick->VAL; //��ȡ��ǰ��ֵ�Ĵ���ֵ
		
			if(tnow!=told) //��ǰֵ�����ڿ�ʼֵ˵�����ڼ���
			{
					if(tnow<told) //��ǰֵС�ڿ�ʼ��ֵ��˵��δ�Ƶ�
						tcnt+=told-tnow; //����ֵ=��ʼֵ-��ǰֵ

					else //��ǰֵ���ӿ�ʼ��ֵ��˵���ѼƵ�6�����¼ƽ�
						tcnt+=reload-tnow+told;//����ֵ-��װ��ֵ- ��ǰֵ+��ʼֵ//�Ѵӿ�ʼֵ�Ƶ�
					 
					told=tnow;//���¿�ʼֵ
					if(tcnt>=ticks)break; //ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�
			}
	}
			xTaskResumeAll();//�ָ�0S����
	#endif
}



/**
  * @brief  inserts a delay time.
  * @param  sec: specifies the delay time, in seconds.
  * @retval none
  */
void delay_sec(uint16_t sec)
{
  uint16_t index;
  for(index = 0; index < sec; index++)
  {
    delay_ms(500);
    delay_ms(500);
  }
}





/*********************************光耦检测处理****************************************/



/*
   PC11/UART4_RX   PC10/UART4_TX
   
   UART4配置用于光耦检测
*/  

void uart4_config(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

  /* enable the usart2 and gpio clock */
  crm_periph_clock_enable(CRM_UART4_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the uart4 tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_10;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOC, &gpio_init_struct);

  /* configure the uart4 rx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_11;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOC, &gpio_init_struct);

  /* config usart nvic interrupt */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(UART4_IRQn, 0, 0);
 

  /* configure uart4 param */
  usart_init(UART4, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(UART4, TRUE);
  usart_receiver_enable(UART4, TRUE);

  /* enable uart4  interrupt */
  usart_interrupt_enable(UART4, USART_RDBF_INT, TRUE);
  usart_enable(UART4, TRUE);

  usart_interrupt_enable(UART4, USART_TDBE_INT, TRUE);
  
}


static char optocoupler_signal_state;
/* 
  获取光耦检测电机的状态 
  @ret val:optocoupler_signal_state
 
  接收数据：
  0x55  其他点位
  0xAA：停止状态
  0xCC  首个点位
 
 
*/

optocoupler_state get_optocoupler_signal(void)
{  	
  return optocoupler_signal_state;
}

extern SensorPack sensorpack; 
//更新压力传感器AD值
void update_pressure__ad(void)
{
   sensorpack.DownPressure = Read_HX712_A()>>9;  //下压力
   sensorpack.UpPressure = Read_HX712_B()>>9;    //上压力
}


/* 

接收光耦状态进行压力值更新 

*/
void UART4_IRQHandler(void)
{
	static uint16_t uart4_buffer;
	
  if(usart_interrupt_flag_get(UART4, USART_RDBF_FLAG) != RESET)
  {
    /* read one byte from the receive data register */
     uart4_buffer = usart_data_receive(UART4);
//	 printf("uart4_buffer:%02x",uart4_buffer);
		
    if(uart4_buffer == 0X55)
	 {
		 optocoupler_signal_state = optocoupler_motor_run;  //到达起始点
		
	 }
    else if(uart4_buffer ==0xcc) 
    {	
	    optocoupler_signal_state = optocoupler_motor_arrive_motor_point;//到达叶片其他点位
	   
		
    }else if(uart4_buffer == 0xAA)   //停止状态
	{
	    optocoupler_signal_state = optocoupler_motor_stop;
       

	}else{
		
	  optocoupler_signal_state = 0Xdd;  //其他数据	
		
	}	
  }

  if(usart_interrupt_flag_get(UART4, USART_TDBE_FLAG) != RESET)
  {
    /* write one byte to the transmit data register */
    usart_data_transmit(UART4, uart4_buffer);

    /* disable the usart3 transmit interrupt */
    //usart_interrupt_enable(UART4, USART_TDBE_INT, FALSE);
    
  }
}
/* end *****************************光耦检测处理结束**************************** end */

