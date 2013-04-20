/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// �ļ���
 * Author:			// ����
 * Date:			// ����
 * Description:		// ģ������
 * Version:			// �汾��Ϣ
 * Function List:	// ��Ҫ�������书��
 *     1. -------
 * History:			// ��ʷ�޸ļ�¼
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/
#include "scr.h"
#include <rtthread.h>
#include "gsm.h"



AUX_IO				PIN_IN[10] = {
	{ GPIOE, GPIO_Pin_8,  78,  19 },    /*������ť*/
	{ GPIOE, GPIO_Pin_9,  64,  26 },    /*ACC*/
	{ GPIOE, GPIO_Pin_7,  71,  19 },    /*����*/
	{ GPIOC, GPIO_Pin_0,  95,  26 },    /*4.Զ��*/
	{ GPIOC, GPIO_Pin_1,  88,  26 },    /*5.����*/
	{ GPIOA, GPIO_Pin_1,  116, 19 },    /*6.���� ����ΪAD����*/
	{ GPIOC, GPIO_Pin_3,  109, 19 },    /*7.��ת ����ΪAD����*/
	{ GPIOC, GPIO_Pin_2,  102, 19 },    /*8.��ת*/
	{ GPIOE, GPIO_Pin_11, 95,  19 },    /*9.ɲ��*/
	{ GPIOE, GPIO_Pin_10, 88,  19 },    /*10.��ˢ*/
};

AUX_IO				PIN_OUT[] = {
	{ GPIOB, GPIO_Pin_1, 0, 0 },        /*�̵���*/
	{ GPIOB, GPIO_Pin_6, 0, 0 },        /*������*/
};

const unsigned char res_cross[] = {
	/* 84218421*/
	0x90,                               /*[*  *    ]*/
	0x60,                               /*[ **     ]*/
	0x60,                               /*[ **     ]*/
	0x90,                               /*[*  *    ]*/
};
DECL_BMP( 4, 4, res_cross );

const uint8_t res_rtc_ok[]={
	0x00,0x00,
	0x07,0xE0,
	0x18,0x18,
	0x20,0x04,
	0x40,0x02,
	0x40,0x06,
	0x80,0x09,
	0x80,0x11,
	0xA0,0x21,
	0x90,0x41,
	0x48,0x82,
	0x45,0x02,
	0x22,0x04,
	0x18,0x18,
	0x07,0xE0,
	0x00,0x00,
};
DECL_BMP(16,16, res_rtc_ok );


const uint8_t res_rtc_err[]={
	0x00,0x00,
	0x07,0xE0,
	0x18,0x18,
	0x20,0x04,
	0x40,0x02,
	0x48,0x22,
	0x84,0x41,
	0x82,0x81,
	0x81,0x01,
	0x82,0x81,
	0x44,0x42,
	0x48,0x22,
	0x20,0x04,
	0x18,0x18,
	0x07,0xE0,
	0x00,0x00
};
DECL_BMP(16,16, res_rtc_err );


const unsigned char res_iccard_empty[]={
0x00,0x00,0x00,
0x08,0x00,0x04,
0x10,0x00,0x08,
0x20,0x00,0x10,
0x40,0x00,0x20,
0xff,0xff,0xc0,
0x00,0x00,0x00,
0x00,0x00,0x00,
};

DECL_BMP( 24, 8, res_iccard_empty );


const unsigned char res_iccard_insert[]={
0x00,0x00,0x00,
0x09,0xff,0xe4,
0x13,0xff,0xc8,
0x27,0xff,0x90,
0x40,0x00,0x20,
0xff,0xff,0xc0,
0x00,0x00,0x00,
0x00,0x00,0x00,
};

DECL_BMP( 24, 8, res_iccard_insert );



static char cam_ch[4]={0x20,0x20,0x20,0x20};


/*AD���*/

#define ADC1_DR_Address ( (uint32_t)0X4001204C )

uint16_t ADC_ConValue[3];   //   3  ��ͨ��ID    0 : ��� 1: ����   2:  ����

