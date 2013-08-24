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

static uint32_t keyvalue = 0;

static uint32_t keycheck	= 0;
static uint32_t keycount	= 0;
static uint8_t	key_err		= 0;    /*û�д���*/

/*��ʾ״̬��Ϣ*/
static void showinfo( void )
{
	char buf[5] = { '0', '0', '0', '0', 0 };

	if( keyvalue == 0 )             /*��̧��*/
	{
		lcd_fill(0);
		lcd_text12( 0, 0, "Һ����������", 12, LCD_MODE_SET );
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
		
		if( keycheck == 0x0F )      /*�ĸ�������*/
		{
			pscr = &scr_1_idle;
			pscr->show( (void*)0 );
		}
		if( key_err )
		{
			lcd_text12( 0, 0, "δ������Ч����", 16, LCD_MODE_INVERT );
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
		key_err = keyvalue; /*��¼�����ֵ*/
	}
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
	if( key ^ keyvalue ) /*������ͬ*/
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
