/********************************************************************************************************
 * @file	app_buffer.h
 *
 * @brief	This is the header file for BLE SDK
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
#ifndef APP_BUFFER_H_
#define APP_BUFFER_H_

#include "tl_common.h"


#if (__PROJECT_B91_AUDIO_BIS_SYNC__)



/******************************** BIS SYNC RX ISO PDU buffer FIFO allocation, Begin **********************************/
/**
 * @brief	bisMaxRxPDU is the maximum number of data octets that can be carried in a BIS Data PDU
 * refer to BLE SPEC "2.6 ISOCHRONOUS PHYSICAL CHANNEL PDU"
 * usage limitation:
 * 1. should be in range of 0 ~ 251
 */
#define		BIS_SYNC_MAX_RX_PDU			44 //LC3 10ms-> 40B, mic 4B plus, 44B enough

/**
 * @brief	BIS SYNC RX buffer size & number
 *  		BIS SYNC RX buffer is shared by all bis_sync to hold LinkLayer RF TX BIS data.
 * usage limitation for MAX_RX_ISO_FIFO_SIZE:
 * 1. should be greater than or equal to XXX, TODO
 * 2. should be be an integer multiple of 4 (4 Byte align)
 * 3. user can use formula:  size = BIS_PDU_ALLIGN4_RXBUFF(bisMaxRxPDU)
 * usage limitation for ISO_RX_FIFO_NUM:
 * 1. must be: 2^n, (power of 2)
 * 2. at least 4; recommended value: 8, 16
 */
#define		MAX_RX_ISO_FIFO_SIZE		BIS_PDU_ALLIGN4_RXBUFF(BIS_SYNC_MAX_RX_PDU)
#define		ISO_RX_FIFO_NUM				(32)


extern	u8	app_bisSyncRxfifo[];
/******************************* BIS SYNC RX ISO PDU buffer FIFO allocation, End *************************************/




/*************************** BIS SYNC RX ISOAL SDU buffer FIFO allocation, Begin *************************************/
#define		MAX_RX_SDU_LEN				IAL_SDU_ALLIGN4_BUFF(48) //LC3 10ms-> 40B, iso hdr 8B plus, 48B enough

#define		IAL_SDU_RX_FIFO_NUM			(32)

extern	u8	bltSduRxFifo[];
/***************************** BIS SYNC RX ISOAL SDU buffer FIFO allocation, End  ************************************/




/********************************** BIS BYNC Global Data Definition, Begin *****************************************/
#define		APP_BIG_BCST_NUMBER			0	// Number of Supported BIG_BCST number (must <= LL_BIG_BCST_NUM_MAX)
#define		APP_BIG_SYNC_NUMBER			1	// Number of Supported BIG_SYNC number (must <= LL_BIG_SYNC_NUM_MAX)
#define		APP_BIS_NUM_IN_PER_BIG_BCST	2	// Number of Supported BISes in Per BIG_BCST (must <= LL_BIS_IN_PER_BIG_BCST_NUM_MAX)
#define		APP_BIS_NUM_IN_PER_BIG_SYNC	2	// Number of Supported BISes in Per BIG_SYNC (must <= LL_BIS_IN_PER_BIG_SYNC_NUM_MAX)

// The total number of BIS supported by BIG_BCST
#define		APP_BIS_NUM_IN_ALL_BIG_BCST	(APP_BIG_BCST_NUMBER * APP_BIS_NUM_IN_PER_BIG_BCST)
// The total number of BIS supported by BIG_SYNC
#define		APP_BIS_NUM_IN_ALL_BIG_SYNC	(APP_BIG_SYNC_NUMBER * APP_BIS_NUM_IN_PER_BIG_SYNC)
// The total number of BIS supported by BIG (BCST and SYNC)
#define		APP_TOTAL_BIS_NUMBER		(APP_BIS_NUM_IN_ALL_BIG_BCST + APP_BIS_NUM_IN_ALL_BIG_SYNC)

extern	u8	app_bigSyncParam[];
extern	u8  app_bisToatlParam[];
/********************************** BIS SYNC Global Data Definition, end  *****************************************/



#endif /* APP_BUFFER_H_ */

#endif
