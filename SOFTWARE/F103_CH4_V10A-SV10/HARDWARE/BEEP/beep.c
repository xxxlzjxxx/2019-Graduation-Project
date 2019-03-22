/*******************************************************************************
* @file     beep.c                                                               
* @brief    蜂鸣器驱动                                                 
* @author                                                                                                          *
* @version                                                                
* @date     2019.03.20                                                        
* @license  GNU General Public License (GPL)                                                                                                               
*******************************************************************************/
#include "beep.h"

void BEEP_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化
	GPIO_ResetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);						 //PB10输出高
}

 
