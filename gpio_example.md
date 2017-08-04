### 使用 GPIO

- PIN_NAME：GPIO 管脚号。
- PIN_MODE：GPIO 模式配置。

#### 输入
|函数|说明|
|:---|:---:|
|nrf_gpio_cfg_input(PIN_NAME, PIN_MODE);|输入配置|
|nrf_gpio_pin_read(DOOR_SWITCH_PIN);|读输入|


|PIN_MODE|说明|
|:---|:---:|
|NRF_GPIO_PIN_NOPULL|悬空|
|NRF_GPIO_PIN_PULLDOWN|下拉|
|NRF_GPIO_PIN_PULLUP|上拉|

#### 输出
|函数|说明|
|:---|:---:|
|nrf_gpio_cfg_output(PIN_NAME);|配置输出|
|nrf_gpio_pin_clear(PIN_NAME);|输出低电平|
|nrf_gpio_pin_set(PIN_NAME);|输出高电平|



#### 按键检测应用（单击、双击、长按）
```c
// button_driver start
#include <stdint.h>
#include <string.h>

#define key_input

#define N_KEY			0
#define S_KEY			1
#define D_KEY			2
#define L_KEY			3

#define KEY_STATE_0		0
#define KEY_STATE_1		1
#define KEY_STATE_2		2
#define KEY_STATE_3		3

uint8_t key_read(uint8_t key_press);

uint8_t key_driver(uint8_t key_press)
{
	static uint8_t key_state = KEY_STATE_0, key_time = 0;
	uint8_t key_return = N_KEY;

	switch(key_state)
	{
		case KEY_STATE_0:
			if(!key_press)
			{
				key_state = KEY_STATE_1;
			}
			break;
		case KEY_STATE_1:
			if(!key_press)
			{
				key_time = 0;
				key_state = KEY_STATE_2;
			}
			else
			{
				key_state = KEY_STATE_0;
			}
			break;
		case KEY_STATE_2:
			if(key_press)
			{
				key_return = S_KEY;
				key_state = KEY_STATE_0;
			}
			else if(++key_time >= 200) // long key timeout.
			{
				key_return = L_KEY;
				key_state = KEY_STATE_3;
			}
			else
			{
			
			}
			break;
		case KEY_STATE_3:
			if(key_press)
			{
				key_state = KEY_STATE_0;
			}
			break;
	}
	return key_return;
}

uint8_t key_read(uint8_t key_press)
{
	static uint8_t key_m = KEY_STATE_0, key_time_1 = 0;
	uint8_t key_return = N_KEY, key_temp;
	
	key_temp = key_driver(key_press);
	
	switch(key_m)
	{
		case KEY_STATE_0:
			if(key_temp == S_KEY)
			{
				key_time_1 = 0;
				key_m = KEY_STATE_1;
			}
			else
			{
				key_return = key_temp;
			}
			break;
		case KEY_STATE_1:
			if(key_temp == S_KEY)
			{
				key_return = D_KEY;
				key_m = KEY_STATE_0;
			}
			else
			{
				if(++key_time_1 >= 50)
				{
					key_return = S_KEY;
					key_m = KEY_STATE_0;
				}
			}
			break;
	}
	return key_return;
}
// button_driver end

// application layer start

#include <stdint.h>
#include <string.h>
#include "bsp.h"
#include "nrf.h"
#include "app_trace.h"

#define DOOR_SWITCH_PIN				9

void door_switch_init(void);

/*
把 door_switch_event 放在 10 ms 的定时器事件中。
*/
void door_switch_event(void);


void door_switch_init(void)
{
	nrf_gpio_cfg_input(DOOR_SWITCH_PIN, NRF_GPIO_PIN_PULLDOWN); 
}

void door_switch_event(void)
{
	uint8_t key_press = 0, key;

	key_press = (uint8_t)nrf_gpio_pin_read(DOOR_SWITCH_PIN);
	
	key = key_read(key_press);
	
	if(S_KEY == key)
	{
		app_trace_log(">>> Single Key\r\n");
	}
	else if(D_KEY == key)
	{
		app_trace_log(">>> Double Key\r\n");
	}
	else if(L_KEY == key)
	{
		app_trace_log(">>> Long Key\r\n");
	}
}
// application layer end
```