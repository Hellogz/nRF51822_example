#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "app_pwm.h"

/*
nRF_Drivers include nrf_drv_timer.c and nrf_drv_ppi.c
nRF_Libraries include app_pwm.c
nrf_drv_config.h enable TIMER1_ENABLED.
*/

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

static volatile bool ready_flag;            // A flag indicating PWM status.

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}

void led_pwm_config(void)
{
	ret_code_t err_code;
	
	/* 1-channel PWM, 1000Hz, output on LED pins. */
  app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(1000L, BSP_LED_0);
    
  /* Initialize and enable PWM. */
  err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback);
  APP_ERROR_CHECK(err_code);
  app_pwm_enable(&PWM1);
}

// Fast Led off to high light to off.
void led_fast_blink(void)
{
	uint32_t value;
	nrf_gpio_cfg_default(BSP_LED_0);
	for (uint8_t i = 0; i < 40; ++i)
	{
		value = (i < 20) ? (i * 5) : (100 - (i - 20) * 5);        // GPIO Out Low LED Light
    // value = (i < 20) ? (100 - (i*5)) : ((i - 20) * 5);        // GPIO Out High LED Light
		
		ready_flag = false;
		/* Set the duty cycle - keep trying until PWM is ready... */
		while (app_pwm_channel_duty_set(&PWM1, 0, value) == NRF_ERROR_BUSY);

		nrf_delay_ms(5);
	}
	while (app_pwm_channel_duty_set(&PWM1, 0, 0) == NRF_ERROR_BUSY);
	nrf_gpio_cfg_default(BSP_LED_0);
}

// Slow Led off to high light to off.
void led_slow_blink(void)
{
	uint32_t value;
	nrf_gpio_cfg_default(BSP_LED_0);
	for (uint8_t i = 0; i < 40; ++i)
	{
		value = (i < 20) ? (i * 5) : (100 - (i - 20) * 5);        // GPIO Out Low LED Light
    // value = (i < 20) ? (100 - (i*5)) : ((i - 20) * 5);        // GPIO Out High LED Light
		
		ready_flag = false;
		/* Set the duty cycle - keep trying until PWM is ready... */
		while (app_pwm_channel_duty_set(&PWM1, 0, value) == NRF_ERROR_BUSY);

		nrf_delay_ms(25);
	}
	while (app_pwm_channel_duty_set(&PWM1, 0, 0) == NRF_ERROR_BUSY);
	nrf_gpio_cfg_default(BSP_LED_0);
}

void test_led(void)
{
  led_pwm_config();
	
  while(true)
  {
    for (uint8_t i = 0; i < 3; ++i)
    {
      led_fast_blink();
    }
		
    nrf_delay_ms(1000);
		
    for (uint8_t i = 0; i < 3; ++i)
    {
      led_slow_blink();
    }
        
    nrf_delay_ms(1000);		
  }
}
