### nRF51822 example and note


- a little nRF51822 example

---

#### BMA423_STS3x_Example.c

- BMA423 获取步数和加速度 Example。
- STS3x 获取温度 Example。

#### command_control_main.c

- 上位机发送 0x00 到开发板，开发板每隔 0.5 秒，发送一个递增的 unsigned int 数据，数据从 0 开始，最大值 100。达到最大值后保持不变。 
- 上位机发送 0x01 到开发板，开发板每隔 0.5 秒，发送一个递减的 unsigned int 数据，数据从 100 开始，最小值 0。达到最小值后保持不变。
- 上位机发送其他命令到开发板，开发板停止发送数据。
- 开发板设备名称为“Nordic_WDL”


#### adc_example.md
- adc example

#### ble_app_hrs_main.c
- hrs example, i forget have what function.

#### ble_send_more_data_example.md
- ble send more data example.

#### create_wechat_ble_device_guid.md
- the job is how to create wechat ble device.

#### flash_example.md
- flash example.

#### go_to_bootload_example.md
- in application go to bootload example.

#### gpio_example.md
- gpio example

#### notepad.md
- develop nRF51822 notepad.

#### pedometer.c
- pedometer algorithm, i not test.

#### README.md
- this file.

---
