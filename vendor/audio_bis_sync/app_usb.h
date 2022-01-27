/********************************************************************************************************
 * @file	app_usb.h
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
#ifndef APP_USB_H_
#define APP_USB_H_

#include "tl_common.h"
#include "app_config.h"

#if (__PROJECT_B91_AUDIO_BIS_SYNC__)
#if (APP_AUDIO_USB_ENABLE)


void app_usb_init(void);
void app_usb_enable(bool enable);


bool app_usb_fill_buffer(u8 *pData);
u8  *app_usb_get_buffer(void);


void app_usb_handler(void);

void app_usb_irq_handler(void);



#endif
#endif //#if (FEATURE_TEST_MODE == TEST_CIS_SLAVE)

#endif //APP_USB_H_
