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
#include <stdio.h>

#include <board.h>
#include <rtthread.h>
//#include <finsh.h>

#include "stm32f4xx.h"
#include "scr.h"
#include "jt808_gps.h"

#include "common.h"


/*
   $GNRMC,074001.00,A,3905.291037,N,11733.138255,E,0.1,,171212,,,A*655220.9*3F0E
   $GNTXT,01,01,01,ANTENNA OK*2B7,N,11733.138255,E,0.1,,171212,,,A*655220.9*3F0E
   $GNGGA,074002.00,3905.291085,N,11733.138264,E,1,11,0.9,8.2,M,-1.6,M,,,1.4*68E
   $GNGLL,3905.291085,N,11733.138264,E,074002.00,A,0*02.9,8.2,M,-1.6,M,,,1.4*68E
   $GPGSA,A,3,18,05,08,02,26,29,15,,,,,,,,,,,,,,,,,,,,,,,,,,1.6,0.9,1.4,0.9*3F8E
   $BDGSA,A,3,04,03,01,07,,,,,,,,,,,,,,,,,,,,,,,,,,,,,1.6,0.9,1.4,0.9*220.9*3F8E
   $GPGSV,2,1,7,18,10,278,29,05,51,063,08,21,052,24,02,24,140,45*4C220.9*3F8E
   $GPGSV,2,2,7,26,72,055,24,29,35,244,37,15,66,224,37*76,24,140,45*4C220.9*3F8E
   $BDGSV,1,1,4,04,27,124,38,03,42,190,34,01,38,146,37,07,34,173,35*55220.9*3F8E

   返回处理的字段数，如果正确的话
 */

static LCD_MSG	lcd_msg;

uint32_t		gps_sec_count = 0;

uint32_t		gps_fixed_sec = 0;
GPS_STATUS		gps_status;
uint32_t		jt808_alarm = 0;

uint8_t			gps_av		= 'V';
uint8_t			gps_year	= 0, gps_month = 0, gps_day = 0;

uint8_t			gps_hour = 0, gps_minute = 0, gps_sec = 0;

