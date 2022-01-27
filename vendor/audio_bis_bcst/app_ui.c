/********************************************************************************************************
 * @file	app_ui.c
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
#include "app.h"
#include "app_config.h"
#include "app_ui.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app_att.h"
#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"


#if (__PROJECT_B91_AUDIO_BIS_BCST__)

int	master_pairing_enable = 0;
int master_unpair_enable = 0;

int master_disconnect_connhandle;   //mark the master connection which is in un_pair disconnection flow




int master_auto_connect = 0;
int user_manual_pairing;



#if (UI_KEYBOARD_ENABLE)

_attribute_ble_data_retention_	int 	key_not_released;
_attribute_ble_data_retention_	int 	key_type;

#define CONSUMER_KEY   	   			1
#define KEYBOARD_KEY   	   			2
#define PAIR_UNPAIR_KEY   	   		3


/**
 * @brief   Check changed key value.
 * @param   none.
 * @return  none.
 */
void key_change_proc(void)
{

	u8 key0 = kb_event.keycode[0];
//	u8 key_buf[8] = {0,0,0,0,0,0,0,0};

	key_not_released = 1;
	if (kb_event.cnt == 2)   //two key press
	{

	}
	else if(kb_event.cnt == 1)
	{
		if(key0 >= CR_VOL_UP )  //volume up/down
		{
			key_type = CONSUMER_KEY;
			u16 consumer_key;
			if(key0 == CR_VOL_UP){  	//volume up
				consumer_key = MKEY_VOL_UP;
				my_dump_str_data(APP_DUMP_EN, "UI send Vol+", 0, 0);

				hci_le_terminateBigParams_t  bigTerm;
				bigTerm.big_handle = 0;
				bigTerm.reason = HCI_ERR_CONN_TERM_BY_LOCAL_HOST;
			#if(APP_DUMP_EN)
				ble_sts_t status = blc_hci_le_terminateBig(&bigTerm);
				my_dump_str_data(APP_DUMP_EN, "LE Terminate BIG: status", &status, 1);
			#else
				blc_hci_le_terminateBig(&bigTerm);
			#endif
				//DBG_CHN8_TOGGLE;

			}
			else if(key0 == CR_VOL_DN){ //volume down
				consumer_key = MKEY_VOL_DN;
				my_dump_str_data(APP_DUMP_EN, "UI send Vol-", 0, 0);

				u8 chm[5] = {0x9a, 0x0d, 0x42, 0x84, 0x18}; // chm[5]
				*(u32*)chm = rand(); //test random chm
			#if(APP_DUMP_EN)
				ble_sts_t status = blc_ll_setHostChannel(chm);
				my_dump_str_data(APP_DUMP_EN, "LL set host channel cmd", chm, 5);
				my_dump_str_data(APP_DUMP_EN, "	status:", &status, 1);
			#else
				blc_ll_setHostChannel(chm);
			#endif
			}


			/*Here is just Telink Demonstration effect. Cause the demo board has limited key to use, when Vol+/Vol- key pressed, we
			send media key "Vol+" or "Vol-" to master for all slave in connection.
			For users, you should known that this is not a good method, you should manage your device and GATT data transfer
			according to  conn_dev_list[]
			 * */
			for(int i=MASTER_MAX_NUM; i < (MASTER_MAX_NUM + SLAVE_MAX_NUM); i++){ //slave index is from "MASTER_MAX_NUM" to "MASTER_MAX_NUM + SLAVE_MAX_NUM - 1"
				if(conn_dev_list[i].conn_state){
					blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
				}
			}
		}
		else{
			key_type = PAIR_UNPAIR_KEY;

			if(key0 == BTN_PAIR)   //Manual pair triggered by Key Press
			{
				master_pairing_enable = 1;
				my_dump_str_data(APP_DUMP_EN, "UI PAIR begin", 0, 0);
			}
			else if(key0 == BTN_UNPAIR) //Manual un_pair triggered by Key Press
			{
				my_dump_str_data(APP_DUMP_EN, "UI UNPAIR begin", 0, 0);
			}

		}

	}
	else   //kb_event.cnt == 0,  key release
	{
		key_not_released = 0;
		if(key_type == CONSUMER_KEY)
		{
			u16 consumer_key = 0;
			//Here is just Telink Demonstration effect. for all slave in connection, send release for previous "Vol+" or "Vol-" to master
			for(int i=MASTER_MAX_NUM; i < (MASTER_MAX_NUM + SLAVE_MAX_NUM); i++){ //slave index is from "MASTER_MAX_NUM" to "MASTER_MAX_NUM + SLAVE_MAX_NUM - 1"
				if(conn_dev_list[i].conn_state){
					blc_gatt_pushHandleValueNotify ( conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
				}
			}
		}
		else if(key_type == KEYBOARD_KEY)
		{

		}
		else if(key_type == PAIR_UNPAIR_KEY){
			if(master_pairing_enable){
				master_pairing_enable = 0;
				my_dump_str_data(APP_DUMP_EN, "UI PAIR end", 0, 0);
			}

			if(master_unpair_enable){
				master_unpair_enable = 0;
			}
		}
	}


}



_attribute_ble_data_retention_		static u32 keyScanTick = 0;

/**
 * @brief      keyboard task handler
 * @param[in]  e    - event type
 * @param[in]  p    - Pointer point to event parameter.
 * @param[in]  n    - the length of event parameter.
 * @return     none.
 */
void proc_keyboard (u8 e, u8 *p, int n)
{
	if(clock_time_exceed(keyScanTick, 10 * 1000)){  //keyScan interval: 10mS
		keyScanTick = clock_time();
	}
	else{
		return;
	}

	kb_event.keycode[0] = 0;
	int det_key = kb_scan_key (0, 1);

	if (det_key){
		key_change_proc();
	}
}




#endif   //end of UI_KEYBOARD_ENABLE




#endif