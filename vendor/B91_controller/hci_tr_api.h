/********************************************************************************************************
 * @file	hci_tr_api.h
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
#ifndef HCI_TR_API_H_
#define HCI_TR_API_H_

#include "common/types.h"
#include "stack/ble/hci/hci.h"
#include "app_buffer.h"

/*! HCI transport layer enable/disable. */
#define HCI_TR_EN            1

/*! HCI transport layer protocol selection. */
#define HCI_TR_H4            0
#define HCI_TR_H5            1
#define HCI_TR_USB           2
#define HCI_TR_MODE          HCI_TR_H4

/*! HCI ACL data packet max size define. */
#define HCI_ACL_BUF_SIZE     (1 + 4 + HCI_RX_FIFO_SIZE)
#define HCI_TR_TX_BUF_SIZE   (1 + 2 + HCI_TX_FIFO_SIZE)

/*! HCI UART transport pin define */
#define HCI_TR_UART_ID       UART0
#define HCI_TR_RX_PIN        UART0_RX_PD3 //UART0_RX_PB3
#define HCI_TR_TX_PIN        UART0_TX_PD2 //UART0_TX_PB2
#define HCI_TR_UART_IRQn     IRQ19_UART0
#define HCI_TR_BAUDRATE      (115200)
#define HCI_UART_IRQHandler  uart0_irq_handler

#define HCI_TIMER_IRQHandler timer0_irq_handler


typedef void (*HciH5PacketHandler_t)(u8 *pPacket, u32 len);

/*! HCI transport layer API. */

/**
 * @brief : HCI transport initialization.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_Init(void);

/**
 * @brief : HCI transport main loop.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_Poll(void);

/**
 * @brief : HCI UART IRQ handler.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_UartIRQHandler(void);


/**
 * @brief : HCI TIMER IRQ handler.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_TimerIRQHandler(void);

/* HCI Rx/Tx handler call-back. This is for compatibility with other codes. */

/**
 * @brief : HCI rx handler call-back.
 * @param : none.
 * @param : none.
 */
int HCI_Tr_RxHandlerCback(void);

/**
 * @brief : HCI tx handler call-back.
 * @param : none.
 * @param : none.
 */
int HCI_Tr_TxHandlerCback(void);


#endif /* HCI_TR_API_H_ */
