---
title: "DHT11与OLED任务冲突"
source: "https://thu.chatopens.vip/c/6a7443fd-cdb8-83ea-a479-53ef040658cd"
author:
published:
created: 2026-08-06
description: "ChatGPT 是一款供日常使用的 AI 聊天机器人。与最先进的 AI 模型互动，探索创意、解决问题以及提升学习效率。"
tags:
  - "clippings"
---
/\* USER CODE BEGIN Header \*/ /\*\* \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* \* @file: main.c \* @brief: Main program body \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* \* @attention \* \* Copyright (c) 2026 STMicroelectronics. \* All rights reserved. \* \* This software is licensed under terms that can be found in the LICENSE file \* in the root directory of this software component. \* If no LICENSE file comes with this software, it is provided AS-IS. \* \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* \*/ /\* USER CODE END Header \*/ /\* Includes ------------------------------------------------------------------\*/ #include "main.h" #include "cmsis\_os.h" /\* Private includes ----------------------------------------------------------\*/ /\* USER CODE BEGIN Includes \*/ #include "FreeRTOS.h" // ARM.FreeRTOS::RTOS:Core #include "task.h" #include "task.h" #include "Driver\_OLED.h" #include "Driver\_DHT11.h" #include "Warning.h" #include "My\_UART.h" /\* USER CODE END Includes \*/ /\* Private typedef -----------------------------------------------------------\*/ /\* USER CODE BEGIN PTD \*/ /\* USER CODE END PTD \*/ /\* Private define ------------------------------------------------------------\*/ /\* USER CODE BEGIN PD \*/ //BaseType\_t xDHT11TaskHandle; //BaseType\_t xOLEDTaskHandle; //BaseType\_t xWarningTaskHandle; /\* USER CODE END PD \*/ /\* Private macro -------------------------------------------------------------\*/ /\* USER CODE BEGIN PM \*/ /\* USER CODE END PM \*/ /\* Private variables ---------------------------------------------------------\*/ I2C\_HandleTypeDef hi2c1; TIM\_HandleTypeDef htim1; TIM\_HandleTypeDef htim2; UART\_HandleTypeDef huart1; /\* Definitions for defaultTask \*/ osThreadId\_t defaultTaskHandle; const osThreadAttr\_t defaultTask\_attributes = {.name = "defaultTask",.stack\_size = 128 \* 4,.priority = (osPriority\_t) osPriorityNormal, }; /\* USER CODE BEGIN PV \*/ /\* USER CODE END PV \*/ /\* Private function prototypes -----------------------------------------------\*/ void SystemClock\_Config(void); static void MX\_GPIO\_Init(void); static void MX\_TIM1\_Init(void); static void MX\_I2C1\_Init(void); static void MX\_TIM2\_Init(void); static void MX\_USART1\_UART\_Init(void); void StartDefaultTask(void \*argument); /\* USER CODE BEGIN PFP \*/ /\* USER CODE END PFP \*/ /\* Private user code ---------------------------------------------------------\*/ /\* USER CODE BEGIN 0 \*/ /\* USER CODE END 0 \*/ /\*\* \* @brief The application entry point. \* @retval int \*/ int main(void) { /\* USER CODE BEGIN 1 \*/ /\* USER CODE END 1 \*/ /\* MCU Configuration--------------------------------------------------------\*/ /\* Reset of all peripherals, Initializes the Flash interface and the Systick. \*/ HAL\_Init(); /\* USER CODE BEGIN Init \*/ // DHT11\_Init(); // OLED\_Init(); // OLED\_Clear(); /\* USER CODE END Init \*/ /\* Configure the system clock \*/ SystemClock\_Config(); /\* USER CODE BEGIN SysInit \*/ /\* USER CODE END SysInit \*/ /\* Initialize all configured peripherals \*/ MX\_GPIO\_Init(); MX\_TIM1\_Init(); MX\_I2C1\_Init(); MX\_TIM2\_Init(); MX\_USART1\_UART\_Init(); /\* USER CODE BEGIN 2 \*/ /\* USER CODE END 2 \*/ /\* Init scheduler \*/ osKernelInitialize(); /\* USER CODE BEGIN RTOS\_MUTEX \*/ /\* add mutexes,... \*/ /\* USER CODE END RTOS\_MUTEX \*/ /\* USER CODE BEGIN RTOS\_SEMAPHORES \*/ /\* add semaphores,... \*/ /\* USER CODE END RTOS\_SEMAPHORES \*/ /\* USER CODE BEGIN RTOS\_TIMERS \*/ /\* start timers, add new ones,... \*/ /\* USER CODE END RTOS\_TIMERS \*/ /\* USER CODE BEGIN RTOS\_QUEUES \*/ /\* add queues,... \*/ /\* USER CODE END RTOS\_QUEUES \*/ /\* Create the thread(s) \*/ /\* creation of defaultTask \*/ defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask\_attributes); /\* USER CODE BEGIN RTOS\_THREADS \*/ /\* add threads,... \*/ xTaskCreate(DHT11Task,"DHT11TASK",128,NULL,osPriorityNormal+1,NULL); xTaskCreate(OLEDTask,"OLEDTASK",128,NULL,osPriorityNormal,NULL); // xTaskCreate(WarningTask,"WarningTASK",128,NULL,osPriorityNormal,NULL); // xTaskCreate(My\_UARTTask,"My\_UARTTask",128,NULL,osPriorityNormal,NULL); /\* USER CODE END RTOS\_THREADS \*/ /\* USER CODE BEGIN RTOS\_EVENTS \*/ /\* add events,... \*/ /\* USER CODE END RTOS\_EVENTS \*/ /\* Start scheduler \*/ osKernelStart(); /\* We should never get here as control is now taken by the scheduler \*/ /\* Infinite loop \*/ /\* USER CODE BEGIN WHILE \*/ while (1) { /\* USER CODE END WHILE \*/ /\* USER CODE BEGIN 3 \*/ } /\* USER CODE END 3 \*/ } void DHT11Task(void \*params) { int err; int temp,humi; DHT11\_Init(); Data DHT11Data; while(1) { vTaskSuspendAll(); err=DHT11ReadData(&temp,&humi); xTaskResumeAll(); if(err==0) { DHT11Data.temp=temp; DHT11Data.humi=humi; DispatchData(&DHT11Data); } else { // OLED\_ShowString(5,0,"err"); DHT11\_Init(); } vTaskDelay(2000); } } void OLEDTask(void\* params) { int len\_temp,len\_humi; int temp,humi; extern int err; OLED\_Init(); OLED\_Clear(); Data OLEDData; /\*创建并注册OLED队列\*/ OLEDQueueHandle=xQueueCreate(10,sizeof(struct DHT11Data)); RegisterQueue(OLEDQueueHandle); while(1) { if(err) { OLED\_ShowString(5,0,"err"); } else { /\*读队列数据\*/ xQueueReceive(OLEDQueueHandle,&OLEDData,portMAX\_DELAY); /\*数据处理\*/ temp=OLEDData.temp; humi=OLEDData.humi; len\_temp=OLED\_ShowString(0,0,"Tem:"); OLED\_ShowDecimal(len\_temp,0,temp); OLED\_ShowString(len\_temp+2,0,"^"); len\_humi=OLED\_ShowString(0,2,"Hum:"); OLED\_ShowDecimal(len\_humi,2,humi); OLED\_ShowString(len\_humi+2,2,"%"); } vTaskDelay(50); } } 帮我分析一下，为什么OLED上电显示err,硬件逻辑都没问题，把OLEDTask任务注释掉，调试发现DHT11接收数据没有任何问题，但一把OLEDTask注释取消，OLED就一直显示err，调试发现DHT11部分卡在ACK那返回-1，

