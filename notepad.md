---
title: BLE开发笔记 
tags: 笔记
grammar_cjkRuby: true
---

## 术语缩写

- GAP Generic Access Profile                                     通用访问配置文件
- GATT Generic Attribute Profile                                 通用属性配置文件
- ATT Attribute Protocol
- L2CAP Logical Link Control and Adaptation Protocol
- HCI Host Controller Interface
- LL Link Layer
- SMP Security Manager Protocol
- GFSK Gaussian Frequency Shift Keying
- S110 从设备使用的协议栈
- S120 主设备使用的协议栈
- S130 主、从设备都可使用的协议栈
- S212 (ANT only)
- S332 (concurrent ANT/BLE) 
- CCCD 客户端特性配置描述符
- BSP Board Support Package
- BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE 限制可发现模式
- BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE 一般可发现模式，用于永远广播。

## Physical Layer
- Band ISM @2.4GHz
- 40 channel of 2MHz
- freq = 2402 + i * 2MHz, i = 0...39
- Advertising physical channel(i = 0, 12, 39)
- Data physical channel(the other 37 RF channels)

|i|channel|freq/MHz|type|
|---|---|---|---|
|0|37|2402|Adv|
|1|0|2404|Data|
|2|1|2406|Data|
|3|2|2408|Data|
|4|3|2410|Data|
|5|4|2412|Data|
|6|5|2414|Data|
|7|6|2416|Data|
|8|7|2418|Data|
|9|8|2420|Data|
|10|9|2422|Data|
|11|10|2424|Data|
|12|38|2426|Adv|
|13|11|2428|Data|
|14|12|2430|Data|
|15|13|2432|Data|
|16|14|2434|Data|
|17|15|2436|Data|
|18|16|2438|Data|
|19|17|2440|Data|
|20|18|2442|Data|
|21|19|2444|Data|
|22|20|2446|Data|
|23|21|2448|Data|
|24|22|2450|Data|
|25|23|2452|Data|
|26|24|2454|Data|
|27|25|2456|Data|
|28|26|2458|Data|
|29|27|2460|Data|
|30|28|2462|Data|
|31|29|2464|Data|
|32|30|2466|Data|
|33|31|2468|Data|
|34|32|2470|Data|
|35|33|2472|Data|
|36|34|2474|Data|
|37|35|2476|Data|
|38|36|2478|Data|
|39|39|2480|Adv|



## BLE 设备的六种状态
- Standby 待机状态
- Advertiser 广播状态
- Scanner 扫描状态（主动扫描、被动扫描）
- Initiator 发起连接状态
- Connection 连接状态（又分为Master 主设备、Slave 从设备）


## BLE 连接状态流程
- Standby -> Advertiser -> Slave
- Standby -> Scanner -> Initiator -> Master

## BLE的主要架构解析

### main 函数

- ble_stack_init();             BLE栈初始化
- gap_params_init();        通用访问配置文件参数初始化
- service_init();                服务初始化
- advertising_init();          广播初始化
- ble_advertising_start(BLE_ADV_MODE_FAST); 开启广播

### 事件派发

- ble_evt_dispatch(ble_evt_t \*p_ble_evt); 所有处理事件的函数都在该函数里。

### 服务的创建

#### 定义服务的结构体

```c
typedef struct ServiceStruct {
	uint16_t conn_handle;			// 连接句柄
	uint16_t service_handle;		// 服务句柄
	ble_gatts_char_handles_t char_handle;	// 服务的特征句柄
} Service;
```

#### 服务的初始化 services_init()

- sd_ble_gatts_service_add(); 添加一个服务
- sd_ble_gatts_characteristic_add(); 添加服务的特征

#### 服务的处理

- define_server_handler(ble_evt_t \*p_ble_evt); 服务的处理函数
- 将服务处理函数添加到 ble_evt_dispatch(ble_evt_t \*p_ble_evt) 函数中

### BLE发送数据

- sd_ble_gatts_hvx(); 发送数据的函数


### UUID
The full 128-bit value of a 16-bit or 32-bit UUID may be computed by a simple arithmetic operation.

128_bit_value = 16_bit_value * 2^96 + Bluetooth_Base_UUID
128_bit_value = 32_bit_value * 2^96 + Bluetooth_Base_UUID
A 16-bit UUID may be converted to 32-bit UUID format by zero-extending the 16-bit value to 32-bits. An equivalent method is to add the 16-bit UUID value to a zero-valued 32-bit UUID.
Note that, in another section, there's a handy mnemonic:

Or, to put it more simply, the 16-bit Attribute UUID replaces the x’s in the follow- ing:

0000xxxx-0000-1000-8000-00805F9B34FB
In addition, the 32-bit Attribute UUID replaces the x's in the following:

xxxxxxxx-0000-1000-8000-00805F9B34FB

### 消耗时间

- 最长数据包：广播 376us、数据 328us
- 包间隔时间 150us
- BLE 数据传输速率为: 1000000bit/s


