/********************************************************************************************************
 * @file	user_config.h
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
#pragma once

#if (__PROJECT_B91_DEMO )
	#include <vendor/B91_demo/app_config.h>
#elif(__PROJECT_B91_MASTER_DONGLE__)
	#include "../B91_master_dongle/app_config.h"
#elif (__PROJECT_B91_FEATURE__)
	#include "../B91_feature/app_config.h"
#elif (__PROJECT_B91_INTERNAL_TEST__ )
	#include "../B91_internal_test/app_config.h"
#elif (__PROJECT_B91_CIS_MASTER__ )
	#include "../vendor/audio_cis_mst/app_config.h"
#elif (__PROJECT_B91_CIS_SLAVE__ )
	#include "../vendor/audio_cis_slv/app_config.h"
#elif (__PROJECT_B91_AUDIO_BIS_BCST__ )
	#include "../vendor/audio_bis_bcst/app_config.h"
#elif (__PROJECT_B91_AUDIO_BIS_SYNC__ )
	#include "../vendor/audio_bis_sync/app_config.h"
#elif(__PROJECT_B91_HCI__)
	#include "../B91_controller/app_config.h"
#elif(__PROJECT_B91_SLAVE)
	#include "../B91_slave/app_config.h"
#else
	#include "../common/default_config.h"
#endif

