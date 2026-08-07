//#include "stm32f10x.h"                  // Device header
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "driver_timer.h"
/****************************************************************************
* 概述：定义三个函数：延迟（us级）、延迟（ms级）、获得系统时间（ns级）
* ---------------------------------------------------------------------------
* 必要性：虽然HAL库有自带的HAL_GetTick()-获得毫秒时间函数、HAL_Delay(...)-
*   延迟（ms级），但也能看出来，他只能停留在ms级，对于高精度us、ns就不行了。
* ---------------------------------------------------------------------------
* 实现:我们是这样实现的，先CubeMx配置一个TIM1定时器，配置部分只需要给他配个
*   内部时钟就成了，然后设置PSC:71,ARR:999，这样定时就是1ms，然后计数器CNT占
*   ARR之比就是更细的划分，更小的ms。（韦东山因为资源有限这一块没有单独拉一个
*   计时器，是复用的HAL库使用的TIM4定时器，这样也行但定时易被干扰，详情见豆。）
*****************************************************************************/

void Delay_us(int us)
{
	/*定时器句柄转个变量存一下*/
	extern TIM_HandleTypeDef htim1;
	TIM_HandleTypeDef *hTimhandle=&htim1;
	
	/*定义变量*/
	uint32_t ticks;
	uint32_t told,tnew,tcnt=0;
	ticks=us*(__HAL_TIM_GET_AUTORELOAD(hTimhandle))/1000;
	
	/*处理逻辑*/
	told=__HAL_TIM_GET_COUNTER(hTimhandle);
	while(1)
	{
		tnew=__HAL_TIM_GET_COUNTER(hTimhandle);
		if(told!=tnew)
		{
			if(tnew>told)
				tcnt+=tnew-told;
			else 
				tcnt+=tnew-told+__HAL_TIM_GET_AUTORELOAD(hTimhandle);
		
			/*下面这一行差点忘记加了*/
			told=tnew;				
			if(tcnt>=ticks) 
				break;
		}
	} 
}

void Delay_ms(int ms)
{
	int i;
	for(i=0;i<ms;i++)
	{
		Delay_us(1000);
	}
}

uint64_t GetSystemTimer_ns(void)
{
	extern TIM_HandleTypeDef htim1;
	TIM_HandleTypeDef *hTimhandle=&htim1;
	
	/*一开始用uint32_t，太小了，得用uint64_t */
	uint64_t ns;
	ns=HAL_GetTick()*1000000;
	
	ns+=__HAL_TIM_GET_COUNTER(hTimhandle)*1000000/__HAL_TIM_GET_AUTORELOAD(hTimhandle);
	
	return ns;
}