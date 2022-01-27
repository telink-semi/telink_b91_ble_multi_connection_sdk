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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app_config.h"
#include "app.h"
#include "app_buffer.h"
#include "../default_att.h"
#include "app_ui.h"



 
 #if (FEATURE_TEST_MODE == TEST_COC)


#define APP_MASTER_ENABLE     1
#define APP_SLAVE_ENABLE      0

#define APP_COC_MTU_SIZE      64
#define APP_COC_MPS_SIZE      64

#define APP_COC_SPSM          L2CAP_COC_SPSM_EATT
#define APP_COC_METHOD        L2CAP_COC_START_CREADIT_CONNECT

int	master_smp_pending = 0; 		// SMP: security & encryption;

const u8	tbl_advData[] = {
	 4,  DT_COMPLETE_LOCAL_NAME, 				'C','O','C',
	 2,	 DT_FLAGS, 								0x05, 					// BLE limited discoverable mode and BR/EDR not supported
	 3,  DT_APPEARANCE, 						0x80, 0x01, 			// 384, Generic Remote Control, Generic category
	 5,  DT_INCOMPLT_LIST_16BIT_SERVICE_UUID,	0x12, 0x18, 0x0F, 0x18,	// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 4,  DT_COMPLETE_LOCAL_NAME, 				'C','O','C',
};

bool gAppStartCreateCOC = false;


#if (APP_MASTER_ENABLE)
	#define APPM_COC_CONN_COUNT        2
	#define APPM_COC_CH_NUM_PER_SERV   5
	#define APPM_COC_SERVICE_COUNT     2
	#define APPM_COC_CHANNEL_COUNT     (APPM_COC_SERVICE_COUNT*APPM_COC_CH_NUM_PER_SERV)
	#define APPM_COC_MTU_NEED_LENGTH   BLC_L2CAP_COC_MTU_NEEDLEN(APPM_COC_SERVICE_COUNT, APPM_COC_CH_NUM_PER_SERV, APP_COC_MTU_SIZE)
	#define APPM_COC_BUFFER_LENGTH     BLC_L2CAP_COC_INIT_NEEDLEN(APPM_COC_CONN_COUNT, APPM_COC_SERVICE_COUNT, APPM_COC_CHANNEL_COUNT*APPM_COC_CH_NUM_PER_SERV)

	#define APPM_COC_INIT_CREDIT       10
	#define APPM_COC_SEND_ENABLE       1
	#define APPM_COC_SEND_INTERVAL     (500*1000) //us

	u8  gAppmAclConnNumber;
	u16 gAppmAclConnHandle[2] = {0};
	u8  gAppmL2capMtuBuffer[APPM_COC_MTU_NEED_LENGTH];
	u8  gAppmL2capCocBuffer[APPM_COC_BUFFER_LENGTH];

	u8  gAppmCocIsReady[2] = {0};
	u32 gAppmCocSendTimer[2] = {0};
	u16 gAppmCocSendScid[2] = {0};
	
	#define APPM_COC_SEND_LENGTH    (APP_COC_MTU_SIZE-2) //<=APP_COC_MTU_SIZE
	u8  gAppmCocData0[APPM_COC_SEND_LENGTH];
	u8  gAppmCocData1[APPM_COC_SEND_LENGTH];
	u32 gAppmCocSendSerial0;
	u32 gAppmCocSendSerial1;