/*�״ζ�λ��ʱ��*/
static uint32_t gps_fixed_sec = 0;
static uint8_t card_status=0;

static uint32_t rtc_ok=0;



/*���Ա�־��
bit 0		RTC����
bit 1      IC��
bit 2 		��
bit 3      ��ӡ
bit 4      CAM1
bit 5      CAM2
bit 6      CAM3
bit 7      CAM4
bit 8		GPS
bit 8		GPRS
*/
#define TEST_BIT_RTC	0x01
#define TEST_BIT_ICCARD	0x02
#define TEST_BIT_MEMS	0x04
#define TEST_BIT_GPS	0x08
#define TEST_BIT_GPRS	0x10
#define TEST_BIT_CAM1	0x20
#define TEST_BIT_CAM2	0x40
#define TEST_BIT_CAM3	0x80
#define TEST_BIT_CAM4	0x100


#define TEST_BIT_ALL	0x1FF
static uint32_t test_flag=0;




/**/
void ad_init( void )
{
	ADC_InitTypeDef			ADC_InitStructure;
	GPIO_InitTypeDef		gpio_init;
	ADC_CommonInitTypeDef	ADC_CommonInitStructure;
	DMA_InitTypeDef			DMA_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE );

	gpio_init.GPIO_Mode = GPIO_Mode_AIN;
	gpio_init.GPIO_Pin	= GPIO_Pin_5;
	GPIO_Init( GPIOC, &gpio_init );

	gpio_init.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init( GPIOA, &gpio_init );

	gpio_init.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init( GPIOC, &gpio_init );

//  3. ADC Common Init
	/* ADC Common configuration *************************************************/
	ADC_CommonInitStructure.ADC_Mode				= ADC_Mode_Independent; /*�ڶ���ģʽ�� ÿ��ADC�ӿڶ�������*/
	ADC_CommonInitStructure.ADC_Prescaler			= ADC_Prescaler_Div4;
	ADC_CommonInitStructure.ADC_DMAAccessMode		= ADC_DMAAccessMode_1;  // ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay	= ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit( &ADC_CommonInitStructure );

	ADC_InitStructure.ADC_Resolution			= ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode			= ENABLE;                   // if used  multi channels set enable
	ADC_InitStructure.ADC_ContinuousConvMode	= ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge	= ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv		= ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign				= ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion		= 3;                        // number of   channel
	ADC_Init( ADC1, &ADC_InitStructure );

//  4. DMA  Config
	/* DMA2 Stream0 channel0 configuration */
	DMA_InitStructure.DMA_Channel				= DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t)ADC1_DR_Address;
	DMA_InitStructure.DMA_Memory0BaseAddr		= (uint32_t)ADC_ConValue;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize			= 3;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single;
	DMA_Init( DMA2_Stream0, &DMA_InitStructure );

	/* DMA2_Stream0 enable */
	DMA_Cmd( DMA2_Stream0, ENABLE );

	/* ADC1 regular channel15 configuration *************************************/
	ADC_RegularChannelConfig( ADC1, ADC_Channel_15, 1, ADC_SampleTime_56Cycles );   // ͨ��1  ��ص���
	/* ADC1 regular channel1 configuration *************************************/
	ADC_RegularChannelConfig( ADC1, ADC_Channel_1, 2, ADC_SampleTime_56Cycles );    //  ͨ��2   ����
	/* ADC1 regular channel13 configuration *************************************/
	ADC_RegularChannelConfig( ADC1, ADC_Channel_13, 3, ADC_SampleTime_56Cycles );   // ͨ��3   ����

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd( ADC1, ENABLE );

	/* Enable ADC1 DMA */
	ADC_DMACmd( ADC1, ENABLE );

	/* Enable ADC3 */
	ADC_Cmd( ADC1, ENABLE );

	ADC_SoftwareStartConv( ADC1 );
}

static uint8_t year,month,day,hour,minute,sec;


__IO uint16_t	IC2Value	= 0;
__IO uint16_t	DutyCycle	= 0;
__IO uint32_t	Frequency	= 0;


