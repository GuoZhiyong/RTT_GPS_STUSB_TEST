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
#include <rtthread.h>
#include <stm32f4xx.h>
#include <finsh.h>
#include "rtc.h"
#include "scr.h"

__IO uint32_t			AsynchPrediv = 0, SynchPrediv = 0;
RTC_TimeTypeDef			RTC_TimeStructure;
RTC_InitTypeDef			RTC_InitStructure;
RTC_DateTypeDef			RTC_DateStructure;

static struct rt_device dev_rtc;


#define RTC_CONFIGED_FLAG 0x32f2

#define RTC_CLOCK_SOURCE_LSE /* LSE used as RTC source clock */


/* #define RTC_CLOCK_SOURCE_LSI */    /* LSI used as RTC source clock. The RTC Clock
                                         may varies due to LSI frequency dispersion. */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RTC_TimeTypeDef		RTC_TimeStructure;
RTC_InitTypeDef		RTC_InitStructure;
RTC_AlarmTypeDef	RTC_AlarmStructure;

__IO uint32_t		uwAsynchPrediv	= 0;
__IO uint32_t		uwSynchPrediv	= 0;

/* Private function prototypes -----------------------------------------------*/
static ErrorStatus RTC_Config( void )
{
	RTC_DateTypeDef RTC_DateStructure;
	__IO uint32_t	timeout = 0xC0000;

	RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE );
	PWR_BackupAccessCmd( ENABLE );

	/* Enable the LSE OSC */
	RCC_LSEConfig( RCC_LSE_ON );

	/* Wait till LSE is ready */

	while( RCC_GetFlagStatus( RCC_FLAG_LSERDY ) == RESET )
	{
		if( timeout )
		{
			timeout--;
		}else
		{
			rt_kprintf( "\r\n%s(%d) error\r\n", __func__, __LINE__ );
			return ERROR;
		}
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig( RCC_RTCCLKSource_LSE );
	/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
	uwSynchPrediv	= 0xFF;
	uwAsynchPrediv	= 0x7F;

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd( ENABLE );

	/* Wait for RTC APB registers synchronisation */
	if( RTC_WaitForSynchro( ) == SUCCESS )
	{
		RTC_InitStructure.RTC_AsynchPrediv	= uwAsynchPrediv;
		RTC_InitStructure.RTC_SynchPrediv	= uwSynchPrediv;
		RTC_InitStructure.RTC_HourFormat	= RTC_HourFormat_24;
		RTC_Init( &RTC_InitStructure );

		/* Set the date: Friday January 11th 2013 */
		RTC_DateStructure.RTC_Year		= 0x13;
		RTC_DateStructure.RTC_Month		= RTC_Month_January;
		RTC_DateStructure.RTC_Date		= 0x11;
		RTC_DateStructure.RTC_WeekDay	= RTC_Weekday_Saturday;
		RTC_SetDate( RTC_Format_BCD, &RTC_DateStructure );

		/* Set the time to 05h 20mn 00s AM */
		RTC_TimeStructure.RTC_H12		= RTC_H12_AM;
		RTC_TimeStructure.RTC_Hours		= 0x05;
		RTC_TimeStructure.RTC_Minutes	= 0x20;
		RTC_TimeStructure.RTC_Seconds	= 0x00;

		RTC_SetTime( RTC_Format_BCD, &RTC_TimeStructure );

		/* Indicator for the RTC configuration */
		RTC_WriteBackupRegister( RTC_BKP_DR0, RTC_CONFIGED_FLAG );
		return SUCCESS;
	}else
	{
		rt_kprintf( "\r\n%s(%d) error\r\n", __func__, __LINE__ );
		return ERROR;
	}
}

/**
 * @brief  Display the current time.
 * @param  None
 * @retval None
 */
