/********************************************************************************************************
 * @file	app.c
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
#if (!LE_AUDIO_DEMO_ENABLE)
	#include "default_att.h"
#else
	#include "app_att.h"
#endif
#include "app_buffer.h"
#include "app_audio.h"
#include "app_ui.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"



#define		TEST_IAL_EN					(!LE_AUDIO_DEMO_ENABLE)
#define  	CIS_INTERVEEL_MS			(10)
#define 	ISO_DL_LEN					(100)
#define 	UNFRAMDE_PACKET_TEST_EN		(1)


int	master_smp_pending = 0; 		// SMP: security & encryption;

u8 app_cis_estab_cnt = 0;

#if(TEST_IAL_EN)
	u8 cig_ret_buffer[3 + 2 * APP_CIS_NUM_IN_PER_CIG_MST]; 								//e.g.: two CISes:  3 +  2*2 =  7
	hci_le_setCigParam_retParam_t	 * pCigRetParam = (hci_le_setCigParam_retParam_t*    )cig_ret_buffer;

	u8 cis_create_buffer[1+ sizeof(cisConnParams_t)*APP_CIS_NUM_IN_PER_CIG_MST];			//e.g.: two CISes:  1 +  4*2 =  9
	hci_le_CreateCisParams_t *pCisParam = (hci_le_CreateCisParams_t*)cis_create_buffer;
#endif




const u8	tbl_advData[] = {
	 8,  DT_COMPLETE_LOCAL_NAME, 				'c','i','s','_','m','s','t',
	 2,	 DT_FLAGS, 								0x05, 					// BLE limited discoverable mode and BR/EDR not supported
	 3,  DT_APPEARANCE, 						0x80, 0x01, 			// 384, Generic Remote Control, Generic category
	 5,  DT_INCOMPLT_LIST_16BIT_SERVICE_UUID,	0x12, 0x18, 0x0F, 0x18,	// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 8, DT_COMPLETE_LOCAL_NAME, 				'c','i','s','_','m','s','t',
};



u16 app_cisConnHandle[TEST_CIS_COUNT] = {0};
u16 app_aclConnHandle[TEST_CIS_COUNT] = {0};



#if (UI_LED_ENABLE)
int led_map[4] = {
		GPIO_LED_BLUE,
		GPIO_LED_GREEN,
		GPIO_LED_WHITE,
		GPIO_LED_RED,
};
#endif


/**
 * @brief      BLE Adv report event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int AA_dbg_adv_rpt = 0;
u32	tick_adv_rpt = 0;

int app_le_adv_report_event_handle(u8 *p)
{
	event_adv_report_t *pa = (event_adv_report_t *)p;
	s8 rssi = pa->data[pa->len];

	#if 0  //debug, print ADV report number every 5 seconds
		AA_dbg_adv_rpt ++;
		if(clock_time_exceed(tick_adv_rpt, 5000000)){
			my_dump_str_data(APP_DUMP_EN, "Adv report", pa->mac, 6);
			tick_adv_rpt = clock_time();
		}
	#endif

	/*********************** Master Create connection demo: Key press or ADV pair packet triggers pair  ********************/
#if (BLE_MASTER_SMP_ENABLE)
	if(master_smp_pending){ 	 //if previous connection SMP not finish, can not create a new connection
		return 1;
	}
#endif

#if (BLE_MASTER_SIMPLE_SDP_ENABLE)
	if(master_sdp_pending){ 	 //if previous connection SDP not finish, can not create a new connection
		return 1;
	}
#endif

	if (master_disconnect_connhandle){ //one master connection is in un_pair disconnection flow, do not create a new one
		return 1;
	}

	int master_auto_connect = 0;
	int user_manual_pairing = 0;

	//manual pairing methods 1: key press triggers
	user_manual_pairing = master_pairing_enable && (rssi > -66);  //button trigger pairing(RSSI threshold, short distance)

	#if (BLE_MASTER_SMP_ENABLE)
		master_auto_connect = blc_smp_searchBondingSlaveDevice_by_PeerMacAddress(pa->adr_type, pa->mac);
	#else
		//search in slave mac_address table to find whether this device is an old device which has already paired with master
		master_auto_connect = user_tbl_slave_mac_search(pa->adr_type, pa->mac);
	#endif

	if(master_auto_connect || user_manual_pairing){

		/* send create connection command to Controller, trigger it switch to initiating state. After this command, Controller
		 * will scan all the ADV packets it received but not report to host, to find the specified device(mac_adr_type & mac_adr),
		 * then send a "CONN_REQ" packet, enter to connection state and send a connection complete event
		 * (HCI_SUB_EVT_LE_CONNECTION_COMPLETE) to Host*/
		u8 status = blc_ll_createConnection( SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, INITIATE_FP_ADV_SPECIFY,  \
								 pa->adr_type, pa->mac, OWN_ADDRESS_PUBLIC, \
								 CONN_INTERVAL_47P5MS, CONN_INTERVAL_50MS, 0, CONN_TIMEOUT_4S, \
								 0, 0xFFFF);


		if(status == BLE_SUCCESS){ //create connection success
			#if (!BLE_MASTER_SMP_ENABLE)
			    // for Telink referenced pair&bonding,
				if(user_manual_pairing && !master_auto_connect){  //manual pair but not auto connect
					blm_manPair.manual_pair = 1;
					blm_manPair.mac_type = pa->adr_type;
					memcpy(blm_manPair.mac, pa->mac, 6);
					blm_manPair.pair_tick = clock_time();
				}
			#endif
		}
	}
	/*********************** Master Create connection demo code end  *******************************************************/


	return 0;
}


