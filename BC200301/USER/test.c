/*
10个输出脚:
PH9、PH10、PH11、PH12、PH13
PI4、PI5、PI6，PI7、PI8
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


//清除接收缓冲
void rBUF_Clear(void)
{
	unsigned char i;
	for(i=0;i<USART_REC_LEN;i++)		//清除
	{
		rBUF[i] = 0x00;
	}
	rLEN = 0;
}

//判断是否接收到指定信息
u8 REC_Check(u8 *s)
{
	if(strstr(rBUF,s) != NULL)	//如果接收到指定信息
	{
		return 1;				//返回1
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
	
	Stm32_Clock_Init(432,25,2,9);//设置时钟,216Mhz
    delay_init(216);			//延时初始化  
	uart_init(108,115200);		//初始化串口波特率为115200	
	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化LED
	Driver_Init();				//驱动器初始化
	TIM8_OPM_RCR_Init(999,216-1); //1MHz计数频率  单脉冲+重复计数模式
	
	printf("红外传感器\r\n");
	
	while(1) 
	{		 	  
		if(REC_Check(fcom))
		{
			DRIVER_DIR(CW);
			TIM8_Startup(500);   //启动定时器8
			rBUF_Clear();
			printf("OK\r\n");
		}
		if(REC_Check(bcom))
		{
			DRIVER_DIR(CCW);
			TIM8_Startup(500);   //启动定时器8
			rBUF_Clear();
			printf("OK\r\n");
		}
		if(REC_Check(scom))
		{
			TIM8->BDTR&=~(1<<15);//MOE 关闭主输出
			TIM8->CR1&=~(1<<0);  //关闭定时器8
			rBUF_Clear();
			printf("OK\r\n");
		}

		sen_new = GPIO_Read();	//读取传感器端口
		if(sen_new != sen_old)	//如果端口状态发生变化
		{
//			printf("0X%x\r\n",sen_new);
			temp = sen_new^sen_old;	//将变化的端口标志为1
//			printf("0x%x\r\n",temp);
			sen_old = sen_new;		//保存当前端口状态
			for(j=0;j<10;j++)			//检查10个端口
			{
				if(temp & (1<<j))	//查找状态为1的端口
				{
					switch(j)
					{
						case 0: if(sen_new & (1<<j))	//如果端口状态是高
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
			LED1(led0sta^=1);		//LED1闪烁
		}
	} 
	
	
	
	
}