### 连接时产生配对请求和判断配对码是否正确
- 利用API：sd_ble_gap_authenticate（）；
```c
uint32_t on_create_authenticate(uint16_t handle)
{
	ble_gap_sec_params_t		params;
	
	params.bond = 0;
	params.mitm = 1;
	
	return sd_ble_gap_authenticate(handle, &params);
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t 	err_code;
	...
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

	    // create authenticate
	    on_create_authenticate(m_conn_handle);
		
            break; // BLE_GAP_EVT_CONNECTED
	case BLE_GAP_EVT_AUTH_STATUS:	// 判断是否配对码正确通过验证
	    if(p_ble_evt->evt.gap_evt.params.auth_status.auth_status == BLE_GAP_SEC_STATUS_SUCCESS)
	    {
		SEGGER_RTT_printf(0, "BLE_GAP_EVT_AUTH_STATUS Success\n");
            }
	    else
	    {
		sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		m_conn_handle = BLE_CONN_HANDLE_INVALID;
		SEGGER_RTT_printf(0, "BLE_GAP_EVT_AUTH_STATUS Failed, Disconnected\n");
	    }
	    break;
...
```

### 使用静态密钥

```c
void add_static_passkey(void)
{
	uint32_t err_code;
	uint8_t passcode[] = "123456";
	
	ble_opt_t	static_option;
	
	static_option.gap_opt.passkey.p_passkey = passcode;
	err_code = sd_ble_opt_set(BLE_GAP_OPT_PASSKEY, &static_option);
	APP_ERROR_CHECK(err_code);
}
```

### 配对

- ble_gap_sec_keyset_t 本地和对等密钥的安全密钥集
- ble_gap_enc_key_t 加密密钥
- ble_gap_sec_params_t GAP 安全参数
- 

### 让bootload支持正常启动app
- 在 bootloader_settings.c 中把如下代码更改即可。
```c
// 把下面代码更改：
uint8_t  m_boot_settings[CODE_PAGE_SIZE]    __attribute__((at(BOOTLOADER_SETTINGS_ADDRESS))) __attribute__((used));   
// 更改为：
uint8_t  m_boot_settings[CODE_PAGE_SIZE]    __attribute__((at(BOOTLOADER_SETTINGS_ADDRESS))) __attribute__((used)) = {BANK_VALID_APP};
```


### 获取 RSSI
- 使用两个函数：uint32_t  sd_ble_gap_rssi_start (uint16_t conn_handle, uint8_t threshold_dbm, uint8_t skip_count) 和 uint32_t 	sd_ble_gap_rssi_get (uint16_t conn_handle, int8_t \*p_rssi)
- sd_ble_gap_rssi_start 有两种用法：
- 把 threshold_dbm 设置为 BLE_GAP_RSSI_THRESHOLD_INVALID，skip_count 为 0，那么每次直接调用 sd_ble_gap_rssi_get 获取最近一次连接事件中的 RSSI 值。
- 把 threshold_dbm 设置为一个有效值，当 RSSI 大于或等于上一次 RSSU 值时就会产生一个 BLE_GAP_EVT_RSSI_CHANGED 事件。skip_count 为忽略的次数。


### 使用按键
- 初始化 uint32_t bsp_event_to_button_action_assign(uint32_t button, bsp_button_action_t action, bsp_event_t event)
- void bsp_event_handler(bsp_event_t event) 中处理初始化时定义的 event 就可以了。

### UART 串口的 APP_UART_COMMUNICATION_ERROR 错误
- 该问题是因为使用 115200 波特率未使用流控导致的，把波特率降低就可以了，设置 9600 时就正常了。 


### 获取 MAC 地址

```c
void get_mac_addr(uint8_t *p_mac_addr)
{
	uint32_t error_code;
	ble_gap_addr_t p_mac_addr_t;
	
	error_code = sd_ble_gap_address_get(&p_mac_addr_t);
	APP_ERROR_CHECK(error_code);

#if 1 // mac address LSB or MSB
	for ( uint8_t i = 6; i > 0; )
	{	
		i--;
		p_mac_addr[5-i]= p_mac_addr_t.addr[i];
	}
#else
	memcpy(p_mac_addr, p_mac_addr_t.addr, 6);
#endif
}
```

### 自定义设备名称
```c
#define DEVICE_NAME                     {'N', 'o', 'r', 'd', 'i', 'c', '-'}         /**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME_LEN			15

uint32_t get_device_name_from_mac_addr(void)
{
	ble_gap_conn_sec_mode_t 	sec_mode;
	unsigned long int		mac_addr;
	char				device_name[DEVICE_NAME_LEN] = DEVICE_NAME;
	
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	
	mac_addr = NRF_FICR->DEVICEADDR[0];	//设备的器件ID
	
	sprintf(device_name+7, "%lX", mac_addr);
	
	return sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) device_name,
                                          DEVICE_NAME_LEN);
}

static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    
    err_code = get_device_name_from_mac_addr();	// 添加在这里
    APP_ERROR_CHECK(err_code);
	...
}
```