/**
 * @brief      BLE Connection complete event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_le_connection_complete_event_handle(u8 *p)
{
	hci_le_connectionCompleteEvt_t *pConnEvt = (hci_le_connectionCompleteEvt_t *)p;

	if(pConnEvt->status == BLE_SUCCESS){
		dev_char_info_insert_by_conn_event(pConnEvt);

		if( pConnEvt->role == LL_ROLE_MASTER ) // master role, process SMP and SDP if necessary
		{
			//Demo effect: any master connection is created, turn on Red LED
			#if (UI_LED_ENABLE)
				gpio_write(led_map[pConnEvt->connHandle&0x0F], 1);
			#endif
			
			#if (BLE_MASTER_SMP_ENABLE)
				master_smp_pending = pConnEvt->connHandle; // this connection need SMP
			#else
				//manual pairing, device match, add this device to slave mac table
				if(blm_manPair.manual_pair && blm_manPair.mac_type == pConnEvt->peerAddrType && !memcmp(blm_manPair.mac, pConnEvt->peerAddr, 6)){
					blm_manPair.manual_pair = 0;
					user_tbl_slave_mac_add(pConnEvt->peerAddrType, pConnEvt->peerAddr);
				}
			#endif



			#if (BLE_MASTER_SIMPLE_SDP_ENABLE)
				memset(&cur_sdp_device, 0, sizeof(dev_char_info_t));
				cur_sdp_device.conn_handle = pConnEvt->connHandle;
				cur_sdp_device.peer_adrType = pConnEvt->peerAddrType;
				memcpy(cur_sdp_device.peer_addr, pConnEvt->peerAddr, 6);

				u8	temp_buff[sizeof(dev_att_t)];
				dev_att_t *pdev_att = (dev_att_t *)temp_buff;

				/* att_handle search in flash, if success, load char_handle directly from flash, no need SDP again */
				if( dev_char_info_search_peer_att_handle_by_peer_mac(pConnEvt->peerAddrType, pConnEvt->peerAddr, pdev_att) ){
					//cur_sdp_device.char_handle[1] = 									//Speaker
					cur_sdp_device.char_handle[2] = pdev_att->char_handle[2];			//OTA
					cur_sdp_device.char_handle[3] = pdev_att->char_handle[3];			//consume report
					cur_sdp_device.char_handle[4] = pdev_att->char_handle[4];			//normal key report
					//cur_sdp_device.char_handle[6] =									//BLE Module, SPP Server to Client
					//cur_sdp_device.char_handle[7] =									//BLE Module, SPP Client to Server

					/* add the peer device att_handle value to conn_dev_list */
					dev_char_info_add_peer_att_handle(&cur_sdp_device);
				}
				else
				{
					master_sdp_pending = pConnEvt->connHandle;  // mark this connection need SDP

					#if (BLE_MASTER_SMP_ENABLE)
						 //service discovery initiated after SMP done, trigger it in "GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE" event callBack.
					#else
						 app_register_service(&app_service_discovery); 	//No SMP, service discovery can initiated now
					#endif
				}
			#endif

			#if (LE_AUDIO_DEMO_ENABLE)
				if((pConnEvt->connHandle&0x0F) == 0){
					app_cisConnHandle[0] = 0;
					app_aclConnHandle[0] = pConnEvt->connHandle;
				}
				#if (TEST_CIS_COUNT >= 2)
				else if((pConnEvt->connHandle&0x0F) == 1){
					app_cisConnHandle[1] = 0;
					app_aclConnHandle[1] = pConnEvt->connHandle;
				}
				app_audio_acl_connect(pConnEvt->connHandle, p, false);
				if(conn_master_num < 2){
					blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
					//printf("connect - Enable Scan\r\n");
				}else{
					blc_ll_setScanEnable(BLC_SCAN_DISABLE, DUP_FILTER_DISABLE);
					//printf("connect - Disable Scan\r\n");
				}
				#endif
			#endif
		}
	}

	return 0;
}



