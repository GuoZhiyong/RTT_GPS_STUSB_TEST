/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		// 模块描述
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/
#include "scr.h"
#include <rtthread.h>
#include "gsm.h"
#include <string.h>
#include <stdio.h>

#include "jt808_gps.h"
#include "sle4442.h"

#include "common.h"

AUX_IO				PIN_IN[10] = {
	{ GPIOE, GPIO_Pin_8,  78,  19 },    /*紧急按钮*/
	{ GPIOE, GPIO_Pin_9,  64,  26 },    /*ACC*/
	{ GPIOE, GPIO_Pin_7,  71,  19 },    /*输入*/
	{ GPIOC, GPIO_Pin_0,  95,  26 },    /*4.远光*/
	{ GPIOC, GPIO_Pin_1,  88,  26 },    /*5.车门*/
	{ GPIOA, GPIO_Pin_1,  116, 19 },    /*6.喇叭 定义为AD输入*/
	{ GPIOC, GPIO_Pin_3,  109, 19 },    /*7.左转 定义为AD输入*/
	{ GPIOC, GPIO_Pin_2,  102, 19 },    /*8.右转*/
	{ GPIOE, GPIO_Pin_11, 95,  19 },    /*9.刹车*/
	{ GPIOE, GPIO_Pin_10, 88,  19 },    /*10.雨刷*/
};

AUX_IO				PIN_OUT[] = {
	{ GPIOB, GPIO_Pin_1, 0, 0 },        /*继电器*/
	//{ GPIOB, GPIO_Pin_6, 0, 0 },        /*蜂鸣器*/
};

const unsigned char res_cross[] = {
	/* 84218421*/
	0x90,                               /*[*  *    ]*/
	0x60,                               /*[ **     ]*/
	0x60,                               /*[ **     ]*/
	0x90,                               /*[*  *    ]*/
};
DECL_BMP( 4, 4, res_cross );

const uint8_t res_rtc_ok[] = {
	0x00, 0x00,
	0x07, 0xE0,
	0x18, 0x18,
	0x20, 0x04,
	0x40, 0x02,
	0x40, 0x06,
	0x80, 0x09,
	0x80, 0x11,
	0xA0, 0x21,
	0x90, 0x41,
	0x48, 0x82,
	0x45, 0x02,
	0x22, 0x04,
	0x18, 0x18,
	0x07, 0xE0,
	0x00, 0x00,
};
DECL_BMP( 16, 16, res_rtc_ok );

const uint8_t res_rtc_err[] = {
	0x00, 0x00,
	0x07, 0xE0,
	0x18, 0x18,
	0x20, 0x04,
	0x40, 0x02,
	0x48, 0x22,
	0x84, 0x41,
	0x82, 0x81,
	0x81, 0x01,
	0x82, 0x81,
	0x44, 0x42,
	0x48, 0x22,
	0x20, 0x04,
	0x18, 0x18,
	0x07, 0xE0,
	0x00, 0x00
};
DECL_BMP( 16, 16, res_rtc_err );