### 创建zip格式的DFU升级包

- 1. Keil 中用命令生成 bin 文件：Option->User->After Build/Rebuild->Run#1:choice and write to command->**C:\Keil_v5\ARM\ARMCC\bin\fromelf.exe .\_build\out_file_name.axf --output .\bin\out_bin_file_name.bin --bin**

```c
# 用 .axf 文件来生成 .bin 文件。
C:\Keil_v5\ARM\ARMCC\bin\fromelf.exe .\_build\out_file_name.axf --output .\bin\out_bin_file_name.bin --bin
```
- 2. 用命令生成 zip 文件：
```c
/*
# 用 nrfutil.exe 这个工具来生成 zip 文件。
# the tool is located in the C:\Program Files (x86)\Nordic Semiconductor\Master Control Panel\<version>\nrf\ folder on Windows
# --application-version version: the version of the application image, for example, 0xff
# --dev-revision version: the revision of the device that should accept the image, for example, 1
# --dev-type type: the type of the device that should accept the image, for example, 1
# --sd-req sd_list: a comma-separated list of FWID values of SoftDevices that are valid to be used with the new image, for example, 0x4f,0x5a
*/
// 0x0080 为 S130_nRF51_2.0.0 。
# C:\Program Files (x86)\Nordic Semiconductor\Master Control Panel\3.10.0.14\nrf>
nrfutil.exe dfu genpkg ota_package_name.zip --application bin_file_name.bin --application-version 1--dev-revision 1 --dev-type 1 --sd-req 0x0080

// bat 脚本
@echo off
set /p package_name=input name.zip for zip package:
nrfutil.exe dfu genpkg %package_name% --application bin_file_name.bin --application-version 1 --dev-revision 1 --dev-type 1 --sd-req 0x0080
pause

```

### 合并 SD、APP、Bootload 为一个hex文件
- 使用 mergehex.exe 文件，该文件在 C:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin 目录下。
- 使用方法：

```c
mergehex.exe -m softdevice.hex app.hex -o without_dfu.hex
mergehex.exe -m without_def.hex bootload.hex firmware.hex
// 文件顺序不能错，应该和文件的启动地址有关。
```

### 16KB RAM 的 Bootload RAM 修改

- APP RAM base address: 0x20000000.
- SoftDevice use RAM: 0x2C00.

|名称|Start|Size|选择|
|---|---|---|---|
|IROM1|0x3C000|0x3C00|Startup|
|IRAM1|0x20002C00|0x1380|None|
|IRMA2|0x20003F80|0x80|NoInit|

### 16KB 使用 DFU 的 Project 设置

- IROM1 is dual bank:132KB / 2 = 66KB = 0x10800 Byte.
- IRAM1 is 16KB - 0x2080 Byte = 0x1F80 Byte.

|名称|Start|Size|选择|
|---|---|---|---|
|IROM1|0x1B000|0x10800|Startup|
|IRAM1|0x20002080|0x1F80|None|

### Bootload 添加看门狗

- 在 bootloader.c 文件的 wait_for_events() 方法添加如下代码：
```c
static void wait_for_events(void)
{
    for (;;)
    {
        // Wait in low power state for any events.
        uint32_t err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
	// add watch dog	
	if ( NRF_WDT->RUNSTATUS & 0x01 )
	{
		NRF_WDT->RR[0] = WDT_RR_RR_Reload;
	}
	// end add
        // Event received. Process it from the scheduler.
        app_sched_execute();

        if ((m_update_status == BOOTLOADER_COMPLETE) ||
            (m_update_status == BOOTLOADER_TIMEOUT)  ||
            (m_update_status == BOOTLOADER_RESET))
        {
		// add watch dog
		if ( NRF_WDT->RUNSTATUS & 0x01 )
		{
			NRF_WDT->RR[0] = WDT_RR_RR_Reload;
		}
		// end add
            	// When update has completed or a timeout/reset occured we will return.
            	return;
        }
    }
}
```

### Bootload 移植注意事项
- 官方例程中定义的按键和 LED 的 GPIO。可以根据需求对 leds_init() 和 buttons_init() 进行修改。

```c
#define BOOTLOADER_BUTTON               BSP_BUTTON_0                                            /**< Button used to enter SW update mode. */
#define UPDATE_IN_PROGRESS_LED          BSP_LED_0                                               /**< Led used to indicate that DFU is active. */

```

- 如果产品使用的不是 16MHz 外部晶振，需要在 Bootload 中进行配置，例如如果使用 32MHz 的外部晶振需要加上该代码：NRF_CLOCK->XTALFREQ = 0; // 32MHz 


### nRF51822 未使用外部 RTC 时钟的情况下，无法正常初始化 SD 时要修改的内容
- NRF_CLOCK_LFCLKSRC 这个宏定义，需要更改，因为默认是使用外部 RTC 时钟的。
- 添加如下宏，并使用：