/**
 * @brief      BLE Disconnection event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int 	app_disconnect_event_handle(u8 *p)
{
	event_disconnection_t	*pCon = (event_disconnection_t *)p;



	//terminate reason
	if(pCon->reason == HCI_ERR_CONN_TIMEOUT){  	//connection timeout

	}
	else if(pCon->reason == HCI_ERR_REMOTE_USER_TERM_CONN){  	//peer device send terminate command on link layer

	}
	//master host disconnect( blm_ll_disconnect(current_connHandle, HCI_ERR_REMOTE_USER_TERM_CONN) )
	else if(pCon->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){

	}
	else{

	}


		//if previous connection SMP & SDP not finished, clear flag
	#if (BLE_MASTER_SMP_ENABLE)
		if(master_smp_pending == pCon->connHandle){
			master_smp_pending = 0;
		}
	#endif

	#if (BLE_MASTER_SIMPLE_SDP_ENABLE)
		if(master_sdp_pending == pCon->connHandle){
			master_sdp_pending = 0;
		}
	#endif

	if(master_disconnect_connhandle == pCon->connHandle){  //un_pair disconnection flow finish, clear flag
		master_disconnect_connhandle = 0;
	}

	dev_char_info_delete_by_connhandle(pCon->connHandle);

#if 0

	if(pCon->connHandle & BLT_CIS_HANDLE)
	{
		if(app_cis_estab_cnt){
			app_cis_estab_cnt--;
		}

		my_dump_str_data(APP_DUMP_EN, "app_cis_estab_cnt", &app_cis_estab_cnt, 1);
	}

	#if (UI_LED_ENABLE)
		if(!conn_master_num){
			gpio_write(led_map[pCon->connHandle&0x0F], 0);
		}
		if(!app_cis_estab_cnt){
			gpio_write(led_map[2], 0);
		}
	#endif


	#if (LE_AUDIO_DEMO_ENABLE)

		#if (TEST_CIS_COUNT >= 2)
			if(conn_master_num < 2){
				blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
				//printf("disconn - Enable Scan\r\n");
			}
		#endif

		if((pCon->connHandle&0x0F) == 0){
			app_cisConnHandle[0] = 0;
			app_aclConnHandle[0] = 0;
		}

		#if (TEST_CIS_COUNT >= 2)
		else if((pCon->connHandle&0x0F) == 1){
			app_cisConnHandle[1] = 0;
			app_aclConnHandle[1] = 0;
		}
		#endif
		app_audio_acl_disconn(pCon->connHandle, p);

		if(pCon->connHandle & BLT_CIS_CONN_HANDLE){

			if(app_cisConnHandle[0] == pCon->connHandle){
				app_cisConnHandle[0] = 0;
			}

			#if (TEST_CIS_COUNT >= 2)
				else if(app_cisConnHandle[1] == pCon->connHandle){
					app_cisConnHandle[1] = 0;
				}
			#endif

			#if (UI_LED_ENABLE)
				gpio_write(led_map[(pCon->connHandle&0x0F) + 2], 0);
			#endif

			app_audio_cis_disconn(pCon->connHandle, p);
	}
	#endif
#endif
	return 0;
}


/**
 * @brief      BLE Connection update complete event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_le_connection_update_complete_event_handle(u8 *p)
{
	hci_le_connectionUpdateCompleteEvt_t *pUpt = (hci_le_connectionUpdateCompleteEvt_t *)p;

	if(pUpt->status == BLE_SUCCESS){

	}

	return 0;
}


/**
 * @brief      BLE controller event handler call-back.
 * @param[in]  h       event type
 * @param[in]  p       Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_controller_event_callback (u32 h, u8 *p, int n)
{
	if (h &HCI_FLAG_EVENT_BT_STD)		//Controller HCI event
	{
		u8 evtCode = h & 0xff;

		//------------ disconnect -------------------------------------
		if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
		{
			app_disconnect_event_handle(p);
		}
		else if(evtCode == HCI_EVT_LE_META)  //LE Event
		{
			u8 subEvt_code = p[0];

			//------hci le event: le connection complete event---------------------------------
			if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)	// connection complete
			{
				app_le_connection_complete_event_handle(p);
			}
			//--------hci le event: le adv report event ----------------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
			{
				//after controller is set to scan state, it will report all the adv packet it received by this event

				app_le_adv_report_event_handle(p);
			}
			//------hci le event: le connection update complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)	// connection update
			{
				app_le_connection_update_complete_event_handle(p);
			}
			//------HCI LE event: LE CIS Established event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CIS_ESTABLISHED)
			{
				hci_le_cisEstablishedEvt_t *pCisEstbEvent = (hci_le_cisEstablishedEvt_t *)p;
				if(pCisEstbEvent->status == BLE_SUCCESS){
				#if (UI_LED_ENABLE)
					gpio_write(led_map[2], 1);
				#endif
					app_cis_estab_cnt++;
					my_dump_str_data(APP_DUMP_EN, "LE CIS Established event status:", &pCisEstbEvent->status, 1);
					my_dump_str_data(APP_DUMP_EN, "                         cisHdl:", &pCisEstbEvent->cisHandle, 2);
					#if (LE_AUDIO_DEMO_ENABLE)
					if((pCisEstbEvent->cisHandle&0x0F) == 0){
						app_cisConnHandle[0] = pCisEstbEvent->cisHandle;
					}
					#if (TEST_CIS_COUNT >= 2)
					else{
						app_cisConnHandle[1] = pCisEstbEvent->cisHandle;
					}
					#endif
					app_audio_cis_connect(pCisEstbEvent->cisHandle, p);
					#endif
				}
			}
		}
	}


	return 0;

}




/**
 * @brief      BLE host event handler call-back.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_host_event_callback (u32 h, u8 *para, int n)
{
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_BEGIN:
		{

		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{

		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
		#if (BLE_MASTER_SMP_ENABLE)
			gap_smp_pairingFailEvt_t *p = (gap_smp_pairingFailEvt_t *)para;

			if( dev_char_get_conn_role_by_connhandle(p->connHandle) == LL_ROLE_MASTER){  //master connection
				if(master_smp_pending == p->connHandle){
					master_smp_pending = 0;
				}
			}
		#endif
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{

		}
		break;

		case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
		#if (BLE_MASTER_SMP_ENABLE)
			if( dev_char_get_conn_role_by_connhandle(p->connHandle) == LL_ROLE_MASTER){  //master connection

				if(master_smp_pending == p->connHandle){
					master_smp_pending = 0;
				}
			}
		#endif

		#if (BLE_MASTER_SIMPLE_SDP_ENABLE)  //SMP finish
			if(master_sdp_pending == p->connHandle){  //SDP is pending
				app_register_service(&app_service_discovery);  //start SDP now
			}
		#endif
		}
		break;
		case GAP_EVT_ATT_EXCHANGE_MTU:
		{

		}
		break;

		case GAP_EVT_GATT_HANDLE_VLAUE_CONFIRM:
		{

		}
		break;

		default:
		break;
	}

	return 0;
}




/**
 * @brief      BLE GATT data handler call-back.
 * @param[in]  connHandle     connection handle.
 * @param[in]  pkt             Pointer point to data packet buffer.
 * @return
 */
