
#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "rs485.h"
#include "stdio.h"
#include "string.h"

u16 ErrorValue = 20;            //设置误差值
u16 Measurheight = 0;           //测量值
u16 Settingheight = 100;        //设定值
			 	
int main(void){	 
	u8 key,key1;
	u8 i=0,t=0;
	u8 rs485buf[4];
    u16 num = 0;
    u8 status = 0;          //当前状态
    u8 sum = 0;             //串口校验和
	 
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	LED_Init();		  		//初始化与LED连接的硬件接口
	LCD_Init();			   	//初始化LCD 	
	KEY_Init();				//按键初始化		 	 
	RS485_Init(9600);	//初始化RS485
 	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(30,10,280,16,16,"    Non-contact liquid level    ");
	LCD_ShowString(30,30,280,16,16,"measuring and controlling device");	
	LCD_ShowString(30,50,280,16,16,"                      __WenXin__");
	LCD_ShowString(30,70,280,16,16,"                      2019.04.01");		
 
 	POINT_COLOR=BLUE;//设置字体为蓝色	  	
    LCD_ShowString(30,110,280,16,16,"Sensor  Data:      ");	//提示接收到的数据
	LCD_ShowString(30,140,280,24,24,"Measure:      mm");	//测量到的数据
    LCD_ShowString(30,170,280,24,24,"Setting: 0000 mm");	//设置数据

    LCD_ShowString(120,200,180,24,24,"<PAUSE>");	//设置数据    
 									  
	while(1){	 
		RS485_Receive_Data(rs485buf,&key);
        //检测串口接收到有数据
		if(key)
		{
			if(key>4)key=4;//最大是4个数据.
 			for(i=0;i<key;i++){
                LCD_ShowxNum(138+i*32,110,rs485buf[i],3,16,0X80);	//显示数据
            }
            sum = rs485buf[0] + rs485buf[1] + rs485buf[2];
            if(sum == rs485buf[3]){
                Measurheight = rs485buf[1] * 256 + rs485buf[2];
                LCD_ShowNum(138,140,Measurheight,4,24);
            }
 		}
        //检测按键
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
        //判断
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
            LCD_ShowString(120,200,180,24,24,"< RUN >");	//运行状态
        }
        else{
            CTL_WATER_IN  = 0;
            CTL_WATER_OUT = 0;
            LCD_ShowString(120,200,180,24,24,"<PAUSE>");	//暂停状态
        }
        //提示系统正在运行
		t++; 
		delay_ms(10);
		if(t==20){
            CTL_BEEP = !CTL_BEEP;
			LED0=!LED0;	
			t=0;
		}	
	} 
}