#endif
#if (APP_SLAVE_ENABLE)
	#define APPS_COC_CONN_COUNT 	   1
	#define APPS_COC_CH_NUM_PER_SERV   5
	#define APPS_COC_SERVICE_COUNT	   1
	#define APPS_COC_CHANNEL_COUNT	   (APPS_COC_SERVICE_COUNT*APPS_COC_CH_NUM_PER_SERV)
	#define APPS_COC_MTU_NEED_LENGTH   BLC_L2CAP_COC_MTU_NEEDLEN(APPS_COC_SERVICE_COUNT, APPS_COC_CH_NUM_PER_SERV, APP_COC_MTU_SIZE)
	#define APPS_COC_BUFFER_LENGTH	   BLC_L2CAP_COC_INIT_NEEDLEN(APPS_COC_CONN_COUNT, APPS_COC_SERVICE_COUNT, APPS_COC_CHANNEL_COUNT*APPS_COC_CH_NUM_PER_SERV)

	#define APPS_COC_INIT_CREDIT       5
	#define APPS_COC_SEND_ENABLE       1
	#define APPS_COC_SEND_INTERVAL     (100*1000) //us

	u16 gAppsAclConnHandle;
	u8  gAppsL2capMtuBuffer[APPS_COC_MTU_NEED_LENGTH];
	u8  gAppsL2capCocBuffer[APPS_COC_BUFFER_LENGTH];

	u8  gAppsCocIsReady;
	u32 gAppsCocSendTimer;

	#define APPS_COC_SEND_LENGTH      (APP_COC_MTU_SIZE-2) //<=APP_COC_MTU_SIZE
	u8  gAppsCocData[APPS_COC_SEND_LENGTH];
	u32 gAppsCocSendSerial = 0;
	u16 gAppsCocSendScid;
