#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "Driver_OLED.h"
#include "driver_timer.h"

//#include "cmsis_os.h"
//#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
//#include "task.h" 
//#include "queue.h"
#include "Driver_DHT11.h"

//QueueHandle_t OLEDQueueHandle;
extern QueueHandle_t OLEDQueueHandle;

extern I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef *hi2C1Handle=&hi2c1;

extern uint8_t ascii_font[128][16];

#define Dev_Addr 0x78
#define OLED_Timeout 50

typedef enum
{
	PumpEnable=1,
	PumpDisable=0,
}Pump_status;

typedef enum
{
	Remapping_Enable=0xA1,
	Remapping_Disable=0xA0,
}segment_remapping_status;

typedef enum
{
	Reverse_Enable=0xC0,
	Reverse_Disable=0xC8,
}COM_scan_direction;

typedef enum
{
	Yes=0xA4,
	No=0xA5,
}light_mode;

typedef enum
{
	Normal=0xA6,
	Reverse=0xA7,
}Display_mode;

typedef enum
{
	ON=0xAF,
	OFF=0xAE,
}DisplayOrNot;

typedef enum
{
	Level=0,
	Vertical=1,
	Page=2,
}MemAddrMode;

/*********************
* OLED发送指令 
* OLED发送数据 
* OLED发送一串数据 
**********************/
static void OLED_SendCmd(uint8_t cmd)
{
	uint8_t buf[2];
	buf[0]=0x00;
	buf[1]=cmd;
	
	HAL_I2C_Master_Transmit(hi2C1Handle, Dev_Addr, buf, 2, OLED_Timeout);
}

static void OLED_SendData(uint8_t data)
{
	uint8_t buf[2];
	buf[0]=0x40;
	buf[1]=data;
	
	HAL_I2C_Master_Transmit(hi2C1Handle, Dev_Addr, buf, 2, OLED_Timeout);
}

static void OLED_SendNDatas(uint8_t *data,uint8_t length)
{
	HAL_I2C_Mem_Write(hi2C1Handle,Dev_Addr,0x40,1,data,length,OLED_Timeout);
}	

/**********************
* OLED初始化
* 十二条指令一一执行
***********************/
static void ChargePumpRegulator_Enable(Pump_status status)
{
	if(status!=0&&status!=1) return;
	OLED_SendCmd(0x8D);
	OLED_SendCmd((status<<4)|0x04);
}

static void Set_multiplexing_ratio(uint8_t height)
{
	if(height>63 || height<15) return;
	OLED_SendCmd(0xA8);
	OLED_SendCmd(height);
}

static void Set_display_vertical_offset(uint8_t offset)
{
	if(offset>63 || offset<0) return;
	OLED_SendCmd(0xD3);
	OLED_SendCmd(offset);
}

static void Set_RAM_start_line(uint8_t line)
{
	if(line>63) return;
	OLED_SendCmd(0x40+line);
}

static void Set_segment_remapping(segment_remapping_status status)
{
	if((status!=Remapping_Enable)&&(status!=Remapping_Disable)) return;
	OLED_SendCmd(status);
}

static void Set_COM_scan_direction(COM_scan_direction status)
{
	if((status!=Reverse_Enable)&&(status!=Reverse_Disable)) return;
	OLED_SendCmd(status);
}

static void Set_contrast_control(uint8_t light)
{
	if(light>0xff || light<0) return;
	OLED_SendCmd(0x81);
}

static void Disable_fullscreen_wake(light_mode status)
{
	if((status!=Yes)&&(status!=No)) return;
	OLED_SendCmd(status);
}
 
static void Set_normal_display(Display_mode status)
{
	if((status!=Normal)&&(status!=Reverse)) return;
	OLED_SendCmd(status);
}

static void Set_oscillator_clock_division(uint8_t freq)
{
	if(freq>0xff) return;
	OLED_SendCmd(0xD5);
	OLED_SendCmd(freq);
}

static void Set_COM_pin_configuration(uint8_t state)
{
	OLED_SendCmd(0xDA);
	OLED_SendCmd(state);
}

static void Turn_on_display(DisplayOrNot state)
{
	OLED_SendCmd(state);
}

