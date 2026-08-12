# STM32F103 + ATK-MB026 调试与问题记录

本工程在正点原子战舰 V4（STM32F103ZET6）上，通过 USART3 驱动
ATK-MB026（ESP32-C3 AT 固件），并通过 USART1 输出调试日志。

本文记录两个彼此独立、但现象容易混在一起的问题：

1. ST-Link 已经完成烧录和校验，但按开发板 RESET 后新程序不运行，断电重上电才生效。
2. STM32 已经运行并打印了 `串口打印测试`，但 MB026 偶尔没有 `ready`、`AT/OK` 或
   `AT+GMR` 输出。

当前修复已在本机和当前接线下验证。修复阶段连续 5 次复位均执行到 `AT+GMR/OK`，之后
用户又手动复位验证了 3 次，均正常，问题基本消失。这个结果用于说明当前方案有效，不代表
所有供电、固件和接线环境都能保证 100% 无故障。

## 硬件连接

P8 跳帽必须选择 `ATK MODULE`，不能选择 `COM3/RS232`：

```text
STM32 PB10 / USART3_TX  ->  ATK-MB026 RXD
STM32 PB11 / USART3_RX  <-  ATK-MB026 TXD
STM32 PA4  / GBC_KEY    ->  ATK-MB026 RST（低电平有效）
5V                      ->  ATK-MB026 VCC
GND                     --- GND
```

MB026 默认串口参数为 `115200 8N1`。模块串口是 TTL 电平，不能直接接 RS232 电平。
PA4 还与开发板上的其他功能复用，本工程运行时将它专用于 MB026 复位，不应让其他外设同时
驱动该引脚。

## 一、烧录成功，但 RESET 后代码不运行

### 1.1 如何判断不是“没有烧进去”

原始上传日志包含：

```text
** Programming Finished **
** Verify Started **
** Verified OK **
** Resetting Target **
```

这表示 ELF 的可加载区域已写入 Flash 且校验通过。下面这条擦除提示只是把擦除范围补齐到
Flash 页边界，不是烧录失败：

```text
Warn : Adding extra erase range, 0x080017d0 .. 0x080017ff
```

真正关键的线索是 OpenOCD 连接目标时发现 CPU 停在：

```text
pc: 0x1ffff020
```

`0x1FFFFxxx` 位于 STM32F103 的系统存储器，说明芯片进入了片内 Bootloader，而不是从
用户 Flash `0x08000000` 启动。因此问题发生在“复位时采样启动模式”阶段，不在编程或
校验阶段。

### 1.2 根因：板载 CH340 一键下载电路仍会影响 BOOT0

即使 BOOT0、BOOT1 跳帽都接 GND，也不能完全排除这个影响。战舰 V4 板载 CH340C 的
DTR/RTS 通过 Q2、Q3 同时连接 RESET 和 BOOT0，用来实现串口一键下载。串口监视器打开、
关闭或 USB 重新枚举时，DTR/RTS 状态可能改变；晶体管支路可在复位采样瞬间把 BOOT0
置为串口下载状态。

这也解释了原现象：

- ST-Link 编程和校验都成功，但 OpenOCD 复位后可能进入 `0x1FFFF020`。
- 按 RESET 仍可能在错误的 BOOT0 状态下重新采样，所以看不到新代码。
- 断电再上电会改变 CH340 和一键下载电路的初始状态，于是程序又能从 Flash 启动。

在当前开发板上实测，串口驱动的逻辑配置 `DTR=1`、`RTS=0` 是正常运行组合。这里写的是
串口 API/PlatformIO 的逻辑值，不应直接等同于经过 CH340 和反相电路后的引脚电平。

### 1.3 工程中的修复

[`platformio.ini`](platformio.ini) 只保留一套上传入口：

```ini
upload_protocol = stlink
monitor_dtr = 1
monitor_rts = 0
extra_scripts = post:scripts/select_stlink_upload.py
```

[`scripts/select_stlink_upload.py`](scripts/select_stlink_upload.py) 完成两件事：

1. 上传前寻找当前机器上的 CH340（VID `1A86`，PID `5523` 或 `7523`），临时打开串口并
   尝试设置 `DTR=1`、`RTS=0`，释放板载一键下载电路对启动状态的影响。
2. 检查 PlatformIO 当前 OpenOCD 包中 `interface/stlink.cfg` 的内容特征：包含 native
   ST-Link 驱动配置时选择 native SWD，否则保留 PlatformIO 默认 HLA 上传命令。

