#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "Driver_LED.h"
#include "Driver_Buzzer.h"
#include "Driver_color_led.h"

//#include "cmsis_os.h"
//#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
//#include "task.h" 
//#include "Queue.h"
#include "Driver_DHT11.h"

//void WarningTask(void *params)
//{
//	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
//	while(1)
//	{
////		SetRed(); 
////		SetBlue(); 
////		SetGreen(); 
////		SetYellow();
////		SetRed_Twinkle();
////		SetGreen_Twinkle();
//		SetYellow_Twinkle();
//	}		
//}

//void WarningTask(void *params)
//{
//	SetBuzzer_freq_duty(1000,1);
//	while(1)
//	{
////		Buzzer_On();
////		Buzzer_Off();
//		Buzzer_toggle();
//	}
//}

//extern TIM_HandleTypeDef htim2;

//void WarningTask(void *params)
//{
////	Buzzer_OFF();
//	while(1)
//	{
//		 HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
////		Buzzer_Twinkle();
//		vTaskDelay(500);
//		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
//		vTaskDelay(500);
//	}
//}

//void WarningTask(void *params)
//{
//	LED_OFF();
//	while(1)
//	{
//	//	LED_Light();
//		LED_Twinkle();
//	vTaskDelay(500);
//	}
//}
extern QueueHandle_t WarnQueueHandle;
extern TaskHandle_t UARTHandle;

typedef enum Grade_1_Value{
	temp_min=30,
	humi_min=80,
}Grade1;

typedef enum Grade_2_Value{
	temp_max=35,
	humi_max=90,
}Grade2;

#define Warn 2
#define Danger 3

void Grade_1_do(void)
{
	/*LED熄灭*/
	LED_OFF();
	/*蜂鸣器停止鸣叫*/
	Buzzer_Off();
	/*全彩灯绿灯、常亮*/
	SetGreen();
}
void Grade_2_do(void)
{
	xTaskNotify(UARTHandle,Warn,eSetValueWithOverwrite);
	/*LED闪烁*/
	LED_Twinkle();
	/*蜂鸣器间断鸣叫*/
	Buzzer_toggle();
	/*全彩灯黄灯、闪烁*/
	SetYellow_Twinkle();
}
void Grade_3_do(void)
{
	xTaskNotify(UARTHandle,Danger,eSetValueWithOverwrite);
	/*LED常亮*/
	LED_Light();
	/*蜂鸣器一直鸣叫*/
	Buzzer_On();
	/*全彩灯红灯、闪烁*/
	SetRed_Twinkle();
//	/*全彩灯红灯、常亮*/
//	SetRed();
}

void WarningTask(void *params)
{
	Data WarnData;
	int temp,humi;
	while(1)
	{
		/*读队列数据*/
		xQueueReceive(WarnQueueHandle,&WarnData,portMAX_DELAY);
		
		/*数据处理*/
		if(WarnData.err==0)
		{
			temp=WarnData.temp;
			humi=WarnData.humi;
			
			/*分级*/
			if(temp<temp_min&&humi<humi_min)
				Grade_1_do();
			else if((temp>temp_min&&temp<temp_max)||(humi>humi_min&&humi<humi_max))
				Grade_2_do();
			else if(temp>temp_max||humi>humi_max)
				Grade_3_do();
		}
		vTaskDelay(50);
	}
}