void datetime( void )
{
	/* Get the current Time */
	RTC_GetTime( RTC_Format_BIN, &RTC_TimeStructure );
	RTC_GetDate(RTC_Format_BIN,&RTC_DateStructure);
	/* Display time Format : hh:mm:ss */
	rt_kprintf( "\r\nRTC=%02d-%02d-%02d %02d:%02d:%02d",\
		RTC_DateStructure.RTC_Year,\
		RTC_DateStructure.RTC_Month,\
		RTC_DateStructure.RTC_Date,\
		RTC_TimeStructure.RTC_Hours,\
		RTC_TimeStructure.RTC_Minutes,\
		RTC_TimeStructure.RTC_Seconds );
}

FINSH_FUNCTION_EXPORT(datetime,show time);

void time_set(uint8_t hour,uint8_t min,uint8_t sec)
{
	RTC_TimeStructure.RTC_H12	  = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours   = hour;
	RTC_TimeStructure.RTC_Minutes = min;
	RTC_TimeStructure.RTC_Seconds = sec; 
	
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);  
}
FINSH_FUNCTION_EXPORT(time_set,set time);

void date_set(uint8_t year,uint8_t month,uint8_t day)
{
	RTC_DateStructure.RTC_Year   = year;
	RTC_DateStructure.RTC_Month = month;
	RTC_DateStructure.RTC_Date = day; 
	
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);  
}
FINSH_FUNCTION_EXPORT(date_set,set date);



/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t rtc_init( rt_device_t dev )
{
	LCD_MSG lcdmsg;
	if( RTC_ReadBackupRegister( RTC_BKP_DR0 ) != RTC_CONFIGED_FLAG )
	{
		rt_kprintf( "\r\n not config 0x32F2\r\n" );
		/* RTC configuration	*/
		if( RTC_Config( ) == ERROR )
		{
			rt_kprintf( "\r\n%s(%d) RTC error\r\n", __func__, __LINE__ );
			goto lbl_rtc_err;
		}
		/* Display the RTC Time and Alarm */
		datetime( );
		rt_kprintf("\r\n RTC OK\r\n");
		goto lbl_rtc_ok;
	}else
	{
		rt_kprintf( "\r\n wait ForSynchro\r\n" );

		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE );

		/* Allow access to RTC */
		PWR_BackupAccessCmd( ENABLE );

		/* Wait for RTC APB registers synchronisation */
		if( RTC_WaitForSynchro( ) == SUCCESS )
		{
			datetime( );
			rt_kprintf("\r\n RTC OK\r\n");
			goto lbl_rtc_ok;
		}else
		{
			rt_kprintf( "\r\n%s(%d) error\r\n", __func__, __LINE__ );
			goto lbl_rtc_err;
		}
	}
lbl_rtc_err:
	lcdmsg.id=LCD_MSG_ID_RTC;
	lcdmsg.info.payload[0]=ERROR;
	pscr->msg(&lcdmsg);
	return 1;
lbl_rtc_ok:
	lcdmsg.id=LCD_MSG_ID_RTC;
	lcdmsg.info.payload[0]=SUCCESS;
	pscr->msg(&lcdmsg);
	return 0;
}

/**/
static rt_err_t rtc_open( rt_device_t dev, rt_uint16_t oflag )
{
	return RT_EOK;
}

/**/
static rt_size_t rtc_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	return RT_EOK;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
static rt_size_t rtc_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	rt_size_t ret = RT_EOK;
	return ret;
}

/**/
static rt_err_t rtc_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	uint32_t	code = *(uint32_t*)arg;
	int			i;
	switch( cmd )
	{
	}
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:在此给打印机断电
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t rtc_close( rt_device_t dev )
{
	return RT_EOK;
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
void rtc_driver_init( void )
{
	dev_rtc.type		= RT_Device_Class_RTC;
	dev_rtc.init		= rtc_init;
	dev_rtc.open		= rtc_open;
	dev_rtc.close		= rtc_close;
	dev_rtc.read		= rtc_read;
	dev_rtc.write		= rtc_write;
	dev_rtc.control		= rtc_control;
	dev_rtc.user_data	= RT_NULL;

	rt_device_register( &dev_rtc, "rtc", RT_DEVICE_FLAG_RDWR );
	rt_device_init( &dev_rtc );
}

/************************************** The End Of File **************************************/