脚本同时处理可执行文件名差异：Windows 使用 `openocd.exe`，macOS/Linux 使用
`openocd`。因此 Windows、macOS、Linux 共用同一个 `platformio.ini`，不需要按系统手工
切换 `upload_protocol`、`upload_command`，也不需要再添加：

```ini
upload_flags =
    -f
    interface/stlink-hla.cfg
```

当前 Linux 主机和 CH340 驱动已实测该方式有效。恰好找到并成功打开一个目标串口时，上传
日志应看到类似提示：

```text
CH340 boot circuit: normal run state (...)
ST-Link upload transport: native SWD
```

如果使用旧 OpenOCD，第二行会显示：

```text
ST-Link upload transport: PlatformIO default HLA
```

### 1.4 自动配置的边界

- 没有配置 `monitor_port` 时，脚本只会在恰好找到一个匹配的 CH340 时自动操作它。
- 如果连接了多个 CH340，必须在 `platformio.ini` 中明确指定对应开发板，例如
  `monitor_port = COM5`、`monitor_port = /dev/cu.wchusbserial...` 或
  `monitor_port = /dev/ttyUSB0`。显式指定后脚本不会再检查 VID/PID，用户必须确认它确实
  是本开发板的 CH340 端口。
- 如果没有找到 CH340，脚本会静默跳过释放操作，不会打印成功提示。Windows/macOS 需要
  正确安装 CH340 驱动，Linux 用户还需具备对应串口的访问权限。
- 如果串口正被监视器、`cat` 或其他程序占用，脚本可能无法设置 DTR/RTS。上传前先关闭
  占用程序。
- 如果 PlatformIO Python 环境缺少 pyserial，脚本会打印警告并继续烧录，但不会释放
  CH340 的启动控制状态。
- 脚本检测的是 OpenOCD 配置文件能力，不是 ST-Link 探头的固件版本。现代 OpenOCD 配合
  低于 V2J24 的旧探头仍可能无法使用 native 驱动，此时应先升级探头固件。
- native/HLA 选择是配置文件特征判断；native 上传若因探头固件或其他原因失败，脚本不会
  在同一次上传中自动重试 HLA。
- 上传脚本设置 DTR/RTS 后会关闭串口。控制线关闭后的保持状态取决于操作系统和 CH340
  驱动，因此不能仅凭这段脚本保证所有 Windows/macOS/Linux 环境行为完全相同。
- `monitor_dtr`、`monitor_rts` 只控制 PlatformIO 串口监视器；上传脚本中的同一组值目前
  是独立设置，修改一处时应同步检查另一处。
- 如果必须长期同时打开串口监视和反复复位，最稳妥的硬件隔离方案是使用只连接
  `TX/RX/GND`、不连接 DTR/RTS 的独立 USB-TTL，同时拔掉板载 CH340 的 USB，或用其他
  方式隔离板载 DTR/RTS 对 BOOT0/RESET 的影响。

### 1.5 ST-Link 固件升级

原日志中的：

```text
DEPRECATED: OpenOCD support for ST-Link HLA transport will be dropped soon!
Consider updating your ST-Link firmware to a version >= V2J24 (2015)
Warn : DEPRECATED! use 'transport select swd', not 'transport select hla_swd'
```

表示 OpenOCD 正在淘汰旧 HLA 驱动，不表示本次 Flash 写入失败。升级步骤如下：

1. 安装意法半导体官方 STM32CubeProgrammer。
2. 将 ST-Link 接到电脑，打开安装包附带的 **ST-Link Upgrade** 工具。
3. 选择 **Open in update mode**，识别探头后选择 **Upgrade**。
4. 升级完成后重新插拔 ST-Link，再执行 PlatformIO 上传进行验证。

本机探头已升级并识别为 `ST-LINK/V2 V2J37S7`，高于 OpenOCD 提示的最低版本 V2J24。
升级探头解决的是 ST-Link/OpenOCD 驱动兼容性；它本身不会消除战舰 V4 上 CH340 对
BOOT0/RESET 的影响，所以上传前的 DTR/RTS 处理仍然保留。

### 1.6 烧录问题排查顺序

1. 确认日志有 `Programming Finished` 和 `Verified OK`。
2. 查看复位后的 PC：`0x080xxxxx` 是用户 Flash，`0x1FFFFxxx` 是系统 Bootloader。
3. 关闭所有占用板载 CH340 串口的程序，再上传一次。
4. 确认上传脚本打印了 `CH340 boot circuit: normal run state`。
5. 多个 CH340 并存时设置正确的 `monitor_port`。
6. 确认 BOOT0 为 0；若仍不稳定，断开板载 CH340 的 DTR/RTS 影响或改用独立 USB-TTL。

