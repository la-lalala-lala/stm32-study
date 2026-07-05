# STM32F103 USART3 连接 ESP32-C3 AT 模块排查记录

## 结论

当前工程使用硬件 USART3 和 ESP32-C3 AT 固件通信，最终验证通过：

- `PB10 = USART3_TX -> ATK-MODULE RXD`
- `PB11 = USART3_RX <- ATK-MODULE TXD`
- 波特率：`115200`
- ATK 模块接口需要通过 `P8` 选择接入串口 3

如果 `P8` 没有选择到 ATK 模块接口，USART3 会连到其他路径，例如 COM3/RS232，ESP32-C3 不会收到指令，STM32 也收不到模块响应。

## 硬件连接要点

参考《战舰 V4 硬件参考手册》：

- `PB10 / USART3_TX` 可通过 `P8` 接到 `ATK-MODULE RXD`
- `PB11 / USART3_RX` 可通过 `P8` 接到 `ATK-MODULE TXD`
- `GBC_TX/GBC_RX` 是 ATK 模块接口信号名，容易和 MCU 侧 TX/RX 方向混淆

实际使用时确认：

1. `P8` 跳帽选择到 `ATK MODULE`，不是 `COM3/RS232`
2. ESP32-C3 模块供电正常
3. ESP32-C3 已烧录 AT 固件
4. ESP32-C3 AT 串口波特率为 `115200`

## 现象

最初烧录后只能看到：

```text
串口打印测试
发送指令：AT+RST=0
```

程序没有继续打印 `读`，说明代码卡在发送阶段。

后续加超时诊断后，曾看到：

```text
ESP32未返回OK(rx=0, SR=0x00C0, PB11=1)
```

含义：

- `rx=0`：USART3 没收到任何字节
- `SR=0x00C0`：发送完成，`TXE=1`、`TC=1`
- `PB11=1`：RX 引脚空闲高电平，没有收到模块 TXD 波形

最终确认主要问题在 `P8` 选择/硬件连接路径。

## 代码侧修复

`src/usart/usart3.c`：

- 使用硬件 USART3 默认映射：`PB10=TX`、`PB11=RX`
- USART3 挂在 APB1，`BRR` 按 `36MHz` 计算
- 初始化时复位 USART3 外设，清理旧状态
- GPIO 配置前先清位再设位
- 发送等待 `TXE` 加超时，避免死等
- 接收改成带超时的轮询读取
- 接收缓冲区按容量传参，返回实际接收字节数
- 清理 `ORE/FE/NE` 等错误状态

`src/wifi/esp32.c`：

- `esp32_send_cmd()` 负责发送 AT 指令并等待响应
- 不再无限等待 `OK`
- 失败时打印 USART3 状态和 RX/TX 引脚电平
- `AT+RST=0\r\n` 后延时并清空串口残留数据

`src/main.c`：

- 不再在 `esp32_send_cmd()` 后二次调用 `esp32_read_response()`
- 主流程末尾加 `while (1)`，避免裸机 `main()` 返回

## 已验证输出

P8 选择到 ATK 模块接口后，日志正常：

```text
串口打印测试
发送指令：AT+RST=0
读
AT+RST=0

OK
...
测试AT启动
发送指令：AT
读
AT

OK

=====================
查看版本信息
发送指令：AT+GMR
读
AT+GMR
AT version:4.2.0.0-dev(...)
SDK version:v5.4.1-643-g8ad0d3d8f2-dirty
Bin version:v4.2.0.0-dev(MINI-1)

OK
```

说明 STM32 USART3 和 ESP32-C3 AT 通信链路已经打通。

## 经验记录

不要把 ATK 模块座上的 `TXD/RXD` 和 STM32 USART3 的 `TX/RX` 机械对应。对 STM32 来说，最终必须是：

```text
STM32 TX -> 模块 RXD
STM32 RX <- 模块 TXD
```

战舰 V4 开发板通过 `P8` 选择接口实现这条路径。串口配置本身不需要交换 USART3 引脚，也不需要软件串口。
