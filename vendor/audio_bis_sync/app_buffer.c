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



#if (__PROJECT_B91_AUDIO_BIS_SYNC__)



/************************* BIS SYNC RX ISO PDU buffer FIFO allocation, Begin ******************************/
/**
 * @brief	BIS SYNC RX buffer. size & number defined in app_buffer.h
 */
_attribute_ble_data_retention_	u8	app_bisSyncRxfifo[MAX_RX_ISO_FIFO_SIZE * ISO_RX_FIFO_NUM * APP_BIS_NUM_IN_ALL_BIG_SYNC] = {0};

/************************* BIS SYNC RX ISO PDU buffer FIFO allocation, End ********************************/



/************************ BIS SYNC RX ISOAL SDU buffer FIFO allocation, Begin *****************************/
_attribute_ble_data_retention_	u8	bltSduRxFifo[MAX_RX_SDU_LEN * IAL_SDU_RX_FIFO_NUM * APP_BIS_NUM_IN_ALL_BIG_SYNC];
/************************ BIS SYNC RX ISOAL SDU buffer FIFO allocation, End  ******************************/




/********************************** BIS BSYNC Global Data Definition, Begin ***************************************/
_attribute_ble_data_retention_	u8  app_bigSyncParam[BIG_SYNC_PARAM_LENGTH * APP_BIG_SYNC_NUMBER];
_attribute_ble_data_retention_	u8  app_bisToatlParam[BIS_PARAM_LENGTH * APP_TOTAL_BIS_NUMBER];
/********************************** BIS BSYNC Global Data Definition, End  ****************************************/



#endif






