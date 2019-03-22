#include "sim900a.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 
#include "key.h"	 	 	 	 	 
#include "lcd.h" 	  
#include "flash.h" 	 
//#include "touch.h" 	 
#include "malloc.h"
#include "string.h"    
#include "text.h"		
#include "usart2.h" 
#include "ff.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//ATK-SIM900A GSM/GPRS模块驱动	  
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/4/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved	
//********************************************************************************
//无
//////////////////////////////////////////////////////////////////////////////////	
 
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//usmart支持部分 
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART2_RX_STA;
//     1,清零USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",USART2_RX_BUF);	//发送到串口
		if(mode)USART2_RX_STA=0;
	} 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM900A 各项测试(拨号测试、短信测试、GPRS测试)共用代码

//sim900a发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向sim900a发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel7->CNDTR!=0);	//等待通道7传输完成   
		USART2->DR=(u32)cmd;
	}else u2_printf("%s\r\n",cmd);//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(sim900a_check_cmd(ack))break;//得到有效数据 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
u8 sim900a_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u8 sim900a_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
//unicode gbk 转换函数
//src:输入字符串
//dst:输出(uni2gbk时为gbk内码,gbk2uni时,为unicode字符串)
//mode:0,unicode到gbk转换;
//     1,gbk到unicode转换;
void sim900a_unigbk_exchange(u8 *src,u8 *dst,u8 mode)
{
	u16 temp; 
	u8 buf[2];
	if(mode)//gbk 2 unicode
	{
		while(*src!=0)
		{
			if(*src<0X81)	//非汉字
			{
				temp=(u16)ff_convert((WCHAR)*src,1);
				src++;
			}else 			//汉字,占2个字节
			{
				buf[1]=*src++;
				buf[0]=*src++; 
				temp=(u16)ff_convert((WCHAR)*(u16*)buf,1); 
			}
			*dst++=sim900a_hex2chr((temp>>12)&0X0F);
			*dst++=sim900a_hex2chr((temp>>8)&0X0F);
			*dst++=sim900a_hex2chr((temp>>4)&0X0F);
			*dst++=sim900a_hex2chr(temp&0X0F);
		}
	}else	//unicode 2 gbk
	{ 
		while(*src!=0)
		{
			buf[1]=sim900a_chr2hex(*src++)*16;
			buf[1]+=sim900a_chr2hex(*src++);
			buf[0]=sim900a_chr2hex(*src++)*16;
			buf[0]+=sim900a_chr2hex(*src++);
 			temp=(u16)ff_convert((WCHAR)*(u16*)buf,0);
			if(temp<0X80){*dst=temp;dst++;}
			else {*(u16*)dst=swap16(temp);dst+=2;}
		} 
	}
	*dst=0;//添加结束符
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//短信测试部分代码

//测试短信发送内容(70个字[UCS2的时候,1个字符/数字都算1个字])
const u8* sim900a_test_msg="测试短信SIM900A test......来自F103_CH4_V10A电路板";
const u8* PhoneNum[11]={"1","8","7","0","2","8","1","9","5","7","7"};
const u8* kbd_tbl1[13]={"1","2","3","4","5","6","7","8","9","*","0","#","DEL"};
u8** kbd_tbl;
void sim900a_load_keyboard(u16 x,u16 y,u8 **kbtbl)
{
	kbd_tbl=kbtbl;		 					   
}
//SIM900A发短信测试 
void sim900a_sms_send_test(void)
{
	u8 *p,*p1,*p2;
//	u8 phonebuf[20]={0x01,0x08,0x07,0x00,0x02,0x08,0x01,0x09,0x05,0x07,0x07}; 		//号码缓存
	u8 phonebuf[20];
    u8 pohnenumlen=0;		//号码长度,最大15个数 
	u8 timex=0;
//	u8 key=0;
	u8 smssendsta=0;		//短信发送状态,0,等待发送;1,发送失败;2,发送成功 
	p=mymalloc(100);	//申请100个字节的内存,用于存放电话号码的unicode字符串
	p1=mymalloc(300);	//申请300个字节的内存,用于存放短信的unicode字符串
	p2=mymalloc(100);	//申请100个字节的内存 存放：AT+CMGS=p1 
 
    sim900a_load_keyboard(0,180,(u8**)kbd_tbl1);//显示键盘
	while(1){
        if(1){
            printf("正在发送\r\n");
            phonebuf[0]=kbd_tbl[1-1][0];    //1
            phonebuf[1]=kbd_tbl[8-1][0];    //8
            phonebuf[2]=kbd_tbl[7-1][0];    //7
            phonebuf[3]=kbd_tbl[11-1][0];    //0
            phonebuf[4]=kbd_tbl[2-1][0];    //2
            phonebuf[5]=kbd_tbl[8-1][0];    //8
            phonebuf[6]=kbd_tbl[1-1][0];    //1
            phonebuf[7]=kbd_tbl[9-1][0];    //9
            phonebuf[8]=kbd_tbl[5-1][0];    //5
            phonebuf[9]=kbd_tbl[7-1][0];    //7
            phonebuf[10]=kbd_tbl[7-1][0];    //7        
            
            smssendsta=1;		 
            sim900a_unigbk_exchange(phonebuf,p,1);				//将电话号码转换为unicode字符串
            sim900a_unigbk_exchange((u8*)sim900a_test_msg,p1,1);//将短信内容转换为unicode字符串.
            sprintf((char*)p2,"AT+CMGS=\"%s\"",p); 
//            if(sim900a_send_cmd(p2,">",200)==0)					//发送短信命令+电话号码
//            { 
                sim900a_send_cmd(p2,">",200);
                u2_printf("%s",p1);		 						//发送短信内容到GSM模块 
//                if(sim900a_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
                sim900a_send_cmd((u8*)0X1A,"+CMGS:",1000);
                    smssendsta=2;//发送结束符,等待发送完成(最长等待10秒钟,因为短信长了的话,等待时间会长一些)
//                } 
//            } 
            if(smssendsta==1){
                printf("发送失败\r\n");	                //显示状态
            }
            else printf("发送成功\r\n");				//显示状态	
            USART2_RX_STA=0;
        }
		if((timex%20)==0)LED0=!LED0;//200ms闪烁 
		timex++;
		delay_ms(10);
		if(USART2_RX_STA&0X8000)sim_at_response(1);//检查从GSM模块接收到的数据 
	}
	myfree(p);
	myfree(p1);
	myfree(p2); 
} 

//sim900a短信测试
//用于读短信或者发短信
//返回值:0,正常
//    其他,错误代码
u8 sim900a_sms_test(void)
{
	u8 key;
	u8 timex=0;
	if(sim900a_send_cmd("AT+CMGF=1","OK",200))return 1;			//设置文本模式 
	if(sim900a_send_cmd("AT+CSCS=\"UCS2\"","OK",200))return 2;	//设置TE字符集为UCS2 
	if(sim900a_send_cmd("AT+CSMP=17,0,2,25","OK",200))return 3;	//设置短消息文本模式参数 
	while(1)
	{
		key=KEY_Scan(0);
        if(key==KEY1_PRES)
		{ 
			sim900a_sms_send_test();
			timex=0;			
		}else if(key==3)break;
		timex++;
		if(timex==20)
		{
			timex=0;
			LED0=!LED0;
		}
		delay_ms(10);
		sim_at_response(1);										//检查GSM模块发送过来的数据,及时上传给电脑
	} 
	sim900a_send_cmd("AT+CSCS=\"GSM\"","OK",200);				//设置默认的GSM 7位缺省字符集
	return 0;
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//sim900a主测试程序
void sim900a_test(void)
{
	u8 key=0; 
	u8 timex=0;
	u8 sim_ready=0;
	u8 *p,*p1,*p2; 
	p=mymalloc(50);//申请50个字节的内存

	while(sim900a_send_cmd("AT","OK",100))//检测是否应答AT指令 
	{
        printf("未检测到模块!!! \r\n");
		delay_ms(800);
        printf("尝试连接模块... \r\n");
		delay_ms(400);  
	} 	 
    printf("连接模块----OK!!!\r\n");
	key+=sim900a_send_cmd("ATE0","OK",200);//不回显
    //查询相关信息。
	USART2_RX_STA=0;
    //查询是否有信号
	if(sim900a_send_cmd("AT+CSQ","+CSQ:",200)==0){ 
        printf("未检测到信号!!! \r\n");
		delay_ms(800);
        printf("等待模块查找信号... \r\n");
		delay_ms(400); 		
	}
    printf("网络信号连接----OK!!!\r\n");
    printf("*********************\r\n");
    //查询制造商名称
	if(sim900a_send_cmd("AT+CGMI","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF+2),"\r\n");
		p1[0]=0;//加入结束符
		sprintf((char*)p,"制造商:%s",USART2_RX_BUF+2);
        printf("制造商:%s \r\n",USART2_RX_BUF+2);
		USART2_RX_STA=0;		
	} 
    //查询模块名字
	if(sim900a_send_cmd("AT+CGMM","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF+2),"\r\n"); 
		p1[0]=0;//加入结束符
		sprintf((char*)p,"模块型号:%s",USART2_RX_BUF+2);
        printf("模块型号:%s \r\n",USART2_RX_BUF+2);
		USART2_RX_STA=0;		
	} 
    //查询产品序列号
	if(sim900a_send_cmd("AT+CGSN","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF+2),"\r\n");
		p1[0]=0;//加入结束符 
		sprintf((char*)p,"序列号:%s",USART2_RX_BUF+2);
        printf("序列号:%s \r\n",USART2_RX_BUF+2);
		USART2_RX_STA=0;		
	}
    //查询本机号码
	if(sim900a_send_cmd("AT+CNUM","+CNUM",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),",");
		p2=(u8*)strstr((const char*)(p1+2),"\"");
		p2[0]=0;//加入结束符
		sprintf((char*)p,"本机号码:%s",p1+2);
        printf("本机号码:%s \r\n",p1+2);
		USART2_RX_STA=0;		
	}
    //查询运营商名字
    if(sim900a_send_cmd("AT+COPS?","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),"\""); 
		if(p1)//有有效数据
		{
			p2=(u8*)strstr((const char*)(p1+1),"\"");
			p2[0]=0;//加入结束符			
			sprintf((char*)p,"运营商:%s",p1+1);
            printf("运营商:%s \r\n",p1+1);
		} 
		USART2_RX_STA=0;		
	}
    //查询信号质量
	if(sim900a_send_cmd("AT+CSQ","+CSQ",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),":");
		p2=(u8*)strstr((const char*)(p1),",");
		p2[0]=0;//加入结束符
		sprintf((char*)p,"信号质量:%s",p1+2);
        printf("信号质量:%s \r\n",p1+2);
		USART2_RX_STA=0;		
	}
    //查询电池电量
	if(sim900a_send_cmd("AT+CBC","+CBC:",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),",");
		p2=(u8*)strstr((const char*)(p1+1),",");
		p2[0]=0;p2[5]=0;//加入结束符
		sprintf((char*)p,"电池电量:%s%%  %smV",p1+1,p2+1);
        printf("电池电量:%s%%  %smV\r\n",p1+1,p2+1);
		USART2_RX_STA=0;		
	} 
    printf("*********************\r\n");

    //查询是否有信号
    if(sim900a_send_cmd("AT+CSQ","+CSQ:",200)==0){ 
        printf("短信发送......\r\n");
        sim900a_send_cmd("AT+CSCS=\"GSM\"\r","OK",200);
        sim900a_send_cmd("AT+CMGF=1\r","OK",200);
        //这里设置发送的电话号码
        sim900a_send_cmd("AT+CMGS=\"18702819577\"","OK",200);
        //这里设置发送的短信内容
        sim900a_send_cmd("Warning:Natural Gas Leakage!!!\r...ID:F103_CH4_V10A","OK",200);
        USART_SendData(USART2,0x1a);//以0x1a来结束，表示内容结束，发送短信
        printf("短信发动完成\r\n");
	} 
	myfree(p);	
}












