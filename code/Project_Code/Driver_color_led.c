#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "Driver_color_led.h"

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 

/*四种颜色LED灯常亮*/
/*————推挽模式下，引脚给低电平全彩lED灯亮*/
void SetRed(void) 
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
}

void SetBlue(void) 
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
}

void SetGreen(void) 
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
}

void SetYellow(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_5,GPIO_PIN_RESET);
}

/*四种颜色LED灯熄灭*/
/*————推挽模式下，引脚给高电平全彩lED灯灭*/
void SetRed_off(void) 
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
}

void SetBlue_off(void) 
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
}

void SetGreen_0ff(void) 
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
}

void SetYellow_0ff(void)
{
	SetRed_off();
	SetGreen_0ff();
}

/*三种颜色（绿、黄、红）LED灯闪烁*/
void SetRed_Twinkle(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	vTaskDelay(500);
//	vTaskDelay(200);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
//	vTaskDelay(500); //这里可以取消注释也可以不取消，注释的话个人感觉效果好一点
//	vTaskDelay(200);
}

void SetGreen_Twinkle(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
	vTaskDelay(500);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
	vTaskDelay(500);
}

void SetYellow_Twinkle(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
//	Delay_ms(500);
//	vTaskDelay(200);
	vTaskDelay(500);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
//	Delay_ms(500);
//	vTaskDelay(200);
//	vTaskDelay(500);//这里可以取消注释也可以不取消，注释的话个人感觉效果好一点
}