void OLED_Init(void)
{
	ChargePumpRegulator_Enable(PumpEnable); /*使能电荷泵稳压器*/
	Set_multiplexing_ratio(0x3f);			/*设置多路复用比（屏幕高度）*/
	Set_display_vertical_offset(0x00);		/*设置显示垂直偏移*/
	Set_RAM_start_line(0);					/*设置显示RAM起始行*/
	Set_segment_remapping(Remapping_Enable);/*设置段重映射（左右镜像）*/				
	Set_COM_scan_direction(Reverse_Disable);/*设置COM扫描方向（上下镜像）*/
	Set_contrast_control(0x7f);				/*设置对比度控制*/
	Disable_fullscreen_wake(Yes);			/*禁用全屏点亮*/
	Set_normal_display(Normal);				/*设置正常显示*/
	Set_oscillator_clock_division(0x80);	/*设置振荡时钟分频和刷新频率*/
	Set_COM_pin_configuration(0x02);		/*设置COM引脚硬件配置*/
	Turn_on_display(ON);					/*开启显示 */
}

/******************************************
* 设置起始页地址和列地址
* 包含四个函数：
*	设置页寻址模式
*	设置页地址
*	设置列地址
*	以及上面两个函数的封装___同时设置页和列
*******************************************/
static void Set_MemAddrMode(MemAddrMode Mode)
{
	if(Mode!=0&&Mode!=1&&Mode!=2) return;
	OLED_SendCmd(0x20);
	OLED_SendCmd(Mode);
}

static void Set_page_addr_pagemode(uint8_t page)
{
	Set_MemAddrMode(Page);
	if(page>7) return;
	OLED_SendCmd(0xB0+page);
}

static void Set_Column_addr_pagemode(uint8_t column)
{
	Set_MemAddrMode(Page);
	if(column>127) return;
	OLED_SendCmd(column&0x0f);
	OLED_SendCmd(0x10+(column>>4));
}

static void Set_addr_pagemode(uint8_t page,uint8_t column)
{
	Set_page_addr_pagemode(page);
	Set_Column_addr_pagemode(column);
}

/**************************
* 显示的三个函数
* ——显示一个字符
* ——显示字符串
* ——显示十进制数
***************************/
void OLED_ShowChar(uint8_t x,uint8_t y,char content)
{
	if(x>15 || y>7) return;
	
	Set_addr_pagemode(y,x*8);
	
	OLED_SendNDatas(&ascii_font[content][0],8);
	
	Set_addr_pagemode(y+1,x*8);
	
	OLED_SendNDatas(&ascii_font[content][8],8);
}

//int OLED_ShowString(uint8_t x,uint8_t y,uint8_t* String)
//{
//	uint8_t *Str=String;
//	int i=0;
//	while(Str[i]!='0')
//	{
//		OLED_ShowChar(x++,y,Str[i]);
//		
//		/*加上下面的这个处理比较好，我没考虑到*/
//		if(x>15)
//		{
//			x=0;
//			y+=2;
//		}			
//		i++;		
//	}
//	return i;
//}

int OLED_ShowString(uint8_t x,uint8_t y,uint8_t* String)
{
	uint8_t *Str=String;
	int i=0;
	//	while(Str[i]!='0') 这个是错误的，本意是想判断是不是到字符串末尾标识符了，但符号不是这个，是'\0'或者0
	while(Str[i]!='\0')
	{
		OLED_ShowChar(x++,y,Str[i]);
		
		/*加上下面的这个处理比较好，我没考虑到*/
		if(x>15)
		{
			x=0;
			y+=2;
		}			
		i++;		
	}
	return i;
}


int OLED_ShowDecimal(uint8_t x,uint8_t y,int32_t Number)
{
	uint8_t Num[10]={0};
	int i=0;
	int length=0;
	if(Number==0)
	{
		OLED_ShowChar(x,y,'0');
//		OLED_ShowChar(x,y,0); 这样写是不对的	
		return 1;
	}
	if(Number<0)
	{		
		OLED_ShowChar(x++,y,'-');
		Number=-Number;// 注意：此处未处理 INT32_MIN (-2147483648) 的极端情况
		length++;
	}
	while(Number)
	{
		Num[i++]=Number%10;
		Number/=10;
	}
	length+=i;
	while(i>0)
	{
		OLED_ShowChar(x++,y,Num[--i]+'0');
//		OLED_ShowChar(x++,y,Num[--i]);同理，这样写不对
	}
	return length;
}

