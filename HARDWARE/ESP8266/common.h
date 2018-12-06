#ifndef __COMMON_H__
#define __COMMON_H__	 
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 
#include "key.h"	 	 	 	 	  	 	   		
#include "usart3.h" 
#include "string.h"
#include "oled.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//ATK-ESP8266 WIFIģ�� ������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/4/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
#define LEN 			30

extern u8 OK[LEN];


u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime);

u8* atk_8266_check_cmd(u8 *str);

u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime);

void atk_8266_at_response(u8 mode);

u8 atk_8266_consta_check(void);

u8 atk_8266_quit_trans(void);
#endif





