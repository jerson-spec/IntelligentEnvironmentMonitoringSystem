//#include "stm32f10x.h"                  // Device header
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "driver_timer.h"
#include "Driver_DHT11.h"
#include "Driver_OLED.h"

#include "cmsis_os.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 
//#include "queue.h"


/***********************
* DHT11相关函数
*  ——DHT11初始化
*  ——DHT11读取数据
*  ——测试程序
************************/

static int DHT11_PinRead(void);

static void GPIO_Set(int val)
{
	if(val) HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
}

void DHT11_Init(void)
{
	GPIO_Set(1);
	Delay_ms(2000);
}

static void DHT11_Start(void)
{
	GPIO_Set(0);
	Delay_ms(20);
	GPIO_Set(1);
//	Delay_us(30);
}

static int WaitForBit(int val,int timeout_us)
{
	while (timeout_us--)
	{
		if (DHT11_PinRead() == val)
			return 0; /* ok */
		Delay_us(1);
	}
	return -1; /* err */
}

static int DHT11ReadByte(void)
{
	//	int Byte;Byte初始化没赋值，是一个不知道什么值，那下面的数据肯定就是错的，所以一定要初始化值0。
	int Byte=0;
	for(int i=0;i<8;i++)
	{
		if(WaitForBit(1,1000))
		{
			return -1;
		}
		Delay_us(40);
//		Byte<<1;——我靠，这里这个没改，OLED一直显示err，都快给我整崩溃了。
		Byte<<=1;
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==1)
//			Byte|=(1<<i);DHT11是先发数据高位再发低位（高位先出），bit7-bit6...这样的顺序，这样写恰巧完全反了。
			Byte|=1;
		if(WaitForBit(0,1000))
		{
			return -1;
		}
	}
	return Byte;
}

static int DHT11_PinRead(void)
{
    if (GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
		return 1;
	else
		return 0;
}

static int DHT11_Wait_Ack()
{
	Delay_us(60);
	return DHT11_PinRead();
}


int DHT11ReadData(int *temp,int *humi)
{
	int humi_h,humi_l,temp_h,temp_l,check;
	
	/*主机发起始信号*/
	DHT11_Start();
	
	/*等待响应*/
	if(DHT11_Wait_Ack()!=0)
	{
		return -1;
	}
	
	/*准备接收数据*/
	if(WaitForBit(1,1000))
	{
		return -1;
	}
	
	if(WaitForBit(0,1000))
	{
		return -1;
	}
	
	/*接收五个字节*/
	humi_h=DHT11ReadByte();
	humi_l=DHT11ReadByte();
	temp_h=DHT11ReadByte();
	temp_l=DHT11ReadByte();
	check=DHT11ReadByte();
	
	/*验证数据*/
	if(check==humi_h+humi_l+temp_h+temp_l)
	{
		*humi=humi_h;
		*temp=temp_h;
		return 0;
	}
	else return -1;
	
	/*数据处理*/
//	*humi=humi_h+(float)humi_l/100;
//	if(temp_l>>7)
//	{
//		temp_l=(temp_l<<1)/2;
//		*temp=-(temp_h+(float)temp_l/100);
//	}		
//	else *temp=temp_h+(float)temp_l/100;
}

//void DHT11_Test(void)
//{
//	int len_temp,len_humi;
//	int temp,humi;
//	int err;
//	OLED_Init();
//	OLED_Clear(); 
//	DHT11_Init();
//	while(1)
//	{
//		vTaskSuspendAll();
//		err=DHT11ReadData(&temp,&humi);
//		xTaskResumeAll();
//		
//		if(err==0)
//		{
//			len_temp=OLED_ShowString(0,0,"Tem:");
//			OLED_ShowDecimal(len_temp,0,temp);
//			OLED_ShowString(len_temp+2,0,"^");
//			
//			len_humi=OLED_ShowString(0,2,"Hum:");
//			OLED_ShowDecimal(len_humi,2,humi);
//			OLED_ShowString(len_humi+2,2,"%");	
//		}
//		else
//		{
//			OLED_ShowString(5,0,"err");
//			DHT11_Init();
//		}			
//		Delay_ms(2000);
//	}
//}		


//void DHT11Task(void *params)
//{
//	DHT11_Test();
//}

QueueHandle_t Queues[10];
int QueueCnt;

void RegisterQueue(QueueHandle_t QueueHandle)
{
	if(QueueCnt<10)
	{
		Queues[QueueCnt]=QueueHandle;
		QueueCnt++;	
	}
}

//typedef struct DHT11Data{
//	int temp;
//	int humi;
//}Data;


void DispatchData(struct DHT11Data *pData)
{
	for(int i=0;i<QueueCnt;i++)
	{
		xQueueSend(Queues[i],pData,0);
	}
}

void DHT11Task(void *params)
{
	int err;
	int temp,humi;
//	DHT11_Init(); 这里初始化最好放到任务创建前解决
	Data DHT11Data;
	while(1)
	{
		/*我们这个项目没有用到中断，所以用开关调度器保护就行，但凡涉及到中断，就要用开关中断，牢牢保护DHT11的数据读取*/
		vTaskSuspendAll();
		err=DHT11ReadData(&temp,&humi);
		xTaskResumeAll();
		if(err==0)
		{
			DHT11Data.temp=temp;
			DHT11Data.humi=humi;
			DHT11Data.err=0;//这里本来是使用全局变量err的，但是容易出错，还是把他写进结构体的一部分吧。	
		}
		else
		{
			DHT11Data.err=1;
//			OLED_ShowString(5,0,"err");	
			DHT11_Init();
		}
		DispatchData(&DHT11Data);
		
		vTaskDelay(2000);
	}
}