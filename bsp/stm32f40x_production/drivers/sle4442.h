/*SLE4442 初始化硬件*/
#ifndef _SLE4442_H_
#define _SLE4442_H_

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>


#define  C_50Ms     1638 //50ms时钟中断

#define  TRUE   1
#define  FALSE  0

extern uint8_t IC_Card_Checked;

void Init_4442(void);
void CheckICCard(void);

#endif