## 二、MB026 偶尔没有 ready 或 AT 输出

### 2.1 原因不是单一串口接线错误

旧实现同时存在几项问题，叠加后表现为“有时正常、有时完全不打印”：

1. 使用了错误命令 `AT+RST=0`。MB026 手册规定的软件复位执行命令是 `AT+RST`，没有
   `=0` 参数。
2. 把异步启动信息 `ready` 当作初始化成功的唯一同步点。`ready` 不是 `AT` 指令的应答；
   它可能在 USART3 初始化前已经发出，也可能夹在启动日志中，所以程序不应要求每次都看到它。
3. 开发板 RESET 按钮只复位 STM32 和 LCD，并不会复位插在 ATK MODULE 接口上的 MB026。
   STM32 和模块的启动状态因此不同步，断电重上电反而更容易成功。
4. 旧 USART3 接收没有可靠的总超时和缓冲区边界，并按一次 IDLE/一段数据判断响应。
   ESP-AT 响应可能分段到达，`OK` 落在下一段时就会被漏判。
5. 延时和串口分频依赖固定时钟假设。若实际系统时钟与假设不一致，复位时序、超时和
   115200 波特率都会偏离。

因此，是否出现 `ready` 本身不是通信成功标准。可靠同步方式是：主动硬件复位模块，等待
模块启动，然后反复发送 `AT\r\n`，直到收到 `OK` 或达到重试上限。

### 2.2 当前初始化流程

[`src/wifi/esp32.c`](src/wifi/esp32.c) 中的 `ESP32_Init()` 现在执行：

```text
PA4 拉低 MB026 RST 100 ms
        |
PA4 释放为高电平，等待 500 ms
        |
初始化 USART3（PB10/PB11，115200 8N1）
        |
最多 10 次发送 AT\r\n
        |
每次等待 OK 约 500 ms；收到即初始化成功
```

这个时序和“最多 10 次 `AT`、每次等待 500 ms”的策略来自官方 MB026 示例。代码不再
为了初始化发送 `AT+RST=0`，也不再等待 `ready`。由于 USART3 是在模块复位等待完成后
才初始化，启动横幅或 `ready` 完全看不到也是正常情况。

### 2.3 AT 响应接收修复

`ESP32_Send_CMD()` 和 USART3 驱动现在具备：

- 1024 字节累计缓冲区，最多保存 1023 字节并始终保留字符串结束符。
- 每条命令发送前清理旧接收数据，命令全部写入 USART 后开始计算等待时间。
- 单次轮询首字节等待 20 ms，收到数据后以 10 ms 字节间静默结束当前分块。
- 多个分块追加到同一缓冲区，再从累计数据中查找期待字符串，因此能处理分段响应。
- 首字节/字节间超时、整体等待控制和容量边界，失败时返回而不是永久阻塞或越界写入。
- 正确处理 USART 的 ORE、FE、NE、PE 错误；轮询中不为清 IDLE 额外读取 DR，避免在
  SR/DR 两次读取之间到达的新字节被误吞。

当前实现适合本工程的短 AT 命令和 `AT+GMR` 响应，但仍是主循环轮询，并非中断/DMA
环形缓冲。大量异步 URC、持续高速数据，或在 USART1 打印较长日志时，仍可能产生丢包；
若后续要长期处理异步消息，应升级为 USART3 中断或 DMA 环形缓冲。

### 2.4 时钟和延时修复

- USART1/USART3 根据 `SystemCoreClock` 和 RCC 实际 APB 分频计算 BRR，不再把 72 MHz
  或 36 MHz 写死在波特率算法中。
- HSE 启动使用 DWT 计数器执行真实的 100 ms 超时。8 MHz HSE 正常时配置为 72 MHz；
  HSE 失败时回退到 8 MHz HSI。
- 延时和 AT 接收超时改为基于 DWT `CYCCNT`，不再占用或重配 SysTick，并按当前
  `SystemCoreClock` 计算。

这些动态计算目前覆盖本工程使用的 delay、USART1 和 USART3，不能据此推断工程中所有
其他外设都已自动适配 HSI fallback。DWT 的 32 位超时换算在 72 MHz 下应保持小于约
59.65 秒；当前使用的 100/500/1000 ms 均在安全范围内。

### 2.5 当前程序的正常输出逻辑

[`src/main.c`](src/main.c) 的顺序是：

