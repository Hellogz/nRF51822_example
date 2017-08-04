### 应用中跳转到 Bootload 方法

```c

/*

import bootloader_util.c to Project

include paths: ..\..\..\..\..\..\components\libraries\bootloader_dfu

*/

#include "bootloader_util.h"
#include "app_error.h"
#include "nrf_sdm.h"


#define IRQ_ENABLED            0x01 /**< Field that identifies if an interrupt is enabled. */
#define MAX_NUMBER_INTERRUPTS  32   /**< Maximum number of interrupts available. */


static void interrupts_disable(void)
{
    uint32_t interrupt_setting_mask;
    uint32_t irq;

    // Fetch the current interrupt settings.
    interrupt_setting_mask = NVIC->ISER[0];

    // Loop from interrupt 0 for disabling of all interrupts.
    for (irq = 0; irq < MAX_NUMBER_INTERRUPTS; irq++)
    {
        if (interrupt_setting_mask & (IRQ_ENABLED << irq))
        {
            // The interrupt was enabled, hence disable it.
            NVIC_DisableIRQ((IRQn_Type)irq);
        }
    }
}



static void go_to_bootload(void)
{
	uint32_t err_code;
	
    err_code = sd_power_gpregret_set(BOOTLOADER_DFU_START);
    APP_ERROR_CHECK(err_code);

    err_code = sd_softdevice_disable();
    APP_ERROR_CHECK(err_code);

    err_code = sd_softdevice_vector_table_base_set(NRF_UICR->NRFFW[0]);
    APP_ERROR_CHECK(err_code);

    NVIC_ClearPendingIRQ(SWI2_IRQn);
    interrupts_disable();
    bootloader_util_app_start(NRF_UICR->NRFFW[0]);
}

```