/********************************************************************************************************
 * @file	audio_csip.h
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
#ifndef _AUDIO_CSIP_H_
#define _AUDIO_CSIP_H_

#if (BLC_AUDIO_CSIP_ENABLE)

#define BLC_AUDIO_CSIP_LOCK_TIMEOUT_DEAFULT     60000000 // 60s


#define BLC_AUDIO_CSIP_LOCK      0x02
#define BLC_AUDIO_CSIP_UNLOCK    0x01


typedef enum{
	BLT_AUDIO_CSIP_FLAG_NONE        = 0x00,
	BLT_AUDIO_CSIP_FLAG_LOCK_TIMER  = 0x02,

	BLT_AUDIO_CSIP_FLAG_NOTY_SIZE   = 0x04,
	BLT_AUDIO_CSIP_FLAG_NOTY_LOCK   = 0x08,
}BLT_AUDIO_CSIP_FLAGS_ENUM;

typedef enum{
	BLT_AUDIO_CSIP_ERRCODE_LOCK_DENIED          = 0x80, //The lock cannot be granted because the server is already locked.
	BLT_AUDIO_CSIP_ERRCODE_RELEASE_NOT_ACLLOWED = 0x81,
	BLT_AUDIO_CSIP_ERRCODE_INVALID_LOCK_VALUE   = 0x82,
	BLT_AUDIO_CSIP_ERRCODE_OOB_SIRK_ONLY        = 0x83,
	BLT_AUDIO_CSIP_ERRCODE_LOCK_ALREADY_GRANTED = 0x84, //The client that made the request is the current owner of thelock.
}BLT_AUDIO_CSIP_ERRCODE_ENUM;


//readyEvt.size = pCsip->size;
//	readyEvt.lock = pCsip->lock;
//	readyEvt.rank = pCsip->rank;

typedef enum{
	BLT_AUDIO_CSIP_SDP_FLAG_NONE  = 0x00,
	BLT_AUDIO_CSIP_SDP_FLAG_FIRST = 0x80,
	BLT_AUDIO_CSIP_SDP_FLAG_SIRK  = 0x01,
	BLT_AUDIO_CSIP_SDP_FLAG_SIZE  = 0x02,
	BLT_AUDIO_CSIP_SDP_FLAG_RANK  = 0x04,
	BLT_AUDIO_CSIP_SDP_FLAG_LOCK  = 0x08,
}BLT_AUDIO_CSIP_SDP_FLAGS_ENUM;


int blc_audio_csipAttRead(u16 connHandle, void* p);
int blc_audio_csipAttWrite(u16 connHandle, void* p);

int blc_audio_csipSetLock(u16 connHandle, bool isLock);

void blc_audio_csisSih(u8 sirk[16], u8 prand[3], u8 hash[3]);
void blc_audio_csisGeneratePsri(u8 sirk[16], u8 prand[3], u8 psri[6]);
bool blc_audio_csisPsriIsValid(u8 sirk[16], u8 psri[6]);



#endif //#if (BLC_AUDIO_CSIP_ENABLE)

#endif //_AUDIO_CSIP_H_