int app_gatt_data_handler (u16 connHandle, u8 *pkt)
{

	if( dev_char_get_conn_role_by_connhandle(connHandle) == LL_ROLE_MASTER )   //GATT data for Master
	{
			#if (BLE_MASTER_SIMPLE_SDP_ENABLE)
				if(master_sdp_pending == connHandle ){  //ATT service discovery is ongoing on this conn_handle
					//when service discovery function is running, all the ATT data from slave
					//will be processed by it,  user can only send your own att cmd after  service discovery is over
					host_att_client_handler (connHandle, pkt); //handle this ATT data by service discovery process
				}
			#endif

			//so any ATT data before service discovery will be dropped
			dev_char_info_t* dev_info = dev_char_info_search_by_connhandle (connHandle);
			if(dev_info)
			{
				//-------	user process ------------------------------------------------
				rf_packet_att_t *pAtt = (rf_packet_att_t*)pkt;

				if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)  //slave handle notify
				{

				}
				else if (pAtt->opcode == ATT_OP_HANDLE_VALUE_IND)
				{

				}
			}
#if(LE_AUDIO_DEMO_ENABLE)
		app_audio_gattHandler(connHandle, pkt);
#endif
	}
	else{   //GATT data for Slave


	}


	return 0;
}


#if (TEST_IAL_EN)
	u8 sdu[MAX_TX_SDU_LEN] = {
			0,0,0,0,
			0,0,
			0,0,

			0x20, 0x40,  //handle

			U16_LO(ISO_DL_LEN+8), U16_HI(ISO_DL_LEN+8),		// iso_dl_len

			0,0,0,0,	//timestamp
			1,0,   //sn
			U16_LO(ISO_DL_LEN), U16_HI(ISO_DL_LEN),  //sdu_len

			1,2,3,4,5,6,7,8,9,10,
	};
	u32 sdu_tick = 0;

	u32 send_cnt = 0;
