# N32WB03X BLE 倒计时控制器

基于 N32WB031（Cortex-M0）的 BLE 倒计时控制器。设备通过 BLE 接收设置时长、停止和查询剩余时间命令，处理完成后使用 Notify 返回应用层 ACK；运行期间驱动 TM1650 数码管显示倒计时，并控制继电器输出。

## 功能概览

- 通过 BLE 设置倒计时分钟数。
- 支持停止倒计时并关闭继电器。
- 支持查询剩余时间，结果按分钟向上取整。
- TM1650 四位数码管显示剩余时间。
- 倒计时运行期间控制继电器输出。
- 每 500 ms 刷新显示并切换运行指示灯。
- 使用 CRC32 校验自定义 BLE 数据帧。
- 通过 LPUART1 输出调试日志。

## 主要硬件

| 外设 | 引脚/接口 | 说明 |
|---|---|---|
| MCU | N32WB031 | Cortex-M0，系统时钟 64 MHz |
| TM1650 | PB6=SDA，PB7=SCL | 软件 I2C，7 位地址 0x48 |
| 继电器 | PB11 | 高电平打开，低电平关闭 |
| 指示灯 1 | PA2 | 低电平点亮 |
| 指示灯 2 | PA3 | 低电平点亮 |
| 指示灯 3 | PB0 | 低电平点亮，运行期间每 500 ms 翻转 |
| 蜂鸣器 | PA6 | 高电平打开，当前主流程未使用 |
| 按键 1/2 | PA0、PA1 | 已有驱动，当前主流程未调用 |
| 调试串口 | LPUART1 PB1_TX | 115200，8N1，仅发送日志 |
| 备用串口 | USART2 PB4_TX、PB5_RX | 115200、DMA 收发驱动已提供，当前未初始化 |

## 软件结构

```text
applications/
├── main.c                     程序入口
├── User/
│   ├── App/                   应用业务和运行调度
│   │   ├── App_RUNTIME.*      初始化与主循环调度
│   │   ├── App_COMMAND.*      BLE 命令处理
│   │   ├── App_TIMER.*        倒计时状态
│   │   └── App_BOARD.*        板级周期任务
│   ├── Mid/                   中间层
│   │   ├── Mid_BLE.*          BLE 通知发送封装
│   │   ├── Mid_FRAME.*        帧收发、解析和组帧
│   │   └── Mid_TIME.*         毫秒时基封装
│   ├── Int/                   板级设备层
│   │   ├── Int_LED.*
│   │   ├── Int_BEEP.*
│   │   ├── Int_RELAY.*
│   │   ├── Int_TM1650.*
│   │   └── Int_KEY.*
│   ├── Dri/                   底层驱动
│   │   ├── Dri_TICK.*         SysTick 1 ms 时基
│   │   ├── Dri_SoftI2C.*      软件 I2C
│   │   └── Dri_UART.*         USART2 DMA 驱动
│   └── Com/
│       └── Com_CRC.*          CRC32 计算
├── app/                       厂商 BLE 应用模板
└── MDK-ARM/                   Keil 工程
```

`firmware/`、`middlewares/` 和 `applications/app/` 中包含厂商或第三方代码，使用时请遵循各自许可证。

## 运行流程

```text
上电 / 复位
    |
    v
App_RUNTIME_Init()
    ├── Mid_TIME_Init()
    ├── App_BOARD_Init()
    ├── App_TIMER_Init()
    ├── App_COMMAND_Init()
    └── app_core_init()
    |
    v
while (1)
    ├── rwip_schedule()
    └── App_RUNTIME_Process()
            ├── App_TIMER_Process()
            ├── App_COMMAND_Process()
            └── App_BOARD_Process()
```

`SysTick_Handler()` 每 1 ms 执行一次，并增加系统毫秒计数。应用层的倒计时和周期任务均通过无符号时间差判断，因此能够正确处理计数器回绕。

## BLE 接口

设备广播名称为 `PLKJ_BLE`。主要使用两个自定义 128 位特征：

