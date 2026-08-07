#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "Driver_LED.h"
#include "driver_timer.h"

void LED_Light(void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
}

void LED_Twinkle(void)
{
	HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
//	Delay_ms(500);
//	vTaskDelay(200);
	vTaskDelay(500);
	HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);	
}

void LED_OFF(void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);	
}