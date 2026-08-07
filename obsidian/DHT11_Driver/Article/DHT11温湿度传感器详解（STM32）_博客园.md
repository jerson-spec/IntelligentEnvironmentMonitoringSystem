---
title: "DHT11温湿度传感器详解（STM32）"
source: "https://www.cnblogs.com/yfceshi/p/19002522"
author:
  - "[[yfceshi]]"
published: 2025-07-24
created: 2026-07-09
description: "目录 一、介绍 二、传感器原理1.原理图2.工作时序3.起始信号与响应信号4.读数据时序 5.DHT11数据格式三、程序设计 main.c文件 dht11.h文件 dht11.c文件四、实验效果展示 五、资料获取 项目分享"
tags:
  - "clippings"
---
**目录**

[一、介绍](#t0)

[二、传感器原理](#t1)

[1.原理图](#t2)

[2.工作时序](#t3)

[3.起始信号与响应信号](#t4)

[4.读数据时序](#t5)

[5.DHT11数据格式](#t6)

[三、程序设计](#t7)

[main.c文件](#t8)

[dht11.h文件](#t9)

[dht11.c文件](#t10)

[四、实验效果展示](#t11)

[五、资料获取](#t12)

[项目分享](#t13)

## 一、介绍

DHT11是一款含有已校准数字信号输出的温湿度复合传感器，采用了自主研发的集成式数字温湿度元件，应用专用的数字模块采集技术和温湿度传感技术，确保产品具有极高的可靠性与卓越的长期稳定性。DHT11传感器内包含一个温湿度测量元件和一个高性能MCU。

![](https://i-blog.csdnimg.cn/direct/d2af7e8ff09d4afc8c6192986a9395fd.jpeg)

以下是 [DHT11温湿度传感器](https://so.csdn.net/so/search?q=DHT11%E6%B8%A9%E6%B9%BF%E5%BA%A6%E4%BC%A0%E6%84%9F%E5%99%A8&spm=1001.2101.3001.7020) 的参数：

| 供电电压 | DC：3.3-5.5V |
| --- | --- |
| 工作范围(温度) | \-20~+60℃ |
| 量程范围(湿度) | 5~95%RH |
| 温度精度 | ±2℃ |
| 湿度精度 | ±5%RH |
| 重复性 | 温度：±1℃ ；湿度：±1%RH |
| 迟滞(温度) | ±0.3℃ |
| 迟滞(湿度) | ±0.3%RH |

**哔哩哔哩视频链接：**

**[https://www.bilibili.com/video/BV182421Z7by/?share\_source=copy\_web&vd\_source=097fdeaf6b6ecfed8a9ff7119c32faf2](https://www.bilibili.com/video/BV182421Z7by/?share_source=copy_web&vd_source=097fdeaf6b6ecfed8a9ff7119c32faf2 "https://www.bilibili.com/video/BV182421Z7by/?share_source=copy_web&vd_source=097fdeaf6b6ecfed8a9ff7119c32faf2")**

**（资料分享见文末）**

## 二、传感器原理

### 1.原理图

![](https://i-blog.csdnimg.cn/direct/a44b9b88713c4f9c95deafecf723319a.png) 单总线上必须有一个上拉电阻（R1）以实现单总线闲置时，其处于高电平状态

### 2.工作时序

![](https://i-blog.csdnimg.cn/direct/b646d1b9897d415286995e320580fcca.png)

![](https://i-blog.csdnimg.cn/direct/3178f54b378243229c6a82e91d2c2eac.png)

| 符号 | 参数 | 最小 | 典型 | 最大 | 单位 |
| --- | --- | --- | --- | --- | --- |
| Tbe | 主机起始信号拉低时间 | 18 | 20 | 30 | ms |
| Tgo | 主机释放单总线时间 | 10 | 13 | 35 | us |
| Trel | 响应低电平时间 | 78 | 83 | 88 | us |
| Treh | 响应高电平时间 | 80 | 87 | 92 | us |

### 3.起始信号与响应信号

![](https://i-blog.csdnimg.cn/direct/f8c6337df5244a278c3d6470fc418313.png)

### 4.读数据时序

![](https://i-blog.csdnimg.cn/direct/01584343ab9e40cd844b667755645825.png)

![](https://i-blog.csdnimg.cn/direct/285027a70d924aa18cc27578c9da2e86.png)

### 5.DHT11数据格式

![](https://i-blog.csdnimg.cn/direct/e6537633c8be447ca67b9245d1d6277d.png)

34H + 01H + 18H + 8CH = D9H

湿度高8位（整数）为34H，低8位（小数）为01H，将两部分数值转换为十进制后可以得出52.1，即湿度为52.1%RH。同理可以得出图7中的温度为-24.12℃。此处温度为负值时因为温度数据的低8位的最高位Bit7为1；当最高位Bit7为0时，数值为正值。

## 三、程序设计

1.使用STM32F103C8T6读取DHT11温湿度传感器采集的数据，通过串口发送至电脑

2.将读取得到的温湿度数据同时在OLED上显示

| DHT11 | PB5 |
| --- | --- |
| OLED\_SCL | PB11 |
| OLED\_SDA | PB10 |
| 串口 | 串口1 |

### main.c文件

```cpp
#include "stm32f10x.h"
#include "led.h"
#include "usart.h"
#include "delay.h"
#include "dht11.h"
#include "oled.h"
/*****************辰哥单片机设计******************
                      STM32
 * 项目            :    1.DHT11温度湿度传感器实验
 * 版本            :   V1.0
 * 日期            :   2024.8.4
 * MCU            :    STM32F103C8T6
 * 接口            :    参看DHT11.h
 * BILIBILI        :    辰哥单片机设计
 * CSDN            :    辰哥单片机设计
 * 作者            :    辰哥
**********************BEGIN***********************/
u8 temp;
u8 humi;
int main(void)
{
SystemInit();//配置系统时钟为72M
delay_init(72);
LED_Init();
LED_On();
USART1_Config();//串口初始化
OLED_Init();
printf("Start \n");
delay_ms(1000);
while(DHT11_Init())
{
printf("DHT11 Error \r\n");
delay_ms(1000);
}
//显示“温度：”
OLED_ShowChinese(1,1, 0);
OLED_ShowChinese(1,2, 1);
OLED_ShowChar(1, 5, ':');
OLED_ShowChar(1, 9, 'C');
//显示“湿度：”
OLED_ShowChinese(2,1, 2);
OLED_ShowChinese(2,2, 1);
OLED_ShowChar(2, 5, ':');
OLED_ShowChar(2, 9, '%');
while (1)
{
DHT11_Read_Data(&temp,&humi);//
printf("temp %d ,humi %d\r\n",temp,humi);
LED_Toggle();
delay_ms(1000);
//显示温度数据
OLED_ShowNum(1,6,temp,2);
//显示湿度数据
OLED_ShowNum(2,6,humi,2);
}
}
```

### dht11.h文件

```cpp
#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"                  // Device header
#include "delay.h"
/*****************辰哥单片机设计******************
                    STM32
 * 文件            :    DHT11温度湿度传感器h文件
 * 版本            :   V1.0
 * 日期            :   2024.8.4
 * MCU            :    STM32F103C8T6
 * 接口            :    见代码
 * BILIBILI        :    辰哥单片机设计
 * CSDN            :    辰哥单片机设计
 * 作者            :    辰哥
**********************BEGIN***********************
/***************根据自己需求更改****************/
//DHT11引脚宏定义
#define DHT11_GPIO_PORT  GPIOA
#define DHT11_GPIO_PIN   GPIO_Pin_6
#define DHT11_GPIO_CLK   RCC_APB2Periph_GPIOA
/*********************END**********************/
//输出状态定义
#define OUT 1
#define IN  0
//控制DHT11引脚输出高低电平
#define DHT11_Low  GPIO_ResetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
#define DHT11_High GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
u8 DHT11_Init(void);//初始化DHT11
u8 DHT11_Read_Data(u8 *temp,u8 *humi);//读取温湿度数据
u8 DHT11_Read_Byte(void);//读取一个字节的数据
u8 DHT11_Read_Bit(void);//读取一位的数据
void DHT11_Mode(u8 mode);//DHT11引脚输出模式控制
u8 DHT11_Check(void);//检测DHT11
void DHT11_Rst(void);//复位DHT11
#endif
```

### dht11.c文件

```cpp
#include "dht11.h"
#include "delay.h"
/*****************辰哥单片机设计******************
                                            STM32
 * 文件            :    DHT11温度湿度传感器c文件
 * 版本            : V1.0
 * 日期            : 2024.8.4
 * MCU            :    STM32F103C8T6
 * 接口            :    见dht11.h文件
 * BILIBILI    :    辰哥单片机设计
 * CSDN            :    辰哥单片机设计
 * 作者            :    辰哥
**********************BEGIN***********************/
//复位DHT11
void DHT11_Rst(void)
{
DHT11_Mode(OUT);     //SET OUTPUT
DHT11_Low;           //拉低DQ
delay_ms(20);        //主机拉低18~30ms
DHT11_High;             //DQ=1
delay_us(13);         //主机拉高10~35us
}
//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
u8 DHT11_Check(void)
{
u8 retry=0;
DHT11_Mode(IN);//SET INPUT
while (GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry100)//DHT11会拉低40~80us
{
retry++;
delay_us(1);
};
if(retry>=100)return 1;
else retry=0;
while (!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry100)//DHT11拉低后会再次拉高40~80us
{
retry++;
delay_us(1);
};
if(retry>=100)return 1;
return 0;
}
//从DHT11读取一个位
//返回值：1/0
u8 DHT11_Read_Bit(void)
{
u8 retry=0;
while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry100)//等待变为低电平
{
retry++;
delay_us(1);
}
retry=0;
while(!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry100)//等待变高电平
{
retry++;
delay_us(1);
}
delay_us(40);//等待40us
if(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN))return 1;
else return 0;
}
//从DHT11读取一个字节
//返回值：读到的数据
u8 DHT11_Read_Byte(void)
{
u8 i,dat;
dat=0;
for (i=0;i8;i++)
{
dat1;
dat|=DHT11_Read_Bit();
}
return dat;
}
//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
u8 DHT11_Read_Data(u8 *temp,u8 *humi)
{
u8 buf[5];
u8 i;
DHT11_Rst();
if(DHT11_Check()==0)
{
for(i=0;i5;i++)//读取40位数据
{
buf[i]=DHT11_Read_Byte();
}
if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
{
*humi=buf[0];
*temp=buf[2];
}
}
else return 1;
return 0;
}
//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在
u8 DHT11_Init(void)
{
GPIO_InitTypeDef  GPIO_InitStructure;
RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);     //使能PA端口时钟
GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;                 //PG11端口配置
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;          //推挽输出
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);                 //初始化IO口
GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN);                         //PG11 输出高
DHT11_Rst();  //复位DHT11
return DHT11_Check();//等待DHT11的回应
}
void DHT11_Mode(u8 mode)
{
GPIO_InitTypeDef GPIO_InitStructure;
if(mode)
{
GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
}
else
{
GPIO_InitStructure.GPIO_Pin =  DHT11_GPIO_PIN;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
}
GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}
```

![](https://i-blog.csdnimg.cn/direct/21e7f2eb560243d5837513e6f6e0daef.jpeg)