#endif

	_attribute_ram_code_ void app_timer_test_irq_proc(void)
	{
	#if (TEST_IAL_EN)

		for(int cis_conn_idx=0; cis_conn_idx<bltCisMng.maxNum_cisMaster; cis_conn_idx++)
		{
			ll_cis_conn_t * cur_pCisConn = (ll_cis_conn_t *) (global_pCisConn + cis_conn_idx);

//			ll_cis_conn_t * cur_pCisConn = (ll_cis_conn_t *) (global_pCisConn + 0);
//			ll_iso_sud_t *pSdu =(ll_iso_sud_t *) sdu;
			if((cur_pCisConn->connState == CONN_STATUS_ESTABLISH) &&(cur_pCisConn->cisEventCnt>=10) )//&& (send_cnt<3)
			{
				send_cnt ++;
				//DBG_CHN13_TOGGLE;
			#if	(UNFRAMDE_PACKET_TEST_EN)
				blc_ial_cis_splitSdu2UnframedPdu(cur_pCisConn->cis_connHandle, (iso_data_packet_t*)sdu);
			#else
				u8 *p = (sduCisMng.tx_fifo_b + sduCisMng.max_tx_size * \
											((cur_pCisConn->txSduFifoWptr&(sduCisMng.tx_fifo_num-1))));
				sdu[20]++;
				smemcpy(p, sdu, MAX_TX_SDU_LEN);
				cur_pCisConn->txSduFifoWptr++;
			#endif
				//DBG_CHN13_TOGGLE;
			}
		}
	#endif
	}




#if (TEST_IAL_EN)
ble_sts_t  cisTest_configureParam(u8 cis_count)
{
	ble_sts_t status;
	u8 cig_cmd_buffer[15 + sizeof(cigParamTest_cisCfg_t) * APP_CIS_NUM_IN_PER_CIG_MST]; 	//e.g.: two CISes: 15 + 14*2 = 43
	hci_le_setCigParamTest_cmdParam_t* pCigCmdParam = (hci_le_setCigParamTest_cmdParam_t*)cig_cmd_buffer;


	pCigCmdParam->cig_id = CIG_ID_0;
	pCigCmdParam->sdu_int_m2s[0] = U32_BYTE0(1000*CIS_INTERVEEL_MS);  //unit:us
	pCigCmdParam->sdu_int_m2s[1] = U32_BYTE1(1000*CIS_INTERVEEL_MS);
	pCigCmdParam->sdu_int_m2s[2] = U32_BYTE2(1000*CIS_INTERVEEL_MS);
	pCigCmdParam->sdu_int_s2m[0] = U32_BYTE0(1000*CIS_INTERVEEL_MS);  //unit:us
	pCigCmdParam->sdu_int_s2m[1] = U32_BYTE1(1000*CIS_INTERVEEL_MS);
	pCigCmdParam->sdu_int_s2m[2] = U32_BYTE2(1000*CIS_INTERVEEL_MS);
	pCigCmdParam->ft_m2s = 1;
	pCigCmdParam->ft_s2m = 1;
	pCigCmdParam->iso_intvl = CIS_INTERVEEL_MS*1000/1250; //X*1.25ms
	pCigCmdParam->sca = 0; //251 ppm to 500 ppm
	pCigCmdParam->packing = PACK_SEQUENTIAL;//PACK_INTERLEAVED;//PACK_SEQUENTIAL;
	pCigCmdParam->framing = UNFRAMDE_PACKET_TEST_EN ? UNFRAMED : FRAMED;
	pCigCmdParam->cis_count = cis_count;

	for(int i = 0; i < cis_count; i++){
		pCigCmdParam->cisCfg[i].cis_id = i;
		pCigCmdParam->cisCfg[i].nse = 2;
		pCigCmdParam->cisCfg[i].max_sdu_m2s = ISO_DL_LEN;
		pCigCmdParam->cisCfg[i].max_sdu_s2m = ISO_DL_LEN;
		pCigCmdParam->cisCfg[i].max_pdu_m2s = ISO_DL_LEN;
		pCigCmdParam->cisCfg[i].max_pdu_s2m = ISO_DL_LEN;
		pCigCmdParam->cisCfg[i].phy_m2s = PHY_PREFER_2M;
		pCigCmdParam->cisCfg[i].phy_s2m = PHY_PREFER_2M;
		pCigCmdParam->cisCfg[i].bn_m2s = 1;
		pCigCmdParam->cisCfg[i].bn_s2m = 1;
	}

	status = blc_hci_le_setCigParamsTest( pCigCmdParam, pCigRetParam);

	if(status == BLE_SUCCESS){
		my_dump_str_data(APP_DUMP_EN, "LE setCigParamsTest success", 0, 0);
	}
	else{
		my_dump_str_data(APP_DUMP_EN, "LE setCigParamsTest failed", &status, 1);
	}

	return status;
}
#endif

