#ifndef __UART4_H
#define __UART4_H	 
#include "sys.h"  
#define UART4_MAX_RECV_LEN		800					//�����ջ����ֽ���
#define UART4_MAX_SEND_LEN		100					//����ͻ����ֽ���
#define UART4_RX_EN 			1					//0,������;1,����.

extern u8  UART4_RX_BUF[UART4_MAX_RECV_LEN]; 		//���ջ���,���USART2_MAX_RECV_LEN�ֽ�
extern u8  UART4_TX_BUF[UART4_MAX_SEND_LEN]; 		//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
extern u16 UART4_RX_STA;   						//��������״̬

void UART4_Init(void);				//����2��ʼ�� 
void UART4_IRQHandler(void);
#endif













