# 智能环境监测系统 (Intelligent Environment Monitoring System)

基于 STM32F103 + FreeRTOS 的温湿度监测与多级报警系统。通过 DHT11 传感器采集环境温湿度数据，根据阈值分级触发 LED 指示灯、蜂鸣器和串口告警。

## 硬件平台

| 组件 | 型号/规格 | 引脚 | 说明 |
|------|----------|------|------|
| MCU | STM32F103C8 (Cortex-M3, 72MHz) | - | HSE 8MHz × PLL9 = 72MHz |
| 温湿度传感器 | DHT11 | PA5 | 开漏输出 + 内部上拉，单总线协议 |
| OLED 显示屏 | SSD1306 128×64 | PB6(SCL) / PB7(SDA) | I2C1, 100kHz, 设备地址 0x78 |
| 板载 LED | - | PC13 | 开漏输出，低电平点亮 |
| 全彩 LED | - | PB3(R) / PB4(B) / PB5(G) | 推挽输出，低电平点亮 |
| 蜂鸣器 | 有源/无源 | PA0 (TIM2_CH1) | PWM 驱动，默认 1kHz |
| 串口 | USART1 | PA9(TX) / PA10(RX) | 115200-8-N-1 |

## 软件架构

### 技术栈

- **RTOS**: FreeRTOS V10.3.1（CMSIS-RTOS V2 封装）
- **HAL 库**: STM32F1xx HAL Driver
- **构建工具**: STM32CubeMX + Keil MDK-ARM
- **堆管理**: heap_4（6KB）

### 时钟与定时器分配

| 定时器 | 用途 | 配置 |
|--------|------|------|
| SysTick | FreeRTOS 调度 tick | 1ms (1000Hz) |
| TIM4 | HAL 库时间基 | 1ms 中断，替代 SysTick |
| TIM1 | 微秒级延时 | PSC=71, ARR=999, 计数频率 1MHz |
| TIM2 | 蜂鸣器 PWM | PSC=71, ARR 动态可调, CH1 → PA0 |

### 任务架构

系统共创建 5 个任务，通过 FreeRTOS 队列和任务通知进行数据流转：

```
                    ┌─────────────┐
                    │  DHT11Task  │  优先级: Normal+1
                    │  每 2s 读取  │
                    └──────┬──────┘
                           │ DispatchData()
                    ┌──────┴──────┐
              ┌──────▼──────┐ ┌────▼────────┐
              │  OLEDTask   │ │ WarningTask │  优先级: Normal
              │ I2C 显示    │ │ 分级报警    │
              └─────────────┘ └──────┬──────┘
                                     │ xTaskNotify()
                              ┌──────▼──────┐
                              │ My_UARTTask │  优先级: Normal
                              │ 串口告警    │
                              └─────────────┘
```

#### 数据流

1. **DHT11Task**：每 2 秒读取一次 DHT11 温湿度数据，通过 `vTaskSuspendAll()` 保护时序敏感的单总线通信。读取成功后将数据封装为 `DHT11Data` 结构体，通过 `DispatchData()` 分发到 OLED 队列和 Warning 队列。
2. **OLEDTask**：阻塞等待 OLED 队列数据，收到后显示温度和湿度，或显示 "err"。
3. **WarningTask**：阻塞等待 Warning 队列数据，根据温湿度阈值分为三个等级执行报警动作，并通过任务通知唤醒 UART 任务。
4. **My_UARTTask**：阻塞等待任务通知，收到后通过 USART1 发送告警信息。

### 报警分级

| 等级 | 温度条件 | 湿度条件 | LED | 全彩灯 | 蜂鸣器 | 串口 |
|------|---------|---------|-----|--------|--------|------|
| 正常 | < 30°C | < 80% | 熄灭 | 绿色常亮 | 关闭 | - |
| 警告 | 30~35°C | 80~90% | 闪烁 | 黄色闪烁 | 间断鸣叫 | `Warning!\n\r` |
| 危险 | > 35°C | > 90% | 常亮 | 红色闪烁 | 持续鸣叫 | `Danger!\n\r` |

