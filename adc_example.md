### 使用多个 ADC Channel
- 使用 nrf_drv_adc_channel_enable 函数来添加多个 ADC Channel。
- 使用 nrf_drv_adc_sample_convert 函数来开启单个 ADC Channel 的转换。

```c
#include <stdint.h>
#include <string.h>
#include "bsp.h"
#include "nrf.h"
#include "nrf_drv_adc.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "app_trace.h"

static nrf_drv_adc_channel_t m_battery_adc_config = NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_3); /**< Channel instance. Default configuration used. */
static nrf_adc_value_t       battery_adc_value = 0;

static nrf_drv_adc_channel_t m_motor_adc_config = NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_4); /**< Channel instance. Default configuration used. */
static nrf_adc_value_t       motor_adc_value = 0;

/**
 * @brief ADC interrupt handler.
 */
static void adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
	
}

/**
 * @brief ADC initialization.
 */
void adc_config(void)
{
    ret_code_t ret_code;

    ret_code = nrf_drv_adc_init(NULL, adc_event_handler);
    APP_ERROR_CHECK(ret_code);

    nrf_drv_adc_channel_enable(&m_battery_adc_config);
	nrf_drv_adc_channel_enable(&m_motor_adc_config);
}

void start_battery_adc_convert(void)
{
	nrf_drv_adc_sample_convert(&m_battery_adc_config, &battery_adc_value);
}

void start_motor_adc_convert(void)
{
	nrf_drv_adc_sample_convert(&m_motor_adc_config, &motor_adc_value);
}

nrf_adc_value_t get_motor_adc_value(void)
{
	start_motor_adc_convert();
	app_trace_log("motor_adc_value:%d \r\n", motor_adc_value);
	return motor_adc_value;
}

nrf_adc_value_t get_battery_adc_value(void)
{
	start_battery_adc_convert();
	app_trace_log("battery_adc_value:%d \r\n", motor_adc_value);
	return battery_adc_value;
}

``` 