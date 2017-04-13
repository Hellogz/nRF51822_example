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
- 

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
- Scanner 扫描状态
- Initiator 发起连接状态
- Master 主设备
- Slave 从设备

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
