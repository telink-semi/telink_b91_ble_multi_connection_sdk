/********************************************************************************************************
 * @file	app_audio.c
 *
 * @brief	This is the source file for BLE SDK
 *
 * @author	BLE GROUP
 * @date	2021.01
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
#include "app_audio.h"
#include "app_config.h"
#include "app_lc3.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "algorithm/lc3/inc/lc3_types.h"
#include "algorithm/lc3/inc/lc3_enc.h"
#include "algorithm/lc3/inc/lc3_dec.h"



#if (__PROJECT_B91_AUDIO_BIS_BCST__)
#if (LE_AUDIO_DEMO_ENABLE)


u8 gAppLc3CurMode = 0;
LC3_ENC_STRU gAppLc3EncCtrl;


int app_lc3_init(u8 interval)
{
	LC3ENC_Error ret = 0;

	if(interval == APP_LC3_INTERVAL_10MS){
		ret = lc3enc_init(&gAppLc3EncCtrl, 16000, 1, 32000, 0);
	}else if(interval == APP_LC3_INTERVAL_7_5MS){
		ret = lc3enc_init(&gAppLc3EncCtrl, 16000, 1, 32000, 1);
	}else{
		return -1;
	}
	if(ret != LC3ENC_OK){}

	gAppLc3CurMode = interval;
	
	return 0;
}


int app_lc3_encode(u8 *pRawData, u16 dataLen, u8 *pDstBuff)
{
	LC3ENC_Error ret;

	if(gAppLc3CurMode == 0){
		if(dataLen != 320) return -1;
	}else if(gAppLc3CurMode == 0){
		if(dataLen != 240) return -1;
	}

	ret = lc3enc_encode_frame(&gAppLc3EncCtrl, (s16*)pRawData, pDstBuff);
	if(ret != LC3ENC_OK) return-1;

	return 0;
}



#endif //#if (LE_AUDIO_DEMO_ENABLE)
#endif //#if (__PROJECT_B91_AUDIO_BIS_BCST__)