/**/
uint8_t process_rmc( uint8_t * pinfo )
{
	//检查数据完整性,执行数据转换
	uint8_t		year = 0, mon = 0, day = 0, fDateModify = 0;
	uint32_t	degrees, minutes;
	uint8_t		count;

	uint8_t		buf[16];
	uint8_t		gps_ew, gps_ns;

	uint8_t		*psrc = pinfo + 7; //指向开始位置

	//rt_kprintf("\r\ngps>%s",pinfo);

/*时间处理 $GNRMC,023548.00,V,,,,,,,270313,,,N*6F*/
	count = 0;
	while( ( *psrc != ',' ) && ( count < 16 ) )
	{
		buf[count++]	= *psrc;
		buf[count]		= 0;
		psrc++;
	}
	if( count != 0 )
	{
		gps_hour	= ( buf[0] - 0x30 ) * 10 + ( buf[1] - 0x30 ) + 8;
		gps_minute	= ( buf[2] - 0x30 ) * 10 + ( buf[3] - 0x30 );
		gps_sec		= ( buf[4] - 0x30 ) * 10 + ( buf[5] - 0x30 );
		if( gps_hour > 23 )
		{
			fDateModify = 1;
			gps_hour	-= 24;
		}
	}
/*A_V处理*/
	psrc++;
	count	= 0;
	gps_av	= 'V';
	while( ( *psrc != ',' ) && ( count < 16 ) )
	{
		buf[count++]	= *psrc;
		buf[count]		= 0;
		psrc++;
	}
	if( ( buf[0] == 'A' ) || ( buf[0] == 'V' ) )
	{
		gps_av = buf[0];
	}
	if( gps_av == 'V' )
	{
		return;
	}

/*纬度处理ddmm.mmmmmm*/
	psrc++;
	count	= 0;
	degrees = 0;
	minutes = 0;
	while( ( *psrc != ',' ) && ( count < 16 ) )
	{
		buf [count++]	= *psrc;
		buf [count]		= 0;
		psrc++;
	}
	if( count != 0 )
	{
		degrees = ( ( buf [0] - 0x30 ) * 10 + ( buf [1] - 0x30 ) ) * 60 * 100000;
		minutes = ( buf [2] - 0x30 ) * 1000000 +
		          ( buf [3] - 0x30 ) * 100000 +
		          ( buf [5] - 0x30 ) * 10000 +
		          ( buf [6] - 0x30 ) * 1000 +
		          ( buf [7] - 0x30 ) * 100 +
		          ( buf [8] - 0x30 ) * 10 +
		          ( buf [9] - 0x30 );
	}
/*N_S处理*/
	psrc++;
	count	= 0;
	gps_ns	= 'N';
	while( ( *psrc != ',' ) && ( count < 16 ) )
	{
		buf[count++]	= *psrc;
		buf[count]		= 0;
		psrc++;
	}
	if( ( buf[0] == 'N' ) || ( buf[0] == 'S' ) )
	{
		gps_ns = buf[0];
	}

/*经度处理*/
	psrc++;
	count = 0;
	while( ( *psrc != ',' ) && ( count < 16 ) )
	{
		buf[count++]	= *psrc;
		buf[count]		= 0;
		psrc++;
	}
	if( count != 0 )
	{
		degrees = ( ( buf [0] - 0x30 ) * 100 + ( buf [1] - 0x30 ) * 10 + ( buf [2] - 0x30 ) ) * 60 * 100000;
		minutes = ( buf [3] - 0x30 ) * 1000000 +
		          ( buf [4] - 0x30 ) * 100000 +
		          ( buf [6] - 0x30 ) * 10000 +
		          ( buf [7] - 0x30 ) * 1000 +
		          ( buf [8] - 0x30 ) * 100 +
		          ( buf [9] - 0x30 ) * 10 +
		          ( buf [10] - 0x30 );
	}
/*N_S处理*/
	psrc++;
	count	= 0;
	gps_ns	= 'E';
	while( ( *psrc != ',' ) && ( count < 16 ) )
	{
		buf[count++]	= *psrc;
		buf[count]		= 0;
		psrc++;
	}
	if( ( buf[0] == 'E' ) || ( buf[0] == 'W' ) )
	{
		gps_ns = buf[0];
	}

/*速度处理*/
	psrc++;
	count = 0;
	while( *psrc != ',' )
	{
		buf [count++]	= *psrc;
		buf [count]		= 0;
		psrc++;
	}

/*方向处理*/
	psrc++;
	count = 0;
	while( ( *psrc != ',' ) && ( count < 12 ) )
	{
		buf [count++]	= *psrc;
		buf [count]		= 0;
		psrc++;
	}

/*日期处理*/
	psrc++;
	count = 0;
	while( ( *psrc != ',' ) && ( count < 12 ) )
	{
		buf [count++]	= *psrc;
		buf [count]		= 0;
		psrc++;
	}
	if( count == 0 )
	{
		return 8;
	}

	day		= ( ( buf [0] - 0x30 ) * 10 ) + ( buf [1] - 0x30 );
	mon		= ( ( buf [2] - 0x30 ) * 10 ) + ( buf [3] - 0x30 );
	year	= ( ( buf [4] - 0x30 ) * 10 ) + ( buf [5] - 0x30 );

	if( fDateModify )
	{
		day++;
		if( mon == 2 )
		{
			if( ( year % 4 ) == 0 )
			{
				if( day == 30 )
				{
					day = 1; mon++;
				}
			} else
			if( day == 29 )
			{
				day = 1; mon++;
			}
		} else
		if( ( mon == 4 ) || ( mon == 6 ) || ( mon == 9 ) || ( mon == 11 ) )
		{
			if( day == 31 )
			{
				mon++; day = 1;
			}
		} else
		{
			if( day == 32 )
			{
				mon++; day = 1;
			}
			if( mon == 13 )
			{
				mon = 1; year++;
			}
		}
	}
	gps_year	= year;
	gps_month	= mon;
	gps_day		= day;
	if( gps_fixed_sec == 0 )
	{
		time_set( gps_hour, gps_minute, gps_sec );
		date_set( gps_year, gps_month, gps_day );
		gps_fixed_sec	= gps_sec_count;
		test_flag		|= TEST_BIT_GPS;
		rt_kprintf( "\r\ntest_flag=%x", test_flag );
		rt_kprintf( "\r\n>gps fixed sec=%d", gps_fixed_sec );
	}

	return 0;
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
uint8_t process_gga( uint8_t * pinfo )
{
	//检查数据完整性,执行数据转换
	uint8_t NoSV;
	uint8_t i;
	uint8_t buf[20];
	uint8_t commacount	= 0, count = 0;
	uint8_t *psrc		= pinfo + 7; //指向开始位置

	while( *psrc++ )
	{
		if( *psrc != ',' )
		{
			buf[count++]	= *psrc;
			buf[count]		= 0;
			continue;
		}
		commacount++;
		switch( commacount )
		{
			case 1: /*时间处理 */
				if( count < 6 )
				{
					return 1;
				}
				break;

			case 2: /*纬度处理ddmm.mmmmmm*/
				break;

			case 3: /*N_S处理*/
				break;

			case 4: /*经度处理*/

				break;
			case 5: /*E_W处理*/
				break;
			case 6: /*定位类型*/
				break;
			case 7: /*NoSV,卫星数*/
				if( count < 1 )
				{
					break;
				}
				NoSV = 0;
				for( i = 0; i < count; i++ )
				{
					NoSV	= NoSV * 10;
					NoSV	+= ( buf[i] - 0x30 );
				}
				gps_status.NoSV = NoSV;
				break;
			case 8: /*HDOP*/
				return 0;

			case 9: /*MSL Altitute*/

				return 0;
		}
		count	= 0;
		buf[0]	= 0;
	}
	return 9;
}


extern void printer_port_init( void );
/***********************************************************
* Function:
* Description:gps收到信息后的处理，头两个字节为长度
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void gps_rx( uint8_t * pinfo, uint16_t length )
{
	uint8_t ret;
	char	* psrc;
	psrc = (char*)pinfo;

	*( psrc + length ) = 0;

	if( strncmp( psrc + 3, "GGA,", 4 ) == 0 )
	{
		if( strncmp( psrc + 1, "GN", 2 ) == 0 )
		{
			gps_status.mode = MODE_BDGPS;
		}else if( strncmp( psrc + 1, "GP", 2 ) == 0 )
		{
			gps_status.mode = MODE_GPS;
		}else if( strncmp( psrc + 1, "BD", 2 ) == 0 )
		{
			gps_status.mode = MODE_BD;
		}
		process_gga( (uint8_t*)psrc );
	}

	if( strncmp( psrc + 3, "RMC,", 4 ) == 0 )
	{
		gps_sec_count++;
		
		ret = process_rmc( (uint8_t*)psrc );
	}

	if( strncmp( psrc + 3, "TXT,", 4 ) == 0 )
	{
		if(bd_model!=0x3017)		/*型号切换 打印的3.3V 蜂鸣器*/
		{

			bd_model=0x3017;			printer_port_init();
			rt_kprintf("\n发现3017 bd_model=%x",bd_model);
			GPIO_SetBits(GPIOB,GPIO_Pin_6);
		}
		if( strncmp( psrc + 24, "OK", 2 ) == 0 )
		{
			gps_status.Antenna_Flag = 0;
			jt808_alarm				&= ~( BIT_ALARM_GPS_OPEN | BIT_ALARM_GPS_SHORT );
		}else if( strncmp( psrc + 24, "OPEN", 4 ) == 0 )
		{
			gps_status.Antenna_Flag = 1;
			jt808_alarm				|= BIT_ALARM_GPS_OPEN; /*bit5 天线开路*/
		}else if( strncmp( psrc + 24, "SHORT", 4 ) == 0 )
		{
			gps_status.Antenna_Flag = 1;
			jt808_alarm				|= BIT_ALARM_GPS_SHORT;
		}

	}
}

/************************************** The End Of File **************************************/
