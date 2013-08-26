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
#include <sed1520.h>
#include "gps.h"

#define BD_IDLE		0                       /*空闲*/
#define BD_BUSY		1                       /*操作过程中*/
#define BD_COMPLETE 2                       /*操作完成,不论成功失败*/
#define BD_ERR		3

static uint8_t		fupgrading	= BD_IDLE;
static rt_thread_t	tid_upgrade = RT_NULL;  /*开始更新*/

/*显示菜单*/
static void menu_disp( void )
{
	uint8_t i;
	lcd_fill( 0 );
	if( fupgrading == BD_ERR )
	{
		lcd_text12( 0, 4, "启动更新失败", 12, LCD_MODE_SET );
	}
	lcd_update( 0, 31 );
}

/*处理自检状态的消息*/
static void msg( void *pmsg )
{
	unsigned int	len;
	char			*pinfo;

	if( fupgrading )
	{
		lcd_fill( 0 );
		pinfo	= (char*)pmsg;
		len		= strlen( pinfo );
		lcd_text12( 0, 10, pinfo + 1, len - 1, LCD_MODE_SET );
		if( pinfo[0] == 'E' )             /*出错或结束*/
		{
			fupgrading	= BD_COMPLETE;
			tid_upgrade = RT_NULL;
		}
		lcd_update( 0, 31 );
	}else
	{
		menu_disp( );
	}
}

/**/
static void show( void *parent )
{
	tid_upgrade = rt_thread_create( "check", thread_gps_check_ver, (void*)msg, 1024, 5, 5 );
	if( tid_upgrade == RT_NULL )
	{
		fupgrading = BD_ERR;
	}else
	{
		fupgrading = BD_BUSY;
		rt_thread_startup( tid_upgrade );
	}
}

/*按键处理*/
static void keypress( unsigned int key )
{
	if( key == 0 )
	{
		return;
	}
	if( ( fupgrading == BD_ERR ) || ( fupgrading == BD_COMPLETE ) )
	{
		pscr = &scr_1_idle;
		pscr->show( RT_NULL );
	}
}

/*系统时间*/
static void timetick( unsigned int systick )
{
}

SCR scr_4_bdcheck =
{
	&show,
	&keypress,
	&timetick,
	&msg,
};

/************************************** The End Of File **************************************/