## 项目结构

```
IntelligentEnvironmentMonitoringSystem/
├── Core/                           # CubeMX 生成的核心代码
│   ├── Inc/
│   │   ├── FreeRTOSConfig.h        # FreeRTOS 配置
│   │   ├── main.h
│   │   └── stm32f1xx_hal_conf.h    # HAL 库配置
│   └── Src/
│       ├── main.c                  # 主函数 + 外设初始化 + 任务创建
│       ├── freertos.c              # FreeRTOS 框架文件
│       ├── stm32f1xx_hal_msp.c     # 外设 MSP 初始化 (GPIO/I2C/TIM/UART 底层配置)
│       ├── stm32f1xx_hal_timebase_tim.c  # HAL 时间基 (TIM4)
│       └── stm32f1xx_it.c          # 中断服务函数
│
├── Project_Code/                   # 用户应用代码
│   ├── driver_timer.c/.h           # 微秒级延时 (基于 TIM1)
│   ├── Driver_DHT11.c/.h           # DHT11 驱动 + 数据分发 + DHT11Task
│   ├── Driver_OLED.c/.h            # SSD1306 OLED 驱动 + OLEDTask
│   ├── Driver_LED.c/.h             # 板载 LED 控制
│   ├── Driver_color_led.c/.h       # RGB 全彩 LED 控制
│   ├── Driver_Buzzer.c/.h          # 蜂鸣器 PWM 驱动
│   ├── Warning.c/.h                # 报警分级逻辑 + WarningTask
│   ├── My_UART.c/.h                # 串口告警发送 + My_UARTTask
│   └── font.c                      # ASCII 字模 (8×16)
│
├── Drivers/                        # ST 标准驱动库
│   ├── CMSIS/                      # ARM CMSIS Core + DSP + NN
│   └── STM32F1xx_HAL_Driver/       # STM32F1 HAL 库
│
├── Middlewares/                    # 中间件
│   └── Third_Party/FreeRTOS/      # FreeRTOS 内核源码
│
├── MDK-ARM/                        # Keil 工程文件
│   ├── IntelligentEnvironmentMonitoringSystem.uvprojx
│   └── startup_stm32f103xb.s      # 启动文件
│
└── IntelligentEnvironmentMonitoringSystem.ioc  # CubeMX 配置文件
```

## 关键实现说明

### 微秒级延时 (driver_timer.c)

基于 TIM1 计数器轮询实现，独立于 HAL 的 `HAL_Delay()`（仅毫秒级）和 FreeRTOS 的 `vTaskDelay()`（受调度器影响）。TIM1 配置为 PSC=71/ARR=999，计数频率 1MHz（1 个 tick = 1us）。

> 注意：CubeMX 生成的 `MX_TIM1_Init()` 不包含 `HAL_TIM_Base_Start()`，需手动添加以启动计数器。

### DHT11 单总线通信 (Driver_DHT11.c)

采用 GPIO 开漏输出模式 + 内部上拉模拟单总线协议。通信过程中使用 `vTaskSuspendAll()` / `xTaskResumeAll()` 保护时序（本项目无中断冲突，如涉及中断需使用 `taskENTER_CRITICAL()`）。

### 数据分发机制

`DHT11Data` 结构体（含 temp、humi、err 字段）通过队列分发给多个消费者任务。`RegisterQueue()` 实现队列注册，`DispatchData()` 遍历所有已注册队列发送数据。队列和任务的创建在 `osKernelStart()` 之前统一完成，避免任务执行时队列尚未创建的竞态问题。

## 编译与烧录

1. 使用 STM32CubeMX 打开 `.ioc` 文件可查看/修改外设配置
2. 使用 Keil MDK-ARM 打开 `MDK-ARM/IntelligentEnvironmentMonitoringSystem.uvprojx`
3. 编译后通过 ST-Link 下载到 STM32F103C8
