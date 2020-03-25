#include "driver.h"
#include "delay.h"
#include "usart.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//此例程有参考论坛网友例程的一部分(http://www.openedv.com/thread-41832-1-1.html)
//ALIENTEK 阿波罗F7开发板
//步进电机驱动器 测试代码			   
//lycreturn@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2016/05/12
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved	
//********************************************************************************
//修改日期:2016/05/12
//////////////////////////////////////////////////////////////////////////////////
/********** 驱动器 端口定义 **************
//DRIVER_DIR   PF7 ()
//DRIVER_OE    PF9 ()
//STEP_PULSE   PC7 (TIM8_CH2  DCMI_D1)
******************************************/

u8 rcr_remainder;   //重复计数余数部分
u8 is_rcr_finish=1; //重复计数器是否设置完成
long rcr_integer;	//重复计数整数部分
long target_pos=0;  //有符号方向
long current_pos=0; //有符号方向
DIR_Type motor_dir=CW;//顺时针

/************** 驱动器控制信号线初始化 ****************/
void Driver_Init(void)
{
	RCC->AHB1ENR|=1<<5;//使能PORTF时钟
	GPIO_Set(GPIOF,PIN7|PIN9,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PF7,PF9设置

	RCC->AHB1ENR|=1<<8;     //使能PORTI时钟
	GPIO_Set(GPIOI,PIN4|PIN5|PIN6|PIN7|PIN8,GPIO_MODE_IN,0,0,GPIO_PUPD_PU);
	
	RCC->AHB1ENR|=1<<7;     //使能PORTH时钟
	GPIO_Set(GPIOH,PIN9|PIN10|PIN11|PIN12|PIN13,GPIO_MODE_IN,0,0,GPIO_PUPD_PU);
	
	DRIVER_DIR(1);   	//PF7输出高 顺时针方向
	//DRIVER_OE(0); 		//PF9输出低 使能输出
}


/***********************************************
//TIM8_CH2(PC7) 单脉冲输出+重复计数功能初始化
//TIM8 时钟频率 108*2=216MHz
//arr:自动重装值
//psc:时钟预分频数
************************************************/
void TIM8_OPM_RCR_Init(u16 arr,u16 psc)
{		 					 
	//此部分需手动修改IO口设置	
	RCC->APB2ENR|=1<<1; 	//TIM8时钟使能    
	RCC->AHB1ENR|=1<<2;   	//使能PORTC时钟	
	GPIO_Set(GPIOC,PIN7,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PD);//复用功能,下拉输出
	GPIO_AF_Set(GPIOC,7,3);	//PC7,AF3  
	
	TIM8->ARR=arr;			//设定计数器自动重装值 
	TIM8->PSC=psc;			//预分频器设置	
	TIM8->CCR2=TIM8->ARR>>1;//比较值
	TIM8->CR1|=1<<2;   		//设置只有计数溢出作为更新中断
	TIM8->CR1|=1<<3;   		//单脉冲模式	
	TIM8->CCMR1|=7<<12;  	//CH2 PWM2模式	
	TIM8->CCMR1|=1<<11; 	//CH2预装载使能			
	TIM8->CCER|=1<<4;   	//OC2 输出使能
	TIM8->BDTR|=1<<15; 		//MOE 主输出使能
	TIM8->CR1|=0x0080;  	//ARPE使能 
	TIM8->DIER|=1<<0;   	//允许更新中断
	MY_NVIC_Init(1,1,TIM8_UP_TIM13_IRQn,2);//抢占1，子优先级1，组2	
	TIM8->SR=0;//清除所有标志位
	TIM8->EGR|=0x01;   //产生一个更新事件 重新初始化计数器
//	TIM8->CR1|=0x01; //使能定时器8 										  
}
/******* TIM8更新中断服务程序 *********/
void TIM8_UP_TIM13_IRQHandler(void)
{
	if(TIM8->SR&(1<<0))//更新中断
	{
		TIM8->SR&=~(1<<0);//清除更新中断标志位	
		TIM8->EGR|=0x01;   //产生一个更新事件 重新初始化计数器
		TIM8->BDTR|=1<<15; //MOE 主输出使能
		TIM8->CR1|=0x01;   //使能定时器8
//		if(is_rcr_finish==0)//重复计数器未设置完成
//		{
//			if(rcr_integer!=0) //整数部分脉冲还未发送完成
//			{
//				TIM8->RCR=RCR_VAL;//设置重复计数值
//				rcr_integer--;//减少RCR_VAL+1个脉冲				
//			}else if(rcr_remainder!=0)//余数部分脉冲 不位0
//			{
//				TIM8->RCR=rcr_remainder-1;//设置余数部分
//				rcr_remainder=0;//清零
//				is_rcr_finish=1;//重复计数器设置完成				
//			}else goto out;//rcr_remainder=0，直接退出			
//			TIM8->EGR|=0x01;   //产生一个更新事件 重新初始化计数器
//			TIM8->BDTR|=1<<15; //MOE 主输出使能
//			TIM8->CR1|=0x01;   //使能定时器8			
//			if(motor_dir==CW)  //如果方向为顺时针   
//				current_pos+=(TIM8->RCR+1);//加上重复计数值
//			else      //否则方向为逆时针
//				current_pos-=(TIM8->RCR+1);//减去重复计数值			
//		}else
//		{
//out:		is_rcr_finish=1;//重复计数器设置完成
//			TIM8->BDTR&=~(1<<15);//MOE 关闭主输出
//			TIM8->CR1&=~(1<<0);  //关闭定时器8						
//			printf("当前位置=%ld\r\n",current_pos);//打印输出
//		}	
	}
}
/***************** 启动TIM8 *****************/
void TIM8_Startup(u32 frequency)   //启动定时器8
{
	TIM8->ARR=1000000/frequency-1; //设定重装值	
	TIM8->CCR2=TIM8->ARR>>1;   //匹配值2等于重装值一半，是以占空比为50%	
	TIM8->CNT=0;//计数器清零
	TIM8->CR1|=1<<0;   //启动定时器TIM8计数
}






