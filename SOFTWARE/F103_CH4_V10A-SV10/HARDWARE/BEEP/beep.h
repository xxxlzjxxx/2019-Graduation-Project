#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define BEEP                PBout(10)	
#define CTL_MQ4_EN          PBout(15)	
#define CTL_GPRS_EN         PBout(14)	
#define CTL_GPRS_PWRKEY     PBout(13)	

void BEEP_Init(void);//��ʼ��
		 				    
#endif
