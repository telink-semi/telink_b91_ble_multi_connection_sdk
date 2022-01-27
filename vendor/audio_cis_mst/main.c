/********************************************************************************************************
 * @file	main.c
 *
 * @brief	This is the source file for BLE SDK
 *
 * @author	BLE GROUP
 * @date	2020.06
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *******************************************************************************************************/
#include "app.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"



/**
 * @brief		BLE RF interrupt handler.
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void rf_irq_handler(void)
{
	DBG_CHN14_HIGH;

	blc_sdk_irq_handler ();

	DBG_CHN14_LOW;
}

/**
 * @brief		System timer interrupt handler.
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void stimer_irq_handler(void)
{
	DBG_CHN15_HIGH;

	blc_sdk_irq_handler ();
	
	DBG_CHN15_LOW;
}

/**
 * @brief		timer0 interrupt handler.
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void timer0_irq_handler(void)
{
#if (0)//Attention: AES_CCM_Encryption in IRQ, AES_CCM_Decryption in main_loop maybe overlap!!!
	   //At present, the current CIS_CCM_NONCE is maintained before calling AES_CCM_ENC_V2, and
	   //the saved CIS_CCM_NONCE is restored after the encryption is over.
	/* So here you can open the macro for testing. Currently, the method of polling and sending ISO DATA in main_loop is used. */
	if(timer_get_irq_status(TMR_STA_TMR0)){
		app_timer_test_irq_proc();
		timer_clr_irq_status(TMR_STA_TMR0);
	}
#endif
}

/**
 * @brief		This is main function
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ int main(void)
{
	sys_init(DCDC_1P4_DCDC_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6);

	/* detect if MCU is wake_up from deep retention mode */
	int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

	CCLK_32M_HCLK_32M_PCLK_16M;

	rf_drv_ble_init();

	gpio_init(!deepRetWakeUp);

	/* load customized freq_offset cap value. */
	blc_app_loadCustomizedParameters();  

	if( deepRetWakeUp ){ //MCU wake_up from deepSleep retention mode
		user_init_deepRetn ();
	}
	else{ //MCU power_on or wake_up from deepSleep mode
		user_init_normal();
	}


	irq_enable();

	while(1)
	{
		main_loop ();
	}
	return 0;
}