/*OLED清屏函数*/
void OLED_Clear(void)
{
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<128;j++)
		{
			Set_addr_pagemode(i,j);
			OLED_SendData(0x00);
		}
	}	
}

/*OLED测试函数*/
void OLED_Test(void)
{
	int Tem=0x20;
	int Humi=95;
	int len_temp,len_humi,flag_temp;
	int flag_humi=0;
	OLED_Init();
	OLED_Clear();
	while(1)
	{		
		if(Tem>25)
		{
			if(flag_temp) 
			{
				Tem=0x20;
				flag_temp=0;
			}
			Tem--;
		}
		if(Tem<=25)
		{
			flag_temp=1;
			Tem++;
		}
		
		if(Humi>82)
		{
			if(flag_humi) 
			{
				Humi=95;
				flag_humi=0;
			}
			Humi--;
		}
		if(Humi<=82)
		{
			flag_humi=1;
			Humi++;
		}
		
//		OLED_ShowChar(15,0,'A');
//		OLED_ShowString(0,2,"BCD");
//		OLED_ShowDecimal(4,6,98765);
		len_temp=OLED_ShowString(0,0,"Tem:");
		OLED_ShowDecimal(len_temp,0,Tem);
		OLED_ShowString(len_temp+2,0,"^");
		
		len_humi=OLED_ShowString(0,2,"Hum:");
		OLED_ShowDecimal(len_humi,2,Humi);
		OLED_ShowString(len_humi+2,2,"%");
		
		OLED_ShowString(len_temp+4,0,"Warning!");
		OLED_ShowString(len_temp+4,2,"Danger!");
		OLED_ShowString(len_temp+4,0,"        ");
		OLED_ShowString(len_temp+4,2,"        ");
		HAL_Delay(1000);
		OLED_ShowString(len_temp+4,0,"Warning!");
		OLED_ShowString(len_temp+4,2,"Danger!");
		HAL_Delay(1000);
	}
}

//void OLEDTask(void* params)
//{
//	int len_temp,len_humi;
//	int temp,humi;
//	int err;
//	OLED_Init();
//	OLED_Clear(); 	
//	while(1)
//	{	
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
////			DHT11_Init();
//		}			
//		vTaskDelay(2000);
//	}
//}

void OLEDTask(void* params)
{
	int len_temp,len_humi;
	int temp,humi;
	
//	OLED_Init();这里初始化这些应该放到任务创建前解决，在任务中执行初始化很容易打断DHT11的数据处理
//	OLED_Clear(); 	
	
	Data OLEDData;
	
//	/*创建并注册OLED队列*/ 如果是这样，在任务里创建、注册队列，那那边DHT11任务先执行,分发数据给队列，
						//发现OLED队列还没创建，就容易出错，所以最好是：先统一创建队列，再统一创建任务。
//	OLEDQueueHandle=xQueueCreate(10,sizeof(struct DHT11Data));
//	RegisterQueue(OLEDQueueHandle);
	
	while(1)
	{	
		/*读队列数据*/
		xQueueReceive(OLEDQueueHandle,&OLEDData,portMAX_DELAY);
			
		if(OLEDData.err==1)
		{
			OLED_ShowString(5,0,"err");
		}	
		else if(OLEDData.err==0)
		{
			/*数据处理*/
			temp=OLEDData.temp;
			humi=OLEDData.humi;
			
			len_temp=OLED_ShowString(0,0,"Tem:");
			OLED_ShowDecimal(len_temp,0,temp);
			OLED_ShowString(len_temp+2,0,"^");
			
			len_humi=OLED_ShowString(0,2,"Hum:");
			OLED_ShowDecimal(len_humi,2,humi);
			OLED_ShowString(len_humi+2,2,"%");	
		}
		vTaskDelay(50);
	}
}




	