1. 初始化系统时钟和 USART1，打印 `串口打印测试`。
2. 硬件复位 MB026，并在内部通过 `AT/OK` 重试完成初始化。
3. 打印 `测试AT启动`，再次发送 `AT`，等待 `OK` 500 ms。
4. 打印 `查看版本信息`，发送 `AT+GMR`，等待最终 `OK` 1000 ms。

启动阶段可能出现如下任一种情况，都不代表异常：

- 第一次 `AT` 只接收到 ESP32-C3 启动横幅。
- 后续一次接收到异步 `ready`，但没有 `OK`，程序继续重试。
- 再下一次 `AT` 收到 `OK`，初始化成功并继续执行 `AT+GMR`。
- 启动横幅和 `ready` 都已错过，第一次或后续 `AT` 直接收到 `OK`。

关键成功条件是主动发送的 `AT` 获得 `OK`，以及 `AT+GMR` 最终获得 `OK`，而不是日志中
必须出现 `ready`。

## 三、快速定位到底是哪一层失败

### 完全没有 `串口打印测试`

这行在访问 MB026 之前打印。看不到它时，应先排查 STM32 是否从用户 Flash 启动、USART1
监视端口和波特率是否正确，以及 CH340 DTR/RTS 是否再次影响 BOOT0；不要先归因于 MB026。

### 有 `串口打印测试`，但显示 `ESP32初始化失败`

依次检查：

1. P8 是否选择 `ATK MODULE`。
2. `PB10 -> 模块 RXD`、`PB11 <- 模块 TXD` 的方向是否正确，并且共地。
3. 模块供电是否稳定，串口是否为 `115200 8N1`。
4. PA4/GBC_KEY 是否确实连接模块 RST，且没有被其他板载功能占用。
5. 模块是否运行 AT 固件，而不是停在自身固件下载模式。
6. 超时日志里实际收到了什么：启动横幅、`ready`、`ERROR`、乱码或完全没有字节。

### `AT` 成功，但 `AT+GMR` 失败

先看累计响应中是否已有版本信息而只缺最后的 `OK`。当前 `AT+GMR` 等待 1000 ms；若替换
了模块固件、响应明显变长或加入大量异步 URC，可适当增大超时，并考虑使用中断/DMA 接收。

## 四、手册依据

- [`ATK-MB026 WIFI & BLE模块用户手册_V1.0.pdf`](<ATK-MB026 WIFI & BLE模块/ATK-MB026 WIFI & BLE模块用户手册_V1.0.pdf>)
  - PDF 第 5 页（正文第 3 页）：模块 RST 低电平有效，TXD/RXD 为 TTL 电平。
  - PDF 第 7 页（正文第 5 页）：默认 UART 为 115200、8 数据位、1 停止位、无校验。
  - PDF 第 8 页（正文第 6 页）：命令以 `\r\n` 结尾；软件复位命令为 `AT+RST`。
- [`ATK-MB026 WIFI & BLE模块使用说明_V1.0.pdf`](<ATK-MB026 WIFI & BLE模块/ATK-MB026 WIFI & BLE模块使用说明_V1.0.pdf>)
  - PDF 第 5 页（正文第 3 页）：官方初始化先硬件复位，再最多 10 次发送 `AT`，每次等待
    `OK` 500 ms。
- [`M48Z-M3最小系统板STM32F103版.zip`](<ATK-MB026 WIFI & BLE模块/3，程序源码/ATK-MB026模块TCP透传实验/M48Z-M3最小系统板STM32F103版.zip>)
  - 官方示例中的 `Drivers/BSP/ATK_MB026/atk_mb026.c`：RST 拉低 100 ms，释放后等待
    500 ms；随后执行 `AT/OK` 测试。
- [`战舰V4 硬件参考手册_V1.0.pdf`](<战舰V4 硬件参考手册_V1.0.pdf>)
  - 第 26 页：RESET 按钮连接 STM32 和 LCD；BOOT0=0 时从用户 Flash 启动。
  - 第 34 页：P8 将 ATK 模块接到 USART3，GBC_KEY 接 PA4。
  - 第 40 页：CH340C 的 DTR/RTS 一键下载电路同时驱动 RESET 和 BOOT0。

## 五、常用命令

```bash
# 构建
pio run

# 使用自动选择的 ST-Link 配置上传
pio run -t upload

# 查看完整上传命令和 OpenOCD 输出
pio run -t upload -v

# 串口监视（115200）
pio device monitor
```

上传与串口监视切换时，如果再次出现 `0x1FFFFxxx` 或按 RESET 无输出，优先按 1.6 节检查
CH340 的端口占用及 DTR/RTS 状态。
