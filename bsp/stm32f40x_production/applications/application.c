/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */


/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>

#include "stm32f4xx.h"
#include <board.h>
#include <rtthread.h>

#include "common.h"


/*
   应该在此处初始化必要的设备和事件集
 */
void rt_init_thread_entry( void* parameter )
{
	uint16_t	sector	= 0;
	__IO uint32_t	addr	= 0;
	uint16_t	i, j;
	uint8_t		buf[256];
	uint8_t		test_df = 1;



	mma8451_driver_init( );
	printer_driver_init( );
	usbh_init( );
	spi_sd_init( );


	
	while( 1 )
	{
		if( rtc_ok == 0 )
		{
			rtc_retry++;
			if( rtc_init( ) == 0 )
			{
				rtc_ok		= 1;
				test_flag	|= TEST_BIT_RTC;
				rt_kprintf( "\r\ntest_flag=%x", test_flag );
			}
		}
		if( test_df == 1 )
		{
			addr = sector * 4096;
			rt_kprintf( "\r\n%d>test df sector:%d", rt_tick_get( ), sector );
			sst25_erase_4k( addr );
			for( i = 0; i < 256; i++ )
			{
				buf[i] = i;
			}
			sst25_write_through( addr, buf, 256 );
			rt_thread_delay( RT_TICK_PER_SECOND );
			for( i = 0; i < 256; i++ )
			{
				buf[i] = 0;
			}
			sst25_read( addr, buf, 256 );
			for( i = 0; i < 256; i++ )
			{
				if( buf[i] != i )
				{
					test_df_error_sector[test_df_error] = sector;
					test_df_error++;
					rt_kprintf( "\r\n%d>DF error index=%d sector=%d", rt_tick_get( ), i, sector );
					break;
				}
			}
			sst25_erase_4k( addr );
			test_df_count++;
			sector += 100;
			if( sector > 1023 )
			{
				test_df		= 0;
				test_flag	|= TEST_BIT_DF;
				rt_kprintf( "\r\ntest_flag=%x", test_flag );
			}
		}
		rt_thread_delay( RT_TICK_PER_SECOND * 2 );
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
int rt_application_init( )
{
	rt_thread_t tid;

	tid = rt_thread_create( "init",
	                        rt_init_thread_entry, RT_NULL,
	                        2048, RT_THREAD_PRIORITY_MAX - 2, 20 );

	if( tid != RT_NULL )
	{
		rt_thread_startup( tid );
	}
	return 0;
}

/*@}*/
