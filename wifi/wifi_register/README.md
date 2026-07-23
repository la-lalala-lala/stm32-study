# STM32F103 USART3 连接 ESP32-C3 AT 模块记录

## 结论
当前工程已验证可通过硬件 USART3 与 ESP32-C3 AT 固件通信。

- `PB10 = USART3_TX -> ATK-MODULE RXD`
- `PB11 = USART3_RX <- ATK-MODULE TXD`
- 波特率：`115200`
- ATK 模块接口通过 `P8` 选择到 `ATK MODULE`

当前 ESP32 侧把返回串分成两类：

- `RESPONSE_OK = "OK"`：普通 AT 命令
- `RESPONSE_READY = "ready"`：`AT+RST=0` 重启完成标志

ESP32 发送接口现在是：

```c
esp32_send_cmd(const char *cmd, uint16_t cmd_length, char *expect_result)
```

其中 `expect_result` 决定这条命令要等待的返回串。

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

## 接口使用
`esp32_init()` 内部会先发送 `AT+RST=0`，等待 `ready`，然后延时等待模块稳定。

示例：

```c
esp32_init();
esp32_send_cmd("AT\r\n", 4, RESPONSE_OK);
esp32_send_cmd("AT+GMR\r\n", 8, RESPONSE_OK);
```

`usart3_receive()` 和 `usart3_receive_string()` 仍然保持通用接收用途；ESP32 层额外使用 `usart3_receive_idle_chunk()` 来读取一段以空闲帧结束的数据。

## 现象
`AT+RST=0` 会先输出 ESP32 的启动日志，再输出 `ready`，这不是普通 AT 命令的 `OK` 返回。

如果 `P8` 没有选择到 `ATK MODULE`，USART3 会连到其他路径，例如 `COM3/RS232`，ESP32-C3 不会收到指令，STM32 也收不到模块响应。

当前失败提示文本仍沿用“`ESP32未返回OK`”，但实际判断条件由 `expect_result` 决定。

## 代码侧修复
`src/usart/usart3.c`：

- 使用硬件 USART3 默认映射：`PB10=TX`、`PB11=RX`
- USART3 挂在 APB1，`BRR` 按 `36MHz` 计算
- 初始化时复位 USART3 外设，清理旧状态
- GPIO 配置前先清位再设位
- 发送等待 `TXE` 加超时，避免死等
- 接收保留通用单字节/字符串接口，并额外提供 `usart3_receive_idle_chunk()`

`src/wifi/esp32.c`：

- `esp32_send_cmd()` 负责发送 AT 指令并等待指定返回串
- 普通 AT 命令使用 `RESPONSE_OK`
- `AT+RST=0` 使用 `RESPONSE_READY`
- `esp32_read_response()` 只负责清 buffer 并读取一段 IDLE 结束的数据

`src/usart/usart1.c`：

- 继续作为 `printf` 输出串口
- 增加了 `AFIO` 时钟、USART1 复位和默认重映射清理
- 发送/接收加入超时处理

`src/main.c`：

- 现在直接按 `esp32_send_cmd("AT\r\n", 4, RESPONSE_OK)` 和 `esp32_send_cmd("AT+GMR\r\n", 8, RESPONSE_OK)` 使用

## 已验证输出
P8 选择到 ATK 模块接口后，日志表现为：

```text
串口打印测试
发送指令：AT+RST=0
... ESP32 boot log ...
ready
=====================
测试AT启动
发送指令：AT
AT

OK

=====================
查看版本信息
发送指令：AT+GMR
AT+GMR
... version ...
OK

=====================
```

## 经验记录
不要把 ATK 模块座上的 `TXD/RXD` 和 STM32 USART3 的 `TX/RX` 机械对应。对 STM32 来说，最终必须是：

```text
STM32 TX -> 模块 RXD
STM32 RX <- 模块 TXD
```

战舰 V4 开发板通过 `P8` 选择接口实现这条路径。串口配置本身不需要交换 USART3 引脚，也不需要软件串口。
