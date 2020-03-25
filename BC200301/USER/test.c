/*
10�������:
PH9��PH10��PH11��PH12��PH13
PI4��PI5��PI6��PI7��PI8
*/

#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "led.h"
#include "key.h"
#include "driver.h"
#include <string.h>


u8 fcom[] = "FORWARD";
u8 bcom[] = "BACKWARD";
u8 scom[] = "STOP";


//������ջ���
void rBUF_Clear(void)
{
	unsigned char i;
	for(i=0;i<USART_REC_LEN;i++)		//���
	{
		rBUF[i] = 0x00;
	}
	rLEN = 0;
}

//�ж��Ƿ���յ�ָ����Ϣ
u8 REC_Check(u8 *s)
{
	if(strstr(rBUF,s) != NULL)	//������յ�ָ����Ϣ
	{
		return 1;				//����1
	}
	else
	{
		return 0;
	}
}


int main(void)
{   
 	u8 i,j;
	u8 led0sta=1;
	
	u16 sen_old = 0x03ff;
	u16 sen_new,temp;
	
	Stm32_Clock_Init(432,25,2,9);//����ʱ��,216Mhz
    delay_init(216);			//��ʱ��ʼ��  
	uart_init(108,115200);		//��ʼ�����ڲ�����Ϊ115200	
	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ��LED
	Driver_Init();				//��������ʼ��
	TIM8_OPM_RCR_Init(999,216-1); //1MHz����Ƶ��  ������+�ظ�����ģʽ
	
	printf("���⴫����\r\n");
	
	while(1) 
	{		 	  
		if(REC_Check(fcom))
		{
			DRIVER_DIR(CW);
			TIM8_Startup(500);   //������ʱ��8
			rBUF_Clear();
			printf("OK\r\n");
		}
		if(REC_Check(bcom))
		{
			DRIVER_DIR(CCW);
			TIM8_Startup(500);   //������ʱ��8
			rBUF_Clear();
			printf("OK\r\n");
		}
		if(REC_Check(scom))
		{
			TIM8->BDTR&=~(1<<15);//MOE �ر������
			TIM8->CR1&=~(1<<0);  //�رն�ʱ��8
			rBUF_Clear();
			printf("OK\r\n");
		}

		sen_new = GPIO_Read();	//��ȡ�������˿�
		if(sen_new != sen_old)	//����˿�״̬�����仯
		{
//			printf("0X%x\r\n",sen_new);
			temp = sen_new^sen_old;	//���仯�Ķ˿ڱ�־Ϊ1
//			printf("0x%x\r\n",temp);
			sen_old = sen_new;		//���浱ǰ�˿�״̬
			for(j=0;j<10;j++)			//���10���˿�
			{
				if(temp & (1<<j))	//����״̬Ϊ1�Ķ˿�
				{
					switch(j)
					{
						case 0: if(sen_new & (1<<j))	//����˿�״̬�Ǹ�
											printf("OA1\r\n");
										else
											printf("CA1\r\n");
										break;
						case 1: if(sen_new & (1<<j))
											printf("OB2\r\n");
										else
											printf("CB2\r\n");
										break;
						case 2: if(sen_new & (1<<j))
											printf("OC3\r\n");
										else
											printf("CC3\r\n");
										break;
						case 3: if(sen_new & (1<<j))
											printf("OD4\r\n");
										else
											printf("CD4\r\n");
										break;
						case 4: if(sen_new & (1<<j))
											printf("OE5\r\n");
										else
											printf("CE5\r\n");
										break;
						case 5: if(sen_new & (1<<j))
											printf("OF6\r\n");
										else
											printf("CF6\r\n");
										break;
						case 6: if(sen_new & (1<<j))
											printf("OG7\r\n");
										else
											printf("CG7\r\n");
										break;
						case 7: if(sen_new & (1<<j))
											printf("OH8\r\n");
										else
											printf("CH8\r\n");
										break;
						case 8: if(sen_new & (1<<j))
											printf("OI9\r\n");
										else
											printf("CI9\r\n");
										break;
						case 9: if(sen_new & (1<<j))
											printf("OJ10\r\n");
										else
											printf("CJ10\r\n");
										break;
					}
				}
			}
		}
		delay_ms(10);
		i++;
		if(i==50)	
		{	
			i=0;
			LED1(led0sta^=1);		//LED1��˸
		}
	} 
	
	
	
	
}



