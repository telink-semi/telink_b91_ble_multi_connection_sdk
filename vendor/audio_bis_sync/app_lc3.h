/********************************************************************************************************
 * @file	app_lc3.h
 *
 * @brief	This is the header file for BLE SDK
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
#ifndef APP_LC3_H_
#define APP_LC3_H_


#include "tl_common.h"
#include "app_config.h"
#if (__PROJECT_B91_AUDIO_BIS_SYNC__)

#if (LE_AUDIO_DEMO_ENABLE)


#define APP_LC3_INTERVAL_10MS     0
#define APP_LC3_INTERVAL_7_5MS    1


int app_lc3_init(u8 interval);

int app_lc3_decode(u8 *pRawData, u16 dataLen, u8 *pDstBuff);



#endif

#endif //#if (__PROJECT_B91_AUDIO_BIS_SYNC__)

#endif //APP_LC3_H_
