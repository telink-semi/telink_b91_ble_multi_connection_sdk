/********************************************************************************************************
 * @file	app_buffer.c
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
#include "app_buffer.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"





/********************* ACL connection LinkLayer TX & RX data FIFO allocation, Begin *******************************/

/**
 * @brief	ACL RX buffer. size & number defined in app_buffer.h
 * ACL RX FIFO is shared by all connections to hold LinkLayer RF RX data, user should define this buffer
 * if either ACl connection master role or ACl connection slave role is used.
 */
_attribute_ble_data_retention_	u8	app_acl_rxfifo[ACL_RX_FIFO_SIZE * ACL_RX_FIFO_NUM] = {0};


/**
 * @brief	ACL TX buffer. size & number defined in app_buffer.h
 *  ACL MASTER TX buffer should be defined only when ACl connection master role is used.
 *  ACL SLAVE  TX buffer should be defined only when ACl connection slave role is used.
 */
_attribute_ble_data_retention_	u8	app_acl_mstTxfifo[ACL_MASTER_TX_FIFO_SIZE * ACL_MASTER_TX_FIFO_NUM * MASTER_MAX_NUM] = {0};
_attribute_ble_data_retention_	u8	app_acl_slvTxfifo[ACL_SLAVE_TX_FIFO_SIZE * ACL_SLAVE_TX_FIFO_NUM * SLAVE_MAX_NUM] = {0};

/******************** ACL connection LinkLayer TX & RX data FIFO allocation, End ***********************************/






/***************** ACL connection L2CAP layer MTU TX & RX data FIFO allocation, Begin ********************************/
_attribute_ble_data_retention_	u8 mtu_m_rx_fifo[MASTER_MAX_NUM * MTU_M_BUFF_SIZE_MAX];

_attribute_ble_data_retention_	u8 mtu_s_rx_fifo[SLAVE_MAX_NUM * MTU_S_BUFF_SIZE_MAX];
_attribute_ble_data_retention_	u8 mtu_s_tx_fifo[SLAVE_MAX_NUM * MTU_S_BUFF_SIZE_MAX];
/***************** ACL connection L2CAP layer MTU TX & RX data FIFO allocation, End **********************************/




/************************* CIS connection RX/TX ISO PDU buffer FIFO allocation, Begin ******************************/
/**
 * @brief	CIS RX buffer. size & number defined in app_buffer.h
 * CIS RX FIFO is shared by all connections to hold LinkLayer RF RX ISO data, user should define this buffer
 * if either CIS connection master role or CIS connection slave role is used.
 */
_attribute_ble_data_retention_	u8	app_cis_rxfifo[MAX_RX_ISO_FIFO_SIZE * ISO_RX_FIFO_NUM] = {0};

/**
 * @brief	CIS TX buffer. size & number defined in app_buffer.h
 * CIS TX FIFO is shared by all connections to hold LinkLayer RF TX ISO data, user should define this buffer
 * if either CIS connection master role or CIS connection slave role is used.
 */
_attribute_ble_data_retention_	u8	app_cis_txfifo[MAX_TX_ISO_FIFO_SIZE * ISO_TX_FIFO_NUM * APP_TOTAL_CIS_NUMBER] = {0};

/**
 * @brief	CIS RX evt buffer. size & number defined in app_buffer.h
 * CIS RX EVT FIFO is shared by all connections to hold LinkLayer RF RX ISO data, user should define this buffer
 * if either CIS connection master role or CIS connection slave role is used.
 */
_attribute_ble_data_retention_	u8  app_cis_rxEvtfifo[MAX_RX_ISO_EVT_FIFO_SIZE * ISO_RX_EVT_FIFO_NUM];
/************************* CIS connection RX/TX ISO PDU buffer FIFO allocation, End ********************************/




/************************ CIS connection RX/TX ISOAL SDU buffer FIFO allocation, Begin *****************************/
_attribute_ble_data_retention_	u8	bltSduRxFifo[MAX_RX_SDU_LEN * IAL_SDU_RX_FIFO_NUM * APP_TOTAL_CIS_NUMBER];
_attribute_ble_data_retention_	u8	bltSduTxFifo[MAX_TX_SDU_LEN * IAL_SDU_TX_FIFO_NUM * APP_TOTAL_CIS_NUMBER];
/************************ CIS connection RX/TX ISOAL SDU buffer FIFO allocation, End  ******************************/




/********************************** CIG Slave Global Data Definition, Begin ***************************************/
_attribute_ble_data_retention_	u8  app_cig_slave_param[CIG_SLV_PARAM_LEN * APP_CIG_SLV_NUMBER];
_attribute_ble_data_retention_	u8  app_cis_conn_param[CIS_CONN_PARAM_LENGTH * APP_TOTAL_CIS_NUMBER];
/********************************** CIG Slave Global Data Definition, End  ****************************************/



