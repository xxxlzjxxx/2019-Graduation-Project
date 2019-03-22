/*******************************************************************************
* @file     main.c                                                               
* @brief    摘要                                                  
* @author   作者                                                                       
* @version  版本                                                              
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
u16 CH4_WarningValue = 1000;        //甲烷浓度报警阈值
#define ADC_BatChannel 10
#define ADC_UsbChannel 11
#define ADC_MQ4Channel 12
/******************************************************************************/
/*******************************************************************************
* @param		OLED_ShowLogo
* @brief		摘要
* @arg			列表说明参数
* @return       返回值说明
* @retval		返回值类型说明
* @see			参看
* @attention	注意
* @note			注解	
* @author   	作者                                                                                                                                                                   
* @date     	2019.03.20            				
*******************************************************************************/
void OLED_ShowLogo(void){
    OLED_ShowString(0,0, "Gas-Alarm-System",16);
    OLED_ShowString(0,16,"ADC:      ",12);
    OLED_ShowString(0,28,"VRL:      mV",12);
    OLED_ShowString(0,40,"PPM:      ppm",12);
	OLED_Refresh_Gram();//更新显示到OLED    
}
/*******************************************************************************
* @param		OLED_ShowLogo
* @brief		摘要
* @arg			列表说明参数
* @return       返回值说明
* @retval		返回值类型说明
* @see			参看
* @attention	注意
* @note			注解	
* @author   	作者                                                                                                                                                              
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
        AdcValue = (AdcValue - 180) * 200 / 100; //天然气检测，电压升高100mV，浓度增加200ppm。仅供参考
    }
    OLED_ShowNum(25,40,AdcValue,12,5);
	OLED_Refresh_Gram();//更新显示到OLED   
    //甲烷浓度判断，超过预设阈值就开始报警
    if(AdcValue >= CH4_WarningValue){
        TIM3_Int_Init(1000-1,7200-1);
        printf("!!!报警短信发送中!!!\r\n");
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
* @brief		摘要
* @arg			列表说明参数
* @return       返回值说明
* @retval		返回值类型说明
* @see			参看
* @attention	注意
* @note			注解	
* @author   	作者                                                                                                                                                                
* @date     	2019.03.20            				
*******************************************************************************/
void SIM900A_Init(void){
    CTL_GPRS_EN = 0;
    CTL_GPRS_PWRKEY = 0;
    delay_ms(200);
}
/*******************************************************************************
* @param		main
* @brief		摘要
* @arg			列表说明参数
* @return       返回值说明
* @retval		返回值类型说明
* @see			参看
* @attention	注意
* @note			注解	
* @author   	作者                                                                                                                             
* @date     	2019.03.20            				
*******************************************************************************/
int main(void){	
 	u8 t=0;	
 
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //设置中断优先级分组2
    uart_init(115200);	 	//串口初始化为9600
    USART2_Init(115200);	 	//串口初始化为9600
 	LED_Init();
    KEY_Init();
    BEEP_Init();
    TIM3_Int_Init(10000-1,7200-1);//10Khz的计数频率，计数到5000为500ms 
    Adc_Init();
    OLED_Init();
    SIM900A_Init(); 
 	mem_init();				//初始化内存池	    
 	exfuns_init();			//为fatfs相关变量申请内存  
 	f_mount(fs[1],"1:",1); 	//挂载FLASH.
       
    CTL_MQ4_EN = 1;
	OLED_ShowLogo(); 
    printf("*************************************************************\r\n");
    printf(">>                   GAS-ALARM-SYSTEM                        \r\n");
    printf(">>                天然气泄漏远程报警系统                     \r\n");
    printf(">>                            ----覃志强                     \r\n");
    printf(">>                            2019.03.25                     \r\n");
    printf("*************************************************************\r\n");
    
	while(1){	
        OLED_ShowAdc();
        //按下WKUP键，让报警LED、蜂鸣器恢复到正常模式
		t=KEY_Scan(0);		//得到键值
		switch(t){				 
			case WKUP_PRES:				
                TIM3_Int_Init(10000-1,7200-1);//10Khz的计数频率，计数到5000为500ms
                //关闭GPRS模块
                CTL_GPRS_EN = 0;                
				break;
			default:
				delay_ms(10);	
		} 
	}	  
}