```c
// This configuration will set RC oscillator, and check for temperature delta every 4 second, and calibrate the RC every 8 second.
#define NRF_CLOCK_LFCLKSRC_2      {.source        = NRF_CLOCK_LF_SRC_RC,            \
                                 .rc_ctiv       = 16,                               \
                                 .rc_temp_ctiv  = 2,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_250_PPM}

```
### SoftDevice RAM size

 0> RAM START ADDR 0x20002080 should be adjusted to 0x20001FE8
 0> RAM SIZE should be adjusted to 0x6018 
 0> sd_ble_enable: RAM START at 0x20002080
 0> sd_ble_enable: app_ram_base should be adjusted to 0x20001FF8

### Keil 里的 RAM 和 Flash 大小计算
https://devzone.nordicsemi.com/tutorials/26/
- RAM usage is: ZI-data + RW-data
- Flash usage is: Code + RO-data + RW-data
- ZI-data: Zero initialized data, data variables set to 0.
- RW-data: Data variables that are different from 0.
- RO-data: Constants placed in flash.

### 开启和关闭 SEGGER_RTT Debug
- define ENABLE_DEBUG_LOG_SUPPORT 
- define NRF_LOG_USES_RTT=1

```c
#if defined(NRF_LOG_USES_RTT) && NRF_LOG_USES_RTT == 0
#define SEGGER_RTT_printf(...)
#elif defined(NRF_LOG_USES_RTT) && NRF_LOG_USES_RTT == 1
#include "SEGGER_RTT.h"
#else

#endif
```

### 如何在超过 DEVICE_MANAGER_MAX_BONDS 时清空 Bond 列表
- 如果超过 DEVICE_MANAGER_MAX_BONDS 后继续连接会导致出错，使设备重启，再不清空 bond 列表的情况下连接仍然会出错，所以这个问题要在 DEVICE_MANAGER_MAX_BONDS 刚好满时会在 device_instance_allocate() 这个方法产生一个 DM_DEVICE_CONTEXT_FULL 错误，这时可以利用这个标志来在这个时候清空 bond 列表。
- SDK11.0 这里没有方便删除的 API 。需要修改 device_manager_peripheral.c 和 device_manager.h 。

```c
// 在 device_manager.h 里添加：
#define DM_EVT_DEVICE_CONTEXT_FULL	   0x17

// 在 device_manager_peripheral.c 里添加：
void dm_ble_evt_handler(ble_evt_t * p_ble_evt)
{
	...
	case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		...
		DM_LOG("[DM]: Security parameter request failed, reason 0x%08X.\r\n", err_code);
		event.event_id = DM_EVT_DEVICE_CONTEXT_FULL;	// 添加这行
		event_result = err_code;
		notify_app   = true;
		
// 在 main.c 里添加：
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
	...
	if(p_event->event_id == DM_EVT_SECURITY_SETUP_COMPLETE)
	{
		...
	}
	else if(p_event->event_id == DM_EVT_DEVICE_CONTEXT_FULL)	// 添加该 else if 分支
	{
		// 断开连接操作
		err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		m_conn_handle = BLE_CONN_HANDLE_INVALID;
		// 清空 bond 列表
		device_manager_init(true);																	
		SEGGER_RTT_printf(0, "DM_EVT_DEVICE_CONTEXT_FULL\n");
	}
```

### BLE 广播分析

