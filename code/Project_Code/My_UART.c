#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "My_UART.h"

#include "cmsis_os.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 
#include "string.h"

extern UART_HandleTypeDef huart1;
UART_HandleTypeDef *UART_Handle=&huart1;

#define WarnNews "Warning!\n\r" 
#define DangerNews "Danger!\n\r"

//extern Warn;
//extern Danger;
#define Warn 2
#define Danger 3

void UART_TransNews(void* string)
{
	int len;
	len=strlen(string);
	uint8_t *Data=string;
	HAL_UART_Transmit(UART_Handle,Data,len,HAL_MAX_DELAY);
}

void My_UARTTask(void*params)
{
	int Value; 
	while(1)
	{
		do 
		{
			xTaskNotifyWait(~0,~0,&Value,portMAX_DELAY);
		}
		while(Value!=Warn&&Value!=Danger);
		
		if(Value==Warn)
		{
			UART_TransNews(WarnNews);
		}
		else if(Value==Danger)
		{
			UART_TransNews(DangerNews);
		}
		vTaskDelay(1000);
	}	
}