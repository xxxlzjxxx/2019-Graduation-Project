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
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//ATK-SIM900A GSM/GPRSģ������	  
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/4/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved	
//********************************************************************************
//��
//////////////////////////////////////////////////////////////////////////////////	
 
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//usmart֧�ֲ��� 
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART2_RX_STA;
//     1,����USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		printf("%s",USART2_RX_BUF);	//���͵�����
		if(mode)USART2_RX_STA=0;
	} 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM900A �������(���Ų��ԡ����Ų��ԡ�GPRS����)���ô���

//sim900a���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��sim900a��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel7->CNDTR!=0);	//�ȴ�ͨ��7�������   
		USART2->DR=(u32)cmd;
	}else u2_printf("%s\r\n",cmd);//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(sim900a_check_cmd(ack))break;//�õ���Ч���� 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//��1���ַ�ת��Ϊ16��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��16������ֵ
u8 sim900a_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim900a_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
//unicode gbk ת������
//src:�����ַ���
//dst:���(uni2gbkʱΪgbk����,gbk2uniʱ,Ϊunicode�ַ���)
//mode:0,unicode��gbkת��;
//     1,gbk��unicodeת��;
void sim900a_unigbk_exchange(u8 *src,u8 *dst,u8 mode)
{
	u16 temp; 
	u8 buf[2];
	if(mode)//gbk 2 unicode
	{
		while(*src!=0)
		{
			if(*src<0X81)	//�Ǻ���
			{
				temp=(u16)ff_convert((WCHAR)*src,1);
				src++;
			}else 			//����,ռ2���ֽ�
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
	*dst=0;//��ӽ�����
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//���Ų��Բ��ִ���

//���Զ��ŷ�������(70����[UCS2��ʱ��,1���ַ�/���ֶ���1����])
const u8* sim900a_test_msg="���Զ���SIM900A test......����F103_CH4_V10A��·��";
const u8* PhoneNum[11]={"1","8","7","0","2","8","1","9","5","7","7"};
const u8* kbd_tbl1[13]={"1","2","3","4","5","6","7","8","9","*","0","#","DEL"};
u8** kbd_tbl;
void sim900a_load_keyboard(u16 x,u16 y,u8 **kbtbl)
{
	kbd_tbl=kbtbl;		 					   
}
//SIM900A�����Ų��� 
void sim900a_sms_send_test(void)
{
	u8 *p,*p1,*p2;
//	u8 phonebuf[20]={0x01,0x08,0x07,0x00,0x02,0x08,0x01,0x09,0x05,0x07,0x07}; 		//���뻺��
	u8 phonebuf[20];
    u8 pohnenumlen=0;		//���볤��,���15���� 
	u8 timex=0;
//	u8 key=0;
	u8 smssendsta=0;		//���ŷ���״̬,0,�ȴ�����;1,����ʧ��;2,���ͳɹ� 
	p=mymalloc(100);	//����100���ֽڵ��ڴ�,���ڴ�ŵ绰�����unicode�ַ���
	p1=mymalloc(300);	//����300���ֽڵ��ڴ�,���ڴ�Ŷ��ŵ�unicode�ַ���
	p2=mymalloc(100);	//����100���ֽڵ��ڴ� ��ţ�AT+CMGS=p1 
 
    sim900a_load_keyboard(0,180,(u8**)kbd_tbl1);//��ʾ����
	while(1){
        if(1){
            printf("���ڷ���\r\n");
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
            sim900a_unigbk_exchange(phonebuf,p,1);				//���绰����ת��Ϊunicode�ַ���
            sim900a_unigbk_exchange((u8*)sim900a_test_msg,p1,1);//����������ת��Ϊunicode�ַ���.
            sprintf((char*)p2,"AT+CMGS=\"%s\"",p); 
//            if(sim900a_send_cmd(p2,">",200)==0)					//���Ͷ�������+�绰����
//            { 
                sim900a_send_cmd(p2,">",200);
                u2_printf("%s",p1);		 						//���Ͷ������ݵ�GSMģ�� 
//                if(sim900a_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
                sim900a_send_cmd((u8*)0X1A,"+CMGS:",1000);
                    smssendsta=2;//���ͽ�����,�ȴ��������(��ȴ�10����,��Ϊ���ų��˵Ļ�,�ȴ�ʱ��᳤һЩ)
//                } 
//            } 
            if(smssendsta==1){
                printf("����ʧ��\r\n");	                //��ʾ״̬
            }
            else printf("���ͳɹ�\r\n");				//��ʾ״̬	
            USART2_RX_STA=0;
        }
		if((timex%20)==0)LED0=!LED0;//200ms��˸ 
		timex++;
		delay_ms(10);
		if(USART2_RX_STA&0X8000)sim_at_response(1);//����GSMģ����յ������� 
	}
	myfree(p);
	myfree(p1);
	myfree(p2); 
} 

//sim900a���Ų���
//���ڶ����Ż��߷�����
//����ֵ:0,����
//    ����,�������
u8 sim900a_sms_test(void)
{
	u8 key;
	u8 timex=0;
	if(sim900a_send_cmd("AT+CMGF=1","OK",200))return 1;			//�����ı�ģʽ 
	if(sim900a_send_cmd("AT+CSCS=\"UCS2\"","OK",200))return 2;	//����TE�ַ���ΪUCS2 
	if(sim900a_send_cmd("AT+CSMP=17,0,2,25","OK",200))return 3;	//���ö���Ϣ�ı�ģʽ���� 
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
		sim_at_response(1);										//���GSMģ�鷢�͹���������,��ʱ�ϴ�������
	} 
	sim900a_send_cmd("AT+CSCS=\"GSM\"","OK",200);				//����Ĭ�ϵ�GSM 7λȱʡ�ַ���
	return 0;
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//sim900a�����Գ���
void sim900a_test(void)
{
	u8 key=0; 
	u8 timex=0;
	u8 sim_ready=0;
	u8 *p,*p1,*p2; 
	p=mymalloc(50);//����50���ֽڵ��ڴ�

	while(sim900a_send_cmd("AT","OK",100))//����Ƿ�Ӧ��ATָ�� 
	{
        printf("δ��⵽ģ��!!! \r\n");
		delay_ms(800);
        printf("��������ģ��... \r\n");
		delay_ms(400);  
	} 	 
    printf("����ģ��----OK!!!\r\n");
	key+=sim900a_send_cmd("ATE0","OK",200);//������
    //��ѯ�����Ϣ��
	USART2_RX_STA=0;
    //��ѯ�Ƿ����ź�
	if(sim900a_send_cmd("AT+CSQ","+CSQ:",200)==0){ 
        printf("δ��⵽�ź�!!! \r\n");
		delay_ms(800);
        printf("�ȴ�ģ������ź�... \r\n");
		delay_ms(400); 		
	}
    printf("�����ź�����----OK!!!\r\n");
    printf("*********************\r\n");
    //��ѯ����������
	if(sim900a_send_cmd("AT+CGMI","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF+2),"\r\n");
		p1[0]=0;//���������
		sprintf((char*)p,"������:%s",USART2_RX_BUF+2);
        printf("������:%s \r\n",USART2_RX_BUF+2);
		USART2_RX_STA=0;		
	} 
    //��ѯģ������
	if(sim900a_send_cmd("AT+CGMM","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF+2),"\r\n"); 
		p1[0]=0;//���������
		sprintf((char*)p,"ģ���ͺ�:%s",USART2_RX_BUF+2);
        printf("ģ���ͺ�:%s \r\n",USART2_RX_BUF+2);
		USART2_RX_STA=0;		
	} 
    //��ѯ��Ʒ���к�
	if(sim900a_send_cmd("AT+CGSN","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF+2),"\r\n");
		p1[0]=0;//��������� 
		sprintf((char*)p,"���к�:%s",USART2_RX_BUF+2);
        printf("���к�:%s \r\n",USART2_RX_BUF+2);
		USART2_RX_STA=0;		
	}
    //��ѯ��������
	if(sim900a_send_cmd("AT+CNUM","+CNUM",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),",");
		p2=(u8*)strstr((const char*)(p1+2),"\"");
		p2[0]=0;//���������
		sprintf((char*)p,"��������:%s",p1+2);
        printf("��������:%s \r\n",p1+2);
		USART2_RX_STA=0;		
	}
    //��ѯ��Ӫ������
    if(sim900a_send_cmd("AT+COPS?","OK",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),"\""); 
		if(p1)//����Ч����
		{
			p2=(u8*)strstr((const char*)(p1+1),"\"");
			p2[0]=0;//���������			
			sprintf((char*)p,"��Ӫ��:%s",p1+1);
            printf("��Ӫ��:%s \r\n",p1+1);
		} 
		USART2_RX_STA=0;		
	}
    //��ѯ�ź�����
	if(sim900a_send_cmd("AT+CSQ","+CSQ",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),":");
		p2=(u8*)strstr((const char*)(p1),",");
		p2[0]=0;//���������
		sprintf((char*)p,"�ź�����:%s",p1+2);
        printf("�ź�����:%s \r\n",p1+2);
		USART2_RX_STA=0;		
	}
    //��ѯ��ص���
	if(sim900a_send_cmd("AT+CBC","+CBC:",200)==0){ 
		p1=(u8*)strstr((const char*)(USART2_RX_BUF),",");
		p2=(u8*)strstr((const char*)(p1+1),",");
		p2[0]=0;p2[5]=0;//���������
		sprintf((char*)p,"��ص���:%s%%  %smV",p1+1,p2+1);
        printf("��ص���:%s%%  %smV\r\n",p1+1,p2+1);
		USART2_RX_STA=0;		
	} 
    printf("*********************\r\n");

    //��ѯ�Ƿ����ź�
    if(sim900a_send_cmd("AT+CSQ","+CSQ:",200)==0){ 
        printf("���ŷ���......\r\n");
        sim900a_send_cmd("AT+CSCS=\"GSM\"\r","OK",200);
        sim900a_send_cmd("AT+CMGF=1\r","OK",200);
        //�������÷��͵ĵ绰����
        sim900a_send_cmd("AT+CMGS=\"18702819577\"","OK",200);
        //�������÷��͵Ķ�������
        sim900a_send_cmd("Warning:Natural Gas Leakage!!!\r...ID:F103_CH4_V10A","OK",200);
        USART_SendData(USART2,0x1a);//��0x1a����������ʾ���ݽ��������Ͷ���
        printf("���ŷ������\r\n");
	} 
	myfree(p);	
}












