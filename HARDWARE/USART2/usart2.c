#include "sys.h"
#include "usart2.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
/*#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//ѭ������,ֱ���������   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif */

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART2, (uint8_t) ch);

	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART2->SR & USART_FLAG_RXNE));

    return ((int)(USART2->DR & 0x1FF));
}
*/
 
#if EN_USART2_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART2_REC_LEN���ֽ�.

//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���ĿUSART_RX_STA
u16 USART2_RX_STA=0;       //����״̬���	  


void uart2_init(u32 bound){
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
	 /* Enable the USART2 Pins Software Remapping */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 
    
  
    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
      

   //Usart2 NVIC ����

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART2, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//���������ж�
		USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//���������жϣ�����һ֡����ʱ����ʹ��
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 

}

uint8_t count = 0;//ͳ��֡�ֽڸ���

void USART2_IRQHandler(void)                	//����1�жϷ������
	{
	u8 *p;
	//u16 showBuffer[9];
	//	u8 Res;
	//int RxCount=0;
	uint8_t i; 
	uint8_t j=0;
			//printf("%s\t","interrupt2");
	//for(i=0;i<9;i++) showBuffer[i]=0;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0xaa��β)
		{
			  USART2_RX_BUF[count++] = USART2->DR;   //�Ѷ������ֽڱ��棬�����ַ��1
		}		
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)   //���������жϣ��ж��Ƿ�������һ֡����
	{
		i = USART2->DR;
		
		
		
		for(i=0;i<count;i++)
		{
				USART_SendData(USART2, USART2_RX_BUF[i]);//???1????
				while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);    //���ڷ���,��PC��usb���Ӵ������ֽ��е���
				//USART_SendData(USART3, USART2_RX_BUF[i]);//???1????
				//delay_ms(200);
				//while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);    //wifi����
		}
		
						
			
		count=0;
		
		
	}
		/*Res =USART_ReceiveData(USART2);//(USART2->DR);	//��ȡ���յ�������
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		showBuffer[USART2_RX_STA] = Res; 
		USART2_RX_STA++;
		
			if(USART2_RX_STA>9)//����δ���
			USART2_RX_STA=0;
			USART_SendData(USART1, Res);//???1????
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	*/
			
			
/*			if(*showBuffer == 0xA5)
			{
					USART2_RX_BUF[0]=*showBuffer;
					for(i=1;i<9;i++)
					{
						USART2_RX_BUF[i] = *(showBuffer+i);
					}
			}
*/			
			
			
			
			//for(i=0;i<9;i++)
			//USART2_RX_BUF[i]=showBuffer[i];
									  
			 
				
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif
} 
#endif	


