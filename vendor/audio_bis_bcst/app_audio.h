/********************************************************************************************************
 * @file	app_audio.h
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
#ifndef APP_AUDIO_H_
#define APP_AUDIO_H_

#include "tl_common.h"
#include "app_config.h"

#if (__PROJECT_B91_AUDIO_BIS_BCST__)
#if (LE_AUDIO_DEMO_ENABLE)


int  app_audio_init(void);
void app_audio_deinit(void);
int  app_audio_enable(bool enable);
void app_audio_handler(void);
void app_audio_send_handler(void);


#endif
#endif //#if (__PROJECT_B91_AUDIO_BIS_BCST__)

#endif //APP_AUDIO_H_
