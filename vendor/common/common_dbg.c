/********************************************************************************************************
 * @file	common_dbg.c
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
#include "tl_common.h"
#include "common_dbg.h"


#if (APP_DUMP_EN)



MYFIFO_INIT_IRAM(print_fifo, 288, 32);



void app_dmup_debug_init(void)
{
	my_usb_init(0x120, &print_fifo);
	usb_set_pin_en ();
}

#endif





#if (UART_LOW_POWER_DEBUG_EN)


#define UART0_DMA_CHANNEL_TX            DMA4
#define UART0_BAUDRATE                  1000000  //115200

int lp_uart_init = 0;  //attention: can not be retention data !!!
void low_power_uart_debug_init(void)
{
#if (UART_LOW_POWER_DEBUG_EN)
	uart0_init(UART0_BAUDRATE);
	uart_set_tx_dma_config(UART0, UART0_DMA_CHANNEL_TX);
	uart_clr_tx_done(UART0);
	dma_clr_irq_mask(UART0_DMA_CHANNEL_TX,TC_MASK|ABT_MASK|ERR_MASK);

	lp_uart_init = 1;
#endif
}

#endif
