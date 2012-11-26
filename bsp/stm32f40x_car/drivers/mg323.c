/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		mg323
 * Author:			bitter
 * Date:			// ����
 * Description:		// ģ������
 * Version:			// �汾��Ϣ
 * Function List:	// ��Ҫ�������书��
 *     1. -------
 * History:			// ��ʷ�޸ļ�¼
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/

#include <stdio.h>
#include <rtthread.h>
#include "stm32f4xx.h"

#include <finsh.h>

#include <gsm.h>

#ifdef MG323

typedef void ( *URC_CB )( char *s, uint16_t len );

typedef rt_err_t ( *RESP_FUNC )( char *s, uint16_t len );

#define GSM_PWR_PORT	GPIOB
#define GSM_PWR_PIN		GPIO_Pin_15 //PB.15

#define GSM_TERMON_PORT GPIOB
#define GSM_TERMON_PIN	GPIO_Pin_8  //PB.8

#define GSM_RST_PORT	GPIOB
#define GSM_RST_PIN		GPIO_Pin_9  //PB.9

/*����һ��gsm�豸*/
static struct rt_device dev_gsm;


/*����һ��uart�豸ָ��,ͬgsmģ�����ӵĴ���
   ָ��һ���Ѿ��򿪵Ĵ���
 */
static rt_device_t			dev_uart;

static struct rt_timer		tmr_gsm;
static struct rt_semaphore	sem_uart;

static struct rt_semaphore	sem_gsmrx;

#define GSM_RX_SIZE 2048
static uint8_t		gsm_rx[GSM_RX_SIZE];
static uint16_t		gsm_rx_wr = 0;

static T_GSM_STATE	gsmstate = GSM_IDLE;

static uint32_t		lastticks		= 0;
static uint32_t		action_timeout	= 0;

static rt_tick_t	last_state_ticks;   /*��һ��״̬��tickֵ*/

static rt_thread_t	tid_gsm_subthread = RT_NULL;

static uint8_t		fConnectToGprs = 0; /*�Ƿ����ӵ�����ģʽ*/

/* ���彨����socket*/