- [参考资料](http://blog.chinaunix.net/uid-28852942-id-5176579.html)
- BLE addr LSB Format。
- max 31 Byte, but 2 byte use to length and type, so can use 29 Byte.
- adv_data_encode 方法可以查看到所有的广播数据的编码
- Advertising Address
- Advertising Data
	flags, 16 bit uuids (complete), manufacturer specific data, local name...
	
- [What's the maximum length of a BLE Manufacturer Specific Data AD?](https://stackoverflow.com/questions/33535404/whats-the-maximum-length-of-a-ble-manufacturer-specific-data-ad)	
``` c	
From the spec- Supplement to the Bluetooth Core Specification Version 4 Part A 1.4:

1.4.1 Description

The Manufacturer Specific data type is used for manufacturer specific data. The first two data octets shall contain a company identifier code from the Assigned Numbers - Company Identifiers document. The interpretation of any other octets within the data shall be defined by the manufacturer specified by the company identifier.

1.4.2 Format Data Type <>

Description: Size: 2 or more octets The first 2 octets contain the Company Identifier Code followed by additional manufacturer specific data

Table 1.4: Manufacturer Specific Data Type

So there is no limit except the advertising packet length itself, which is 31 bytes per advertising data and another 31 bytes for scan response.

There is the requirement for the 3 bytes at the beginning of advertising data that have the Flags ad type (required for any non-zero length advertising, see Core V4.0 Volume 3 Part C 11.1.3), reducing your advertising data length by 3.

Then there is the manufacturing ad type flags and length that get added to your manufacturing data, minus another 2 bytes. This leaves you with:

26 bytes in the advertising data or
29 bytes in the scan response
for manufacturing data itself (although two of those bytes should be used for specifying the Company Identifier Code) which would bring it to:

24 bytes of actual data in the advertising data or
27 bytes in the scan response.
```
	
	
### DFU 保存 App Data

- DFU 在升级时默认会擦除掉 App 代码起始地址到 DFU BootLoader 代码起始地址之间的全部区域，就包括了 App Data。
- 避免方法是在 Bootloader 工程中的 dfu_types.h 的一个宏定义中修改即可, 改为 CODE_PAGE_SIZE * 3

``` c
#ifndef DFU_APP_DATA_RESERVED
#define DFU_APP_DATA_RESERVED           CODE_PAGE_SIZE * 0  /**< Size of Application Data that must be preserved between application updates. This value must be a multiple of page size. Page size is 0x400 (1024d) bytes, thus this value must be 0x0000, 0x0400, 0x0800, 0x0C00, 0x1000, etc. */
#endif
```

- 实际情况是，在使用 pstorage 注册了 16x16 Byte，占用 2 个 PSTORAGE_FLASH_PAGE_SIZE（pstorage_register 方法中的 page_count 计算而知），所以要保存 App Data 不被清除，需要定义 CODE_PAGE_SIZE * 3 的空间大小，其中有 1 个 PSTORAGE_FLASH_PAGE_SIZE 大小是 Swap 占用了（可能是）。
- Application 工程的 pstorage_platform.h 的几个参数定义

``` c
#define PSTORAGE_NUM_OF_PAGES       2	// 2 页，根据需要使用的大小来定义
#define PSTORAGE_MAX_APPLICATIONS   1	// 不清楚用途
#define PSTORAGE_MIN_BLOCK_SIZE     0x0010 // 最小字节 16 byte，最大是 1Kbyte。
#define PSTORAGE_MAX_BLOCK_SIZE     PSTORAGE_FLASH_PAGE_SIZE	// 最大是 1Kbyte。
```

### BLE 连接参数

- MIN_CONN_INTERVAL 最小连接间隔
- MAX_CONN_INTERVAL 最大连接间隔
- SLAVE_LATENCY     从机潜伏次数，意思为:"当无有效数据传输的时候，允许从机跳过的连接事件的次数"
- CONN_SUP_TIMEOUT  连接超时时间
- BLE 蓝牙的通讯值通过连接事件来完成的，其中连接事件一直伴随着整个蓝牙连接的周期，不管这其中有没有数据要传输，连接事件一直在周期的产生，这个周期也就决定了蓝牙通讯的速率，周期短，通讯速率就快，相应的功耗就高，连接周期长，通讯速率就慢，功耗就低，用户应该根据自己的需求来设置合理的值，在功耗与速率之间寻求一个平衡。 

 ``` c
 #define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */

 ```
 
 #### iOS 的蓝牙连接规范
 
+ The connection parameter request may be rejected if it does not comply with all of these rules: 
- Interval Max * (Slave Latency + 1) ≤ 2 seconds 
- Interval Min ≥ 20 ms 
- Interval Min + 20 ms ≤ Interval Max 
- Slave Latency ≤ 4 
- connSupervisionTimeout ≤ 6 seconds 
- Interval Max * (Slave Latency + 1) * 3 < connSupervisionTimeout
- 蓝牙在连接之初，采用的是默认的连接参数，从机可以在连接建立之后在向主机发送更新连接参数的请求。可以在 gap_params_init 方法中设置。
- BLE 协议里 InterVal 的范围是 7.5ms - 32s，如果从机请求修改的参数不符合该规范，iOS 会拒绝更新参数。

``` c
/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of 
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(uint16_t min_conn_interval, uint16_t max_conn_interval, uint16_t slave_latency, uint16_t conn_sup_timeout)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = min_conn_interval;
    gap_conn_params.max_conn_interval = max_conn_interval;
    gap_conn_params.slave_latency     = slave_latency;
    gap_conn_params.conn_sup_timeout  = conn_sup_timeout;

    // Set the connection params in stack
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
										  
    // set Radio output power
    err_code = sd_ble_gap_tx_power_set(config_get_ble_tx_mode());
    APP_ERROR_CHECK(err_code);									  
}
```

### BLE 广播时间参数

```c
#define DEVICE_NAME                          "ADV_NAME" 
#define	COMPANY_IDENTIFIER		     0x0201	/* Bluetooth sig company identifier */
#define BLE_UUID_WECHAT_SERVICE		     0xFEE7
#define APP_ADV_INTERVAL                     40         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS           0          /**< The advertising timeout in units of seconds. */
#define MANUF_DATA_LEN                       12

static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advdata_t          advdata;
    
    // Variables used for manufacturer specific data
    ble_advdata_manuf_data_t adv_manuf_data;
    uint8_array_t            adv_manuf_data_array;
    uint8_t                  adv_manuf_data_data[MANUF_DATA_LEN] = {0};
	
    ble_uuid_t adv_uuids[] =
    {
        {BLE_UUID_WECHAT_SERVICE,         BLE_UUID_TYPE_BLE}
    };

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = false;
    advdata.include_ble_device_addr = true;	// advertising include device addr.
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = adv_uuids;
	
    // Configuration of manufacturer specific data
    adv_manuf_data_data[0] = 0x01;
    ... 
    adv_manuf_data_data[11] = 0x0B;
	
    adv_manuf_data_array.p_data = adv_manuf_data_data;
    adv_manuf_data_array.size = sizeof(adv_manuf_data_data);
    
    adv_manuf_data.company_identifier = COMPANY_IDENTIFIER;
    adv_manuf_data.data = adv_manuf_data_array;
    
    advdata.p_manuf_specific_data = &adv_manuf_data;
    // ---------------------------------------------
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}
```

- APP_ADV_INTERVAL 广播的间隔，单位为0.625ms，设置表示广播间隔大，功耗低，反正，相反。
- APP_ADV_TIMEOUT_IN_SECONDS 广播超时时间，单位为s ，这个参数与 ble_advdata_t.flags 这个参数有关系。

#### 不间断广播

- advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE; 并且APP_ADV_TIMEOUT_IN_SECONDS = 0； 这样就广播永远不会停止。
- advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE 模式的时候，表示广播 APP_ADV_TIMEOUT_IN_SECONDS 秒，系统会产生蓝牙超时时间，并且不再广播。
- BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE 有限可发现模式下的最大超时时间是 180 s，定义在 BLE_GAP_ADV_TIMEOUT_LIMITED_MAX 。最小超时时间是 1.8 s，定义在 BLE_GAP_ADV_TIMEOUT_HIGH_DUTY_MAX。


#### 休眠和唤醒

##### 唤醒方式
- LPCOMP 模块产生的 ANADETECT signal。
- 复位
- GPIO 产生 DETECT signal。

``` c

#define GPIO_PIN_CNF_SENSE_Disabled (0x00UL) /*!< Disabled. */
#define GPIO_PIN_CNF_SENSE_High     (0x02UL) /*!< Wakeup on high level. */
#define GPIO_PIN_CNF_SENSE_Low      (0x03UL) /*!< Wakeup on low level. */

// GPIO 产生 DETECT signal 配置方法
// Prepare wakeup GPIO.
uint32_t new_cnf = NRF_GPIO->PIN_CNF[GPIO_PIN];
uint32_t new_sense = GPIO_PIN_CNF_SENSE_Low;	// 唤醒方式
new_cnf &= ~GPIO_PIN_CNF_SENSE_Msk;
new_cnf |= (new_sense << GPIO_PIN_CNF_SENSE_Pos);
NRF_GPIO->PIN_CNF[GPIO_PIN] = new_cnf;

```
#### 休眠
- 调用协议栈的 sd_power_system_off 方法进入休眠。

``` c

// Go to system-off mode (this function will not return; wakeup will cause a reset).
uint32_t err_code = sd_power_system_off();
APP_ERROR_CHECK(err_code);

```

#### UART 错误分析
- 官方代码在串口出现错误时会引起设备重启。
``` c
void uart_event_handle(app_uart_evt_t * p_event)
{
    uint8_t rx_byte = 0;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&rx_byte));
            break;
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);	// 串口出现错误时会引起设备重启
            break;
```
- 一般可能会出现的错误：
``` c

APP_UART_COMMUNICATION_ERROR = p_event->data.error_communication;

> APP_UART_COMMUNICATION_ERROR: 0x01
溢出错误
接收到一个起始位，而前一个数据仍位于RXD中。
（以前的数据丢失。）

> APP_UART_COMMUNICATION_ERROR: 0x02
奇偶校验错误
如果启用了硬件奇偶校验，则接收到奇偶校验错误的字符。

> APP_UART_COMMUNICATION_ERROR: 0x04
Framing error occurred 发生成帧错误
在接收到字符中的所有位后，在串行数据输入中未检测到有效的停止位

> APP_UART_COMMUNICATION_ERROR: 0x08
Break condition
串行数据输入为'0'的时间长于数据帧的长度。 （数据帧长度为10位，不带校验位，11位带校验位）。
``` 

- Framing error occurred 错误分析，能产生该错误的情况：当上位机发送数据，而设备串口正在初始化，设备正好收到了一个不完整的帧，所以产生 Framing error occurred。如果设备串口初始化好前，上位机不发送数据，则不会出现该错误。

#### [Nordic's Secure DFU bootloader](https://devzone.nordicsemi.com/b/blog/posts/getting-started-with-nordics-secure-dfu-bootloader)

##### 0.环境准备

- nRF5_SDK_12 及以上版本
- SDK 11 use: Install version [4.9-2015-q3-update of the GCC compiler toolchain for ARM](https://launchpad.net/gcc-arm-embedded/+download). 
- SDK 15 use: Install version [7-2017-q4-update of the GCC compiler toolchain for ARM](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads#).  
- make 工具 MinGW, install [MSYS](https://sourceforge.net/projects/mingw/files/MSYS/Base/msys-core/msys-1.0.11/MSYS-1.0.11.exe/download) 

##### 1.生成密钥

- Nordic提供 [nRFutil.exe](https://github.com/NordicSemiconductor/pc-nrfutil/releases) 工具来生成这些密钥，直接下载 nRFutil.exe 文件。
- 生成你自己的私钥，生成 DFU.zip 包时使用。
``` c
nrfutil.exe keys generate private.key
```
- 根据私钥生成公钥，编译 bootloader_secure 工程时使用。
``` c
nrfutil.exe keys display --key pk --format code private.key --out_file public_key.c
```

##### 2.构建引导程序

###### 编译 uECC 库，生成 micro_ecc_lib_nrf52.lib, SDK11

- 把 [uECC](https://github.com/kmackay/micro-ecc.git) 库克隆到 SDKFolder\external\micro-ecc\ 目录下。
- 在 SDKFolder\external\micro-ecc\nrf52_keil\armgcc 目录下 make，这里使用的是 nRF52 keil 环境，不同环境在不同目录下 make。

###### 编译 uECC 库，生成 micro_ecc_lib_nrf52.lib, SDK15

- 在 SDKFolder\external\micro-ecc\ 目录下，运行 build_all.bat(windows) 或者 build_all.sh(linux or Mac)。

###### 编译 bootloader_secure 工程

- bootloader_secure 工程里把 dfu_public_key.c 删掉，在工程里添加生成的 public_key.c 文件。然后编译，编译通过。
- 如果使用新的密钥，要重新生成密钥，然后工程里替换新的 public_key.c 文件，并重新编译工程。

##### 3.生成 DFU.zip 包
- 使用 [nRFutil.exe](https://github.com/NordicSemiconductor/pc-nrfutil/releases) 工具生成，命令如下。

``` c
nrfutil pkg generate --hw-version 52 --application-version 1 --application nrf52832_xxaa.hex --sd-req 0x98 --key-file private.key app_dfu_package.zip

bat 脚本

@echo off
set /p package_name=input name.zip for zip package:
nrfutil pkg generate --hw-version 52 --application-version 1 --application nrf52832_xxaa.hex --sd-req 0x98 --key-file private.key %package_name%
pause

```

- [sd-req 版本说明](https://github.com/NordicSemiconductor/pc-nrfutil/blob/master/README.md)

|SoftDevice	|FWID (sd-req)|
|:---:|:---|
|s130_nrf51_1.0.0|0x67|
|s130_nrf51_2.0.0|0x80|
|s132_nrf52_2.0.0|0x81|
|s130_nrf51_2.0.1|0x87|
|s132_nrf52_2.0.1|0x88|
|s132_nrf52_3.0.0|0x8C|
|s132_nrf52_3.1.0|0x91|
|s132_nrf52_4.0.0|0x95|
|s132_nrf52_4.0.2|0x98|
|s132_nrf52_4.0.3|0x99|
|s132_nrf52_4.0.4|0x9E|
|s132_nrf52_4.0.5|0x9F|
|s132_nrf52_5.0.0|0x9D|
|s132_nrf52_5.1.0|0xA5|

#### Enable DCDC
- 9.7 mA peak RX, 8 mA peak TX (0 dBm) with DC/DC
- 硬件使用 DCDC

![](http://ww1.sinaimg.cn/large/6c1ebe8egy1fqxzfvacpoj20hm0eqgnd.jpg)
- 软件使能 DCDC。
``` c
    ...
    ble_stack_init();
	
    if(NRF_SUCCESS != sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE)
    {
    	NRF_LOG_PRINTF("sd_power_dcdc_mode_set failed\r\n");
    }
    ...

```

#### 自定义广播数据中的 MAC 地址
``` c

/**@defgroup BLE_GAP_ADDR_TYPES GAP Address types
 * @{ */
#define BLE_GAP_ADDR_TYPE_PUBLIC                        0x00 /**< Public address. */
#define BLE_GAP_ADDR_TYPE_RANDOM_STATIC                 0x01 /**< Random Static address. */
#define BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE     0x02 /**< Private Resolvable address. */
#define BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE 0x03 /**< Private Non-Resolvable address. */
/**@} */

/**@defgroup BLE_GAP_ADDR_CYCLE_MODES GAP Address cycle modes
 * @{ */
#define BLE_GAP_ADDR_CYCLE_MODE_NONE      0x00 /**< Set addresses directly, no automatic address cycling. */
#define BLE_GAP_ADDR_CYCLE_MODE_AUTO      0x01 /**< Automatically generate and update private addresses. */
/** @} */

static void gap_params_init(void)
{
	uint32_t			err_code;
	ble_gap_addr_t			gap_addr;
	
	gap_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;	// change mac address type is here
	memset(gap_addr.addr, 0, 6);			// change mac address is here
	err_code = sd_ble_gap_address_set(BLE_GAP_ADDR_CYCLE_MODE_NONE, &gap_addr);
	APP_ERROR_CHECK(err_code);
	
	...
}
```

#### Beacon Advertising
##### 广播包格式
![](http://ww1.sinaimg.cn/large/6c1ebe8ely1frd75v1j6gj20t604zjsc.jpg)

- Data Payload 字段是有一个或多个 \[length, type, data\] 数组构成.

##### [Apple’s iBeacon 包格式](http://www.argenox.com/a-ble-advertising-primer/)
![](http://ww1.sinaimg.cn/large/6c1ebe8ely1frd71xhzm1j20ns05w3yx.jpg)

###### Flags Advertising Data Type
This packet has data type 0x01 indicating various flags. The length is 2 because there are two bytes, the data type and the actual flag value. The flag value has several bits indicating the capabilities of the iBeacon:

Bit0 – Indicates LE Limited Discoverable Mode

Bit1 – Indicates LE General Discoverable Mode

Bit 2 – Indicates whether BR/EDR is supported. This is used if your iBeacon is Dual Mode device

Bit3 – Indicates whether LE and BR/EDR Controller operates simultaneously

Bit4 – Indicates whether LE and BR/EDR Host operates simultaneously

Most iBeacons are single mode devices BR/EDR is not used. For iBeacons, General discoverability mode is used.

###### iBeacon Data Type

The most important advertisement data type is the second one. The first byte indicates the number of bytes, 0x1A for a total of 26 bytes, 25 for payload and one for the type. The AD type is the Manufacturer Specific 0xFF, so Apple has defined their own Advertisement Data.

The first two bytes indicate the company identifier 0x4C00. You can see identifiers for other companies as well.

The second two bytes are beacon advertisement indicators. These are always 0x02 and 0x15.

The critical fields are the iBeacon proximity UUID which uniquely identifies the iBeacon followed by a major and minor fields.

Each iBeacon has to have a unique UUID so that an iPhone app can know exactly where it is located relative to one or more iBeacons.

Finally, there is also a 2’s complement of the calibrated TX power that can be used to improve location accuracy knowing the power level of the beacon.

There’s nothing stopping you from creating your own beacons with a different manufacturer format. The problem is that Apple specifically detects iBeacons with the particular format, so there won’t be any interoperability.

## Apple Notification Center Service(ANCS)
#### 基本过程
1. 设备与手机连接后，设备通过 ble_ancs_c_notif_source_notif_enable 方法，使能通知源，此时设备与手机会进行配对；
2. 之后手机收到消息后会通知设备，设备的 BLE_ANCS_C_EVT_NOTIF 事件；
3. 通过 ble_ancs_c_request_attrs 方法，可获取通知的属性内容；
4. 通过 BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE 事件获取属性内容；

#### Detailed Description
Apple Notification Center Service Client Module.

Disclaimer: This client implementation of the Apple Notification Center Service can be changed at any time by Nordic Semiconductor ASA. Server implementations such as the ones found in iOS can be changed at any time by Apple and may cause this client implementation to stop working.

This module implements the Apple Notification Center Service (ANCS) client. This client can be used as a Notification Consumer (NC) that receives data notifications from a Notification Provider (NP). The NP is typically an iOS device acting as a server. For terminology and up-to-date specs, see http://developer.apple.com.

The term "notification" is used in two different meanings:

An iOS notification is the data received from the Notification Provider.
A GATTC notification is a way to transfer data with Bluetooth Smart. In this module, we receive iOS notifications using GATTC notifications. We use the full term (iOS notification or GATTC notification) where required to avoid confusion.
Upon initializing the module, you must add the different iOS notification attributes you would like to receive for iOS notifications. ble_ancs_c_attr_add.

Once a connection is established with a central device, the module does a service discovery to discover the ANVS server handles. If this succeeds (BLE_ANCS_C_EVT_DISCOVERY_COMPLETE) The handles for the CTS server are part of the ble_ancs_c_evt_t structure and must be assigned to a ANCS_C instance using the ble_ancs_c_handles_assign function. For more information about service discovery, see the ble_discovery module documentation Database Discovery Module.

The application can now subscribe to iOS notifications using ble_ancs_c_notif_source_notif_enable. They arrive in the BLE_ANCS_C_EVT_NOTIF event. ble_ancs_c_request_attrs can be used to request attributes for the notifications. They arrive in the BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE event.

![ANCS](http://ww1.sinaimg.cn/large/6c1ebe8ely1g1xihnn9s1j20p00dhq2v.jpg)

