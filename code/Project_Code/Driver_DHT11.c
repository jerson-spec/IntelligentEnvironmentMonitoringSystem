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
	int Byte=0;
	for(int i=0;i<8;i++)
	{
		if(WaitForBit(1,1000))
		{
			return -1;
		}
		Delay_us(40);

		Byte<<=1;
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==1)
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
}
/*****************
* 注册队列
* 分发队列
*****************/
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
	Data DHT11Data;
	while(1)
	{
		/*我们这个项目没有用到中断，所以用开关调度器保护就行。*/
		vTaskSuspendAll();
		err=DHT11ReadData(&temp,&humi);
		xTaskResumeAll();
		if(err==0)
		{
			DHT11Data.temp=temp;
			DHT11Data.humi=humi;
			DHT11Data.err=0;
		}
		else
		{
			DHT11Data.err=1;
			DHT11_Init();
		}
		DispatchData(&DHT11Data);
		
		vTaskDelay(2000);
	}
}