///////////////////////////////////////////

/**
 * @brief		user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]	none
 * @return      none
 */
_attribute_no_inline_ void user_init_normal(void)
{
	/* random number generator must be initiated here( in the beginning of user_init_nromal).
	 * When deepSleep retention wakeUp, no need initialize again */
	random_generator_init();

//////////////////////////// BLE stack Initialization  Begin //////////////////////////////////

	u8  mac_public[6];
	u8  mac_random_static[6];
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);


	//////////// LinkLayer Initialization  Begin /////////////////////////
	blc_ll_initBasicMCU();

	blc_ll_initStandby_module(mac_public);						   //mandatory

    blc_ll_initLegacyAdvertising_module(); 	//adv module: 		 mandatory for BLE slave,

    blc_ll_initLegacyScanning_module(); 	//scan module: 		 mandatory for BLE master

	blc_ll_initInitiating_module();			//initiate module: 	 mandatory for BLE master

	blc_ll_initAclConnection_module();
	blc_ll_initAclMasterRole_module();
	blc_ll_initAclSlaveRole_module();

	blc_ll_init2MPhyCodedPhy_feature();

	// ISO features must support CSA#2
	blc_ll_initChannelSelectionAlgorithm_2_feature();

	blc_ll_setMaxConnectionNumber( MASTER_MAX_NUM, SLAVE_MAX_NUM);

	blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_MASTER_MAX_TX_OCTETS, ACL_SLAVE_MAX_TX_OCTETS);

	/* all ACL connection share same RX FIFO */
	blc_ll_initAclConnRxFifo(app_acl_rxfifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM);
	/* ACL Master TX FIFO */
	blc_ll_initAclConnMasterTxFifo(app_acl_mstTxfifo, ACL_MASTER_TX_FIFO_SIZE, ACL_MASTER_TX_FIFO_NUM, MASTER_MAX_NUM);
	/* ACL Slave TX FIFO */
	blc_ll_initAclConnSlaveTxFifo(app_acl_slvTxfifo, ACL_SLAVE_TX_FIFO_SIZE, ACL_SLAVE_TX_FIFO_NUM, SLAVE_MAX_NUM);

	blc_ll_setAclMasterConnectionInterval(CONN_INTERVAL_47P5MS);

	rf_set_power_level_index (RF_POWER_P3dBm);

	//////////// LinkLayer Initialization  End /////////////////////////
		
	///// CIS Master initialization //////////////
	blc_ll_initCisMaster_module();

	blc_ial_initSdu_module(); //IAL module register

	blc_ll_initCisMasterParameters(app_cig_param, APP_CIG_MST_NUMBER);

	blc_ll_initCisConnectionParameters(app_cis_conn_param, APP_CIS_NUM_IN_ALL_CIG_MST, APP_CIS_NUM_IN_ALL_CIG_SLV);

	blc_setHostFeatureISOChannel_en(LL_FEATURE_ENABLE);

	/* CIS TX buffer init */
	blc_ll_initCisTxFifo(app_cis_txfifo, MAX_TX_ISO_FIFO_SIZE, ISO_TX_FIFO_NUM);
	/* CIS RX buffer init */
	blc_ll_initCisRxFifo(app_cis_rxfifo, MAX_RX_ISO_FIFO_SIZE, ISO_RX_FIFO_NUM);
	/* CIS RX EVT buffer init */
	blc_ll_initCisRxEvtFifo(app_cis_rxEvtfifo, MAX_RX_ISO_EVT_FIFO_SIZE, ISO_RX_EVT_FIFO_NUM);
	/* IAL SDU buff init */
	blc_ial_initCisSduBuff(bltSduRxFifo,MAX_RX_SDU_LEN, IAL_SDU_RX_FIFO_NUM, bltSduTxFifo,MAX_TX_SDU_LEN, IAL_SDU_TX_FIFO_NUM);

	//////////// HCI Initialization  Begin /////////////////////////
	blc_hci_registerControllerDataHandler (blc_l2cap_pktHandler);

	blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(		HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
									|	HCI_LE_EVT_MASK_ADVERTISING_REPORT \
									|   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE \
									|	HCI_LE_EVT_MASK_CIS_ESTABLISHED \
									| 	HCI_LE_EVT_MASK_CIS_REQUESTED  );


	u8 check_status = blc_controller_check_appBufferInitialization();
	if(check_status != BLE_SUCCESS){
		/* here user should set some log to know which application buffer incorrect*/
		write_log32(0x88880000 | check_status);
		while(1){
		#if (APP_DUMP_EN)
			myudb_usb_handle_irq ();
		#endif
		}
	}
	//////////// HCI Initialization  End /////////////////////////


	//////////// Host Initialization  Begin /////////////////////////
	/* Host Initialization */
	/* GAP initialization must be done before any other host feature initialization !!! */
	blc_gap_init();

	/* L2CAP buffer Initialization */
	blc_l2cap_initAclConnMasterMtuBuffer(mtu_m_rx_fifo, MTU_M_BUFF_SIZE_MAX, 			0,					 0);
	blc_l2cap_initAclConnSlaveMtuBuffer(mtu_s_rx_fifo, MTU_S_BUFF_SIZE_MAX, mtu_s_tx_fifo, MTU_S_BUFF_SIZE_MAX);
	#if (LE_AUDIO_ENABLE)
		blc_att_setSlaveRxMTUSize(64);
		blc_att_setMasterRxMTUSize(64);
	#endif
	/* GATT Initialization */
	my_gatt_init();
	#if (BLE_MASTER_SIMPLE_SDP_ENABLE)
		host_att_register_idle_func (main_idle_loop);
	#endif
	blc_gatt_register_data_handler(app_gatt_data_handler);

	/* SMP Initialization */
	#if (BLE_SLAVE_SMP_ENABLE || BLE_MASTER_SMP_ENABLE)
		blc_smp_configPairingSecurityInfoStorageAddressAndSize(FLASH_ADR_SMP_PAIRING, FLASH_SMP_PAIRING_MAX_SIZE);
	#endif

	#if (BLE_SLAVE_SMP_ENABLE)  //Slave SMP Enable
		blc_smp_setSecurityLevel_slave(Unauthenticated_Pairing_with_Encryption);  //LE_Security_Mode_1_Level_2
	#else
		blc_smp_setSecurityLevel_slave(No_Security);
	#endif

	#if (BLE_MASTER_SMP_ENABLE) //Master SMP Enable
		blc_smp_setSecurityLevel_master(Unauthenticated_Pairing_with_Encryption);  //LE_Security_Mode_1_Level_2
	#else
		blc_smp_setSecurityLevel_master(No_Security);
		user_master_host_pairing_management_init(); 		//TeLink referenced pairing&bonding without standard pairing in BLE Spec
	#endif

	blc_smp_smpParamInit();


	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PAIRING_BEGIN 			|  \
						  GAP_EVT_MASK_SMP_PAIRING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PAIRING_FAIL				|  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);
	//////////// Host Initialization  End /////////////////////////