#endif


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

			#if (APP_MASTER_ENABLE)
			if((pConnEvt->connHandle&0x0F) == 0){
				gAppmAclConnHandle[0] = pConnEvt->connHandle;
				blc_l2cap_cocConnectHandler(gAppmAclConnHandle[0]);
				#if (UI_LED_ENABLE)
				gpio_write(GPIO_LED_BLUE, 1);
				#endif
				my_dump_str_data(APP_DUMP_EN, "Master 0 Connect", &pConnEvt->connHandle, 2);
			}
			else if((pConnEvt->connHandle&0x0F) == 1){
				gAppmAclConnHandle[1] = pConnEvt->connHandle;
				blc_l2cap_cocConnectHandler(gAppmAclConnHandle[1]);
				#if (UI_LED_ENABLE)
				gpio_write(GPIO_LED_GREEN, 1);
				#endif
				my_dump_str_data(APP_DUMP_EN, "Master 1 Connect", &pConnEvt->connHandle, 2);
			}
			gAppmAclConnNumber ++;
			if(gAppmAclConnHandle[0] == 0 || gAppmAclConnHandle[1] == 0){
				blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
			}else{
				blc_ll_setScanEnable(BLC_SCAN_DISABLE, DUP_FILTER_DISABLE);
			}
			#endif
		}
		else if( pConnEvt->role == LL_ROLE_SLAVE ){
			#if (UI_LED_ENABLE)
				if( dev_char_get_conn_role_by_connhandle(pConnEvt->connHandle) == LL_ROLE_SLAVE){
					//gpio_write(GPIO_LED_RED, 1);
				}
			#endif
			
			#if (APP_SLAVE_ENABLE)
				gAppsAclConnHandle = pConnEvt->connHandle;
				blc_l2cap_cocConnectHandler(gAppsAclConnHandle);
				#if (UI_LED_ENABLE)
				gpio_write(GPIO_LED_BLUE, 1);
				#endif
				my_dump_str_data(APP_DUMP_EN, "Slave Connect", &pConnEvt->connHandle, 2);
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
	event_disconnection_t	*pDisEvt = (event_disconnection_t *)p;

	//terminate reason
	if(pDisEvt->reason == HCI_ERR_CONN_TIMEOUT){  	//connection timeout

	}
	else if(pDisEvt->reason == HCI_ERR_REMOTE_USER_TERM_CONN){  	//peer device send terminate command on link layer

	}
	//master host disconnect( blm_ll_disconnect(current_connHandle, HCI_ERR_REMOTE_USER_TERM_CONN) )
	else if(pDisEvt->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){

	}
	else{

	}

	//if previous connection SMP & SDP not finished, clear flag
#if (BLE_MASTER_SMP_ENABLE)
	if(master_smp_pending == pDisEvt->connHandle){
		master_smp_pending = 0;
	}
#endif
#if (BLE_MASTER_SIMPLE_SDP_ENABLE)
	if(master_sdp_pending == pDisEvt->connHandle){
		master_sdp_pending = 0;
	}
#endif

	if(master_disconnect_connhandle == pDisEvt->connHandle){  //un_pair disconnection flow finish, clear flag
		master_disconnect_connhandle = 0;
	}

	dev_char_info_delete_by_connhandle(pDisEvt->connHandle);

	#if (APP_MASTER_ENABLE)
	if(pDisEvt->connHandle == gAppmAclConnHandle[0] || pDisEvt->connHandle == gAppmAclConnHandle[1]){
		if(gAppmAclConnNumber != 0) gAppmAclConnNumber--;
		if(pDisEvt->connHandle == gAppmAclConnHandle[0]){
			gAppmCocIsReady[0] = 0;
			gAppmCocSendTimer[0] = 0;
			gAppmAclConnHandle[0] = 0;
			blc_l2cap_cocDisconnHandler(gAppmAclConnHandle[0]);
			#if (UI_LED_ENABLE)
			gpio_write(GPIO_LED_BLUE, 0);
			gpio_write(GPIO_LED_WHITE, 0);
			#endif
			my_dump_str_data(APP_DUMP_EN, "Master 0 Disconn", &pDisEvt->connHandle, 2);
		}else{
			gAppmCocIsReady[1] = 0;
			gAppmCocSendTimer[1] = 0;
			gAppmAclConnHandle[1] = 0;
			blc_l2cap_cocDisconnHandler(gAppmAclConnHandle[1]);
			#if (UI_LED_ENABLE)
			gpio_write(GPIO_LED_GREEN, 0);
			gpio_write(GPIO_LED_RED, 0);
			#endif
			my_dump_str_data(APP_DUMP_EN, "Master 1 Disconn", &pDisEvt->connHandle, 2);
		}
		if(gAppmAclConnNumber < 2){
			blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
		}else{
			blc_ll_setScanEnable(BLC_SCAN_DISABLE, DUP_FILTER_DISABLE);
		}
		#if (UI_LED_ENABLE)
			gpio_write(led_map[pDisEvt->connHandle&0x0F], 0);
		#endif
	}
	#endif
	#if (APP_SLAVE_ENABLE)
	if(pDisEvt->connHandle == gAppsAclConnHandle){
		#if (UI_LED_ENABLE)
			gpio_write(GPIO_LED_BLUE, 0);
			gpio_write(GPIO_LED_GREEN, 0);
		#endif
		gAppsAclConnHandle = 0;
		gAppsCocIsReady = false;
		gAppsCocSendTimer = 0;
		blc_l2cap_cocDisconnHandler(gAppsAclConnHandle);
		my_dump_str_data(APP_DUMP_EN, "Slave Disconn", &pDisEvt->connHandle, 2);
	}
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

//////////////////////////////////////////////////////////
// event call back
//////////////////////////////////////////////////////////
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

	}
	else{   //GATT data for Slave


	}


	return 0;
}


