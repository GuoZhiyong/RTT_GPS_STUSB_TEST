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

static uint32_t keyvalue = 0;

static uint32_t keycheck	= 0;
static uint32_t keycount	= 0;
static uint8_t	key_err		= 0;    /*没有错误*/

/*显示状态信息*/
static void showinfo( void )
{
	char buf[5] = { '0', '0', '0', '0', 0 };

	if( keyvalue == 0 )             /*键抬起*/
	{
		lcd_fill(0);
		lcd_text12( 0, 0, "液晶按键测试", 12, LCD_MODE_SET );
		if( keycheck & 0x08 )
		{
			buf[0] = '1';
		}
		if( keycheck & 0x04 )
		{
			buf[1] = '1';
		}
		if( keycheck & 0x02 )
		{
			buf[2] = '1';
		}
		if( keycheck & 0x01 )
		{
			buf[3] = '1';
		}
		lcd_text12( 48, 12, buf, 4, LCD_MODE_SET );
		
		if( keycheck == 0x0F )      /*四个键按下*/
		{
			pscr = &scr_1_idle;
			pscr->show( (void*)0 );
		}
		if( key_err )
		{
			lcd_text12( 0, 0, "未读到有效按键", 16, LCD_MODE_INVERT );
			if( key_err & 0x08 )
			{
				buf[0] = '1';
			}
			if( key_err & 0x04 )
			{
				buf[1] = '1';
			}
			if( key_err & 0x02 )
			{
				buf[2] = '1';
			}
			if( key_err & 0x01 )
			{
				buf[3] = '1';
			}
			lcd_text12( 48, 12, buf, 4, LCD_MODE_SET );
		}
		lcd_update(0,31);
		return;
	}

	if( ( keyvalue == KEY_MENU_PRESS ) || ( keyvalue == KEY_MENU_REPEAT ) )
	{
		keycheck |= KEY_MENU_PRESS;
		if( keyvalue > 0xF )
		{
			lcd_fill( 0xFF );
		}
	}else if( ( keyvalue == KEY_OK_PRESS ) || ( keyvalue == KEY_OK_REPEAT ) )
	{
		keycheck |= KEY_OK_PRESS;
		
		if( keyvalue > 0xF )
		{
			lcd_fill( 0xFF );
		}
	}else if( ( keyvalue == KEY_UP_PRESS ) || ( keyvalue == KEY_UP_REPEAT ) )
	{
		keycheck |= KEY_UP_PRESS;
		
		if( keyvalue > 0xF )
		{
			lcd_fill( 0xFF );
		}
	}else if( ( keyvalue == KEY_DOWN_PRESS ) || ( keyvalue == KEY_DOWN_REPEAT ) )
	{
		keycheck |= KEY_DOWN_PRESS;
		
		if( keyvalue > 0xF )
		{
			lcd_fill( 0xFF );
		}
	}else
	{
		key_err = keyvalue; /*记录错误的值*/
	}
	lcd_update( 0, 31 );
}

/*
   只是显示信息，并没有子菜单
   显示3页的信息 经纬度 定位 时间
 */

static void show( void *parent )
{
	showinfo( );
}

/*按键处理*/
static void keypress( unsigned int key )
{
	if( key ^ keyvalue ) /*按键不同*/
	{
		keyvalue = key;
		showinfo( );
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
static void timetick( unsigned int systick )
{
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
}

SCR scr_0_lcd_key =
{
	&show,
	&keypress,
	&timetick,
	&msg,
};

/************************************** The End Of File **************************************/