uint32_t mems_alarm_tick=0;



/*����PA.0 ��Ϊ�ⲿ�������*/
void pulse_init( void )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	TIM_ICInitTypeDef	TIM_ICInitStructure;

	/* TIM5 clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM5, ENABLE );

	/* GPIOA clock enable */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE );

	/* TIM5 chennel1 configuration : PA.0 */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	/* Connect TIM pin to AF0 */
	GPIO_PinAFConfig( GPIOA, GPIO_PinSource0, GPIO_AF_TIM5 );

	/* Enable the TIM5 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	TIM_ICInitStructure.TIM_Channel		= TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity	= TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter	= 0x0;

	TIM_PWMIConfig( TIM5, &TIM_ICInitStructure );

	/* Select the TIM5 Input Trigger: TI1FP1 */
	TIM_SelectInputTrigger( TIM5, TIM_TS_TI1FP1 );

	/* Select the slave Mode: Reset Mode */
	TIM_SelectSlaveMode( TIM5, TIM_SlaveMode_Reset );
	TIM_SelectMasterSlaveMode( TIM5, TIM_MasterSlaveMode_Enable );

	/* TIM enable counter */
	TIM_Cmd( TIM5, ENABLE );

	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig( TIM5, TIM_IT_CC2, ENABLE );
}

/*TIM5_CH1*/
void TIM5_IRQHandler( void )
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq( &RCC_Clocks );

	TIM_ClearITPendingBit( TIM5, TIM_IT_CC2 );

	/* Get the Input Capture value */
	IC2Value = TIM_GetCapture2( TIM5 );

	if( IC2Value != 0 )
	{
		/* Duty cycle computation */
		//DutyCycle = ( TIM_GetCapture1( TIM5 ) * 100 ) / IC2Value;
		/* Frequency computation   TIM4 counter clock = (RCC_Clocks.HCLK_Frequency)/2 */
		//Frequency = (RCC_Clocks.HCLK_Frequency)/2 / IC2Value;
/*�ǲ��Ƿ����·?*/
		DutyCycle	= ( IC2Value * 100 ) / TIM_GetCapture1( TIM5 );
		Frequency	= ( RCC_Clocks.HCLK_Frequency ) / 2 / TIM_GetCapture1( TIM5 );
	}else
	{
		DutyCycle	= 0;
		Frequency	= 0;
	}
}

/*��ʾ״̬��Ϣ*/
static void showinfo(void)
{
	char buf[32];

	lcd_asc0608(122-6*6,0,"041813",LCD_MODE_INVERT);
	if( gps_fixed_sec )
	{
		sprintf( buf, "%02d:%02d", gps_fixed_sec / 60, gps_fixed_sec % 60 );
		lcd_asc0608( 0, 8, buf, LCD_MODE_SET );
	}

	if(card_status==1)
	{
		lcd_bitmap( 122 - 6*4, 24, &bmp_res_iccard_insert, LCD_MODE_SET );
	}
	else
	{
		lcd_bitmap( 122 - 6*4, 24, &bmp_res_iccard_empty, LCD_MODE_SET );
	}
	if(rtc_ok)
		lcd_asc0608( 0, 24, "RTC", LCD_MODE_SET );
		
	lcd_update( 0, 31 );
}



/*
   ֻ����ʾ��Ϣ����û���Ӳ˵�
   ��ʾ3ҳ����Ϣ ��γ�� ��λ ʱ��
 */

static void show( void *parent )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	uint8_t				i;
	char				buf[64];

	scr_1_idle.parent = (PSCR)parent;

	pulse_init( );
	//ad_init( );

	for( i = 0; i < sizeof( PIN_OUT ) / sizeof( AUX_IO ); i++ )
	{
		GPIO_InitStructure.GPIO_Pin		= PIN_OUT[i].pin;
		GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
		GPIO_Init( PIN_OUT[i].port, &GPIO_InitStructure );
	}

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;

	for( i = 0; i < sizeof( PIN_IN ) / sizeof( AUX_IO ); i++ )
	{
		GPIO_InitStructure.GPIO_Pin = PIN_IN[i].pin;
		GPIO_Init( PIN_IN[i].port, &GPIO_InitStructure );
	}
