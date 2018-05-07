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

#define UMBRELLA_BLOCK						0
#define LOCK_PASSWORD_BLOCK					1
#define AES_KEY_BLOCK						2

void test_flash_block(void);

void init_flash_store(void);
void erase_flash_store(void);

void write_flash(pstorage_size_t block, uint8_t *write, pstorage_size_t length, pstorage_size_t offset);
void read_flash(pstorage_size_t block, uint8_t *read, pstorage_size_t length, pstorage_size_t offset);

#define  M_BLOCK_COUNT      3
#define  M_BLOCK_SIZE       16 

pstorage_handle_t m_flash_handle;
static pstorage_handle_t base_flash_handle;


static void flash_pstorage_cb_handler(pstorage_handle_t * handle,uint8_t op_code,uint32_t result, uint8_t * p_data, uint32_t data_len) 
{ 
	switch(op_code) 
	{ 
		case PSTORAGE_STORE_OP_CODE:	
			if (result == NRF_SUCCESS) 
			{ 
				app_trace_log("PSTORAGE_STORE_OP_CODE Success\r\n");
			} 
			else
			{
				app_trace_log("PSTORAGE_STORE_OP_CODE Failed\r\n");
			}
			break; 
		case PSTORAGE_UPDATE_OP_CODE: 
			if (result == NRF_SUCCESS) 
			{ 
				app_trace_log("PSTORAGE_UPDATE_OP_CODE Success\r\n");
			} 
			else
			{
				app_trace_log("PSTORAGE_UPDATE_OP_CODE Failed\r\n");
			}
			break; 
		case PSTORAGE_LOAD_OP_CODE: 
			if (result == NRF_SUCCESS) 
			{ 
				app_trace_log("PSTORAGE_LOAD_OP_CODE Success\r\n");
			} 
			else
			{
				app_trace_log("PSTORAGE_LOAD_OP_CODE Failed\r\n");
			}
			break; 
		case PSTORAGE_CLEAR_OP_CODE: 
			if (result == NRF_SUCCESS) 
			{ 
				app_trace_log("PSTORAGE_CLEAR_OP_CODE Success\r\n");
			} 
			else
			{
				app_trace_log("PSTORAGE_CLEAR_OP_CODE Failed\r\n");
			}
			break; 
		default: 
				break; 
	} 
} 
void flash_init()
{
	uint32_t retval;
	
	retval = pstorage_init();
	if(retval == NRF_SUCCESS)
	{

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

void write_flash(pstorage_size_t block, uint8_t *write, pstorage_size_t length, pstorage_size_t offset)
{
	update_flash(block, write, length, offset);
}

void read_flash(pstorage_size_t block, uint8_t *read, pstorage_size_t length, pstorage_size_t offset)
{
	load_flash(block, read, length, offset);
	
	NRF_LOG_PRINTF("GET_Flash %d:", length);
	for(uint8_t i = 0; i < length; i++)
	{
		NRF_LOG_PRINTF("%02X ", read[i]);
	}
	NRF_LOG_PRINTF("Done.\r\n");
}

void erase_flash_store(void)
{
	clear_flash();
}

void init_flash_store(void)
{
	flash_init();
	flash_regeist();

	NRF_LOG_PRINTF("Flash Init OK.\r\n");
}

void test_flash_block(void)
{
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
}

/*********************************************************************/
// main.c 中使用 Flash

static uint8_t change_lock_password[16] = {0};
static uint8_t lock_password[16] = {0};

void update_lock_password(void)
{
	write_flash(LOCK_PASSWORD_BLOCK, change_lock_password, 16, 0);
	nrf_delay_ms(500);    // waiting flash operation done.
	memcpy(lock_password, change_lock_password, 16);    // update new password.
	app_trace_log(">>> Update Lock Password Done.\r\n");
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
	load_lock_password();
	for(;;)
	{
		check_event();
		power_manage();
	}
}

```
