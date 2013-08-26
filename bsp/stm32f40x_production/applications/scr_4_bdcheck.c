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
#include <sed1520.h>
#include "gps.h"

#define BD_IDLE		0                       /*����*/
#define BD_BUSY		1                       /*����������*/
#define BD_COMPLETE 2                       /*�������,���۳ɹ�ʧ��*/
#define BD_ERR		3

static uint8_t		fupgrading	= BD_IDLE;
static rt_thread_t	tid_upgrade = RT_NULL;  /*��ʼ����*/

/*��ʾ�˵�*/
static void menu_disp( void )
{
	uint8_t i;
	lcd_fill( 0 );
	if( fupgrading == BD_ERR )
	{
		lcd_text12( 0, 4, "��������ʧ��", 12, LCD_MODE_SET );
	}
	lcd_update( 0, 31 );
}

/*�����Լ�״̬����Ϣ*/
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
		if( pinfo[0] == 'E' )             /*��������*/
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

/*��������*/
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

/*ϵͳʱ��*/
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