//////////////////////////// BLE stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for BLE application ////////////////////////////
#if 0
	blc_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	blc_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
	blc_ll_setAdvParam(ADV_INTERVAL_30MS, ADV_INTERVAL_30MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
	blc_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable
#endif
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_200MS, SCAN_WINDOW_200MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////

#if (TEST_IAL_EN)
	for(u16 i=0; i< ISO_DL_LEN; i++)
	{
		sdu[20+i] = i;
	}
	cisTest_configureParam(2);
	app_cis_estab_cnt = 0;
#if (0)//Attention: AES_CCM_Encryption in IRQ, AES_CCM_Decryption in main_loop maybe overlap!!!
	   //At present, the current CIS_CCM_NONCE is maintained before calling AES_CCM_ENC_V2, and
	   //the saved CIS_CCM_NONCE is restored after the encryption is over.
	/* So here you can open the macro for testing. Currently, the method of polling and sending ISO DATA in main_loop is used. */
	plic_set_priority(IRQ4_TIMER0, 1);
	timer_set_init_tick(TIMER0, 0);
	plic_interrupt_enable(IRQ4_TIMER0);
	timer_set_mode(TIMER0, TIMER_MODE_SYSCLK);
	timer_set_cap_tick(TIMER0, sys_clk.pclk*CIS_INTERVEEL_MS*1000);
	timer_start(TIMER0);
#endif

#endif

#if (LE_AUDIO_DEMO_ENABLE)
	app_audio_init();
#endif


}



/**
 * @brief		user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]	none
 * @return      none
 */
void user_init_deepRetn(void)
{

}


/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////


/**
 * @brief     BLE main idle loop
 * @param[in]  none.
 * @return     none.
 */
