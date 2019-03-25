
#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "rs485.h"
#include "stdio.h"
#include "string.h"

u16 ErrorValue = 20;            //�������ֵ
u16 Measurheight = 0;           //����ֵ
u16 Settingheight = 100;        //�趨ֵ
			 	
int main(void){	 
	u8 key,key1;
	u8 i=0,t=0;
	u8 rs485buf[4];
    u16 num = 0;
    u8 status = 0;          //��ǰ״̬
    u8 sum = 0;             //����У���
	 
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
	LCD_Init();			   	//��ʼ��LCD 	
	KEY_Init();				//������ʼ��		 	 
	RS485_Init(9600);	//��ʼ��RS485
 	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(30,10,280,16,16,"    Non-contact liquid level    ");
	LCD_ShowString(30,30,280,16,16,"measuring and controlling device");	
	LCD_ShowString(30,50,280,16,16,"                      __WenXin__");
	LCD_ShowString(30,70,280,16,16,"                      2019.04.01");		
 
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ	  	
    LCD_ShowString(30,110,280,16,16,"Sensor  Data:      ");	//��ʾ���յ�������
	LCD_ShowString(30,140,280,24,24,"Measure:      mm");	//������������
    LCD_ShowString(30,170,280,24,24,"Setting: 0000 mm");	//��������

    LCD_ShowString(120,200,180,24,24,"<PAUSE>");	//��������    
 									  
	while(1){	 
		RS485_Receive_Data(rs485buf,&key);
        //��⴮�ڽ��յ�������
		if(key)
		{
			if(key>4)key=4;//�����4������.
 			for(i=0;i<key;i++){
                LCD_ShowxNum(138+i*32,110,rs485buf[i],3,16,0X80);	//��ʾ����
            }
            sum = rs485buf[0] + rs485buf[1] + rs485buf[2];
            if(sum == rs485buf[3]){
                Measurheight = rs485buf[1] * 256 + rs485buf[2];
                LCD_ShowNum(138,140,Measurheight,4,24);
            }
 		}
        //��ⰴ��
		key1=KEY_Scan(1);
        switch(key1){
            case KEY0_PRES:
                num++;
                if(num > 2000){
                    num = 2000;
                }
                LCD_ShowNum(138,170,num,4,24);
                break;
            case KEY1_PRES:
                num--;
                if(num <= 0){
                    num = 0;
                }
                LCD_ShowNum(138,170,num,4,24);
                break;
            case WKUP_PRES:
                Settingheight = num;
                status = !status;
                break;
        }
        //�ж�
        if(status){
            if(Measurheight < (Settingheight-ErrorValue)){
                CTL_WATER_IN  = 1;
                CTL_WATER_OUT = 0;
            }
            else if(Measurheight > (Settingheight+ErrorValue)){
                CTL_WATER_IN  = 0;
                CTL_WATER_OUT = 1;
            }
            
            else if((Measurheight >= (Settingheight-ErrorValue)) && (Measurheight <= (Settingheight+ErrorValue))){
                CTL_WATER_IN  = 0;
                CTL_WATER_OUT = 0;
            }
            LCD_ShowString(120,200,180,24,24,"< RUN >");	//����״̬
        }
        else{
            CTL_WATER_IN  = 0;
            CTL_WATER_OUT = 0;
            LCD_ShowString(120,200,180,24,24,"<PAUSE>");	//��ͣ״̬
        }
        //��ʾϵͳ��������
		t++; 
		delay_ms(10);
		if(t==20){
            CTL_BEEP = !CTL_BEEP;
			LED0=!LED0;	
			t=0;
		}	
	} 
}


