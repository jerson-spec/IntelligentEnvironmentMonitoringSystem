# 🌡️ IntelligentEnvironmentMonitoringSystem

> 基于 **STM32F103C8T6 (Cortex-M3)** + **FreeRTOS V10.3.1 (CMSIS-RTOS V2)** 构建的实时环境监测与分级预警系统。采用分层架构设计：HAL 驱动 → BSP 设备驱动 → FreeRTOS 多任务应用，通过 DHT11 采集温湿度，结合 OLED、三色 LED、蜂鸣器、串口实现多通道声光预警。

---

## 🏗️ 技术栈总览

```
┌───────────────────────────────────────────────────────────────┐
│                    Application Layer  (用户应用)               │
│  DHT11采集  │  OLED显示  │  UART  │ LED/蜂鸣器/RGB 预警任务   │
├───────────────────────────────────────────────────────────────┤
│                    RTOS Layer  (系统调度)                      │
│                FreeRTOS V10.3.1 + CMSIS-RTOS V2               │
│     抢占式调度 │ 56级优先级 │ Heap_4 8KB │ Queue/TaskNotify   │
├───────────────────────────────────────────────────────────────┤
│                  BSP / Driver Layer (板级支持包)               │
│  Driver_DHT11 │ Driver_OLED │ Driver_LED │ Driver_Buzzer      │
│  Driver_color_led │ driver_timer │ font (8x16 ASCII 字库)     │
├───────────────────────────────────────────────────────────────┤
│                  HAL / CMSIS Layer (硬件抽象层)                │
│     STM32F1xx HAL V1.8.7 │ CMSIS-Core M3 │ DSP Lib            │
├───────────────────────────────────────────────────────────────┤
│                  Hardware Layer (硬件平台)                     │
│   STM32F103C8T6 @ 72MHz │ DHT11 │ SSD1306 (128x64) │ 外设    │
└───────────────────────────────────────────────────────────────┘
```

---

## 🔌 硬件规格与引脚定义

### 核心芯片

| 参数 | 规格 |
|------|------|
| **MCU 型号** | STM32F103C8T6（LQFP48 封装） |
| **内核架构** | ARM® Cortex™-M3 32-bit RISC |
| **最高主频** | **72 MHz**（HSE 8MHz × PLL x9） |
| **Flash** | 64 KB |
| **SRAM** | 20 KB |
| **工作电压** | 2.0 V ~ 3.6 V |

### 时钟树配置