int main_idle_loop (void)
{

	////////////////////////////////////// BLE entry /////////////////////////////////
	blc_sdk_main_loop();


	////////////////////////////////////// UI entry /////////////////////////////////
	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard (0,0, 0);
	#endif


	proc_master_role_unpair();

	#if (LE_AUDIO_DEMO_ENABLE)
		app_audio_handler();
	#endif

	#if (TEST_IAL_EN)
		if(1 && clock_time_exceed(sdu_tick, CIS_INTERVEEL_MS * 1000)){
			sdu_tick = clock_time();
			app_timer_test_irq_proc();
		}

		#if(0)

			if((startCISTest==1) && (conn_master_num)){
				my_dump_str_data(APP_DUMP_EN, "creat ONE CIS in ACL", &startCISTest, 1);

				startCISTest = 0;
				pCisParam->cis_count = 1;
				pCisParam->cisConn[0].acl_handle = conn_dev_list[0].conn_handle;
				pCisParam->cisConn[0].cis_handle = (!app_cis_estab_cnt) ? pCigRetParam->cis_connHandle[0]:pCigRetParam->cis_connHandle[1];

				ble_sts_t status = blc_hci_le_createCis(pCisParam);
				if(status == BLE_SUCCESS){ //Start to Send LL_CIS_REQ pkt
					my_dump_str_data(APP_DUMP_EN, "CreateCis success", 0, 0);
				}
				else{
					my_dump_str_data(APP_DUMP_EN, "CreateCis failed", &status, 1);
				}
			}
			else if((startCISTest==2) && (conn_master_num>=2)){
				my_dump_str_data(APP_DUMP_EN, "creat CIS in ACL1", &startCISTest, 1);

				startCISTest = 0;
				pCisParam->cis_count = 1;
				pCisParam->cisConn[0].acl_handle = conn_dev_list[1].conn_handle;
				pCisParam->cisConn[0].cis_handle = pCigRetParam->cis_connHandle[1];

				ble_sts_t status = blc_hci_le_createCis(pCisParam);
				if(status == BLE_SUCCESS){ //Start to Send LL_CIS_REQ pkt

					my_dump_str_data(APP_DUMP_EN, "CreateCis success", 0, 0);
				}
				else{
					my_dump_str_data(APP_DUMP_EN, "CreateCis failed", &status, 1);
				}
			}
			else
			{
				startCISTest = 0;
			}
		#else
			if(0&(startCISTest==1) && (conn_master_num)){
				startCISTest = 0;
				my_dump_str_data(APP_DUMP_EN, "creat CIS0 and CIS1 in ACL0 together", &startCISTest, 1);
				pCisParam->cis_count = 2;
				pCisParam->cisConn[0].acl_handle = conn_dev_list[0].conn_handle;
				pCisParam->cisConn[0].cis_handle = pCigRetParam->cis_connHandle[0];
				pCisParam->cisConn[1].acl_handle = conn_dev_list[0].conn_handle;
				pCisParam->cisConn[1].cis_handle = pCigRetParam->cis_connHandle[1];

				ble_sts_t status = blc_hci_le_createCis(pCisParam);
				if(status == BLE_SUCCESS){ //Start to Send LL_CIS_REQ pkt
					my_dump_str_data(APP_DUMP_EN, "CreateCis success", 0, 0);
				}
				else{
					my_dump_str_data(APP_DUMP_EN, "CreateCis failed", &status, 1);
				}
			}

			else if((startCISTest==2) && (conn_master_num>=2)){

				startCISTest = 0;
				my_dump_str_data(APP_DUMP_EN, "creat CIS0 and CIS1 in ACL0 and ACL1 respectively", &startCISTest, 1);
				pCisParam->cis_count = 2;
				pCisParam->cisConn[0].acl_handle = conn_dev_list[0].conn_handle;
				pCisParam->cisConn[0].cis_handle = pCigRetParam->cis_connHandle[0];
				pCisParam->cisConn[1].acl_handle = conn_dev_list[1].conn_handle;
				pCisParam->cisConn[1].cis_handle = pCigRetParam->cis_connHandle[1];

				ble_sts_t status = blc_hci_le_createCis(pCisParam);
				if(status == BLE_SUCCESS){ //Start to Send LL_CIS_REQ pkt

					my_dump_str_data(APP_DUMP_EN, "CreateCis success", 0, 0);
				}
				else{
					my_dump_str_data(APP_DUMP_EN, "CreateCis failed", &status, 1);
				}
			}
			else
			{
				startCISTest = 0;
			}
		#endif

	#endif

	return 0; //must return 0 due to SDP flow
}



/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
_attribute_no_inline_ void main_loop (void)
{
	main_idle_loop ();

	#if (BLE_MASTER_SIMPLE_SDP_ENABLE)
		simple_sdp_loop ();
	#endif
}





