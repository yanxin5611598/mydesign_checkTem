#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "dht11.h" 	
#include "oled.h"
#include "stm32f10x_adc.h"
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_gpio.h" 
#include "usart2.h"
#include "common.h"
#include "usart3.h"
#include "uart4.h"
#include "mq135adc.h"

#define VREF                    3300
#define MAX_CONVERTED_VALUE     4095
#define ADC1_DR_Address ((u32)0x4001244C)
u8 GetAirValueBuf[800];
u8 UART4_PM10_RX_BUF[800]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.

//===========================================================GPRSģ�������==============================================//
//����
#define Success 1U
#define Failure 0U

//�������
unsigned long  Time_Cont = 0;       //��ʱ��������

char TCPServer[] = "1856o325q1.iok.la";		//TCP��������ַ
char Port[] = "52038";						//�˿�

void errorLog(int num);
void phone(char *number);
unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry);
void sendMessage(char *number,char *msg);
void Sys_Soft_Reset(void);
void gprs_at_response(u8 mode);
double middle_filter(double value[],u8 num);//��λֵ�˲��㷨
void limit_value(double a,double b,double value);//�޷��˲��㷨
u8 start_flag;
u8 stop_flag;
unsigned int count11 = 0;
//===========================================================GPRSģ�������==============================================//
int main(void)
 {	 	    
	u8 temperature;  	    
	u8 humidity;    	   
	u8 x,times;
	float ch2oValue,pm25Value,pm10Value,co2Value;
	double chFinalvalue,chMiddlevalue;
	double pm25FinalValue,pm10FinalValue,pm25MiddleValue,pm10MiddleValue,co2FinalValue;
	double ch_value_array[10];
	double pm25_value_array[10];
	double pm10_value_array[10];
	u8 ch_value_buf[10] = {0};
	u8 pm25_value_buf[10] = {0}; 
	u8 pm10_value_buf[10] = {0}; 
	char send_buf[100] = {0};
	char send_buf1[100] = {0};
	//char cStr [ 300 ] = { 0 };
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
 	uart2_init(9600);
	usart3_init(115200);
	UART4_Init(); 
	LED_Init();			     //LED�˿ڳ�ʼ��
	delay_ms(100);
	OLED_Init();
	OLED_Clear();
	//��ʾϵͳ��ʼ������//��ʾ��ϵͳ��ʼ����
	OLED_ShowCHinese(0,2,17);
	OLED_ShowCHinese(16,2,18);
	OLED_ShowCHinese(32,2,19);
	OLED_ShowCHinese(48,2,20);
	OLED_ShowCHinese(64,2,21);
	OLED_ShowChar(80,2,'.');
	OLED_ShowChar(85,2,'.');
	OLED_ShowChar(90,2,'.');
	delay_ms(18000);
	OLED_Clear();
	//link state
	OLED_ShowCHinese(0,2,22);
	OLED_ShowCHinese(16,2,23);
	OLED_ShowCHinese(32,2,24);
	OLED_ShowCHinese(48,2,25);
	OLED_ShowChar(64,2,':');
	OLED_ShowCHinese(48,4,27);
	OLED_ShowCHinese(64,4,22);
	OLED_ShowCHinese(80,4,23);
	//GPRS----------------------------------------------------------------------start
	printf("Welcome to use!\r\n");
	printf("DATA from GPRS Module!\r\n");
	
	printf ( "\r\n��������GPRSģ�������ĵȴ�...\r\n" );//�򴮿�1����Ϣ
	u3_printf("AT\r\n");
	if (sendCommand("AT+RST\r\n", "OK\r\n", 3000, 10) == Success);
	else errorLog(1);
	delay_ms(10);

	if (sendCommand("AT\r\n", "OK\r\n", 3000, 10) == Success);
	else errorLog(2);
	delay_ms(10);

	if (sendCommand("AT+CPIN?\r\n", "READY", 1000, 10) == Success);
	else errorLog(3);
	delay_ms(10);

	if (sendCommand("AT+CREG?\r\n", "CREG: 1", 1000, 10) == Success);
	else errorLog(4);
	delay_ms(10);

	if (sendCommand("AT+CGATT=1\r\n", "OK\r\n", 1000, 10) == Success);
	else errorLog(5);
	delay_ms(10);

	if (sendCommand("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n", "OK\r\n", 1000, 10) == Success);
	else errorLog(6);
	delay_ms(10);


	if (sendCommand("AT+CGACT=1,1\r\n","OK\r\n", 2000, 8) == Success);
	else errorLog(7);
	delay_ms(10);

	
	
	
	memset(send_buf, 0, 100);    //���
	strcpy(send_buf, "AT+CIPSTART=\"TCP\",\"");
	strcat(send_buf, TCPServer);
	strcat(send_buf, "\",");
	strcat(send_buf, Port);
	strcat(send_buf, "\r\n");			
	//if (sendCommand("AT+CIPSTART=\"TCP\",\"1856o325q1.iok.la\",\"46965\"\r\n","CONNECT",10000, 10) == Success);
	if (sendCommand(send_buf, "CONNECT", 5000, 5) == Success);
	else errorLog(8);
	delay_ms(10);
	
	//����͸��ģʽ
	if (sendCommand("AT+CIPTMODE=1\r\n","OK\r\n", 2000, 3) == Success);
	else errorLog(9);
	delay_ms(10);
	printf ( "\r\n����GPRSģ�� OK��\r\n" );
	OLED_Clear();
	//link state
	OLED_ShowCHinese(0,2,22);
	OLED_ShowCHinese(16,2,23);
	OLED_ShowCHinese(32,2,24);
	OLED_ShowCHinese(48,2,25);
	OLED_ShowChar(64,2,':');
	OLED_ShowCHinese(48,4,26);
	OLED_ShowCHinese(64,4,22);
	OLED_ShowCHinese(80,4,23);
	delay_ms(2000);
	OLED_Clear();
	//=================�¶���ʾ��ʼ��=======================
	OLED_ShowCHinese(0,0,9);//��ʾ���¡�
	OLED_ShowCHinese(16,0,11);//��ʾ���ȡ�
	OLED_ShowCHinese(45,0,12);
	//==================ʪ����ʾ��ʼ��================================
	OLED_ShowCHinese(68,0,10);//��ʾ��ʪ��
	OLED_ShowCHinese(84,0,11);//��ʾ���ȡ�
	OLED_ShowString(116,0,"%");
	//===================PM2.5���ģ���ʼ��================================
	OLED_ShowChar(0,2,'P');
	OLED_ShowChar(8,2,'M');
	OLED_ShowChar(16,2,'2');
	OLED_ShowChar(24,2,'.');
	OLED_ShowChar(30,2,'5');
	OLED_ShowChar(38,2,':');
	//===================��ȩ���ģ���ʼ��================================
	OLED_ShowCHinese(0,4,7);//��ʾ���ס�
	OLED_ShowCHinese(16,4,8);//��ʾ��ȩ��
	OLED_ShowChar(32,4,':');
	//===================PM10���ģ���ʼ��================================
	OLED_ShowChar(0,6,'P');
	OLED_ShowChar(8,6,'M');
	OLED_ShowChar(16,6,'1');
	OLED_ShowChar(24,6,'0');
	OLED_ShowChar(32,6,':');
	//send deviceID to server
	memset(send_buf1, 0, 100);    //��
	sprintf ((char*)send_buf1, "D0001\n");
	if (sendCommand(send_buf1, send_buf1, 3000, 1) == Success);
	else errorLog(10);
	delay_ms(100);
	printf ( "send deviceID %s success", (char*)send_buf1 );
	memset(send_buf1, 0, 100);    //���
	//GPRS----------------------------------------------------------------------end
	while(1)
	{	  	
			DHT11_Read_Data(&temperature,&humidity);		//��ȡ��ʪ��===�¶�ʹ�õ���DHT11��������
			times++;
		  if(times%100==0){
				times = 0;
				for (x=0;x< 10;x++)
				{
					ch2oValue = USART2_RX_BUF[4]*256+USART2_RX_BUF[5];
					delay_ms(10);
					chMiddlevalue = (double)ch2oValue/1000;
					pm25Value = UART4_RX_BUF[3]*256+UART4_RX_BUF[2];
					pm10Value = UART4_RX_BUF[5]*256+UART4_RX_BUF[4];
					delay_ms(10);
					pm25MiddleValue = (double)pm25Value/10;
					pm10MiddleValue = (double)pm10Value/10;
					//�������򵥵��޷��˲�  ���β���֮��ı仯���ȹ���
					if(x > 0){
						limit_value(ch_value_array[x-1],chMiddlevalue,2.0);
						limit_value(pm25_value_array[x-1],pm25MiddleValue,500);
						limit_value(pm10_value_array[x-1],pm10MiddleValue,500);
					}
					
					ch_value_array[x] = chMiddlevalue;
					pm25_value_array[x] = pm25MiddleValue;
					pm10_value_array[x] = pm10MiddleValue;
					delay_ms(10);
				}
				//�Բ�������n��������λֵ�˲�����
				
				chFinalvalue = middle_filter(ch_value_array,10);
				pm25FinalValue = middle_filter(pm25_value_array,10);
				pm10FinalValue = middle_filter(pm10_value_array,10);
				
				//�޸�����Ĵ洢��������
				sprintf((char*)ch_value_buf,"%.3lf",chFinalvalue);
				sprintf((char*)pm25_value_buf,"%.1lf",pm25FinalValue);
				sprintf((char*)pm10_value_buf,"%.1lf",pm10FinalValue);
				//��ʱ�������������䣬�򵥴���
				if(chFinalvalue < 10 & pm25FinalValue < 2000 & pm10FinalValue < 2500){
					if(	start_flag == 1){   //����ʹ��
						char send_buf2[100] = {0};
						//��ջ�����			
						memset(send_buf2, 0, 100);    //���send_buf 
						delay_ms(8000);
						sprintf ((char*)send_buf2, "D0001 %2.1f %2.1f %4.3f %4.1f %4.1f\n", 
									(float)temperature ,(float)humidity,chFinalvalue, pm25FinalValue,pm10FinalValue);			
						OLED_ShowNum(32,0,temperature,2,16);
						OLED_ShowNum(102,0,humidity,2,16);
						OLED_ShowString(44,2,pm25_value_buf);
						OLED_ShowString(42,4,ch_value_buf);
						OLED_ShowString(40,6,pm10_value_buf);
						printf ( "%s", (char*)send_buf2 );
						LED0=!LED0;	
						
						if (sendCommand(send_buf2, send_buf2, 3000, 1) == Success);
						else errorLog(10);
						delay_ms(100);
						memset(send_buf2, 0, 100);    //���
						send_buf2[0] = 0x1a;
						delay_ms(100);
						//if (sendCommand(send_buf2, send_buf2, 3000, 5) == Success);
						//else errorLog(11);
						//delay_ms(100);
				}
				if(stop_flag == 1){
					printf("�˳�whileѭ��1");
					OLED_Display_Off();
					printf("�˳�whileѭ��");
					break;
				}
				gprs_at_response(1);
			}
			delay_ms(100);
		}
	}
}

