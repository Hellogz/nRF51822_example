### 协议栈中使用 Flash
- pstorage_init 函数初始化
- pstorage_register 申请 Flash 块。
- pstorage_clear 擦除 Flash 块。
- pstorage_store 存储 Flash 块。
- pstorage_update 更新 Flash 块。
- pstorage_load 获取 Flash 块。

#### 遇到过的问题：
- 可参考下面代码例程。
- Q：写入 Flash 的数据与读取 Flash 的数据不一致？
- A：写入的 Flash 数据必须为 static 的。写入 Flash 过程中会拷贝数据，如果是临时数据则会出现该问题。
- Q：写入 Flash 后马上读取 Flash，导致设备复位？
- A：可以通过设置标志在 main 函数中来进行 Flash 的操作，尽量不要在数据处理函数中直接操作 Flash，因为这样操作可能会出错，导致设备复位。
- Q：DFU 升级固件后 Flash 存储的数据被擦除了，怎么办？
- A：根据 APP 里使用的 Flash 大小来修改 在 Bootloader 工程中的 dfu_types.h 的一个宏定义中修改 CODE_PAGE_SIZE 即可，注意 Swap 会占用一个 PSTORAGE_FLASH_PAGE_SIZE 大小。详细说明看 [notpad.md](https://github.com/Hellogz/nRF51822_example/blob/master/notepad.md) 里的 DFU 保存 App Data 章节。

```c
#include "ble_flash.h"

#define  M_BLOCK_COUNT      16
#define  M_BLOCK_SIZE       16 

APP_TIMER_DEF(m_wait_operate_done_timer_id); 
pstorage_handle_t m_flash_handle;
static pstorage_handle_t base_flash_handle;
static volatile bool operate_flash_done = false;
static volatile bool wait_operate_done_timeout = false;

static uint32_t wait_operate_done_timer_start(uint32_t timeout)
{
	uint32_t err_code = 0;
	
	wait_operate_done_timeout = false;
	err_code = app_timer_start(m_wait_operate_done_timer_id, APP_TIMER_TICKS(timeout, 0), NULL);
	if(err_code != 0) { NRF_LOG_PRINTF("wait_operate_done_timer_start:%d, %d\r\n", timeout, err_code); }
	return err_code;
}

static uint32_t wait_operate_done_timer_stop(void)
{
	return app_timer_stop(m_wait_operate_done_timer_id);
}

static void wait_operate_done_timer_event(void * p_context)
{
	UNUSED_PARAMETER(p_context);

	wait_operate_done_timeout = true;
}

void wait_operate_done_timer_init(void)
{
	uint32_t err_code;
	
	err_code = app_timer_create(&m_wait_operate_done_timer_id, APP_TIMER_MODE_SINGLE_SHOT, wait_operate_done_timer_event);
	APP_ERROR_CHECK(err_code); 
}

static void flash_pstorage_cb_handler(pstorage_handle_t * handle,uint8_t op_code,uint32_t result, uint8_t * p_data, uint32_t data_len) 
{ 
	switch(op_code) 
	{ 
		case PSTORAGE_STORE_OP_CODE:	
			if (result == NRF_SUCCESS) 
			{ 
				operate_flash_done = true;
			} 
			else
			{
				operate_flash_done = false;
				NRF_LOG_PRINTF("PSTORAGE_STORE_OP_CODE Failed\r\n");
			}
			break; 
		case PSTORAGE_UPDATE_OP_CODE: 
			if (result == NRF_SUCCESS) 
			{ 
				operate_flash_done = true;
			} 
			else
			{
				operate_flash_done = false;
				NRF_LOG_PRINTF("PSTORAGE_UPDATE_OP_CODE Failed\r\n");
			}
			break; 
		case PSTORAGE_LOAD_OP_CODE: 
			if (result == NRF_SUCCESS) 
			{ 
				operate_flash_done = true;
			} 
			else
			{
				operate_flash_done = false;
				NRF_LOG_PRINTF("PSTORAGE_LOAD_OP_CODE Failed\r\n");
			}
			break; 
		case PSTORAGE_CLEAR_OP_CODE: 
			if (result == NRF_SUCCESS) 
			{ 
				operate_flash_done = true;
			} 
			else
			{
				operate_flash_done = false;
				NRF_LOG_PRINTF("PSTORAGE_CLEAR_OP_CODE Failed\r\n");
			}
			break; 
		default: 
				break; 
	} 
} 
void flash_init()
{
	uint32_t retval;
	
	wait_operate_done_timer_init();
	retval = pstorage_init();
	if(retval == NRF_SUCCESS)
	{
		NRF_LOG_PRINTF("Flash Init OK.\r\n");
	}
	else
	{

	}
}
void flash_regeist()
{
	pstorage_module_param_t param;
	uint32_t retval;
	
	param.block_size = M_BLOCK_SIZE;
	param.block_count = M_BLOCK_COUNT;
	param.cb = flash_pstorage_cb_handler;
	retval = pstorage_register(&param, &base_flash_handle);
	if (retval == NRF_SUCCESS)
	{
		;// Registration successful.
		NRF_LOG_PRINTF("Flash Registration successful.\r\n");
	}
	else
	{
		;// Failed to register, take corrective action.
	}

}
void clear_flash(void) 
{ 
	uint32_t err_code; 
	
	err_code = pstorage_clear(&base_flash_handle, M_BLOCK_SIZE*M_BLOCK_COUNT); 
	APP_ERROR_CHECK(err_code); 
} 

void storage_flash(pstorage_size_t block_num, uint8_t *p_source_data, pstorage_size_t dat_length, pstorage_size_t offset ) 
{ 
	uint32_t err_code; 
	
	err_code = pstorage_block_identifier_get(&base_flash_handle, block_num, &m_flash_handle);
	APP_ERROR_CHECK(err_code);
	err_code = pstorage_store(&m_flash_handle, p_source_data, dat_length, offset); 
	APP_ERROR_CHECK(err_code); 
} 
void update_flash(pstorage_size_t block_num, uint8_t *p_source_data, pstorage_size_t dat_length, pstorage_size_t offset ) 
{ 
	uint32_t err_code; 
	
	err_code = pstorage_block_identifier_get(&base_flash_handle, block_num, &m_flash_handle);
	APP_ERROR_CHECK(err_code); 
	err_code = pstorage_update(&m_flash_handle, p_source_data, dat_length, offset); 
	APP_ERROR_CHECK(err_code); 
} 

void load_flash(pstorage_size_t block_num, uint8_t * p_source_data, pstorage_size_t dat_length, pstorage_size_t offset) 
{ 
	uint32_t err_code; 
	
	err_code = pstorage_block_identifier_get(&base_flash_handle, block_num, &m_flash_handle); 
	APP_ERROR_CHECK(err_code); 
	err_code = pstorage_load(p_source_data, &m_flash_handle, dat_length, offset); 
	APP_ERROR_CHECK(err_code); 
} 

uint8_t write_flash(pstorage_size_t block, uint8_t *write, pstorage_size_t length, pstorage_size_t offset)
{
	operate_flash_done = false;
	
	update_flash(block, write, length, offset);
	
	wait_operate_done_timer_start(1000);
	
	while(wait_operate_done_timeout == false)
	{
		if(true == operate_flash_done)
		{
			operate_flash_done = false;
			wait_operate_done_timer_stop();
			break;
		}
	}
	
	if(wait_operate_done_timeout == true)
	{
		return FLASH_FAIL;
	}
	else
	{
		return FLASH_SUCCESS;
	}
}

uint8_t read_flash(pstorage_size_t block, uint8_t *read, pstorage_size_t length, pstorage_size_t offset)
{
	operate_flash_done = false;
	
	load_flash(block, read, length, offset);
	
#if 0
	NRF_LOG_PRINTF("GET %d Block->%d:", length);
	for(uint8_t i = 0; i < length; i++)
	{
		NRF_LOG_PRINTF("%02X ", read[i]);
	}
	NRF_LOG_PRINTF("Done.\r\n");
#endif
	
	wait_operate_done_timer_start(500);
	
	while(wait_operate_done_timeout == false)
	{
		if(true == operate_flash_done)
		{
			operate_flash_done = false;
			wait_operate_done_timer_stop();
			break;
		}
	}
	
	if(wait_operate_done_timeout == true)
	{
		return FLASH_FAIL;
	}
	else
	{
		return FLASH_SUCCESS;
	}	
}

void erase_flash_store(void)
{
	clear_flash();
}

void init_flash_store(void)
{
	flash_init();
	flash_regeist();
}

void test_flash_block(void)
{
#if 0
	static uint8_t block_1[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
	static uint8_t block_2[16] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
	static uint8_t block_3[16] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F};
	static uint8_t buffer[16*3] = {0};
	
	write_flash(UMBRELLA_BLOCK, block_1, 16, 0);
	nrf_delay_ms(500);
	write_flash(LOCK_PASSWORD_BLOCK, block_2, 16, 0);
	nrf_delay_ms(500);
	write_flash(AES_KEY_BLOCK, block_3, 16, 0);
	nrf_delay_ms(500);
	
	read_flash(UMBRELLA_BLOCK, buffer, 16, 0);
	read_flash(LOCK_PASSWORD_BLOCK, buffer+16, 16, 0);
	read_flash(AES_KEY_BLOCK, buffer+32, 16, 0);
#else
	uint8_t config[16*16] = {0};
	static uint8_t block[16] = {0};
	
	for(uint8_t i = 0; i < M_BLOCK_COUNT; i++)
	{
		memset(block, i, M_BLOCK_SIZE);
		write_flash(i, config+(i*M_BLOCK_SIZE), M_BLOCK_SIZE, 0);
		nrf_delay_ms(100);
	}
	
	for(uint8_t i = 0; i < M_BLOCK_COUNT; i++)
	{
		read_flash(i, block, M_BLOCK_SIZE, 0);
	}
	
#endif
}

/* .H FILES */
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "nrf_log.h"
#include "pstorage.h"
#include "nrf_delay.h"
#include "app_timer.h"

#define FLASH_SUCCESS						0
#define FLASH_FAIL						1

void test_flash_block(void);

void init_flash_store(void);
void erase_flash_store(void);

uint8_t write_flash(pstorage_size_t block, uint8_t *write, pstorage_size_t length, pstorage_size_t offset);
uint8_t read_flash(pstorage_size_t block, uint8_t *read, pstorage_size_t length, pstorage_size_t offset);

/*********************************************************************/
// main.c 中使用 Flash

static uint8_t change_lock_password[16] = {0};
static uint8_t lock_password[16] = {0};

void update_lock_password(void)
{
	write_flash(LOCK_PASSWORD_BLOCK, change_lock_password, 16, 0);
	nrf_delay_ms(500);    // waiting flash operation done.
	memcpy(lock_password, change_lock_password, 16);    // update new password.
	NRF_LOG_PRINTF(">>> Update Lock Password Done.\r\n");
}

void load_lock_password(void)
{
	uint8_t temp[6] = {0};
	
	memset(temp, 0xFF, 6);
	
	read_flash(LOCK_PASSWORD_BLOCK, change_lock_password, 6, 0);

	if(memcmp(change_lock_password, temp, 6) != 0)
	{
		memcpy(lock_password, change_lock_password, 6);
	}
	
	printf_data(">>> Load Lock Password: ", lock_password, 6);
}

void check_event(void)
{
	if(get_update_lock_password_flag())
	{
		set_update_lock_password_flag(false);
		update_lock_password();
	}
}

int main(void)
{
	...
	init_flash_store();
	load_lock_password();
	for(;;)
	{
		check_event();
		power_manage();
	}
}

```