> 来源：[IntelligentEnvironmentMonitoringSystem.ioc](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/IntelligentEnvironmentMonitoringSystem.ioc#L146-L166)

```
HSE External OSC (8 MHz)
        │
        └──► PLL Source ──► PLL ×9 ──────────────────┐
                                                      │
SYSCLK = 72 MHz ◄─────────────────────────────────────┘
  │
  ├─► AHB Prescaler = /1  ──► HCLK/AHB = 72 MHz (CPU/DMA/SDIO)
  │     │
  │     ├─► APB1 Prescaler = /2 ──► APB1 = 36 MHz
  │     │     ├─► APB1 Timer Clk = ×2 = 72 MHz (TIM2~7)
  │     │
  │     └─► APB2 Prescaler = /1 ──► APB2 = 72 MHz
  │           ├─► APB2 Timer Clk = ×1 = 72 MHz (TIM1/8)
  │
  └─► USB/SDIO/RTC: 72 MHz
```

### 外设引脚分配

| 外设/模块 | 引脚 | 复用功能 | GPIO 模式 | 备注 |
|-----------|------|----------|-----------|------|
| **HSE 晶振** | PD0 / PD1 | OSC_IN / OSC_OUT | 模拟 | 8MHz 外部晶振 |
| **SWD 调试** | PA13 / PA14 | JTMS-SWDIO / JTCK-SWCLK | 复用 | ST-Link 下载调试 |
| **DHT11 数据** | PA5 | GPIO_Output | 开漏输出 + 上拉 | 单总线，可双向切换 |
| **USART1 TX** | PA9 | USART1_TX | 复用推挽 | 115200 8N1 |
| **USART1 RX** | PA10 | USART1_RX | 浮空输入 | |
| **LED (板载)** | PC13 | GPIO_Output | 开漏输出 + 上拉 | 低电平点亮 |
| **RGB LED R** | PB3 | GPIO_Output | 推挽输出 | 低电平亮 |
| **RGB LED G** | PB4 | GPIO_Output | 推挽输出 | 低电平亮 |
| **RGB LED B** | PB5 | GPIO_Output | 推挽输出 | 低电平亮 |
| **I2C1 SCL** | PB6 | I2C1_SCL | 复用开漏 | 标准 100 kHz |
| **I2C1 SDA** | PB7 | I2C1_SDA | 复用开漏 | |
| **TIM2 CH1** | PA0 (复用功能) | PWM 输出 | 复用推挽 | 蜂鸣器驱动 |
| **TIM1** | 无外部引脚 | 内部计数 | - | μs 级延时基准 |
| **TIM4** | 无外部引脚 | HAL SysTick 时基 | - | FreeRTOS tick 源 |

---

## ⚙️ 系统层配置（FreeRTOS + HAL）

### FreeRTOS 内核参数

> 来源：[FreeRTOSConfig.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Core/Inc/FreeRTOSConfig.h)

| 参数 | 值 | 说明 |
|------|----|------|
| **FreeRTOS 版本** | V10.3.1 | Amazon LTS 版 |
| **CMSIS-RTOS 封装** | V2 | CubeMX 默认接入 |
| **调度方式** | 抢占式 + 时间片 (configUSE_PREEMPTION=1) | 同优先级轮转 |
| **最大优先级数** | 56 (osPriorityNone ~ osPriorityISR) | |
| **Tick 频率** | 1000 Hz (1 ms) | configTICK_RATE_HZ |
| **最小任务栈** | 128 Word (512 Byte) | configMINIMAL_STACK_SIZE |
| **总堆大小** | **8000 Byte** | `configTOTAL_HEAP_SIZE=8000`（原6000，8/7升级） |
| **内存管理** | **Heap_4 算法** (USE_FreeRTOS_HEAP_4) | 合并碎片的动态堆 |
| **分配方式** | 动态 + 静态 双支持 | configSUPPORT_{DYNAMIC,STATIC}_ALLOCATION=1 |
| **互斥量 / 递归互斥** | ✅ 启用 | `configUSE_MUTEXES`, `configUSE_RECURSIVE_MUTEXES`=1 |
| **计数信号量** | ✅ 启用 | `configUSE_COUNTING_SEMAPHORES`=1 |
| **软件定时器** | ✅ 启用 | 优先级=2, 队列深=10, 栈深=256 |
| **追踪调试** | ✅ 启用 | `configUSE_TRACE_FACILITY`=1 + uxTaskGetStackHighWaterMark |

### NVIC 中断分组与 FreeRTOS 安全边界

```c
NVIC_PRIORITYGROUP_4     // 4 位抢占优先级，0 位子优先级
                         // 抢占优先级范围: 0 (最高) ~ 15 (最低)

configKERNEL_INTERRUPT_PRIORITY        = 15  // SVC/PendSV/SysTick: 最低级
configMAX_SYSCALL_INTERRUPT_PRIORITY   = 5   // 可安全调用 FreeRTOS API 的最高抢占优先级
                                             // 优先级 0~4 的 ISR 禁止使用 FromISR API
```

> HAL 使用 TIM4 作为 SysTick 时基，优先级设为 15（最低级），确保不阻塞任何业务中断。

---

## 🧵 BSP 驱动层（板级支持包）

所有驱动源码位于：[code/Project_Code/](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code)

### 1. 高精度延时驱动 driver_timer

**源码**：[driver_timer.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/driver_timer.h) / [driver_timer.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/driver_timer.c)

基于 **TIM1** 独立定时器构建，避开复用 HAL 的 TIM4 时基，抗干扰更强。

```
TIM1 配置:  PSC = 71  →  72MHz / (71+1) = 1 MHz (每 1μs 加一)
            ARR = 999 →  计数 0~999, 每 1ms 自动重载
```

| 接口函数 | 功能 | 精度 |
|----------|------|------|
| `void Delay_us(int us)` | 微秒级阻塞延时 | ±1 μs |
| `void Delay_ms(int ms)` | 毫秒级阻塞延时 | ±1 ms |
| `uint64_t GetSystemTimer_ns()` | 获取系统累计时间 | ns 级返回值 |

---

### 2. DHT11 温湿度驱动

**源码**：[Driver_DHT11.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_DHT11.h) / [Driver_DHT11.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_DHT11.c)

- **通信协议**：单总线 (1-Wire 类)，PA5 开漏 + 外部上拉
- **数据帧**：5 Byte (湿度整 / 湿度小 / 温度整 / 温度小 / 校验)
- **数据结构**：

```c
typedef struct DHT11Data {
    int temp;    // 温度 (整数)
    int humi;    // 湿度 (整数)
    int err;     // 0=成功, -1=超时, -2=校验错
} Data;
```

| 接口函数 | 功能 |
|----------|------|
| `void DHT11_Init(void)` | 上电稳定延时 |
| `int DHT11ReadData(int *temp, int *humi)` | 单次读取（阻塞 ~20ms） |
| `void DHT11Task(void *params)` | **FreeRTOS 采集任务**（周期读 + 队列广播） |
| `void RegisterQueue(QueueHandle_t)` / `DispatchData(Data*)` | 订阅/分发机制 |

---

### 3. OLED SSD1306 显示驱动

**源码**：[Driver_OLED.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_OLED.h) / [Driver_OLED.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_OLED.c) / [font.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/font.c)

| 项目 | 规格 |
|------|------|
| 驱动芯片 | SSD1306 (128×64 Dot Matrix) |
| 接口 | I2C1，地址 `0x78`，标准 100 kHz |
| 字库 | 内置 ASCII 8×16 点阵 (`ascii_font[128][16]`) |
| 显存模式 | Page Addressing Mode (水平页扫描) |

| 接口函数 | 功能 |
|----------|------|
| `void OLED_Init(void)` | 12 条初始化指令序列（含电荷泵、COM 重映射、反色设置） |
| `void OLED_Clear(void)` | 全屏清空 |
| `void OLED_ShowChar(x,y,char)` | 指定行列 (以8像素高为1行) 显示字符 |
| `int OLED_ShowString(x,y,char*)` | 显示字符串 |
| `int OLED_ShowDecimal(x,y,int32_t)` | 显示带符号十进制整数 |
| `void OLEDTask(void* params)` | **FreeRTOS 显示任务**：队列接收 Data → 实时刷新温湿度 |

---

### 4. 蜂鸣器 PWM 驱动

**源码**：[Driver_Buzzer.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_Buzzer.h) / [Driver_Buzzer.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_Buzzer.c)

- **驱动方式**：TIM2 CH1 PWM 输出
- **时基**：72MHz / PSC=71 = 1MHz；ARR 按频率动态计算
- **动态算法**：`SetBuzzer_freq_duty(freq, duty)` 在运行时重算 `ARR` 与 `CCR1` 实现任意频率与占空比

| 接口函数 | 功能 |
|----------|------|
| `SetBuzzer_freq_duty(int freq, int duty)` | 设定频率(Hz)与占空比(0~100%) |
| `Buzzer_On(void)` / `Buzzer_Off(void)` | 启动 / 停止 PWM 输出 |
| `Buzzer_toggle(void)` | 响 200ms + 停 200ms（警告级间歇模式） |

---

### 5. 板载 LED 与 RGB 三色 LED 驱动

**源码**：
- 板载 LED: [Driver_LED.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_LED.h) / [Driver_LED.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_LED.c)
- RGB LED: [Driver_color_led.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_color_led.h) / [Driver_color_led.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/Driver_color_led.c)

> **极性约定**：PC13 板载 LED 与 PB3/4/5 RGB 均为 **共阳极接法，低电平点亮**

| 状态 | 板载 LED (PC13) | RGB 三色 (PB3-R / PB4-G / PB5-B) |
|------|------------------|-----------------------------------|
| 常亮 / 颜色 | `LED_Light()` (低) | `SetRed()`/`SetBlue()`/`SetGreen()`/`SetYellow(R+G)` |
| 闪烁 | `LED_Twinkle()` (500ms翻转) | `SetRed_Twinkle()` / `SetYellow_Twinkle()` / `SetGreen_Twinkle()` |
| 熄灭 | `LED_OFF()` (高) | 独立 `Set*_off()` 或全引脚拉高 |

---

### 6. UART 通信模块

**源码**：[My_UART.h](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/My_UART.h) / [My_UART.c](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Project_Code/My_UART.c)

- **硬件**：USART1 (PA9/PA10)
- **协议**：波特率 115200，数据位 8，停止位 1，无校验，无流控

| 接口函数 | 功能 |
|----------|------|
| `UART_TransNews(void* string)` | 阻塞式发送字符串（HAL_UART_Transmit HAL_MAX_DELAY） |
| `void My_UARTTask(void* params)` | **FreeRTOS 任务**：`xTaskNotifyWait` 等待 Warn/Danger 通知，并发串口文字 + OLED 顶部闪烁"Warn!"/"Danger!" |

---

## 🧭 应用层：FreeRTOS 多任务架构

### 任务清单

所有任务在 [main.c L192-L199](file:///d:/Desktop/IntelligentEnvironmentMonitoringSystem_0/code/Core/Src/main.c#L192-L199) 中创建。

| 任务名 | 入口函数 | 栈大小 (Word) | 优先级 (数值越大越高) | 核心职责 |
|--------|----------|--------------|-----------------------|----------|
| `DHT11TASK` | `DHT11Task()` | 128 | **osPriorityNormal + 2** (最高) | 周期性读取 DHT11，打包成 `Data`，写入 4 个订阅队列 |
| `WarningTASK`×3 | `LED_WarningTask()` / `Buzzer_WarningTask()` / `ColorLED_WarningTask()` | 128 每个 | **osPriorityNormal + 1** | 从各自队列读取数据，按阈值分级执行对应报警动作；用 `xTaskNotify` 通知 UART 任务 |
| `My_UARTTask` | `My_UARTTask()` | 128 | osPriorityNormal (普通) | 等待 TaskNotify；Warn/Danger 分档输出串口 + OLED 闪烁文字 |
| `OLEDTASK` | `OLEDTask()` | 128 | osPriorityNormal (普通) | 队列阻塞读，刷新温湿度两行显示 |
| `defaultTask` | `StartDefaultTask()` | 128 | osPriorityNormal | CMSIS-RTOS 默认空任务 |

### IPC 通信拓扑（队列 + 任务通知）

```
                    ┌───────────────────┐
                    │    DHT11Task      │  采集数据
                    │  (Normal + 2)     │
                    └─────────┬─────────┘
                              │ DispatchData() 多写
            ┌─────────────────┼─────────────────┬───────────────────┐
            ▼                 ▼                 ▼                   ▼
 ┌──────────────────┐ ┌────────────────┐ ┌────────────────┐ ┌──────────────┐
 │ OLEDQueueHandle  │ │ LED_WarnQueue  │ │Buzzer_WarnQueue│ │ColorLED_Queue│
 │   (depth 10)     │ │   (depth 10)   │ │   (depth 10)   │ │ (depth 10)   │
 └────────┬─────────┘ └───────┬────────┘ └───────┬────────┘ └──────┬───────┘
          │                   │                   │                 │
          ▼                   ▼                   ▼                 ▼
 ┌──────────────────┐ ┌────────────────┐ ┌────────────────┐ ┌──────────────────┐
 │    OLEDTask      │ │LED_WarningTask │ │Buzzer_WarnTask │ │ColorLED_WarnTask │
 └──────────────────┘ └───────┬────────┘ └───────┬────────┘ └────────┬─────────┘
                              │                   │                  │
                              │     xTaskNotify( eSetValueWithOverwrite )
                              │         Warn=2  /  Danger=3
                              └───────────────────┼──────────────────┘
                                                  ▼
                                    ┌──────────────────────────┐
                                    │       My_UARTTask         │
                                    │ 串口输出 + OLED Warn/Danger │
                                    │        闪烁提示             │
                                    └──────────────────────────┘
```

> 队列元素大小统一为 `sizeof(struct DHT11Data)`。DHT11 采集任务使用"**分发器模式**"——读取一次数据后通过 `DispatchData` 同时广播给 4 个消费者队列，实现一源多消费。

### 三级预警阈值与响应

所有 Warning 任务阈值完全一致（统一硬编码，可宏化重构）：

```c
/* Grade 1 — Warning (警告) */
temp ≥ 30℃   OR   humi ≥ 90%
/* Grade 2 — Danger (危险) */
temp > 35℃   OR   humi > 93%
```

| 报警通道 | Normal (正常) | Warning (警告) | Danger (危险) |
|----------|:------------:|:--------------:|:-------------:|
| 板载 LED (PC13) | 灭 | **闪烁** (500ms翻转) | **常亮** |
| 蜂鸣器 (TIM2 PWM) | 关 | **间歇鸣叫** (200ms响/200ms停) | **持续鸣叫** |
| RGB 三色 LED | 🟢 绿色常亮 | 🟡 **黄色闪烁** (500ms) | 🔴 **红色闪烁** (500ms) |
| OLED 主显示 | 温湿度数值 | Warn/Danger 闪烁叠加 | 同 Warning |
| USART1 串口 | - | `"Warning!\r\n"` | `"Danger!\r\n"` |

---

## 📁 项目目录结构

```
IntelligentEnvironmentMonitoringSystem_0/
├── code/
│   ├── IntelligentEnvironmentMonitoringSystem.ioc   ★ STM32CubeMX 工程配置
│   │
│   ├── Core/                          ★ HAL 框架 & 初始化
│   │   ├── Inc/
│   │   │   ├── main.h
│   │   │   ├── FreeRTOSConfig.h        ★ FreeRTOS V10.3.1 内核配置
│   │   │   ├── stm32f1xx_hal_conf.h
│   │   │   └── stm32f1xx_it.h
│   │   └── Src/
│   │       ├── main.c                  ★ 系统初始化 + 队列/任务创建
│   │       ├── freertos.c
│   │       ├── stm32f1xx_hal_msp.c
│   │       ├── stm32f1xx_it.c          ★ 中断向量 (PendSV/SVCall/TIM4)
│   │       └── system_stm32f1xx.c
│   │
│   ├── Drivers/
│   │   ├── CMSIS/                      ★ Cortex-M3 核心头文件 + 启动文件 + DSP Lib
│   │   └── STM32F1xx_HAL_Driver/       ★ ST 官方 HAL V1.8.7 驱动
│   │
│   ├── Middlewares/Third_Party/FreeRTOS/  ★ FreeRTOS 内核源码 (Port + Heap_4)
│   │
│   └── Project_Code/                   ★ 用户 BSP 层 (重点)
│       ├── driver_timer.c/h            # TIM1 μs/ms 延时 + ns 级时戳
│       ├── Driver_DHT11.c/h            # DHT11 采集 + 队列分发任务
│       ├── Driver_OLED.c/h             # SSD1306 I2C 驱动 + 显示任务
│       ├── Driver_LED.c/h              # 板载 LED (PC13)
│       ├── Driver_Buzzer.c/h           # TIM2 PWM 蜂鸣器
│       ├── Driver_color_led.c/h        # RGB 三色 LED
│       ├── font.c                      # 8×16 ASCII 点阵字库
│       ├── LED_Warning.c/h             # LED 预警任务
│       ├── Buzzer_Warning.c/h          # 蜂鸣器预警任务
│       ├── ColorLED_Warning.c/h        # RGB 预警任务
│       ├── Warning.c/h                 # (已废弃) 原统一警告模块
│       └── My_UART.c/h                 # USART1 + OLED 告警任务
│
├── obsidian/                            ★ 开发笔记 / 问题记录
│   ├── 修改说明.md                      # 版本修改日志
│   ├── Summary/
│   │   ├── 7_26为止总结.md
│   │   └── 8_7为止总结.md
│   ├── Q&S/                             # 问题与解决方案
│   ├── DHT11_Driver/                    # DHT11 驱动开发思路
│   ├── Warning_Driver/                  # 警告模块说明
│   └── My_UART/                         # 串口模块思路
│
└── README.md                            ← (本文件)
```

---

## 🔨 编译与下载

### 环境要求

| 工具 | 推荐版本 |
|------|----------|
| STM32CubeMX | **6.17.0**（与 `.ioc` 工程版本一致，避免重新生成代码丢失结构） |
| Keil MDK-ARM | **V5.32**（CubeMX 默认目标 Toolchain）或支持 GCC 的 STM32CubeIDE 1.13+ |
| STM32Cube FW_F1 | **V1.8.7**（已内置在 `Drivers/` 目录中） |
| 调试器 | ST-Link V2 / J-Link（SWD 接口 PA13/PA14） |

### 标准流程

1. **打开工程**：双击 `code/IntelligentEnvironmentMonitoringSystem.ioc` 用 STM32CubeMX 打开（**不要**点击"GENERATE CODE"以免覆盖用户区）。
2. **编译**：
   - Keil：打开 MDK-ARM 目录下的 `.uvprojx` → 点 **Build (F7)**。
   - CubeIDE：导入为 Existing Project → Build All。
3. **下载**：
   - Keil：Flash → Download (F8)，或使用 ST-Link Utility 烧录 `.hex`。
4. **验证**：连接 USB-TTL 到 USART1 (PA9-TX → TTL-RX)，波特率 **115200 8N1**，可看到 Warning/Danger 文本输出；同时观察 OLED 与 RGB LED 状态。

---

## 🔭 已知改进项

> 来源于项目内部开发笔记，按优先级排列：

| # | 改进方向 | 说明 |
|---|----------|------|
| 1 | **警告同步性** | LED / Buzzer / ColorLED 三任务各自轮询，闪烁节拍未完全对齐；可改用 EventGroup 或统一 Warning 调度器 |
| 2 | **优先级细分** | 当前仅 Normal+2 / Normal+1 / Normal 三档；可进一步细分 OLED 更新、UART 等 |
| 3 | **OLED 预警视觉** | Warn 与 Danger 用不同频率/颜色闪烁 |
| 4 | **互斥量保护** | I2C & UART 尚未加 Mutex（目前无竞争）；若加空闲打印等功能需补齐 |
| 5 | **传感器扩展** | 增加 MQ-x 气体/粉尘/光照传感器模块 |
| 6 | **DHT11 小数精度** | 目前丢弃小数位 Byte，可扩展为 float 输出 |
| 7 | **RGB PWM** | 三色 LED 当前为 GPIO 电平切换，可复用 TIM PWM 实现渐变/调色 |

---

## 📄 License

- STM32 HAL / CMSIS：STMicroelectronics BSD-3 许可
- FreeRTOS V10.3.1：MIT 许可 (Amazon.com, Inc.)
- 用户代码（`Project_Code/` 目录）：按作者约定使用

---

## 🎥 效果演示


https://github.com/user-attachments/assets/16e6f633-6d2b-4b0e-9768-ce0443d6eb49



