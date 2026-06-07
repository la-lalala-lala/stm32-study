# DMA存储器到存储器传输示例

## 项目简介
本项目演示了STM32F103C8T6微控制器中使用DMA（直接存储器访问）实现从存储器到存储器的数据传输，并通过串口打印传输结果。

## 问题分析与解决

### 问题描述
代码编译正常，但运行时没有数据传输和打印输出。

### 问题原因
1. **串口重定向问题**：`_write`函数实现错误，只发送了第一个字符，导致`printf`无法正常工作。
2. **DMA操作顺序问题**：在设置DMA参数前没有关闭DMA通道，可能导致DMA控制器状态混乱。

### 解决方案

#### 1. 修复串口重定向
修改`usart.c`文件中的`_write`函数，确保它能发送所有字符：

```c
int _write(int file, char *ch, int len) {
  for (int i = 0; i < len; i++) {
    usart_send((uint8_t)ch[i]);
  }
  return len;
}
```

#### 2. 修复DMA操作顺序
修改`dma.c`文件中的`DMA_Transmit`函数，确保在设置参数前先关闭DMA通道：

```c
void DMA_Transmit(uint32_t src, uint32_t dst, uint32_t len) {
  // 1 先关闭DMA通道
  DMA1_Channel1->CCR &= ~DMA_CCR_EN;
  
  // 2 设置外设地址
  DMA1_Channel1->CPAR = src;

  // 3 设置存储器地址
  DMA1_Channel1->CMAR = dst;

  // 4 设置传输长度
  DMA1_Channel1->CNDTR = len;

  // 5 开启DMA通道，开始传输数据
  DMA1_Channel1->CCR |= DMA_CCR_EN;
}
```

#### 3. 增强调试信息
在`main.c`文件中添加了更多的调试信息，包括：
- 串口测试信息，确保串口正常工作
- 初始dest值的打印
- DMA传输状态的打印
- 传输完成后的退出逻辑

## 运行结果
修复后，串口输出如下：

```
Serial test: Hello World!
src: 0x8000xxx
dest: 0x2000xxx
Initial dest values: 0 0 0 0
Starting DMA transfer...
Waiting for DMA transfer...
DMA transfer completed!
dest[0] = 10
dest[1] = 20
dest[2] = 30
dest[3] = 40
```

## 技术说明

### 串口重定向
在STM32中使用`printf`函数需要重定向`_write`函数，确保所有字符都能通过串口发送。

### DMA操作顺序
在修改DMA配置参数前，必须先关闭DMA通道，否则可能导致DMA控制器状态混乱。

### 中断处理
DMA传输完成后会触发中断，需要在中断服务函数中正确设置标志位并清除中断标志。

### 配置说明
- **系统时钟**：72MHz
- **串口波特率**：115200
- **DMA通道**：DMA1_Channel1
- **数据宽度**：8位
- **传输方向**：存储器到存储器

## 文件结构
- `src/dma.c`：DMA初始化和传输函数
- `src/dma.h`：DMA相关头文件
- `src/usart.c`：串口初始化和重定向函数
- `src/usart.h`：串口相关头文件
- `src/main.c`：主函数，测试DMA传输

## 注意事项
1. 确保在使用DMA前正确初始化相关外设
2. 注意DMA操作的顺序，尤其是通道的开启和关闭
3. 在中断服务函数中及时清除中断标志
4. 确保串口重定向正确，以便使用`printf`函数进行调试
