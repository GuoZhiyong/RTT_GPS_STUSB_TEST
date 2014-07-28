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
#ifndef _H_COMMON_
#define _H_COMMON_

#include "stm32f4xx.h"


/*���Ա�־��
   bit 0		RTC����
   bit 1      IC��
   bit 2        ��
   bit 3      ��ӡ
   bit 4      CAM1
   bit 5      CAM2
   bit 6      CAM3
   bit 7      CAM4
   bit 8		GPS
   bit 8		GPRS
 */
#define TEST_BIT_RTC	0x00000001
#define TEST_BIT_ICCARD 0x00000002
#define TEST_BIT_MEMS	0x00000004
#define TEST_BIT_GPS	0x00000008
#define TEST_BIT_GPRS	0x00000010
#define TEST_BIT_DF		0x00000020
#define TEST_BIT_CAM	0x00000040

#define TEST_BIT_ALL 0x0000007F





/*

   #define TEST_BIT_CAM1_PASS		0x80000000
   #define TEST_BIT_CAM1_FAILED	0x40000000

   #define TEST_BIT_CAM2_PASS		0x20000000
   #define TEST_BIT_CAM2_FAILED	0x10000000

   #define TEST_BIT_CAM3_PASS		0x08000000
   #define TEST_BIT_CAM3_FAILED	0x04000000

   #define TEST_BIT_CAM4_PASS		0x02000000
   #define TEST_BIT_CAM4_FAILED	0x01000000
 */

extern uint8_t	rtc_ok;
extern uint32_t rtc_retry;
extern int8_t	gsm_csq;
extern uint32_t gprs_ok_past_sec;
extern char		gsm_ver[8];
extern uint8_t	mems_status;

extern uint16_t test_df_count;
extern uint32_t test_df_error;
extern uint16_t test_df_error_sector[32];

extern uint32_t test_flag;
extern uint8_t	test_cam_flag[4];

extern uint8_t	ctrlbit_printer_3v3_on;
extern uint8_t	ctrlbit_buzzer;

extern uint32_t mems_alarm_tick;

extern uint8_t	iccard_status;
extern uint8_t			iccard_value;

extern uint32_t iccard_beep_timeout;

extern uint32_t Frequency;

extern uint32_t 	print_testresult_count;

extern uint16_t bd_model;


void ad_init( void );


void pulse_init( void );


#endif

/************************************** The End Of File **************************************/
