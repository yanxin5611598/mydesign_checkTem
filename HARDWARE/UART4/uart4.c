#include "delay.h"
#include "uart4.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	
#include "sys.h"  
//#include "malloc.h"		   

//���ڷ��ͻ����� 	
__align(8) u8 UART4_TX_BUF[UART4_MAX_SEND_LEN]; 	//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
//__align(8) u8 UART4_TX_10_BUF[UART4_MAX_SEND_LEN]; 	//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
#ifdef UART4_RX_EN   								//���ʹ���˽���   	  
//���ڽ��ջ����� 	
u8 UART4_RX_BUF[UART4_MAX_RECV_LEN]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.
//u8 UART4_RX_10_BUF[UART4_MAX_RECV_LEN]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.
//u8 uart4_tx_buf[UART4_MAX_SEND_LEN]; 	//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
u16 UART4_RX_STA=0;   	 
/*void UART4_IRQHandler(void)
{
	u8 res;	    
	if(UART4->SR&(1<<5))//���յ�����
	{	 
		res=UART4->DR; 			 
		if(UART4_RX_STA<UART4_MAX_RECV_LEN)		//�����Խ�������
		{
			TIM5->CNT=0;         					//���������
			if(UART4_RX_STA==0)TIM5_Set(1);	 	//ʹ�ܶ�ʱ��4���ж� 
			UART4_RX_BUF[UART4_RX_STA++]=res;		//��¼���յ���ֵ	 
		}else 
		{
			UART4_RX_STA|=1<<15;					//ǿ�Ʊ�ǽ������
		} 
	}  											 
} */  
//��ʼ��IO ����4
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void UART4_Init(void)
{ 
	 GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
	//ʹ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	//���ý��չܽ�PC11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//���÷��͹ܽ�PC10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	//�����ʡ��ֳ���ֹͣλ����żУ��λ��Ӳ�������ơ��첽����ΪĬ�ϣ������������ã�
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//���������жϣ�����һ֡����ʱ����ʹ��
	USART_Cmd(UART4, ENABLE);			
USART_ClearFlag(UART4 , USART_FLAG_TC);	
}
uint8_t count4 = 0;//ͳ��֡�ֽڸ���
void UART4_IRQHandler(void)                	//����4�жϷ������
{
	//u8 *p;
	//u16 showBuffer[9];
	u8 Res;

	//int RxCount=0;
	uint8_t i; 
		//printf("%s\t","interrupt");
	//uint8_t j=0;
	//for(i=0;i<9;i++) showBuffer[i]=0;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif*/
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0xaa��β)
		{
			 UART4_RX_BUF[count4++] = UART4->DR;   //�Ѷ������ֽڱ��棬�����ַ��1
			//UART4_RX_10_BUF[count4++] = UART4->DR;
		}		
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)   //���������жϣ��ж��Ƿ�������һ֡����
	{
		i = UART4->DR;
		
		
		
		for(i=0;i<count4;i++)
		{
				USART_SendData(UART4, UART4_RX_BUF[i]);//???1????
				while(USART_GetFlagStatus(UART4,USART_FLAG_TXE)==RESET);    //���ڷ���,��PC��usb���Ӵ������ֽ��е���
				//USART_SendData(USART3, USART2_RX_BUF[i]);//???1????
				//delay_ms(200);
				//while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);    //wifi����
		}	
		count4=0;
	}	
 /*u8 ch;
 if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) 
 {
  ch = USART_ReceiveData(UART4);
  UART4_RX_BUF[UART4_RX_STA++]=ch;  
  UART4_RX_STA&=0xff;
  delay_ms(5);
 }	*/
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif
} 
#endif			 















