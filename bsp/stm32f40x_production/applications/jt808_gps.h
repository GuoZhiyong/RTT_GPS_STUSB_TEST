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
#ifndef _JT808_GPS_H_
#define _JT808_GPS_H_

#include <rtthread.h>
#include "stm32f4xx.h"


#define BIT_STATUS_ACC		0x00000001
#define BIT_STATUS_FIXED	0x00000002
#define BIT_STATUS_NS		0x00000004
#define BIT_STATUS_EW		0x00000008  /*������*/
#define BIT_STATUS_SERVICE	0x00000010  /*Ӫ��״̬ 1ͣ��*/
#define BIT_STATUS_ENCRYPT	0x00000020

#define BIT_STATUS_EMPTY	0x00000100
#define BIT_STATUS_FULL		0x00000200

#define BIT_STATUS_OIL		0x00000400  /*��·*/
#define BIT_STATUS_ELEC		0x00000800  /*��·*/
#define BIT_STATUS_DOORLOCK 0x00001000  /*������*/
#define BIT_STATUS_DOOR1	0x00002000
#define BIT_STATUS_DOOR2	0x00004000
#define BIT_STATUS_DOOR3	0x00008000
#define BIT_STATUS_DOOR4	0x00010000
#define BIT_STATUS_DOOR5	0x00020000
#define BIT_STATUS_GPS		0x00040000
#define BIT_STATUS_BD		0x00080000
#define BIT_STATUS_GLONASS	0x00100000
#define BIT_STATUS_GALILEO	0x00200000

#define BIT_ALARM_EMG				0x00000001  /*����*/
#define BIT_ALARM_OVERSPEED			0x00000002  /*����*/
#define BIT_ALARM_OVERTIME			0x00000004  /*��ʱ��ƣ�ͼ�ʻ*/
#define BIT_ALARM_DANGER			0x00000008  /*Σ��Ԥ��*/
#define BIT_ALARM_GPS_ERR			0x00000010  /*GNSSģ�����*/
#define BIT_ALARM_GPS_OPEN			0x00000020  /*���߿�·*/
#define BIT_ALARM_GPS_SHORT			0x00000040  /*���߶�·*/
#define BIT_ALARM_LOW_PWR			0x00000080  /*�ն�����ԴǷѹ*/
#define BIT_ALARM_LOST_PWR			0x00000100  /*�ն�����Դ����*/
#define BIT_ALARM_FAULT_LCD			0x00000200  /*LCD����*/
#define BIT_ALARM_FAULT_TTS			0x00000400  /*TTS����*/
#define BIT_ALARM_FAULT_CAM			0x00000800  /*CAM����*/
#define BIT_ALARM_FAULT_ICCARD		0x00001000  /*IC������*/
#define BIT_ALARM_PRE_OVERSPEED		0x00002000  /*����Ԥ�� bit13*/
#define BIT_ALARM_PRE_OVERTIME		0x00004000  /*��ʱԤ�� bit14*/
#define BIT_ALARM_TODAY_OVERTIME	0x00040000  /*�����ۼƼ�ʻ��ʱ*/
#define BIT_ALARM_STOP_OVERTIME		0x00080000  /*��ʱͣʻ*/
#define BIT_ALARM_VSS				0x01000000  /*VSS����*/
#define BIT_ALARM_OIL				0x02000000  /*�����쳣*/
#define BIT_ALARM_STOLEN			0x04000000  /*����*/
#define BIT_ALARM_IGNITION			0x08000000  /*�Ƿ����*/
#define BIT_ALARM_MOVE				0x10000000  /*�Ƿ���λ*/
#define BIT_ALARM_COLLIDE			0x20000000  /*��ײ*/
#define BIT_ALARM_TILT				0x40000000  /*�෭*/
#define BIT_ALARM_DOOR_OPEN			0x80000000  /*�Ƿ�����*/

/*����λ����Ϣ,��Ϊ�ֽڶ���ķ�ʽ������ʹ�����鷽��*/
typedef __packed struct _gps_baseinfo
{
	uint32_t	alarm;
	uint32_t	status;
	uint32_t	latitude;                       /*γ��*/
	uint32_t	longitude;                      /*����*/
	uint16_t	altitude;
	uint16_t	speed_10x;                      /*�Ե��ٶ� 0.1KMH*/
	uint16_t	cog;                            /*�ԵؽǶ�*/
	uint8_t		datetime[6];                    /*BCD��ʽ*/
}GPS_BASEINFO;

enum BDGPS_MODE
{
	MODE_GET=0,                                 /*��ѯ*/
	MODE_BD =1,
	MODE_GPS,
	MODE_BDGPS,
};

typedef  struct  _gps_status
{
	uint32_t		type;           /*�ͺ� 0:δ�� 0x3020x  0x3017x*/
	enum BDGPS_MODE mode;           /* 1: BD   2:  GPS   3: BD+GPS    ��λģ���״̬*/
	uint8_t			Antenna_Flag;   //��ʾ��ʾ��·
	uint8_t			Raw_Output;     //  ԭʼ�������
	uint8_t			NoSV;
}GPS_STATUS;

extern GPS_STATUS	gps_status;

extern GPS_BASEINFO gps_baseinfo;

extern uint8_t gps_av;
extern uint8_t gps_hour,gps_minute,gps_sec;
extern uint8_t gps_year, gps_month, gps_day;



/*�澯��״̬��Ϣ*/
extern uint32_t jt808_alarm;
extern uint32_t jt808_status;

extern uint32_t gps_sec_count;              /*gps���������*/
extern uint32_t gps_fixed_sec;

void gps_rx( uint8_t * pinfo, uint16_t length );



#endif
/************************************** The End Of File **************************************/