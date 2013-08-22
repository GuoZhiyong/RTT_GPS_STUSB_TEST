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

static uint32_t keystatus = 0;

static uint32_t keycheck	= 0;
static uint32_t keycount	= 0;

/*显示状态信息*/
static void showinfo( void )
{
	char buf[32];
	if( keycount & 0x01 )
	{
		lcd_fill( 0xFF );
	}else
	{
		lcd_fill( 0 );
	}
	if( keystatus == 0 )
	{
		lcd_text12( 0, 0, "显示按键测试", 12, LCD_MODE_SET );
		if( keycheck == 0x0F )
		{
			pscr = &scr_1_idle;
			pscr->show( (void*)0 );
		}
		if(keycheck==0xFF)
		{
			lcd_text12( 0, 0, "未读到有效按键", 16, LCD_MODE_INVERT );
		}
	}else if( ( keystatus == KEY_MENU_PRESS ) || ( keystatus == KEY_MENU_REPEAT ) )
	{
		keycheck |= KEY_MENU_PRESS;
	}else if( ( keystatus == KEY_OK_PRESS ) || ( keystatus == KEY_OK_REPEAT ) )
	{
		keycheck |= KEY_OK_PRESS;
	}else if( ( keystatus == KEY_UP_PRESS ) || ( keystatus == KEY_UP_REPEAT ) )
	{
		keycheck |= KEY_UP_PRESS;
	}else if( ( keystatus == KEY_DOWN_PRESS ) || ( keystatus == KEY_DOWN_REPEAT ) )
	{
		keycheck |= KEY_DOWN_PRESS;
	}else
	{
		keycheck=0xFF;
	}
	keycount++;

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
	if( key ^ keystatus ) /*按键不同*/
	{
		keystatus = key;
		showinfo( );
	}
	if( key == KEY_MENU_REPEAT )
	{
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
