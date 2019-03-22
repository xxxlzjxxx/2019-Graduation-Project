/*******************************************************************************
* @file     beep.c                                                               
* @brief    ����������                                                 
* @author                                                                                                          *
* @version                                                                
* @date     2019.03.20                                                        
* @license  GNU General Public License (GPL)                                                                                                               
*******************************************************************************/
#include "beep.h"

void BEEP_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��
	GPIO_ResetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);						 //PB10�����
}

 