int app_l2cap_coc_evtcb(u16 evtID, u8 *pData, u16 dataLen)
{
	my_dump_str_data(APP_DUMP_EN, "[EVT]app_l2cap_coc_evtcb:", &dataLen, 2);
	if(evtID == GAP_EVT_L2CAP_LE_CREDIT_BASED_CONNECT){
		u8 buff[8] = {0};
		gap_l2cap_leCreditBasedConnectEvt_t *pEvt = (gap_l2cap_leCreditBasedConnectEvt_t*)pData;
		buff[0] = pEvt->connHandle & 0xFF;
		buff[1] = (pEvt->connHandle & 0xFF00) >> 8;
		buff[2] = pEvt->result & 0xFF;
		buff[3] = (pEvt->result & 0xFF00) >> 8;
		buff[4] = pEvt->reason & 0xFF;
		buff[5] = (pEvt->reason & 0xFF00) >> 8;
		buff[6] = pEvt->srv_num;
		my_dump_str_data(APP_DUMP_EN, "     LE_CREDIT_BASED_CONNECT:", buff, 7);
		if(pEvt->result != 0) return -1;
		#if (APP_MASTER_ENABLE)
			if(pEvt->connHandle == gAppmAclConnHandle[0]){
				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_WHITE, 1);
				#endif
				gAppmCocIsReady[0] = 1;
				gAppmCocSendTimer[0] = clock_time()|1;
				gAppmCocSendScid[0] = pEvt->scid;
			}
			if(pEvt->connHandle == gAppmAclConnHandle[1]){
				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_RED, 1);
				#endif
				gAppmCocIsReady[1] = 1;
				gAppmCocSendTimer[1] = clock_time()|1;
				gAppmCocSendScid[1] = pEvt->scid;
			}
		#endif
		#if (APP_SLAVE_ENABLE)
			if(pEvt->connHandle == gAppsAclConnHandle){
				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_GREEN, 1);
				#endif
				gAppsCocIsReady = true;
				gAppsCocSendTimer = clock_time()|1;
				gAppsCocSendScid = pEvt->scid;
			}
		#endif
	}
	else if(evtID == GAP_EVT_L2CAP_CREDIT_BASED_CONNECT){
		u8 buff[8] = {0};
		gap_l2cap_creditBasedConnectEvt_t *pEvt = (gap_l2cap_creditBasedConnectEvt_t*)pData;
		buff[0] = pEvt->connHandle & 0xFF;
		buff[1] = (pEvt->connHandle & 0xFF00) >> 8;
		buff[2] = pEvt->result & 0xFF;
		buff[3] = (pEvt->result & 0xFF00) >> 8;
		buff[4] = pEvt->reason & 0xFF;
		buff[5] = (pEvt->reason & 0xFF00) >> 8;
		buff[6] = pEvt->srv_num;
		buff[7] = pEvt->cid_count;
		my_dump_str_data(APP_DUMP_EN, "     CREDIT_BASED_CONNECT:",  buff, 8);
		if(pEvt->result != 0) return -1;
		#if (APP_MASTER_ENABLE)
			if(pEvt->connHandle == gAppmAclConnHandle[0]){
				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_WHITE, 1);
				#endif
				gAppmCocIsReady[0] = 1;
				gAppmCocSendTimer[0] = clock_time()|1;
				gAppmCocSendScid[0] = pEvt->scid[0];
			}
			if(pEvt->connHandle == gAppmAclConnHandle[1]){
				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_RED, 1);
				#endif
				gAppmCocIsReady[1] = 1;
				gAppmCocSendTimer[1] = clock_time()|1;
				gAppmCocSendScid[1] = pEvt->scid[0];
			}
		#endif
		#if (APP_SLAVE_ENABLE)
			if(pEvt->connHandle == gAppsAclConnHandle){
				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_GREEN, 1);
				#endif
				gAppsCocIsReady = true;
				gAppsCocSendTimer = clock_time()|1;
				gAppsCocSendScid = pEvt->scid[0];
			}
		#endif
	}
	else if(evtID == GAP_EVT_L2CAP_CREDIT_BASED_RECONFIG){
		my_dump_str_data(APP_DUMP_EN, "     CREDIT_BASED_RECONFIG:",  0, 0);
	}
	else if(evtID == GAP_EVT_L2CAP_FLOW_CONTROL_CREDIT){
		my_dump_str_data(APP_DUMP_EN, "     FLOW_CONTROL_CREDIT:",  0, 0);
	}
	else if(evtID == GAP_EVT_L2CAP_DISCONNECT){
		u8 buff[6];
		gap_l2cap_disconnectEvt_t *pEvt = (gap_l2cap_disconnectEvt_t*)pData;
		buff[0] = pEvt->spsm & 0xFF;
		buff[1] = (pEvt->spsm & 0xFF00) >> 8;
		buff[2] = pEvt->scid & 0xFF;
		buff[3] = (pEvt->scid & 0xFF00) >> 8;
		buff[4] = pEvt->dcid & 0xFF;
		buff[5] = (pEvt->dcid & 0xFF00) >> 8;
		my_dump_str_data(APP_DUMP_EN, "     DISCONNECT:",  buff, 6);
	}
	else if(evtID == GAP_EVT_L2CAP_COC_DATA){
		my_dump_str_data(APP_DUMP_EN, "     COC_DATA:",  0, 0);
	}

	return 0;
}
int app_l2cap_coc_datacb(u16 aclHandle, u16 scid, u16 dcid, u16 dataLen, u8 *pData)
{
	u8 buff[12];
	
	buff[0] = scid & 0xFF;
	buff[1] = (scid & 0xFF00) >> 8;
	buff[2] = dcid & 0xFF;
	buff[3] = (dcid & 0xFF00) >> 8;
	buff[4] = dataLen & 0xFF;
	buff[5] = (dataLen & 0xFF00) >> 8;
	buff[6] = pData[0];
	buff[7] = pData[1];
	buff[8] = pData[2];
	buff[9] = pData[3];
	buff[10] = pData[4];
	#if (APP_MASTER_ENABLE)
		if(aclHandle == gAppmAclConnHandle[0]){
			my_dump_str_data(APP_DUMP_EN, "[RECV]MASTER 0 DATA:",  buff, 11);
		}
		if(aclHandle == gAppmAclConnHandle[1]){
			my_dump_str_data(APP_DUMP_EN, "[RECV]MASTER 1 DATA:",  buff, 11);
		}
	#endif
	#if (APP_SLAVE_ENABLE)
		if(aclHandle == gAppsAclConnHandle){
			my_dump_str_data(APP_DUMP_EN, "[RECV]SLAVE DATA:",  buff, 11);
		}
	#endif
	
	return 0;
}

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

	//////////// HCI Initialization  Begin /////////////////////////
	blc_hci_registerControllerDataHandler (blc_l2cap_pktHandler);

	blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(		HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
									|	HCI_LE_EVT_MASK_ADVERTISING_REPORT \
									|   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE);


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

	/* L2CAP Initialization */
	blc_l2cap_initAclConnMasterMtuBuffer(mtu_m_rx_fifo, MTU_M_BUFF_SIZE_MAX, 			0,					 0);
	blc_l2cap_initAclConnSlaveMtuBuffer(mtu_s_rx_fifo, MTU_S_BUFF_SIZE_MAX, mtu_s_tx_fifo, MTU_S_BUFF_SIZE_MAX);

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
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE    |  \
						  GAP_EVT_MASK_L2CAP_LE_CREDIT_BASED_CONNECT | \
						  GAP_EVT_MASK_L2CAP_CREDIT_BASED_CONNECT    | \
						  GAP_EVT_MASK_L2CAP_DISCONNECT              | \
						  GAP_EVT_MASK_L2CAP_COC_DATA);
	//////////// Host Initialization  End /////////////////////////

