#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "Driver_Buzzer.h"

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 

extern TIM_HandleTypeDef htim2;
TIM_HandleTypeDef *TimHandle=&htim2;

void SetBuzzer_freq_duty(int freq,int duty)
{
	TIM_OC_InitTypeDef sConfigOC = {0};
	
	HAL_TIM_PWM_DeInit(TimHandle);
	TimHandle->Instance = TIM2;
	TimHandle->Init.Prescaler = 71;
	TimHandle->Init.CounterMode = TIM_COUNTERMODE_UP;
	TimHandle->Init.Period = 1000000/freq-1;
	TimHandle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TimHandle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(TimHandle);
	
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = (1000000/freq-1)*duty/100;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(TimHandle, &sConfigOC, TIM_CHANNEL_1);
}

void Buzzer_On(void)
{
	HAL_TIM_PWM_Start(TimHandle,TIM_CHANNEL_1);
}

void Buzzer_Off(void)
{
	HAL_TIM_PWM_Stop(TimHandle,TIM_CHANNEL_1);
}

void Buzzer_toggle(void)
{
	Buzzer_On();
//	Delay_ms(500);
	vTaskDelay(200);
	Buzzer_Off();
//	Delay_ms(500);
	vTaskDelay(200);
}

//void Buzzer_On(void)
//{
//	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
//}

//void Buzzer_Twinkle(void)
//{
//	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_4);
//	Delay_ms(500);
//	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_4);	
//}

//void Buzzer_OFF(void)
//{
//	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);	
//}