/*PA0 �ٶ��ź�*/

	showinfo();

}

/*��������*/
static void keypress( unsigned int key )
{
	uint8_t buf[64];
	uint8_t i,h,m;

	if(key==0) return;
	switch( key )
	{
		case KEY_MENU_PRESS:
			break;
		case KEY_OK_PRESS:      /*�����ϼ��˵�*/
			break;
		case KEY_UP_PRESS:		/*����*/
			break;
		case KEY_DOWN_PRESS:    /*��ӡ����*/
			GPIO_ResetBits(GPIOB,GPIO_Pin_6);
		
			printer( "���ƺ���:\r\n���Ʒ���:\r\n����VIN:\r\n��ʻԱ����:\r\n��ʻ֤����:\r\n" );
			
			sprintf( buf, "��ӡʱ��:20%02d/%02d/%02d %02d:%02d:%02d\r\n" ,year,month,day,hour,minute,sec );
			printer( buf );
			printer( "ͣ��ǰ15���ӳ���:\r\n" );
			h=hour;
			m=minute;
			for(i=0;i<15;i++)
			{	
				if(m==0)
				{
					m=60;
					if(h==0) h=24;
					h--;
				}	
				m--;
				sprintf(buf,"%02d:%02d 000km/h\r\n",h,m);
				printer(buf);
			}
			printer( "���һ��ƣ�ͼ�ʻ��¼:\r\n��ƣ�ͼ�ʻ��¼\r\n\n\n\n\n" );
			break;
	}
}

/*ϵͳʱ��50ms*/
static void timetick( unsigned int systick )
{
	static uint8_t	offset = 0;
	uint32_t		sec;
	uint8_t			buf[64];
	uint8_t			i,j;
	void * pmsg;

	offset++;
	if( offset >= 20 )
	{
		offset	= 0;
		sec		= systick / 100;
		sprintf( buf, "%02d:%02d  %04d", sec / 60, sec % 60, Frequency);
		lcd_asc0608( 0, 0, buf, LCD_MODE_SET );
		if( sec & 0x01 ) /*�������*/
		{
			GPIO_SetBits( GPIOB, GPIO_Pin_1 );
		}else
		{
			GPIO_ResetBits( GPIOB, GPIO_Pin_1 );
		}
		showinfo();
	}

	
}

/*�����Լ�״̬����Ϣ*/

extern uint32_t iccard_beep_timeout;

