#ifndef __DRIVER_DHT11_H_
#define __DRIVER_DHT11_H_

#include "cmsis_os.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 
#include "queue.h"

typedef struct DHT11Data{
	int temp;
	int humi;
	int err;
}Data;

void DHT11_Init(void);
int DHT11ReadData(int *temp,int *humi);
void DHT11_Test(void);
void DHT11Task(void *params);

void RegisterQueue(QueueHandle_t QueueHandle);
void DispatchData(struct DHT11Data *pData);

#endif