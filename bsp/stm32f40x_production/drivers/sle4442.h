/*SLE4442 ��ʼ��Ӳ��*/
#ifndef _SLE4442_H_
#define _SLE4442_H_

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>


#define  C_50Ms     1638 //50msʱ���ж�

#define  TRUE   1
#define  FALSE  0

extern uint8_t IC_Card_Checked;

void Init_4442(void);
void CheckICCard(void);

#endif