//////////////////////////// BLE stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for BLE application ////////////////////////////
#if (APP_SLAVE_ENABLE)
	blc_ll_setAdvData((u8 *)tbl_advData, sizeof(tbl_advData));
	blc_ll_setScanRspData((u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
	blc_ll_setAdvParam(ADV_INTERVAL_30MS, ADV_INTERVAL_30MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
	blc_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable
#endif
#if (APP_MASTER_ENABLE)
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_200MS, SCAN_WINDOW_200MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
#endif
////////////////////////////////////////////////////////////////////////////////////////////////

#if (APP_MASTER_ENABLE)
	blc_l2cap_cocInit(APPM_COC_CONN_COUNT, APPM_COC_SERVICE_COUNT, APPM_COC_CHANNEL_COUNT,
		gAppmL2capCocBuffer, APPM_COC_BUFFER_LENGTH);
	blc_l2cap_registerService(APP_COC_SPSM, APPM_COC_SERVICE_COUNT, APPM_COC_CH_NUM_PER_SERV,
		APP_COC_MTU_SIZE, APP_COC_MPS_SIZE, APPM_COC_INIT_CREDIT, gAppmL2capMtuBuffer,
		app_l2cap_coc_evtcb, app_l2cap_coc_datacb);
	my_dump_str_data(APP_DUMP_EN, "Master Init Success!",  0, 0);
	{
		u16 i;
		for(i=5; i<APPM_COC_SEND_LENGTH; i++){
			gAppmCocData0[i] = 0x45+i;
			gAppmCocData1[i] = 0x65+i;
		}
		gAppmCocSendSerial0 = 0;
		gAppmCocData0[0] = 0XFF;
		gAppmCocData0[1] = gAppmCocSendSerial0 & 0xFF;
		gAppmCocData0[2] = (gAppmCocSendSerial0 & 0xFF00)>>8;
		gAppmCocData0[3] = (gAppmCocSendSerial0 & 0xFF0000)>>16;
		gAppmCocData0[4] = (gAppmCocSendSerial0 & 0xFF000000)>>24;
		gAppmCocSendSerial1 = 0;
		gAppmCocData1[0] = 0XFF;
		gAppmCocData1[1] = gAppmCocSendSerial1 & 0xFF;
		gAppmCocData1[2] = (gAppmCocSendSerial1 & 0xFF00)>>8;
		gAppmCocData1[3] = (gAppmCocSendSerial1 & 0xFF0000)>>16;
		gAppmCocData1[4] = (gAppmCocSendSerial1 & 0xFF000000)>>24;
	}
#endif
#if (APP_SLAVE_ENABLE)
	blc_l2cap_cocInit(APPS_COC_CONN_COUNT, APPS_COC_SERVICE_COUNT, APPS_COC_CHANNEL_COUNT,
		gAppsL2capCocBuffer, APPS_COC_BUFFER_LENGTH);
	blc_l2cap_registerService(APP_COC_SPSM, APPS_COC_SERVICE_COUNT, APPS_COC_CH_NUM_PER_SERV,
		APP_COC_MTU_SIZE, APP_COC_MPS_SIZE, APPS_COC_INIT_CREDIT, gAppsL2capMtuBuffer,
		app_l2cap_coc_evtcb, app_l2cap_coc_datacb);
	my_dump_str_data(APP_DUMP_EN, "Slave Init Success!",  0, 0);
	{
		u16 i;
		for(i=4; i<APPS_COC_SEND_LENGTH; i++){
			gAppsCocData[i] = 0x30+i;
		}
		gAppsCocSendSerial = 0;
		gAppsCocData[0] = 0XFF;
		gAppsCocData[1] = gAppsCocSendSerial & 0xFF;
		gAppsCocData[2] = (gAppsCocSendSerial & 0xFF00)>>8;
		gAppsCocData[3] = (gAppsCocSendSerial & 0xFF0000)>>16;
		gAppsCocData[4] = (gAppsCocSendSerial & 0xFF000000)>>24;
	}
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

	if(gAppStartCreateCOC){
		#if (APP_MASTER_ENABLE)
		gAppStartCreateCOC = false;
		if(gAppmAclConnHandle[0] != 0 && gAppmCocIsReady[0] == 0){
			//my_dump_str_data(APP_DUMP_EN, "Master 0 Start COC ...!",  0, 0);
			if(blc_l2cap_startService(gAppmAclConnHandle[0], APP_COC_SPSM, APP_COC_METHOD) == 0){
				gAppStartCreateCOC = false;
				//my_dump_str_data(APP_DUMP_EN, "Master 0 Start COC Success!",  0, 0);
			}
		}
		if(gAppmAclConnHandle[1] != 0 && gAppmCocIsReady[1] == 0){
			//my_dump_str_data(APP_DUMP_EN, "Master 1 Start COC ...!",  0, 0);
			if(blc_l2cap_startService(gAppmAclConnHandle[1], APP_COC_SPSM, APP_COC_METHOD) == 0){
				gAppStartCreateCOC = false;
				//my_dump_str_data(APP_DUMP_EN, "Master 1 Start COC Success!",  0, 0);
			}
		}
		#endif
	}
	#if (APPM_COC_SEND_ENABLE)
		if(gAppmAclConnHandle[0] != 0 && gAppmCocIsReady[0]){
			if(gAppmCocSendTimer[0] != 0 && clock_time_exceed(gAppmCocSendTimer[0], APPM_COC_SEND_INTERVAL)){
				gAppmCocSendTimer[0] = clock_time()|1;
				//my_dump_str_data(APP_DUMP_EN, "Master 0 Send Data ...",  0, 0);
				if(blc_l2cap_cocSendData(gAppmAclConnHandle[0], gAppmCocSendScid[0], NULL, 0, gAppmCocData0, APPM_COC_SEND_LENGTH) == BLE_SUCCESS){
					gAppmCocSendSerial0 ++;
					gAppmCocData0[0] = 0XFF;
					gAppmCocData0[1] = gAppmCocSendSerial0 & 0xFF;
					gAppmCocData0[2] = (gAppmCocSendSerial0 & 0xFF00)>>8;
					gAppmCocData0[3] = (gAppmCocSendSerial0 & 0xFF0000)>>16;
					gAppmCocData0[4] = (gAppmCocSendSerial0 & 0xFF000000)>>24;
					my_dump_str_data(APP_DUMP_EN, "Master 0 Send Data",  &gAppmCocSendSerial0, 4);
				}
			}
		}
		if(gAppmAclConnHandle[1] != 0 && gAppmCocIsReady[1]){
			if(gAppmCocSendTimer[1] != 0 && clock_time_exceed(gAppmCocSendTimer[1], APPM_COC_SEND_INTERVAL)){
				gAppmCocSendTimer[1] = clock_time()|1;
				//my_dump_str_data(APP_DUMP_EN, "Master 1 Send Data ...",  0, 0);
				if(blc_l2cap_cocSendData(gAppmAclConnHandle[1], gAppmCocSendScid[1], NULL, 0, gAppmCocData1, APPM_COC_SEND_LENGTH) == BLE_SUCCESS){
					gAppmCocSendSerial1 ++;
					gAppmCocData1[0] = 0XFF;
					gAppmCocData1[1] = gAppmCocSendSerial1 & 0xFF;
					gAppmCocData1[2] = (gAppmCocSendSerial1 & 0xFF00)>>8;
					gAppmCocData1[3] = (gAppmCocSendSerial1 & 0xFF0000)>>16;
					gAppmCocData1[4] = (gAppmCocSendSerial1 & 0xFF000000)>>24;
					my_dump_str_data(APP_DUMP_EN, "Master 1 Send Data",  &gAppmCocSendSerial1, 4);
				}
			}
		}
	#endif
	#if (APPS_COC_SEND_ENABLE)
		if(gAppsAclConnHandle != 0 && gAppsCocIsReady){
			if(gAppsCocSendTimer != 0 && clock_time_exceed(gAppsCocSendTimer, APPS_COC_SEND_INTERVAL)){
				gAppsCocSendTimer = clock_time()|1;
				my_dump_str_data(APP_DUMP_EN, "Slave Send Data ...",  0, 0);
				if(blc_l2cap_cocSendData(gAppsAclConnHandle, gAppsCocSendScid, NULL, 0, gAppsCocData, APPS_COC_SEND_LENGTH) == BLE_SUCCESS){
					gAppsCocSendSerial ++;
					gAppsCocData[0] = 0XFF;
					gAppsCocData[1] = gAppsCocSendSerial & 0xFF;
					gAppsCocData[2] = (gAppsCocSendSerial & 0xFF00)>>8;
					gAppsCocData[3] = (gAppsCocSendSerial & 0xFF0000)>>16;
					gAppsCocData[4] = (gAppsCocSendSerial & 0xFF000000)>>24;
					my_dump_str_data(APP_DUMP_EN, "Slave Send Data success",  &gAppsCocSendSerial, 4);
				}
			}
		}
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

#endif


