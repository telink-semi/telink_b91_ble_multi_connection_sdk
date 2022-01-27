/********************************************************************************************************
 * @file	hci_tr_h5.h
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
#ifndef __HCI_TR_H5_H
#define __HCI_TR_H5_H

#include "hci_tr_api.h"

/*! H5 transport buffer config macro. */
#define HCI_H5_TR_RX_BUF_SIZE    MEM_SIZE_ALIGN16(2 + (HCI_H5_HEAD_LEN + HCI_ACL_BUF_SIZE + HCI_H5_CRC_LEN) * 2 * 2)
#define HCI_H5_TR_RX_BUF_NUM     4


/**
 * @brief : H5 transport initializaiton.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5Init(void);

/**
 * @brief : Register slip handler
 * @param : func    Pointer point to handler function.
 * @param : none.
 */
void HCI_Tr_H5RegisterSlipHandler(HciH5PacketHandler_t func);

/**
 * @brief : H5 transport rx handler.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5RxHandler(void);

/**
 * @brief : H5 transport IRQ handler.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5UartIRQHandler(void);

/**
 * @brief : H5 Timer IRQ handler.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5TimerIRQHandler(void);

#endif

