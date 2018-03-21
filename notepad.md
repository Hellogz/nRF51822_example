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

### 不间断广播
```c
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advdata_t          advdata;
    ble_adv_modes_config_t options;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}
```
- advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE; 并且APP_ADV_TIMEOUT_IN_SECONDS = 0； 这样就广播永远不会停止。
- advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE模式的时候，表示广播 APP_ADV_TIMEOUT_IN_SECONDS 秒，系统会产生蓝牙超时时间，并且不再广播。

### BLE 广播分析

- [参考资料](http://blog.chinaunix.net/uid-28852942-id-5176579.html)
- BLE addr LSB Format。
- max 31 Byte, but 2 byte use to length and type, so can use 29 Byte.
- adv_data_encode 方法可以查看到所有的广播数据的编码
- Advertising Address
- Advertising Data
	flags, 16 bit uuids (complete), manufacturer specific data, local name...
	
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