从你的现象来看：

- **OLEDTask 注释掉 → DHT11 正常**
- **OLEDTask 开启 → DHT11 在 ACK 阶段失败（返回 -1）**
- **硬件没有问题**

这个基本可以判断： **不是 DHT11 硬件问题，而是 FreeRTOS 多任务环境下的时序/资源冲突问题。**

重点看你的代码，有几个非常明显的风险点。

---

## 1\. 最大嫌疑：OLEDTask 和 DHT11Task 同时操作中断/时序，导致 DHT11 ACK窗口丢失

DHT11 通信非常特殊。

它不是普通串口/I2C/SPI，而是：

- MCU 拉低至少 18ms
- 释放总线
- 等待 DHT11 响应
- DHT11 ACK：
	- 80us低电平
		- 80us高电平
- 后续40bit数据，每个位只有几十微秒窗口

也就是说：

> DHT11通信期间，CPU必须高度连续执行，不能被其他任务打断。

你的代码：

```markdown
vTaskSuspendAll();

err=DHT11ReadData(&temp,&humi);

xTaskResumeAll();
```

这个地方你认为保护了。

但是：

### vTaskSuspendAll()并不能禁止中断

它只是：

> 禁止FreeRTOS任务切换

但是：

- SysTick停止调度
- GPIO中断仍然运行
- I2C中断仍然运行
- UART中断仍然运行
- HAL库内部中断仍然运行

所以如果 OLEDTask 里面触发了什么中断或者HAL延迟，就可能打断DHT11。

---

## 2\. OLEDTask里面这个地方非常危险

```markdown
OLED_Init();
OLED_Clear();
```

你的OLED一般是：

SSD1306 + I2C

初始化的时候会大量调用：

```markdown
HAL_I2C_Master_Transmit()
```

或者：

```markdown
HAL_Delay()
```

问题：

你的启动流程：

```markdown
MX_I2C1_Init();

osKernelInitialize();

xTaskCreate(DHT11Task...)

xTaskCreate(OLEDTask...)

osKernelStart();
```

两个任务开始以后：

