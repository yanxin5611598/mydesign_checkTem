#include "delay.h"
#include "uart4.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	
#include "sys.h"  
//#include "malloc.h"		   

//串口发送缓存区 	
__align(8) u8 UART4_TX_BUF[UART4_MAX_SEND_LEN]; 	//发送缓冲,最大USART2_MAX_SEND_LEN字节
//__align(8) u8 UART4_TX_10_BUF[UART4_MAX_SEND_LEN]; 	//发送缓冲,最大USART2_MAX_SEND_LEN字节
#ifdef UART4_RX_EN   								//如果使能了接收   	  
//串口接收缓存区 	
u8 UART4_RX_BUF[UART4_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
//u8 UART4_RX_10_BUF[UART4_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
//u8 uart4_tx_buf[UART4_MAX_SEND_LEN]; 	//发送缓冲,最大USART2_MAX_SEND_LEN字节
//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
u16 UART4_RX_STA=0;   	 
/*void UART4_IRQHandler(void)
{
	u8 res;	    
	if(UART4->SR&(1<<5))//接收到数据
	{	 
		res=UART4->DR; 			 
		if(UART4_RX_STA<UART4_MAX_RECV_LEN)		//还可以接收数据
		{
			TIM5->CNT=0;         					//计数器清空
			if(UART4_RX_STA==0)TIM5_Set(1);	 	//使能定时器4的中断 
			UART4_RX_BUF[UART4_RX_STA++]=res;		//记录接收到的值	 
		}else 
		{
			UART4_RX_STA|=1<<15;					//强制标记接收完成
		} 
	}  											 
} */  
//初始化IO 串口4
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void UART4_Init(void)
{ 
	 GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
	//使能时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	//配置接收管脚PC11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//配置发送管脚PC10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	//波特率、字长、停止位、奇偶校验位、硬件流控制、异步串口为默认（被屏蔽字设置）
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//开启空闲中断，接收一帧数据时经常使用
	USART_Cmd(UART4, ENABLE);			
USART_ClearFlag(UART4 , USART_FLAG_TC);	
}
uint8_t count4 = 0;//统计帧字节个数
void UART4_IRQHandler(void)                	//串口4中断服务程序
{
	//u8 *p;
	//u16 showBuffer[9];
	u8 Res;

	//int RxCount=0;
	uint8_t i; 
		//printf("%s\t","interrupt");
	//uint8_t j=0;
	//for(i=0;i<9;i++) showBuffer[i]=0;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif*/
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0xaa结尾)
		{
			 UART4_RX_BUF[count4++] = UART4->DR;   //把读到的字节保存，数组地址加1
			//UART4_RX_10_BUF[count4++] = UART4->DR;
		}		
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)   //开启空闲中断，判断是否接收完成一帧数据
	{
		i = UART4->DR;
		
		
		
		for(i=0;i<count4;i++)
		{
				USART_SendData(UART4, UART4_RX_BUF[i]);//???1????
				while(USART_GetFlagStatus(UART4,USART_FLAG_TXE)==RESET);    //串口发送,在PC用usb连接串口助手进行调试
				//USART_SendData(USART3, USART2_RX_BUF[i]);//???1????
				//delay_ms(200);
				//while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);    //wifi发送
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
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif
} 
#endif			 