static void msg( void *plcdmsg )
{
	LCD_MSG		* plcd_msg = (LCD_MSG* )plcdmsg;
	char		ch,buf[100];
	uint32_t	i;
	void *pmsg;

	if( plcd_msg->id == LCD_MSG_ID_GPS )
	{
		if( ( gps_fixed_sec == 0 ) && ( plcd_msg->info.gps_rmc.gps_av == 'A' ) )
		{
			gps_fixed_sec = rt_tick_get( ) * 10 / 1000;
			test_flag|=TEST_BIT_GPS;
			sprintf( buf, "%02d:%02d", gps_fixed_sec / 60, gps_fixed_sec % 60 );
			lcd_asc0608( 0, 8, buf, LCD_MODE_SET );
			i=sprintf(buf,"AT%%TTS=2,3,5,\"475053D2D1B6A8CEBB\"\r\n",ch+0x30);
			buf[i]=0;
			pmsg=rt_malloc(i+1);
			if(pmsg!=RT_NULL)
			{
				memcpy(pmsg,buf,i+1);
				rt_mb_send(&mb_tts,(rt_uint32_t)pmsg);
			}	
		}
		year=plcd_msg->info.gps_rmc.year;
		month=plcd_msg->info.gps_rmc.month;
		day= plcd_msg->info.gps_rmc.day;
		hour= plcd_msg->info.gps_rmc.hour;
		minute= plcd_msg->info.gps_rmc.minitue;
		sec= plcd_msg->info.gps_rmc.sec;

		time_set(hour,minute,sec);

		date_set(year,month,day);

		sprintf( buf, "%c%02d-%02d %02d:%02d:%02d",
		         plcd_msg->info.gps_rmc.gps_av,month,day,hour,minute,sec );
		lcd_asc0608( 122 - 6 * 15, 8, buf, LCD_MODE_SET );
	}
	if( plcd_msg->id == LCD_MSG_ID_GSM )
	{
		if( plcd_msg->info.payload[0] == 1 ) /*ͨ��*/
		{
			rt_kprintf( "\r\nIncoming Call" );
			pscr = &scr_2_call;
			pscr->show( &scr_1_idle );
		}
	}
	if(plcd_msg->id == LCD_MSG_ID_GPRS)
	{
		i=rt_tick_get()/100;
		sprintf( buf, "%02d:%02d  GPRS",i/60,i%60);
		lcd_asc0608( 0,16, buf, LCD_MODE_SET );
		test_flag|=TEST_BIT_GPRS;

	}
	if(plcd_msg->id == LCD_MSG_ID_CAM)
	{
		ch=plcd_msg->info.payload[0];
		switch(ch)
		{
			case 1: test_flag|=TEST_BIT_CAM1; break;
			case 2: test_flag|=TEST_BIT_CAM2; break;
			case 3: test_flag|=TEST_BIT_CAM3; break;
			case 4: test_flag|=TEST_BIT_CAM4; break;

		}
		if(plcd_msg->info.payload[1]==SUCCESS)
		{
			i=sprintf(buf,"AT%%TTS=2,3,5,\"C5C4D5D5%02xD5FDB3A3\"\r\n",ch+0x30);
			cam_ch[ch-1]=0x30+ch;
			lcd_asc0608( 122 - 6*4, 16, cam_ch, LCD_MODE_SET );
		}
		else
		{
			i=sprintf(buf,"AT%%TTS=2,3,5,\"C5C4D5D5%02xD2ECB3A3\"\r\n",ch+0x30);
		}
		buf[i]=0;
		pmsg=rt_malloc(i+1);
		if(pmsg!=RT_NULL)
		{
			memcpy(pmsg,buf,i+1);
			rt_mb_send(&mb_tts,(rt_uint32_t)pmsg);
		}		

	}
	if(plcd_msg->id == LCD_MSG_ID_ICCARD)
	{
		iccard_beep_timeout=10;
		card_status=plcd_msg->info.payload[0];
		test_flag|=TEST_BIT_ICCARD;
	}

	if(plcd_msg->id == LCD_MSG_ID_RTC)
	{
		memset(buf,0,32);
		if(plcd_msg->info.payload[0]==SUCCESS)
		{
			i=sprintf(buf,"AT%%TTS=2,3,5,\"525443D5FDB3A3\"\r\n",ch+0x30);
			test_flag|=TEST_BIT_RTC;
			rtc_ok=1;
		}
		else
		{
			i=sprintf(buf,"AT%%TTS=2,3,5,\"525443D2ECB3A3\"\r\n",ch+0x30);
		}
		rt_kprintf("\r\nRTC len=%d\r\n",i);
		buf[i]=0;
		pmsg=rt_malloc(i+1);
		if(pmsg!=RT_NULL)
		{
			memcpy(pmsg,buf,i+1);
			rt_mb_send(&mb_tts,(rt_uint32_t)pmsg);
		}			
		
	}

	if(test_flag==TEST_BIT_ALL)
	{
		i=sprintf(buf,"AT%%TTS=2,3,5,\"B2E2CAD4CDEAB3C9\"\r\n");
		buf[i]=0;
		pmsg=rt_malloc(i+1);
		if(pmsg!=RT_NULL)
		{
			memcpy(pmsg,buf,i+1);
			rt_mb_send(&mb_tts,(rt_uint32_t)pmsg);
		}
	}

	showinfo();
	
}

SCR scr_1_idle =
{
	&show,
	&keypress,
	&timetick,
	&msg,
};

/************************************** The End Of File **************************************/
