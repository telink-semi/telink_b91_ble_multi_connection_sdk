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
#include "app_buffer.h"
#include "app_config.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"



#if (__PROJECT_B91_AUDIO_BIS_BCST__)



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




/************************* EXT ADV && PERD ADV buffer FIFO allocation, Begin *******************************/
_attribute_ble_data_retention_	u8 app_advSet_buffer[ADV_SET_PARAM_LENGTH * APP_ADV_SETS_NUMBER];
_attribute_ble_data_retention_	u8 app_advData_buffer[APP_MAX_LENGTH_ADV_DATA * APP_ADV_SETS_NUMBER];
_attribute_ble_data_retention_	u8 app_scanRspData_buffer[APP_MAX_LENGTH_SCAN_RESPONSE_DATA * APP_ADV_SETS_NUMBER];

_attribute_ble_data_retention_	u8 testAdvData[APP_MAX_LENGTH_ADV_DATA];
_attribute_ble_data_retention_  u8 testScanRspData[APP_MAX_LENGTH_SCAN_RESPONSE_DATA];

_attribute_ble_data_retention_	u8 app_perdAdvSet_buffer[PERD_ADV_PARAM_LENGTH * APP_PER_ADV_SETS_NUMBER];
_attribute_ble_data_retention_	u8 app_perdAdvData_buffer[APP_MAX_LENGTH_PER_ADV_DATA * APP_PER_ADV_SETS_NUMBER];
/**************************  EXT ADV && PERD ADV buffer FIFO allocation, End *******************************/




/************************* BIS BCST TX ISO PDU buffer FIFO allocation, Begin ******************************/
/**
 * @brief	BIS BCST TX buffer. size & number defined in app_buffer.h
 */
_attribute_ble_data_retention_	u8	app_bisBcstTxfifo[MAX_TX_ISO_FIFO_SIZE * ISO_TX_FIFO_NUM * APP_BIS_NUM_IN_ALL_BIG_BCST] = {0};

/************************* BIS BCST TX ISO PDU buffer FIFO allocation, End ********************************/



/************************ BIS BCST TX ISOAL SDU buffer FIFO allocation, Begin *****************************/
_attribute_ble_data_retention_	u8	bltSduTxFifo[MAX_TX_SDU_LEN * IAL_SDU_TX_FIFO_NUM * APP_BIS_NUM_IN_ALL_BIG_BCST];
/************************ BIS BCST TX ISOAL SDU buffer FIFO allocation, End  ******************************/




/********************************** BIS BCST Global Data Definition, Begin ***************************************/
_attribute_ble_data_retention_	u8  app_bigBcstParam[BIG_BCST_PARAM_LENGTH * APP_BIG_BCST_NUMBER];
_attribute_ble_data_retention_	u8  app_bisToatlParam[BIS_PARAM_LENGTH * APP_TOTAL_BIS_NUMBER];
/********************************** BIS BCST Global Data Definition, End  ****************************************/



#endif






