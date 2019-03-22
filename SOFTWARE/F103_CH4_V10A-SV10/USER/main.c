/*******************************************************************************
* @file     main.c                                                               
* @brief    ժҪ                                                  
* @author   ����                                                                       
* @version  �汾                                                              
* @date     2018.03.20                                                           
* @license  GNU General Public License (GPL)                                                                                                               
*******************************************************************************/
#include "led.h"
#include "lcd.h"
#include "timer.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart2.h"
#include "malloc.h" 
#include "oled.h" 
#include "beep.h"
#include "key.h"
#include "adc.h"
#include "MMC_SD.h" 
#include "ff.h"  
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"
#include "sim900a.h"
/******************************************************************************/
u16 CH4_WarningValue = 1000;        //����Ũ�ȱ�����ֵ
#define ADC_BatChannel 10
#define ADC_UsbChannel 11
#define ADC_MQ4Channel 12
/******************************************************************************/
/*******************************************************************************
* @param		OLED_ShowLogo
* @brief		ժҪ
* @arg			�б�˵������
* @return       ����ֵ˵��
* @retval		����ֵ����˵��
* @see			�ο�
* @attention	ע��
* @note			ע��	
* @author   	����                                                                                                                                                                   
* @date     	2019.03.20            				
*******************************************************************************/
void OLED_ShowLogo(void){
    OLED_ShowString(0,0, "Gas-Alarm-System",16);
    OLED_ShowString(0,16,"ADC:      ",12);
    OLED_ShowString(0,28,"VRL:      mV",12);
    OLED_ShowString(0,40,"PPM:      ppm",12);
	OLED_Refresh_Gram();//������ʾ��OLED    
}
/*******************************************************************************
* @param		OLED_ShowLogo
* @brief		ժҪ
* @arg			�б�˵������
* @return       ����ֵ˵��
* @retval		����ֵ����˵��
* @see			�ο�
* @attention	ע��
* @note			ע��	
* @author   	����                                                                                                                                                              
* @date     	2019.03.20            				
*******************************************************************************/
void OLED_ShowAdc(void){
    u16 AdcValue;
    u16 i = 0;
    
    AdcValue = Get_Adc_Average(ADC_MQ4Channel, 5);
    OLED_ShowNum(25,16,AdcValue,12,5);
    AdcValue = 3406 * AdcValue / 4095;
    OLED_ShowNum(25,28,AdcValue,12,5);
    
    if(AdcValue <= 180){
        AdcValue = 0;
    }
    else{
        AdcValue = (AdcValue - 180) * 200 / 100; //��Ȼ����⣬��ѹ����100mV��Ũ������200ppm�������ο�
    }
    OLED_ShowNum(25,40,AdcValue,12,5);
	OLED_Refresh_Gram();//������ʾ��OLED   
    //����Ũ���жϣ�����Ԥ����ֵ�Ϳ�ʼ����
    if(AdcValue >= CH4_WarningValue){
        TIM3_Int_Init(1000-1,7200-1);
        printf("!!!�������ŷ�����!!!\r\n");
        i = 1;
        if(i){
            CTL_GPRS_EN = 1;
            CTL_GPRS_PWRKEY = 1;
            delay_ms(200);
            sim900a_test();
            i = 0;
        }
    }
    delay_ms(100);
}
/*******************************************************************************
* @param		OLED_ShowLogo
* @brief		ժҪ
* @arg			�б�˵������
* @return       ����ֵ˵��
* @retval		����ֵ����˵��
* @see			�ο�
* @attention	ע��
* @note			ע��	
* @author   	����                                                                                                                                                                
* @date     	2019.03.20            				
*******************************************************************************/
void SIM900A_Init(void){
    CTL_GPRS_EN = 0;
    CTL_GPRS_PWRKEY = 0;
    delay_ms(200);
}
/*******************************************************************************
* @param		main
* @brief		ժҪ
* @arg			�б�˵������
* @return       ����ֵ˵��
* @retval		����ֵ����˵��
* @see			�ο�
* @attention	ע��
* @note			ע��	
* @author   	����                                                                                                                             
* @date     	2019.03.20            				
*******************************************************************************/
int main(void){	
 	u8 t=0;	
 
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //�����ж����ȼ�����2
    uart_init(115200);	 	//���ڳ�ʼ��Ϊ9600
    USART2_Init(115200);	 	//���ڳ�ʼ��Ϊ9600
 	LED_Init();
    KEY_Init();
    BEEP_Init();
    TIM3_Int_Init(10000-1,7200-1);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms 
    Adc_Init();
    OLED_Init();
    SIM900A_Init(); 
 	mem_init();				//��ʼ���ڴ��	    
 	exfuns_init();			//Ϊfatfs��ر��������ڴ�  
 	f_mount(fs[1],"1:",1); 	//����FLASH.
       
    CTL_MQ4_EN = 1;
	OLED_ShowLogo(); 
    printf("*************************************************************\r\n");
    printf(">>                   GAS-ALARM-SYSTEM                        \r\n");
    printf(">>                ��Ȼ��й©Զ�̱���ϵͳ                     \r\n");
    printf(">>                            ----��־ǿ                     \r\n");
    printf(">>                            2019.03.25                     \r\n");
    printf("*************************************************************\r\n");
    
	while(1){	
        OLED_ShowAdc();
        //����WKUP�����ñ���LED���������ָ�������ģʽ
		t=KEY_Scan(0);		//�õ���ֵ
		switch(t){				 
			case WKUP_PRES:				
                TIM3_Int_Init(10000-1,7200-1);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms
                //�ر�GPRSģ��
                CTL_GPRS_EN = 0;                
				break;
			default:
				delay_ms(10);	
		} 
	}	  
}


