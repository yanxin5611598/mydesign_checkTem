#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
 #include "stm32f10x_dma.h"
  #include "stm32f10x_adc.h"
	 #include "stm32f10x.h"
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//ADC ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/7
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
void MQ135ADC_Init();
u16  T_Get_Air_adc(u8 ch); 
u16 Get_Adc_Average(u8 ch,u8 times); 
 
#endif 