| 用途 | 特征 | 属性 | 方向 |
|---|---|---|---|
| 命令写入 | `00002760-08C2-11E1-9073-0E8AC72E0001` | Write Command | 手机 -> 设备 |
| 通知输出 | `00002760-08C2-11E1-9073-0E8AC72E0002` | Notify | 设备 -> 手机 |

命令写入使用 **Write Without Response**，因此 ATT 层不返回 Write Response。设备完成业务处理后，通过 Notify 特征返回应用层 ACK。

手机必须先写入通知特征的 CCCD `0x2902` 并设置为 `0x0001`，才能接收设备通知。

## 私有协议

### 帧格式

```text
+----------+----------+----------+--------+----------+----------+
| Header   | Function | Length   | Data   | CRC32    | End      |
| 2 bytes  | 2 bytes  | 2 bytes  | N      | 4 bytes  | 2 bytes  |
+----------+----------+----------+--------+----------+----------+
```

| 字段 | 值/长度 | 字节序 | 说明 |
|---|---|---:|---|
| Header | `0x50 0x4C` | 固定 | 帧头 |
| Function | 2 bytes | 大端 | 功能码 |
| Length | 2 bytes | 大端 | **仅表示 Data 长度，不包含其他字段** |
| Data | 0-512 bytes | - | 有效数据；当前解析器拒绝长度为 0 的帧 |
| CRC32 | 4 bytes | 大端 | 标准 CRC-32，覆盖 Header、Function、Length、Data |
| End | `0xA5 0x5A` | 固定 | 帧尾 |

若 Data 长度为 `N`，整帧长度为：

```text
2 + 2 + 2 + N + 4 + 2 = N + 12
```

### 功能码

| 功能码 | 名称 | 请求 Data | 响应功能码 | 响应 Data |
|---:|---|---|---:|---|
| `0x0001` | TIMING | 2 字节分钟数，大端 | `0x0002` | `FF FF` |
| `0x0003` | STOP | 当前解析器要求非空；内容忽略 | `0x0004` | `FF FF` |
| `0x0005` | GET_TIME | 当前解析器要求非空；内容忽略 | `0x0006` | 2 字节剩余分钟数，大端，向上取整 |

### 示例

设置 60 分钟倒计时：

```text
50 4C 00 01 00 02 00 3C <CRC32> A5 5A
```

计算内容为：

```text
Length = 0x0002
Data   = 0x003C = 60
```

## 编译与下载

### 环境要求

- Keil MDK-ARM 5
- ARM Compiler 5
- Device Family Pack：`Nationstech.N32WB03X_DFP.1.1.0`
- 可选：VS Code，用于源码编辑和 IntelliSense

### Keil

1. 打开 `applications/MDK-ARM/rdtss.uvprojx`。
2. 选择目标 `N32WB03x`。
3. 执行 `Project -> Build Target`。
4. 连接调试器后执行下载。

构建输出目录：

```text
applications/MDK-ARM/Objects/
applications/MDK-ARM/Listings/
applications/MDK-ARM/bin/
```

这些目录已加入 `.gitignore`。

## 调试日志

启动时会调用 `NS_LOG_INIT()`，日志由 LPUART1 输出：

```text
LPUART1_TX = PB1
Baud rate  = 115200
Data       = 8 bits
Parity     = None
Stop bits  = 1
```

日志等级在 `applications/app/inc/app_user_config.h` 中配置。若不需要调试串口，可将 `NS_LOG_LPUART_ENABLE` 设置为 `0`。

`Dri_UART.c` 提供了 USART2 + DMA 收发驱动，但当前应用层没有调用 `Dri_UART_Init()`。

## 开发注意事项

- `App_RUNTIME_Process()` 中的任务应保持非阻塞。
- BLE Notify 必须由应用主动调用发送函数触发。
- BLE 命令写入使用 Write Command，不能依赖 ATT Write Response。
- `Mid_FRAME_Build()` 的 `Length` 参数表示 Data 段长度。
- 倒计时精度由 1 ms SysTick 时基和主循环调度共同决定。
- 修改系统时钟后应确保 `SystemCoreClock` 与实际时钟一致。