T_GSM_APN		gsm_apn;
T_GSM_SOCKET	gsm_socket[MAX_SOCKETS];


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int ondata_default( uint8_t *pInfo, uint16_t len )
{
	rt_kprintf( "%ld(%d)ondata>", rt_tick_get( ), len );
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
int oncmd_default( uint8_t *pInfo, uint16_t len )
{
	rt_kprintf( "%ld(%d)oncmd>", rt_tick_get( ), len );
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
int onstatus_default( uint32_t *urc )
{
	rt_kprintf( "%ld onstatus>", rt_tick_get( ) );
	return RT_EOK;
}

T_GSM_OPS gsm_ops_default =
{
	ondata_default,
	oncmd_default,
	onstatus_default,
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
static void urc_cb_default( char *s, uint16_t len )
{
	rt_kprintf( "\rrx>%s", s );
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
static void urc_cb_sysstart( char *s, uint16_t len )
{
	if( gsmstate == GSM_POWERON ) /*�ϵ�������յ�������*/
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
static void urc_cb_shutdown( char *s, uint16_t len )
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
static void urc_cb_ciev( char *s, uint16_t len )
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
static void urc_cb_ring( char *s, uint16_t len )
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
static void urc_cb_cring( char *s, uint16_t len )
{
}

/*
   urc: unsolicited result code
 */
struct
{
	char	*code;
	URC_CB	pfunc;
} urc[] =
{
	{ "^SYSSTART", urc_cb_sysstart },
	{ "^SHUTDOWN", urc_cb_shutdown },
	{ "+CIEV:",	   urc_cb_ciev	   },
	{ "RING",	   urc_cb_ring	   },
	{ "+CRING:",   urc_cb_cring	   },
	{ "+CREG:",	   urc_cb_default  },
	{ "^SIS:",	   urc_cb_default  },
	{ "+CGEV:",	   urc_cb_default  },
	{ "+CGREG:",   urc_cb_default  },
	{ "+CMT:",	   urc_cb_default  },
	{ "+CBM:",	   urc_cb_default  },
	{ "+CDS:",	   urc_cb_default  },
	{ "+CALA:",	   urc_cb_default  },
	{ "CME ERROR", urc_cb_default  },
	{ "CMS ERROR", urc_cb_default  },
	{ "",		   NULL			   }
};


/***********************************************************
* Function:		sys_default_cb
* Description:	ȱʡ��ϵͳ�ص���������
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/

static void sys_default_cb( char *pInfo, uint16_t len )
{
	int		i;
	char	match = 0;

	rt_kprintf( "\rrx>%s", pInfo );
/*�п����������Ҷϣ�Ҳ�п����������ӶϿ����Ƿ�Ҫ֪ͨ? ����Ҫ*/
	if( strncmp( pInfo, "%IPCLOSE", 7 ) == 0 )
	{
	}

//�жϷ���������-�����ϱ�����
	for( i = 0;; i++ )
	{
		if( urc[i].pfunc == NULL )
		{
			break;
		}
		if( strncmp( pInfo, urc[i].code, strlen( urc[i].code ) == 0 ) )
		{
			( urc[i].pfunc )( pInfo, len );
			match = 1; //�Ѵ���
			break;
		}
	}
	if( match )
	{
		return;
	}

//AT����Ľ���������������������������APP������
}

/***********************************************************
* Function:		gsmrx_cb
* Description:	gsm�յ���Ϣ�Ĵ���
* Input:			char *s     ��Ϣ
    uint16_t len ����
* Output:
* Return:
* Others:
***********************************************************/
static void gsmrx_cb( char *pInfo, uint16_t len )
{
	int			i, count;

	uint8_t		tbl[24] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
	uint32_t	linknum, infolen;
	char		c, *pmsg;
	uint8_t		*p;
/*��������Ϣ��ֱ��֪ͨ�ϲ�����*/
	if( strncmp( pInfo, "%IPDATA", 7 ) == 0 )
	{
		i = sscanf( pInfo, "%%IPDATA:%d,%d,", &linknum, &infolen );
		if( i != 2 )
		{
			return; //û����ȷ������������
		}
		pmsg = pInfo + 10;
		while( *pmsg != '"' )
		{
			pmsg++;
		}
		pmsg++;
		/*ֱ����pInfoҲ����GsmRx�ϲ���*/
		p		= pInfo;
		count	= 0;
		while( *pmsg != '"' )
		{
			c = tbl[*pmsg - '0'] << 4;
			pmsg++;
			c |= tbl[*pmsg - '0'];
			pmsg++;
			*p++ = c;
			count++;
			if( count >= infolen )
			{
				break;
			}
		}
		*p	= 0;
		p	= pInfo; //ָ����ʼ��
		( (T_GSM_OPS*)( dev_gsm.user_data ) )->ondata( p, count );
	}

/*�ͷ��յ�һ����Ϣ���ź�,Ҫ���Ǵ���ʱ�����������*/
	rt_sem_release( &sem_gsmrx );
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
static uint16_t stripstring( char *from, char *to )
{
	char		*psrc, *pdst;
	uint16_t	len = 0;
	psrc	= from;
	pdst	= to;
	while( *psrc )
	{
		if( *psrc > 0x20 )
		{
			*pdst++ = toupper( *psrc );
			len++;
		}
		psrc++;
	}
	*pdst = 0;
	return len;
}

/***********************************************************
* Function:
* Description:	���յ�serial����������ָʾ
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t mg323_rx_ind( rt_device_t dev, rt_size_t size )
{
	rt_sem_release( &sem_uart );
	return RT_EOK;
}

/***********************************************************
* Function:
* Description: ���ÿص�ܽţ����Ҳ���gsm��Ӧ�Ĵ����豸
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t mg323_init( rt_device_t dev )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GSM_PWR_PIN;
	GPIO_Init( GSM_PWR_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( GSM_PWR_PORT, GSM_PWR_PIN );

	GPIO_InitStructure.GPIO_Pin = GSM_TERMON_PIN;
	GPIO_Init( GSM_TERMON_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );

	dev_uart = rt_device_find( GSM_UART_NAME );
	if( dev_uart != RT_NULL && rt_device_open( dev_uart, RT_DEVICE_OFLAG_RDWR ) == RT_EOK )
	{
		rt_device_set_rx_indicate( dev_uart, mg323_rx_ind );
	}else
	{
		rt_kprintf( "GSM: can not find uart\n" );
		return RT_EEMPTY;
	}
	return RT_EOK;
}

/***********************************************************
* Function:	�ṩ������thread���ã����豸����ʱ�ж�
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t mg323_open( rt_device_t dev, rt_uint16_t oflag )
{
	if( gsmstate == GSM_IDLE )
	{
		gsmstate = GSM_POWERON; //��λ�ϵ������
	}
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
static rt_err_t mg323_close( rt_device_t dev )
{
	gsmstate = GSM_POWEROFF; //��λ�ϵ������
	return RT_EOK;
}

/***********************************************************
* Function:mg323_read
* Description:����ģʽ�¶�ȡ����
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_size_t mg323_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	return RT_EOK;
}

/***********************************************************
* Function:		mg323_write
* Description:	����ģʽ�·������ݣ�Ҫ�����ݽ��з�װ
* Input:		const void* buff	Ҫ���͵�ԭʼ����
       rt_size_t count	Ҫ�������ݵĳ���
       rt_off_t pos		ʹ�õ�socket���
* Output:
* Return:
* Others:
***********************************************************/

static rt_size_t mg323_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	rt_size_t ret = RT_EOK;

	//rt_device_write( dev_uart, 0, buff, count );
	return ret;
}

/***********************************************************
* Function:		mg323_control
* Description:	����ģ��
* Input:		rt_uint8_t cmd	��������
    void *arg       ����,����cmd�Ĳ�ͬ�����ݵ����ݸ�ʽ��ͬ
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t mg323_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	switch( cmd )
	{
		case CTL_STATUS:
			break;
		case CTL_AT_CMD: //����at����,���Ҫ����
			break;
		case CTL_PPP:
			break;
		case CTL_SOCKET:
			break;
	}
	return RT_EOK;
}

/*
   +CREG:0,1
   +CGREG:0,5
 */
rt_err_t RespFunc_CGREG( char *p, uint16_t len )
{
	uint32_t	i, n, code;
	rt_err_t	res = RT_EOK;
	i = sscanf( p, "%*[^:]:%d,%d", &n, &code );
	if( i != 2 )
	{
		return RT_ERROR;
	}
	if( ( code != 1 ) && ( code != 5 ) )
	{
		res = RT_ERROR;
	}
	return res;
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
rt_err_t RespFunc_COPS( char *p, uint16_t len )
{
/*
   char *oper=(char*)0;
   oper=strstr(p,"CHN-CUGSM");
   if(oper){
   strcpy((char*)sys_param.apn,"UNINET");
   return AT_RESP_OK;
   }
   oper=strstr(p,"CHINAMOBILE");
   if(oper){
   strcpy((char*)sys_param.apn,"CMNET");
   return AT_RESP_OK;
   }
 */
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
rt_err_t RespFunc_CIFSR( char *p, uint16_t len )
{
	uint8_t		i	= 0;
	rt_err_t	res = RT_ERROR;
	int32_t		PARAM_LocalIP[4];
	i = sscanf( p, "%u.%d.%d.%d",
	            &( PARAM_LocalIP[0] ),
	            &( PARAM_LocalIP[1] ),
	            &( PARAM_LocalIP[2] ),
	            &( PARAM_LocalIP[3] ) );
	if( i == 4 )
	{
		res = RT_EOK;
	}
	return res;
}

/*
   SIM����IMSI����Ϊ4600 00783208249��
      460 00 18 23 20 86 42

   �ӿں������ֶε�����ΪIMSI����ĺ�12λ
   ��6���ֽڵ�����Ϊ 0x00 0x07 0x83 0x20 0x82 0x49

 */
rt_err_t RespFunc_CIMI( char *p, uint16_t len )
{
	rt_err_t	res = RT_ERROR;
	char		*pimsi, i;
	if( strlen( p ) < 15 )
	{
		return RT_ERROR;
	}
	if( strncmp( p, "460", 3 ) == 0 )
	{
		//strncpy(sys_param.imsi,p,15);
		res = RT_EOK;
	}
	return res;
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
rt_err_t RespFunc_CPIN( char *p, uint16_t len )
{
	rt_err_t	res = RT_ERROR;
	char		*pimsi, i;
	if( strlen( p ) < 15 )
	{
		return RT_ERROR;
	}
	if( strncmp( p, "460", 3 ) == 0 )
	{
		//strncpy(sys_param.imsi,p,15);
		res = RT_EOK;
	}
	return res;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:		+CSQ: 31, 99
* Return:
* Others:
***********************************************************/
rt_err_t RespFunc_CSQ( char *p, uint16_t len )
{
	rt_err_t	res = RT_ERROR;
	uint32_t	i, n, code;

	i = sscanf( p, "+CSQ%*[^:]:%d,%d", &n, &code );
	if(i!=2) return RT_ERROR;
	return RT_EOK;
}

/*�ȴ��̶���Ϣ�ķ���*/
rt_err_t WaitResp( char *resp, rt_tick_t ticks )
{
	rt_tick_t	start		= rt_tick_get( );
	rt_tick_t	localticks	= ticks;
	char		*p;
	rt_err_t	ret;

/*�ȴ��յ���Ϣ*/
lbl_waitresp_again:
	ret = rt_sem_take( &sem_gsmrx, localticks );
	if( ret == -RT_ETIMEOUT )
	{
		return ret;                                 //��ʱ�˳���û�����ݻ�����������
	}
	p = strstr( gsm_rx, resp );
	if( p )
	{
		return RT_EOK;                              /*�ҵ���*/
	}
	/*û�ҵ��������ȣ������Ϣ��δ���?���������ŵ�֪ͨ��URC*/
	localticks = start + ticks - rt_tick_get( );    /*����ʣ���ʱ��*/
	if( localticks > 1 )
	{
		goto lbl_waitresp_again;
	}
	return RT_ETIMEOUT;
}

/*
   ���ָ��ʱ���ڵķ������
   ������ר�ŵĺ����������ؽ��
 */
int8_t CheckResp( uint32_t ticks, RESP_FUNC resp_func )
{
	rt_tick_t	start		= rt_tick_get( );
	rt_tick_t	localticks	= ticks;
	rt_err_t	ret			= RT_EOK;
/*�ȴ��յ���Ϣ*/
lbl_checkresp_again:
	ret = rt_sem_take( &sem_gsmrx, localticks );
	if( ret == -RT_ETIMEOUT )
	{
		return ret;                      //��ʱ�˳���û�����ݻ�����������
	}
	/*�յ���Ϣ��Ҫ�ж��ǲ�����Ҫ��*/
	ret = resp_func( gsm_rx, gsm_rx_wr );
	if( ret == RT_EOK )
	{
		return RT_EOK;
	}
	localticks = start + ticks - rt_tick_get( ); /*����ʣ���ʱ��*/
	if( localticks > 1 )
	{
		goto lbl_checkresp_again;
	}
	return RT_ETIMEOUT;
}

/***********************************************************
* Function:		SendATCmdWaitRespStr
* Description:	��������ȴ�����ָ�����ַ���
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int8_t SendATCmdWaitRespStr( char *atcmd,
                             uint32_t ticks,
                             char * respstring,
                             uint8_t no_of_attempts )
{
	rt_err_t	ret_val = RT_ERROR;
	uint8_t		i;

	for( i = 0; i < no_of_attempts; i++ )
	{
		mg323_write( dev_uart, 0, atcmd, strlen( atcmd ) );
		ret_val = WaitResp( respstring, ticks );
		if( ret_val == RT_EOK )
		{
			break;
		}
	}
	return ( ret_val );
}

/***********************************************************
* Function:		SendATCmdWaitRespFunc
* Description:	��������������ؽ������ָ���Ĵ�������
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int8_t SendATCmdWaitRespFunc( char *atcmd,
                              uint32_t ticks,
                              RESP_FUNC respfunc,
                              uint8_t no_of_attempts )
{
	int8_t	ret_val = RT_ERROR;
	uint8_t i;

	for( i = 0; i < no_of_attempts; i++ )
	{
		//printf("\r\n%d(waitrespfunc)>%s",OSTimeGet(),AT_cmd_string);
		mg323_write( dev_uart, 0, atcmd, strlen( atcmd ) );
		ret_val = CheckResp( ticks, respfunc );
		if( ret_val == RT_EOK )
		{
			break;
		}
	}
	return ( ret_val );
}

/***********************************************************
* Function:
* Description:	ģ���ϵ粢���AT�����ʼ��
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_gsm_poweron( void* parameter )
{
	rt_err_t res;

/*�˴���ͨ��rt_thread_self��ȡ�߳���Ϣ*/
lbl_start_pwr_on:
	rt_kprintf( "\r\n%ld>gsm pwr on start", rt_tick_get( ) );
	GPIO_SetBits( GSM_PWR_PORT, GSM_PWR_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND / 2 );  //500ms
	GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND / 10 ); //100ms
	GPIO_ResetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	rt_kprintf( "\r\n%ld>gsm pwr on end", rt_tick_get( ) );

	res = WaitResp( "^SYSSTART", RT_TICK_PER_SECOND * 10 );
	if( res != RT_EOK )
	{
		rt_kprintf( "\r\n%d>re gsm pwron\r\n", rt_tick_get( ) );
		goto lbl_start_pwr_on;
	}
	res = SendATCmdWaitRespStr( "AT\r\n", 3 * RT_TICK_PER_SECOND, "OK", 3 );
	res = SendATCmdWaitRespStr( "ATE0\r\n", 3 * RT_TICK_PER_SECOND, "OK", 3 );
	res = SendATCmdWaitRespStr( "ATV1\r\n", 3 * RT_TICK_PER_SECOND, "OK", 3 );
	res = SendATCmdWaitRespFunc( "AT+CPIN?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CPIN, 3 );
	if( res != RT_EOK )
	{
		goto lbl_start_pwr_on;
	}
	res			= SendATCmdWaitRespFunc( "AT+COPS?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_COPS, 10 );
	res			= SendATCmdWaitRespFunc( "AT+CREG?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CGREG, 10 );
	res			= SendATCmdWaitRespFunc( "AT+CIMI\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CIMI, 3 );
	res			= SendATCmdWaitRespFunc( "AT+CGREG?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CGREG, 10 );
	res			= SendATCmdWaitRespStr( "AT+CGATT?\r\n", 3 * RT_TICK_PER_SECOND, "+CGATT:1", 10 );
	gsmstate	= GSM_AT; /*�л���AT״̬*/
}

/***********************************************************
* Function:	rt_thread_entry_gsm_poweroff
* Description: �Ҷϵ�ǰ���ӣ����ر�ģ��
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_gsm_poweroff( void* parameter )
{
	rt_err_t res;

/*�˴���ͨ��rt_thread_self��ȡ�߳���Ϣ*/

/*�ر�����*/

/*ģ��ϵ�*/
lbl_start_pwr_off:

	rt_kprintf( "\r\n%ld>gsm pwr off start", rt_tick_get( ) );
	GPIO_SetBits( GSM_PWR_PORT, GSM_PWR_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND / 2 );  //500ms
	GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND / 10 ); //100ms
	GPIO_ResetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );

	res = WaitResp( "^SHUTDOWN", RT_TICK_PER_SECOND * 10 );
	if( res != RT_EOK )
	{
		rt_kprintf( "\r\n%d>re gsm pwron\r\n", rt_tick_get( ) );
		goto lbl_start_pwr_off;
	}

	rt_kprintf( "\r\n%ld>gsm pwr off end", rt_tick_get( ) );
}

/***********************************************************
* Function:	rt_thread_entry_gsm_ppp
* Description: ��������
   ����ֻ�����ض������ӣ�Ҳ���Խ���ȫ��������
* Input:		void* parameter
   ���������Ӻ�  0:ȫ��  ��δʵ��-�����Ƿ�Ҫȡ��
        ��0:�ض������Ӻ�
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_gsm_ppp( void* parameter )
{
	rt_err_t	ret;
	int			i;
	uint8_t		linkid = *(char*)parameter;
	char		buf[100];
	uint8_t		linkid_from = 0, linkid_to = MAX_SOCKETS;
	SendATCmdWaitRespFunc( "AT+COPS?", 20, RespFunc_COPS, 3 );

	sprintf( buf, "AT^SICS=%d,conType,GPRS0\r\n", i );
	SendATCmdWaitRespStr( buf, RT_TICK_PER_SECOND, "OK", 1 );
	sprintf( buf, "AT^SICS=%d,passwd,%s\r\n", i, gsm_apn.password );
	SendATCmdWaitRespStr( buf, RT_TICK_PER_SECOND, "OK", 1 );

	sprintf( buf, "AT^SICS=%d,user,%s\r\n", i, gsm_apn.user );
	SendATCmdWaitRespStr( buf, RT_TICK_PER_SECOND, "OK", 1 );

	sprintf( buf, "AT^SICS=%d,apn,%s\r\n", i, gsm_apn.apn );
	SendATCmdWaitRespStr( buf, RT_TICK_PER_SECOND, "OK", 1 );

	sprintf( buf, "AT^SISS=%d,srvType,socket\r\n", i );
	SendATCmdWaitRespStr( buf, RT_TICK_PER_SECOND, "OK", 1 );

	sprintf( buf, "AT^SISS=%d,srvType,socket\r\n", i );
	SendATCmdWaitRespStr( buf, RT_TICK_PER_SECOND, "OK", 1 );

	gsmstate = GSM_DATA;
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
static void rt_thread_entry_gsm_send( void* parameter )
{
/*�˴���ͨ��rt_thread_self��ȡ�߳���Ϣ*/



}

ALIGN( RT_ALIGN_SIZE )
static char thread_gsm_stack[2048];
struct rt_thread thread_gsm;


/***********************************************************
* Function:       rt_thread_entry_gsm
* Description:    ���մ�����״̬ת��
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_gsm( void* parameter )
{
	rt_tick_t		curr_ticks;
	rt_err_t		res;
	unsigned char	ch, next;

/*gsm��״̬�л�*/
	curr_ticks = rt_tick_get( );
	switch( gsmstate )
	{
		case GSM_IDLE:
			break;
		case GSM_POWERON:                                   /*�ϵ�����У�����IO�Ĳ������ڴ˴�����α�֤ʱ��Ŀɿ�*/
			if( tid_gsm_subthread == RT_NULL )
			{
				tid_gsm_subthread = rt_thread_create( "pwron",
				                                      rt_thread_entry_gsm_poweron, (void*)1,
				                                      512,  /* thread stack size */
				                                      25,   /* thread priority */
				                                      5     /* thread time slice*/
				                                      );
				if( tid_gsm_subthread != RT_NULL )
				{
					rt_thread_startup( tid_gsm_subthread );
				}
			}
			break;
		case GSM_POWEROFF:                                  /*�ϵ������У�����IO�Ĳ������ڴ˴�����α�֤ʱ��Ŀɿ�*/
			if( tid_gsm_subthread == RT_NULL )
			{
				tid_gsm_subthread = rt_thread_create( "pwroff",
				                                      rt_thread_entry_gsm_poweroff, (void*)1,
				                                      512,  /* thread stack size */
				                                      25,   /* thread priority */
				                                      5     /* thread time slice*/
				                                      );
				if( tid_gsm_subthread != RT_NULL )
				{
					rt_thread_startup( tid_gsm_subthread );
				}
			}
			break;
		case GSM_AT:
			if( fConnectToGprs )
			{
				gsmstate = GSM_PPP; /*�л�����*/
			}
			break;
		case GSM_PPP:
			if( tid_gsm_subthread == RT_NULL )
			{
				tid_gsm_subthread = rt_thread_create( "ppp",
				                                      rt_thread_entry_gsm_ppp, (void*)1,
				                                      512,  /* thread stack size */
				                                      25,   /* thread priority */
				                                      5     /* thread time slice*/
				                                      );
				if( tid_gsm_subthread != RT_NULL )
				{
					rt_thread_startup( tid_gsm_subthread );
				}
			}
			break;
		case GSM_DATA:
			break;
	}

/*���ճ�ʱ�ж�*/
	res = rt_sem_take( &sem_uart, RT_TICK_PER_SECOND / 10 );    //�ȴ�100ms,ʵ���Ͼ��Ǳ䳤����ʱ,�100ms
	if( res == -RT_ETIMEOUT )                                   //��ʱ�˳���û�����ݻ�����������
	{
		if( gsm_rx_wr )
		{
			gsmrx_cb( gsm_rx, gsm_rx_wr );
			gsm_rx_wr = 0;
		}
	}else //�յ�����,������1���ֽڴ���һ��,����û�и����յ���<CRLF>���������ǵȵ���ʱͳһ����
	{
		while( rt_device_read( dev_uart, 0, &ch, 1 ) == 1 )
		{
			gsm_rx[gsm_rx_wr++] = ch;
			if( gsm_rx_wr == GSM_RX_SIZE )
			{
				gsm_rx_wr = 0;
			}
			gsm_rx[gsm_rx_wr] = 0;
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
void gsm_init( T_GSM_OPS *gsm_ops )
{
	rt_thread_t tid;

	rt_sem_init( &sem_uart, "sem_uart", 0, 0 );
	rt_sem_init( &sem_gsmrx, "sem_gsmrx", 0, 0 );

	rt_thread_init( &thread_gsm,
	                "gsm",
	                rt_thread_entry_gsm,
	                RT_NULL,
	                &thread_gsm_stack[0],
	                sizeof( thread_gsm_stack ), 7, 5 );
	rt_thread_startup( &thread_gsm );


/*
   rt_timer_init( &tmr_gsm, \
                "tmr_gsm", \
                timer_gsm_cb, NULL, \
                50, \
                RT_TIMER_FLAG_PERIODIC );
 */
	dev_gsm.type	= RT_Device_Class_Char;
	dev_gsm.init	= mg323_init;
	dev_gsm.open	= mg323_open;
	dev_gsm.close	= mg323_close;
	dev_gsm.read	= mg323_read;
	dev_gsm.write	= mg323_write;
	dev_gsm.control = mg323_control;

	dev_gsm.user_data = &gsm_ops_default;
	if( gsm_ops != NULL )
	{
		dev_gsm.user_data = gsm_ops;
	}

	rt_device_register( &dev_gsm, "gsm", RT_DEVICE_FLAG_RDWR );
	rt_device_init( &dev_gsm );
	rt_timer_start( &tmr_gsm );
}

#ifdef TEST_GSM


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t gsm_open( void )
{
	return mg323_open( &dev_gsm, RT_DEVICE_OFLAG_RDWR );
}

FINSH_FUNCTION_EXPORT( gsm_open, open gsm );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t gsm_close( void )
{
	return mg323_close( &dev_gsm );
}

FINSH_FUNCTION_EXPORT( gsm_close, close gsm );

/*�������ӵ�socket����*/
rt_err_t gsm_control_socket( uint8_t linkno, char type, char *ip, uint32_t port )
{
	return mg323_control( &dev_gsm, 0, NULL );
}

FINSH_FUNCTION_EXPORT( gsm_control_socket, control gsm );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_size_t gsm_write( char *sinfo )
{
	return mg323_write( &dev_gsm, 0, sinfo, strlen( sinfo ) );
}

FINSH_FUNCTION_EXPORT( gsm_write, write gsm );

#endif

#endif

/************************************** The End Of File **************************************/