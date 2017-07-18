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
    uint32_t 						err_code;
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

### 高效率发送大数据
- 原理：通过在每次数据传输完成后继续传输后面剩余的数据和减少每次传输之间的间隔时间来提高发送效率，这样使得一个连接事件可以发送多个包（最多6个）。
```c
typedef struct blk_send_msg_s
{
	uint32_t start;				// send start offset
	uint32_t max_len;		// the total length of the data to be sent
	uint8_t	*pdata;
} blk_send_msg_t;

blk_send_msg_t	m_send_msg;

uint32_t ble_send(uint8_t *data, uint16_t len)
{
	ble_gatts_hvx_params_t hvx_params;

	memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = m_test.test_handle.value_handle;
    hvx_params.p_data = data;
    hvx_params.p_len  = &len;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(m_test.conn_handle, &hvx_params);
}

uint32_t send_data(void)
{
	uint8_t temp_len;
	uint32_t dif_value;
	uint32_t err_code = NRF_SUCCESS;
	uint8_t *pdata = m_send_msg.pdata;
	uint32_t start = m_send_msg.start;
	uint32_t max_len = m_send_msg.max_len;
	
	do {
		dif_value = max_len - start;
		temp_len = dif_value > 20? 20:dif_value;
		err_code = ble_send(pdata+start, temp_len);
		if(NRF_SUCCESS == err_code)
		{
			start += temp_len;
		}
	} while((NRF_SUCCESS == err_code) && (max_len - start) > 0);
	m_send_msg.start = start;
	
	return err_code;
}

uint32_t ble_send_data(uint8_t *pdata, uint32_t len)
{
	if(NULL == pdata || len <= 0)
	{
		return NRF_ERROR_INVALID_PARAM;
	}
	else
	{
		uint32_t	err_code = NRF_SUCCESS;
		m_send_msg.start = 0;
		m_send_msg.max_len = len;
		m_send_msg.pdata = pdata;
		
		err_code = send_data();
		
		return err_code;
	}
}

uint32_t ble_send_more_data(void)
{
	uint32_t err_code;
	uint32_t dif_value;
	
	dif_value = m_send_msg.max_len - m_send_msg.start;
	if(0 == dif_value || NULL == m_send_msg.pdata)
	{
		return NRF_SUCCESS;
	}
	else
	{
		err_code = send_data();
		
		return err_code;
	}
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
		case BLE_EVT_TX_COMPLETE:	// 添加发送完成处理
			ble_send_more_data();
			break;
		default:
            // No implementation needed.
            break;
    }
}

uint8_t g_data[500];
/*
初始化
for(uint32_t i = 0; i < 500; i++)
{
	g_data[i] = i;
}
*/
uint32_t test_send_more_data(void)
{
	uint32_t err_code = ble_send_data(g_data, 500);
	NRF_LOG_INFO("first send, err_code: %d\r\n", err_code);
	return err_code;
}
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

### 自定义设备名称
```c
#define DEVICE_NAME                     {'N', 'o', 'r', 'd', 'i', 'c', '-'}         /**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME_LEN					15

uint32_t get_device_name_from_mac_addr(void)
{
	ble_gap_conn_sec_mode_t sec_mode;
	unsigned long int		mac_addr;
	char					device_name[DEVICE_NAME_LEN] = DEVICE_NAME;
	
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
- 用命令生成 bin 文件：
```c
# 用 .axf 文件来生成 .bin 文件。
C:\Keil_v5\ARM\ARMCC\bin\fromelf.exe .\_build\skin_ac_dfu.axf --output .\bin\skin_ac_dfu.bin --bin
```
- 用命令生成 zip 文件：
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
nrfutil.exe dfu genpkg ble_skin_v0.1.zip --application skin_ac_dfu.bin --application-version 1--dev-revision 1 --dev-type 1 --sd-req 0x0080

// bat 脚本
@echo off
set /p package_name=input name.zip for zip package:
nrfutil.exe dfu genpkg %package_name% --application filename.bin --application-version 1 --dev-revision 1 --dev-type 1 --sd-req 0x0080
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

|名称|Start|Size|选择|
|---|---|---|---|
|IROM1|0x3C000|0x3C00|Startup|
|IRAM1|0x20002C00|0x1380|None|
|IRMA2|0x20003F80|0x80|NoInit|

### 16KB 使用 DFU 的 Project 设置
|名称|Start|Size|选择|
|---|---|---|---|
|IROM1|0x1B000|0x25000|Startup|
|IRAM1|0x20002080|0x5F80|None|

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

