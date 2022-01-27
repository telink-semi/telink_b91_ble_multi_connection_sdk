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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "algorithm/lc3/inc/lc3_types.h"
#include "algorithm/lc3/inc/lc3_enc.h"
#include "algorithm/lc3/inc/lc3_dec.h"

#include "app_config.h"
#include "app_audio.h"
#include "app_lc3.h"

#if (LE_AUDIO_DEMO_ENABLE)


u8 gAppLc3CurMode = 0;
LC3_DEC_STRU gAppLc3DecCtrl;

int app_lc3_init(u8 interval)
{
	LC3DEC_Error ret = 0;

	if(interval == APP_LC3_INTERVAL_10MS){
		ret = lc3dec_init(&gAppLc3DecCtrl, 16000, 32000, 1, 0);
	}else if(interval == APP_LC3_INTERVAL_7_5MS){
		ret = lc3dec_init(&gAppLc3DecCtrl, 16000, 32000, 1, 1);
	}else{
		return -1;
	}
	if(ret != LC3DEC_OK){}

	gAppLc3CurMode = interval;

	return 0;
}

int app_lc3_decode(u8 *pRawData, u16 dataLen, u8 *pDstBuff)
{
	LC3DEC_Error ret;

	if(gAppLc3CurMode == 0){
		if(dataLen != 40) return -1;
	}else if(gAppLc3CurMode == 0){
		if(dataLen != 30) return -1;
	}
	
	if(pRawData == NULL) gAppLc3DecCtrl.bec_detect = 1;
	ret = lc3dec_decode_frame(&gAppLc3DecCtrl, pRawData, dataLen, (s16*)pDstBuff);
	if(pRawData == NULL) gAppLc3DecCtrl.bec_detect = 0;
	if(ret != LC3DEC_OK) return-1;

	return 0;
}



#endif