void sendMessage(char *number,char *msg)
{
	char send_buf[20] = {0};
	memset(send_buf, 0, 20);    //���
	strcpy(send_buf, "AT+CMGS=\"");
	strcat(send_buf, number);
	strcat(send_buf, "\"\r\n");
	if (sendCommand(send_buf, ">", 3000, 10) == Success);
	else errorLog(6);


	if (sendCommand(msg, msg, 3000, 1) == Success);
	else errorLog(7);
	delay_ms(100);

	memset(send_buf, 0, 100);    //���
	send_buf[0] = 0x1a;
	if (sendCommand(send_buf, "OK\r\n", 10000, 5) == Success);
	else errorLog(8);
	delay_ms(100);
}
void errorLog(int num)
{
	printf("ERROR%d\r\n",num);
	while (1)
	{
		if (sendCommand("AT\r\n", "OK", 100, 10) == Success)
		{
			Sys_Soft_Reset();
		}
		delay_ms(200);
	}
}

void Sys_Soft_Reset(void)
{  
    SCB->AIRCR =0X05FA0000|(u32)0x04;      
}
unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry)
{
	unsigned char n;
	USART3_CLR_Buf();
	for (n = 0; n < Retry; n++)
	{
		u3_printf(Command); 		//����GPRSָ��
		
		printf("\r\n***************send****************\r\n");
		printf(Command);
		
		Time_Cont = 0;
		while (Time_Cont < Timeout)
		{
			delay_ms(100);
			Time_Cont += 100;
			if (strstr(USART3_RX_BUF, Response) != NULL)
			{				
				printf("\r\n***************receive****************\r\n");
				printf(USART3_RX_BUF);
				USART3_CLR_Buf();
				return Success;
			}
			
		}
		Time_Cont = 0;
	}
	printf("\r\n***************receive****************\r\n");
	printf(USART3_RX_BUF);
	USART3_CLR_Buf();
	return Failure;
}
void gprs_at_response(u8 mode)
{
	u16 rlen = 0,newrlen=0;
	if(USART3_RX_STA&0X8000)		//���յ�һ������
	{ rlen=USART3_RX_STA&0X7FFF;
		newrlen = strlen(USART3_RX_BUF);
		USART3_RX_BUF[newrlen]='\0';//��ӽ�����
		printf("gprs response:%s\r\n",USART3_RX_BUF);	//���͵�����
		if(strcmp((const char*)USART3_RX_BUF,"ON")==0){  
			start_flag = 1;
			printf("start_flag=%d\r\n",start_flag);
		}
		if(strcmp((const char*)USART3_RX_BUF,"OFF")==0){  
			stop_flag = 1;
			printf("stop_flag=%d\r\n",stop_flag);
		}
		if(mode)USART3_RX_STA=0;
	} 
	
}
/*ʵ�ֲ�����֮����޷��˲��㷨----ȡС
a��bΪ�����������ֵ��
valueΪ���Ƶķ�ֵ��С*/
void limit_value(double a , double b,double value){
	if(a - b > value){
		a = b;
	}else if(b - a > value){
		b = a;
	}
}
/*ʵ������λֵ�˲��㷨
	valueΪ��Ҫʹ�ø��㷨�����ݣ���num������ȡ��λֵ
*/
double middle_filter(double value[],u8 num){
	double temp_val=0;
	u8 t,k;
	for(t=0;t<num;t++)
	{
		for(k = 0;k < num-t;k++)
		{
			if(value[k]>value[k+1]){
				//���򽻻�λ��
				temp_val = value[k];
				value[k] = value[k+1];
				value[k+1] = temp_val;
			}
		}
	}
	if(num % 2 == 0){
		return (value[num/2] + value[num/2 + 1])/2.0;
	}else
		return value[num/2];

}