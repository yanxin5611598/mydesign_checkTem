#include "sys.h"
#include "usart2.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
/*#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif */

/*使用microLib的方法*/
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
 
#if EN_USART2_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART2_REC_LEN个字节.

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目USART_RX_STA
u16 USART2_RX_STA=0;       //接收状态标记	  


void uart2_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
	 /* Enable the USART2 Pins Software Remapping */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 
    
  
    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
      

   //Usart2 NVIC 配置

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART2, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启接收中断
		USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启空闲中断，接收一帧数据时经常使用
    USART_Cmd(USART2, ENABLE);                    //使能串口 

}

uint8_t count = 0;//统计帧字节个数

void USART2_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 *p;
	//u16 showBuffer[9];
	//	u8 Res;
	//int RxCount=0;
	uint8_t i; 
	uint8_t j=0;
			//printf("%s\t","interrupt2");
	//for(i=0;i<9;i++) showBuffer[i]=0;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0xaa结尾)
		{
			  USART2_RX_BUF[count++] = USART2->DR;   //把读到的字节保存，数组地址加1
		}		
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)   //开启空闲中断，判断是否接收完成一帧数据
	{
		i = USART2->DR;
		
		
		
		for(i=0;i<count;i++)
		{
				USART_SendData(USART2, USART2_RX_BUF[i]);//???1????
				while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);    //串口发送,在PC用usb连接串口助手进行调试
				//USART_SendData(USART3, USART2_RX_BUF[i]);//???1????
				//delay_ms(200);
				//while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);    //wifi发送
		}
		
						
			
		count=0;
		
		
	}
		/*Res =USART_ReceiveData(USART2);//(USART2->DR);	//读取接收到的数据
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		showBuffer[USART2_RX_STA] = Res; 
		USART2_RX_STA++;
		
			if(USART2_RX_STA>9)//接收未完成
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
									  
			 
				
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif
} 
#endif	


