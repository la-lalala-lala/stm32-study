CH395目录下文件说明：
|
|
|-----EVT
|     |
|     |---EXAM
|     |    |
|     |    |基本应用:
|     |    |---EXAM0_Integrated_Example：     C示例源程序，4个Socket分别工作在TCP客户端、服务器、UDP、IPRAW模式
|     |    |---EXAM1_MACRAW：   C示例源程序，Socket0工作在MACRAW模式
|     |    |---EXAM2_UDP_Server：   C示例源程序，Socket0工作在UDP模式，演示UDP广播功能
|     |    |---EXAM3_IPRAW：   C示例源程序，Socket0工作在IP RAW 模式
|     |    |---EXAM4_TCP_Client：   C示例源程序，Socket0工作在TCP 客户端模式
|     |    |---EXAM5_TCP_Server：   C示例源程序，Socket0工作在TCP 服务器模式，使用8个Socket，演示多连接功能
|     |    |---EXAM6_DHCP：   C示例源程序，演示DHCP客户端功能
|     |    |---EXAM13_EEPROM_GPIO：   C示例源程序，演示GPIO操作，EEPROM操作等
|     |    |---EXAM14_UDP_Multicast：   C示例源程序，演示CH395组播功能
|     |    |---EXAM17_TCP6_Client(CH395X)：   C示例源程序，Socket0工作在TCP6 客户端模式，仅CH395X支持
|     |    |---EXAM18_TCP6_Server(CH395X)：   C示例源程序，Socket0工作在TCP6 服务器模式，仅CH395X支持
|     |    |---EXAM19_UDP6_Multicast(CH395X)：   C示例源程序，Socket0工作在UDP6 组播模式，仅CH395X支持
|     |    |---EXAM20_UDP6_Unicast(CH395X)：   C示例源程序，Socket0工作在UDP6 单播模式，仅CH395X支持
|     |    |---EXAM21_DHCPV6(CH395X)：   C示例源程序，演示DHCPv6客户端功能，仅CH395X支持
|     |    |
|     |    |高级应用:
|     |    |---EXAM7_Web：   C示例源程序，CH395 WEB服务器应用实例，演示网内远程控制和远程监测
|     |    |---EXAM8_DNS：   C示例源程序，CH395 DNS域名解析应用实例，根据域名解析对应IP
|     |    |---EXAM9_Mail：   C示例源程序，CH395 MAIL应用实例，演示使用CH395进行邮件收发
|     |    |---EXAM10_FTPServer：   C示例源程序，CH395 FTP服务器应用实例、支持读写、删除、创建文件功能	 
|     |    |---EXAM11_FTPClient：   C示例源程序，CH395 FTP 客户端应用，实现登陆服务器、搜索文件、下载文件、上传文件等功能
|     |    |---EXAM12_ICMP_Ping：   C示例源程序，CH395 演示ICMP PING功能，主动发送PING包以及回复PING包
|     |    |---EXAM15_CFG：   C示例源程序，CH395 演示搜索配置功能，配合上位机软件CH395CFG.exe，演示CH395模块搜索配置过程
|     |    |---EXAM16_FreeRTOS：   C示例源程序，CH395配合FreeRTOS使用
|     |    |
|     |    |---PUB
|     |    |    |
|     |    |    |---CH395.H：    CH395结构体定义头文件
|     |    |    |---CH395INC.H：    CH395定义头文件
|     |    |    |---CH395CMD.C：    命令接口层子程序文件
|     |    |    |---CH395CMD.H：    命令接口层头文件
|     |    |    |---CH395PARA_HW.C：  硬件总线8位并口连接方式子程序文件
|     |    |    |---CH395SPI_HW.C：   硬件SPI接口连接方式 子程序文件
|     |    |    |---CH395UART_HW.C：     硬件异步串口连接方式子程序文件
|     |    |    
|     |    |---SRC
|     |    |    |---CORE：           内核文件
|     |    |    |---DEBUG：         调试文件
|     |    |    |---FWLIB：          外设文件 
|     |       
|   
|-----PUB
|     |
|     |---CH395A-EVT-R0.PDF：CH395A评估版原理图   
|     |---CH395F-CH395X-EVT-R0.PDF：CH395F/CH395X评估版原理图   
|     |---CH395P-EVT-R0.PDF：CH395P评估版原理图   
|
|-----DEBUG395
|     |
|     |---DEBUG395.EXE：  WINDOWS调试软件
|     |---DEBUG395.PDF：  调试软件使用说明
|
|-----CH395系列模块使用说明.pdf：CH395系列模块使用说明