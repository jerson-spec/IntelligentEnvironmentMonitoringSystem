#include <stdint.h>
#include "Driver_DHT11.h"
#include "Driver_color_led.h"
#include "ColorLED_Warning.h"

extern QueueHandle_t ColorLED_WarnQueueHandle;
extern TaskHandle_t UARTHandle;

typedef enum Grade_1_Value{
	temp_min=30,
//	humi_min=80,
	humi_min=90,
}Grade1;

typedef enum Grade_2_Value{
	temp_max=35,
//	humi_max=90,
	humi_max=93,
}
Grade2;

#define Warn 2
#define Danger 3

void ColorLED_WarningTask(void *params)
{
	Data WarnData;
	int temp,humi;
	while(1)
	{
		/*读队列数据*/
		xQueueReceive(ColorLED_WarnQueueHandle,&WarnData,portMAX_DELAY);
		
		/*数据处理*/
		if(WarnData.err==0)
		{
			temp=WarnData.temp;
			humi=WarnData.humi;
			
			/*分级*/
			if(temp<temp_min&&humi<humi_min)
				SetGreen();
			else if((temp>=temp_min&&temp<=temp_max)||(humi>=humi_min&&humi<=humi_max))
			{
				xTaskNotify(UARTHandle,Warn,eSetValueWithOverwrite);
				SetYellow_Twinkle();
			}
			else if(temp>temp_max||humi>humi_max)
			{
				xTaskNotify(UARTHandle,Danger,eSetValueWithOverwrite);
				SetRed_Twinkle();
			}		
		}
		vTaskDelay(50);
	}
}