### DHT11Task:

```markdown
DHT11_Init();

DHT11ReadData();
```

### OLEDTask:

```markdown
OLED_Init();
OLED_Clear();
```

同时执行。

虽然你初始化外设顺序一样：

```markdown
I2C初始化
TIM初始化
UART初始化
```

但是任务启动以后：

```markdown
DHT11开始通信
        |
        |
OLED开始初始化I2C
        |
        |
DHT11 ACK窗口被打断
```

概率非常高。

---

## 3\. 你的错误变量设计也有问题

这里：

```markdown
extern int err;
```

但是：

DHT11Task里面：

```markdown
int err;
```

这是局部变量。

也就是说：

DHT11Task：

```markdown
int err;
```

这个变量只存在DHT11Task栈里面。

但是OLEDTask：

```markdown
extern int err;
```

访问的是另一个全局变量。

所以实际上：

### DHT11Task:

```markdown
err = -1
```

修改的是：

```markdown
DHT11Task栈里的err
```

OLEDTask：

读取：

```markdown
全局err
```

两个完全不是一个变量。

所以：

```markdown
if(err)
{
    OLED_ShowString(5,0,"err");
}
```

这里逻辑是不可靠的。

---

## 4\. 你的队列注册时机也有问题

OLEDTask:

```markdown
OLEDQueueHandle=xQueueCreate(10,sizeof(struct DHT11Data));

RegisterQueue(OLEDQueueHandle);
```

但是DHT11Task可能先运行：

```markdown
DispatchData(&DHT11Data);
```

此时：

```markdown
OLEDQueueHandle还没有创建
```

可能：

- 空指针
- 丢数据
- 内存异常

应该：

先创建队列，再启动任务。

例如：

main:

```markdown
OLEDQueueHandle=xQueueCreate(10,sizeof(Data));

RegisterQueue(OLEDQueueHandle);

xTaskCreate(DHT11Task,...);
xTaskCreate(OLEDTask,...);
```

不要放OLEDTask里面。

---

## 最可能原因排序

根据你的描述，我认为概率：

| 原因 | 概率 |
| --- | --- |
| OLED初始化/I2C操作干扰DHT11时序 | ★★★★★ |
| DHT11任务被OLED任务抢占 | ★★★★★ |
| 错误变量err设计错误 | ★★★★ |
| 队列创建太晚 | ★★★★ |
| 硬件问题 | ★ |

---

## 推荐修改方案

## 第一：

OLED初始化不要放任务里面。

改：

main:

```markdown
OLED_Init();
OLED_Clear();

OLEDQueueHandle=xQueueCreate(10,sizeof(Data));
RegisterQueue(OLEDQueueHandle);
```

然后：

OLEDTask:

```markdown
void OLEDTask(void *params)
{
    Data OLEDData;

    while(1)
    {
        xQueueReceive(
             OLEDQueueHandle,
             &OLEDData,
             portMAX_DELAY
        );

        OLED_ShowDecimal(...);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

---

## 第二：

DHT11读取必须关闭中断

不要：

```markdown
vTaskSuspendAll();
```

改：

```markdown
taskENTER_CRITICAL();

err=DHT11ReadData(&temp,&humi);

taskEXIT_CRITICAL();
```

因为：

DHT11需要微秒级稳定。

---

## 第三：

不要使用全局err判断OLED状态

改成：

Data增加状态：

比如：

```markdown
typedef struct
{
    int temp;
    int humi;
    int err;

}Data;
```

发送：

```markdown
DHT11Data.err=err;
DispatchData(&DHT11Data);
```

OLED：

```markdown
if(OLEDData.err)
{
 OLED_ShowString(5,0,"err");
}
```

---

## 第四：

DHT11延时单位问题

这里：

```markdown
vTaskDelay(2000);
```

在FreeRTOS里面：

不是2秒。

取决于：

```markdown
configTICK_RATE_HZ
```

如果：

```markdown
#define configTICK_RATE_HZ 1000
```

那么：

```markdown
vTaskDelay(2000)
```

\=2秒。

但是保险写：

```markdown
vTaskDelay(pdMS_TO_TICKS(2000));
```

---

## 一个验证实验

你现在不用改代码。

只改OLEDTask：

把：

```markdown
OLED_Init();
OLED_Clear();
```

移动到main：

然后OLEDTask里面只保留：

```markdown
xQueueReceive()
OLED显示
```

如果马上正常：

100%证明：

> OLED初始化阶段占用了CPU/I2C资源，破坏了DHT11 ACK时序。

---

另外还有一个细节：你的 `DHT11ReadData()` 如果里面使用的是 `HAL_Delay()` 或者 `TIM` 轮询计时，也需要看一下，因为在FreeRTOS下 HAL\_Delay 默认依赖 SysTick，和任务调度可能冲突。

你这个现象非常像典型的 **裸机DHT11代码迁移到FreeRTOS后，被另一个任务打断导致ACK丢失** 。