const unsigned char res_iccard_empty[] = {
	0x00, 0x00, 0x00,
	0x08, 0x00, 0x04,
	0x10, 0x00, 0x08,
	0x20, 0x00, 0x10,
	0x40, 0x00, 0x20,
	0xff, 0xff, 0xc0,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

DECL_BMP( 24, 8, res_iccard_empty );

const unsigned char res_iccard_insert[] = {
	0x00, 0x00, 0x00,
	0x09, 0xff, 0xe4,
	0x13, 0xff, 0xc8,
	0x27, 0xff, 0x90,
	0x40, 0x00, 0x20,
	0xff, 0xff, 0xc0,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

DECL_BMP( 24, 8, res_iccard_insert );

const unsigned char res_iccard_err[] = {
	0x00, 0x00, 0x00,
	0x08, 0x92, 0x44,
	0x11, 0x24, 0x88,
	0x22, 0x49, 0x10,
	0x44, 0x92, 0x20,
	0xff, 0xff, 0xc0,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

DECL_BMP( 24, 8, res_iccard_err );

static char		cam_ch[4] = { 0x20, 0x20, 0x20, 0x20 };

static uint8_t	tick_500ms = 0;

/*要显示的状态信息*/

/*首次定位的时刻*/

//static uint8_t	card_status = 0;

uint8_t gsm_csq_min = 0xff;
uint8_t gsm_csq_max = 0;

/*记录上一次的状态*/
uint8_t ctrlbit_status = 0;

static print_testresult( void )
{
	char	*gps_mode[4] = { "  ", "BD", "GP", "GN" };
	char	buf[64];
	char	i;

/*关掉所有外设*/

	GPIO_ResetBits( GPIOD, GPIO_Pin_10 );   /*关掉gps*/

	GPIO_ResetBits( GPIOD, GPIO_Pin_13 );   /*关掉gsm*/
	GPIO_ResetBits( GPIOD, GPIO_Pin_12 );

	GPIO_ResetBits( GPIOB, GPIO_Pin_8 );    /*关掉CAM*/

	printer( "      ******测试结果******      \r\n\n" );

	sprintf( buf, "打印时间:20%02d/%02d/%02d %02d:%02d:%02d\r\n", gps_year, gps_month, gps_day, gps_hour, gps_minute, gps_sec );
	printer( buf );

	sprintf( buf, "测试用时:%02d:%02d\r\n", rt_tick_get( ) / 100 / 60, rt_tick_get( ) / 100 % 60 );
	printer( buf );

	printer( "\r\n\nBDGPS- " );

	if( jt808_alarm & BIT_ALARM_GPS_ERR )
	{
		printer( "模块异常" );
	}else if( jt808_alarm & BIT_ALARM_GPS_OPEN )
	{
		printer( ",天线开路" );
	}else if( jt808_alarm & BIT_ALARM_GPS_SHORT )
	{
		printer( ",天线短路" );
	}else
	{
		printer( "模块正常" );
	}

	printer( "\r\n" );

	sprintf( buf, "    首次定位用时:%02d:%02d\r\n", gps_fixed_sec / 60, gps_fixed_sec % 60 );
	printer( buf );

	sprintf( buf, "    定位模式:%s 星数:%d\r\n", gps_mode[gps_status.mode], gps_status.NoSV );
	printer( buf );

	sprintf( buf, "\r\n\nGSM-M66 VER:%s\r\n", gsm_ver );
	printer( buf );
	sprintf( buf, "    登网用时:%02d:%02d\r\n", gprs_ok_past_sec / 60, gprs_ok_past_sec % 60 );
	printer( buf );
	sprintf( buf, "    信号强度:%02d[%02d-%02d]\r\n", gsm_csq, gsm_csq_min, gsm_csq_max );
	printer( buf );

	if( test_flag & TEST_BIT_DF )
	{
		if( test_df_error )
		{
			printer( "Flash 异常\r\n" );
		} else
		{
			printer( "Flash 正常\r\n" );
		}
	}else
	{
		printer( "Flash 测试未完成\r\n" );
	}

	if( rtc_ok )
	{
		printer( "RTC  正常\r\n" );
	}else
	{
		printer( "RTC  异常\r\n" );
	}

	if( mems_status == SUCCESS )
	{
		printer( "MEMS 正常\r\n" );
	}else
	{
		printer( "MEMS 异常\r\n" );
	}

	if( test_flag & TEST_BIT_CAM )
	{
		if( test_cam_flag[0] == 1 )
		{
			printer( "拍照 正常\r\n" );
		}else
		{
			printer( "拍照 异常\r\n" );
		}
	}else
	{
		printer( "拍照测试未完成\r\n" );
	}

	if( test_flag & TEST_BIT_ICCARD )
	{
		printer( "IC卡 正常\r\n" );
	}else
	{
		printer( "IC卡测试未完成\r\n" );
	}

	printer( "\r\n\n\n\n\n\n\n\n\n" );
}

/*显示状态信息*/
void showinfo( void )
{
	char	buf[32];
	char	*gps_mode[4] = { "  ", "BD", "GP", "GN" };
	char	i;

	if( pscr != &scr_1_idle )
	{
		return;
	}

	lcd_fill( 0 );

	sprintf( buf, "%02d:%02d  %04d", gps_sec_count / 60, gps_sec_count % 60, Frequency );
	lcd_asc0608( 0, 0, buf, LCD_MODE_SET );

	lcd_asc0608( 122 - 6 * 6, 0, "082901", LCD_MODE_SET );

	if( gps_fixed_sec )
	{
		sprintf( buf, "%02d:%02d", gps_fixed_sec / 60, gps_fixed_sec % 60 );
	}else
	{
		sprintf( buf, "%02d:%02d", gps_sec_count / 60, gps_sec_count % 60 );
	}
	lcd_asc0608( 0, 8, buf, LCD_MODE_SET );

	if( jt808_alarm & ( BIT_ALARM_GPS_ERR | BIT_ALARM_GPS_OPEN | BIT_ALARM_GPS_SHORT ) )
	{
		lcd_asc0608( 32, 8, gps_mode[gps_status.mode], LCD_MODE_INVERT );
	}else
	{
		lcd_asc0608( 32, 8, gps_mode[gps_status.mode], LCD_MODE_SET );
	}
	sprintf( buf, "%02d %c %02d:%02d:%02d", gps_status.NoSV, gps_av, gps_hour, gps_minute, gps_sec );
	lcd_asc0608( 44, 8, buf, LCD_MODE_SET );

	if( gsm_csq != 0xff )
	{
		if( gsm_csq_min > gsm_csq )
		{
			gsm_csq_min = gsm_csq;
		}
		if( gsm_csq_max < gsm_csq )
		{
			gsm_csq_max = gsm_csq;
		}
		sprintf( buf, "%02d[%02d-%02d]", gsm_csq, gsm_csq_min, gsm_csq_max );
		lcd_asc0608( 122 - 6 * 9, 16, buf, LCD_MODE_SET );
	}

	if( gprs_ok_past_sec )
	{
		sprintf( buf, "%02d:%02d", gprs_ok_past_sec / 60, gprs_ok_past_sec % 60 );
		lcd_asc0608( 0, 16, buf, LCD_MODE_SET );
	}
	lcd_asc0608( 44, 16, gsm_ver, LCD_MODE_SET );

	if( iccard_value == 1 )
	{
		lcd_asc0608( 98, 24, "IC", LCD_MODE_SET );
	}else if( iccard_value == 2 )
	{
		lcd_asc0608( 98, 24, "IC", LCD_MODE_INVERT );
	}else
	{
	}

	if( rtc_ok )
	{
		lcd_asc0608( 0, 24, "RTC", LCD_MODE_SET );
	}

	if( test_flag & TEST_BIT_DF )
	{
		if( test_df_error )
		{
			lcd_asc0608( 24, 24, "DF", LCD_MODE_INVERT );
		} else
		{
			lcd_asc0608( 24, 24, "DF", LCD_MODE_SET );
		}
	}

	if( mems_status == SUCCESS )
	{
		lcd_asc0608( 40, 24, "MEMS", LCD_MODE_SET );
	}
	else
	{
		lcd_asc0608( 40, 24, "MEMS", LCD_MODE_INVERT );
	}

	if( test_flag & TEST_BIT_CAM )
	{
		if( test_cam_flag[0] == 1 )
		{
			lcd_asc0608( 70, 24, "CAM", LCD_MODE_SET );
		}else
		{
			lcd_asc0608( 70, 24, "CAM", LCD_MODE_INVERT );
		}
	}
	lcd_update( 0, 31 );
}

/*
   只是显示信息，并没有子菜单
   显示3页的信息 经纬度 定位 时间
 */

static void show( void *parent )
{


	scr_1_idle.parent = (PSCR)parent;

/*PA0 速度信号*/

	showinfo( );
}

/*按键处理*/
static void keypress( unsigned int key )
{
	char	buf[64];
	char	*gps_mode[4] = { "  ", "BD", "GP", "GN" };

	if( key == 0 )
	{
		return;
	}
	switch( key )
	{
		case KEY_MENU_REPEAT:
			pscr = &scr_3_bdupgrade;
			pscr->show( &scr_1_idle );
			break;
		case KEY_OK_REPEAT:
			pscr = &scr_4_bdcheck;
			pscr->show( &scr_1_idle );
			break;
		case KEY_OK_PRESS:      /*返回上级菜单*/
			break;
		case KEY_UP_PRESS:      /*拍照*/
			break;
		case KEY_DOWN_PRESS:    /*打印测试结果*/
			//GPIO_ResetBits(GPIOB,GPIO_Pin_6);
			print_testresult( );
			break;
	}
}

/*系统时间50ms*/

/*定时检查状态变化*/
static void timetick( unsigned int systick )
{
	static uint8_t	offset		= 0;
	static uint32_t relay_sec	= 0;
	uint8_t			buf[64];
	uint8_t			i, j;
	void			* pmsg;
/*发生状态改变*/
	if( ( ctrlbit_printer_3v3_on | ctrlbit_buzzer ) ^ ctrlbit_status )
	{
		//showinfo( );
		ctrlbit_status = ( ctrlbit_printer_3v3_on | ctrlbit_buzzer );
	}

	if( IC_Card_Checked ^ iccard_status )   /*有状态变化*/
	{
		iccard_beep_timeout = 5;
		if( IC_Card_Checked == 0 )          /*卡拔出*/
		{
			//i			= sprintf( buf, "AT%%TTS=2,3,5,\"4943BFA8B0CEB3F6\"\r\n\0" );   /*IC卡拔出*/
			test_flag |= TEST_BIT_ICCARD;
			rt_kprintf( "\r\ntest_flag=%x", test_flag );
		}else if( IC_Card_Checked == 1 )    /*卡插入正确*/
		{
			//i = sprintf( buf, "AT%%TTS=2,3,5,\"4943BFA8D5FDB3A3\"\r\n\0" );             /*IC卡正常*/
			iccard_value = 1;
		}else if( IC_Card_Checked == 2 )    /*卡插入错误*/
		{
			//i = sprintf( buf, "AT%%TTS=2,3,5,\"4943BFA8B4EDCEF3\"\r\n\0" );             /*IC卡错误*/
			iccard_value = 2;
		}

		//pmsg = rt_malloc( i );
		//if( pmsg != RT_NULL )
		//{
		//	memcpy( pmsg, buf, i );
		//	rt_mb_send( &mb_tts, (rt_uint32_t)pmsg );
		//}
		iccard_status = IC_Card_Checked;
	}

	tick_500ms++;
	if( tick_500ms == 10 )
	{
		showinfo( );
		tick_500ms = 0;

		if( test_flag == TEST_BIT_ALL ) /*全部测试完成,播报*/
		{
			if( print_testresult_count == 0 )
			{
				i		= sprintf( buf, "AT%%TTS=2,3,5,\"B2E2CAD4CDEAB3C9\"\r\n" );
				buf[i]	= 0;
				pmsg	= rt_malloc( i + 1 );
				if( pmsg != RT_NULL )
				{
					memcpy( pmsg, buf, i + 1 );
					rt_mb_send( &mb_tts, (rt_uint32_t)pmsg );
				}
			}
			print_testresult_count++; // 500ms计数


			/*取消自动打印
			   if(print_testresult_count==10)
			   {
			   print_testresult();
			   }
			 */
		}
	}

	offset++;
	if( offset >= 20 )
	{
		relay_sec++;
		offset = 0;
		if( relay_sec & 0x01 ) /*输出控制*/
		{
			GPIO_SetBits( GPIOB, GPIO_Pin_1 );
		}else
		{
			GPIO_ResetBits( GPIOB, GPIO_Pin_1 );
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void msg( void *plcdmsg )
{
	LCD_MSG		* plcd_msg = (LCD_MSG* )plcdmsg;
	char		ch, buf[100];
	uint32_t	i;
	void		*pmsg;

	if( plcd_msg->id == LCD_MSG_ID_GSM )
	{
		if( plcd_msg->info.payload[0] == 1 ) /*通话*/
		{
			rt_kprintf( "\r\nIncoming Call" );
			pscr = &scr_2_call;
			pscr->show( &scr_1_idle );
			return;
		}
	}

	showinfo( );
}

SCR scr_1_idle =
{
	&show,
	&keypress,
	&timetick,
	&msg,
};


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void aux_init( void )
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	uint8_t				i;

	for( i = 0; i < sizeof( PIN_OUT ) / sizeof( AUX_IO ); i++ )
	{
		GPIO_InitStructure.GPIO_Pin		= PIN_OUT[i].pin;
		GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_DOWN;
		GPIO_Init( PIN_OUT[i].port, &GPIO_InitStructure );
	}

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;

	for( i = 0; i < sizeof( PIN_IN ) / sizeof( AUX_IO ); i++ )
	{
		GPIO_InitStructure.GPIO_Pin = PIN_IN[i].pin;
		GPIO_Init( PIN_IN[i].port, &GPIO_InitStructure );
	}
}

/************************************** The End Of File **************************************/
