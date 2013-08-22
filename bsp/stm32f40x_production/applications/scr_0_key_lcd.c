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

static uint32_t keystatus = 0;

static uint32_t keycheck	= 0;
static uint32_t keycount	= 0;

/*��ʾ״̬��Ϣ*/
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
		lcd_text12( 0, 0, "��ʾ��������", 12, LCD_MODE_SET );
		if( keycheck == 0x0F )
		{
			pscr = &scr_1_idle;
			pscr->show( (void*)0 );
		}
		if(keycheck==0xFF)
		{
			lcd_text12( 0, 0, "δ������Ч����", 16, LCD_MODE_INVERT );
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
   ֻ����ʾ��Ϣ����û���Ӳ˵�
   ��ʾ3ҳ����Ϣ ��γ�� ��λ ʱ��
 */

static void show( void *parent )
{
	showinfo( );
}

/*��������*/
static void keypress( unsigned int key )
{
	if( key ^ keystatus ) /*������ͬ*/
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
