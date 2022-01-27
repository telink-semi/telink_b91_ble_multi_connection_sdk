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
#include "app_ui.h"
#include "app_audio.h"
#include "app_usb.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#if (LE_AUDIO_DEMO_ENABLE)
	#define		TEST_IAL_EN					(0)
#else
	#define		TEST_IAL_EN					(1)
#endif

#define  	CIS_INTERVEEL_MS			(10)
#define 	ISO_DL_LEN					(100)
#define 	UNFRAMDE_PACKET_TEST_EN		(1)

int	master_smp_pending = 0; 		// SMP: security & encryption;
int	master_sdp_pending = 0;			// SDP: service discovery


u16 app_aclConnHandle;
u16 app_cisConnHandle;



const u8	tbl_advData[] = {
	 8,  DT_COMPLETE_LOCAL_NAME, 				'c','i','s','_','s','l','v',
	 2,	 DT_FLAGS, 								0x05, 					// BLE limited discoverable mode and BR/EDR not supported
	 3,  DT_APPEARANCE, 						0x80, 0x01, 			// 384, Generic Remote Control, Generic category
	 5,  DT_INCOMPLT_LIST_16BIT_SERVICE_UUID,	0x12, 0x18, 0x0F, 0x18,	// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 8, DT_COMPLETE_LOCAL_NAME, 				'c','i','s','_','s','l','v',
};


/**
 * @brief      BLE Connection complete event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_le_connection_complete_event_handle(u8 *p)
{
	hci_le_connectionCompleteEvt_t *pConnEvt = (hci_le_connectionCompleteEvt_t *)p;

	if(pConnEvt->status == BLE_SUCCESS){

		my_dump_str_data(APP_DUMP_EN, "acl handle", &pConnEvt->connHandle, 2);
		#if (UI_LED_ENABLE)
			if( dev_char_get_conn_role_by_connhandle(pConnEvt->connHandle) == LL_ROLE_SLAVE){
				gpio_write(GPIO_LED_RED, 1);
			}
		#endif

		dev_char_info_insert_by_conn_event(pConnEvt);

		#if (UI_LED_ENABLE)
			gpio_write(GPIO_LED_BLUE, 1);
		#endif

		#if (LE_AUDIO_DEMO_ENABLE)
			app_aclConnHandle = pConnEvt->connHandle;
			app_audio_acl_connect(pConnEvt->connHandle, p, false);
		#endif
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

#if 0
	if(pCon->connHandle & BLT_ACL_CONN_HANDLE){
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

		dev_char_info_delete_by_connhandle(pCon->connHandle);

		#if (UI_LED_ENABLE)
			gpio_write(GPIO_LED_BLUE, 0);
			gpio_write(GPIO_LED_GREEN, 0);
		#endif

		#if (LE_AUDIO_DEMO_ENABLE)
			app_audio_acl_disconn(pCon->connHandle, p);
			app_cisConnHandle = 0;
			app_aclConnHandle = 0;
		#endif
	}
	else if(pCon->connHandle & BLT_CIS_CONN_HANDLE){

		app_cisConnHandle = 0;
		#if (LE_AUDIO_DEMO_ENABLE)
			app_audio_cis_disconn(pCon->connHandle, p);
		#endif

		gpio_write(GPIO_LED_WHITE, 0);
	}


	#if (LE_AUDIO_DEMO_ENABLE)
		app_audio_enable(false);
	#endif

#endif

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

			}
			//------hci le event: le connection update complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)	// connection update
			{

			}
			//------HCI LE event: LE CIS Established event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CIS_ESTABLISHED)
			{
				hci_le_cisEstablishedEvt_t *pCisEstbEvent = (hci_le_cisEstablishedEvt_t *)p;

				#if !(LE_AUDIO_DEMO_ENABLE)
				if(pCisEstbEvent->cisHandle == app_cisConnHandle){
					#if (UI_LED_ENABLE)
						gpio_write(GPIO_LED_WHITE, 1);
					#endif
					#if (LE_AUDIO_DEMO_ENABLE)
						app_audio_enable(true);
					#endif
				}
				#else
				app_audio_cis_connect(pCisEstbEvent->cisHandle, p);
				app_cisConnHandle = pCisEstbEvent->cisHandle;
				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_WHITE, 1);
				#endif
				#endif
			}
			//------HCI LE event: LE CIS Request event -------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CIS_REQUESTED)	// connection update
			{
				hci_le_cisReqEvt_t *pCisReqEvt = (hci_le_cisReqEvt_t *)p;

				#if !(LE_AUDIO_DEMO_ENABLE)
				if( dev_char_info_is_connection_state_by_conn_handle(pCisReqEvt->aclHandle) ){
					//TODO: accept condition should be more complex
					app_cisConnHandle = pCisReqEvt->cisHandle;
					blc_hci_le_acceptCisReq(pCisReqEvt->cisHandle);
				}
				#else
				app_cisConnHandle = pCisReqEvt->cisHandle;
				app_audio_cis_request(pCisReqEvt->aclHandle, pCisReqEvt->cisHandle, p);
				#endif
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
			gap_smp_pairingBeginEvt_t *p = (gap_smp_pairingBeginEvt_t *)para;
			(void)p; //remove compiler warning
		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;
			(void)p; //remove compiler warning
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			gap_smp_pairingFailEvt_t* p = (gap_smp_pairingFailEvt_t*)para;
			(void)p; //remove compiler warning
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{

		}
		break;

		case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
			(void)p; //remove compiler warning

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
#if (LE_AUDIO_DEMO_ENABLE)
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

			1,2,3,4,5,6,7,8,9,0,
	};
	u32 sdu_tick = 0;

	u32 send_cnt = 0;
	extern iso_sdu_mng_t sduCisMng;
#endif

_attribute_ram_code_ void app_timer_test_irq_proc(void)
{
#if (TEST_IAL_EN)

	for(int cis_conn_idx=bltCisMng.maxNum_cisMaster; cis_conn_idx<bltCisMng.maxNum_cisSlave; cis_conn_idx++)
	{
		ll_cis_conn_t * cur_pCisConn = (ll_cis_conn_t *) (global_pCisConn + cis_conn_idx);

//		ll_cis_conn_t * cur_pCisConn = (ll_cis_conn_t *) (global_pCisConn + 0);
//		ll_iso_sud_t *pSdu =(ll_iso_sud_t *) sdu;
		if((cur_pCisConn->connState == CONN_STATUS_ESTABLISH) &&(cur_pCisConn->cisEventCnt>=10) )//&& (send_cnt<3)
		{
			send_cnt ++;
			//DBG_CHN15_TOGGLE;
		#if	(UNFRAMDE_PACKET_TEST_EN)
			blc_ial_cis_splitSdu2UnframedPdu(cur_pCisConn->cis_connHandle, (iso_data_packet_t*)sdu);
		#else
			u8 *p = (sduCisMng.tx_fifo_b + sduCisMng.max_tx_size * \
										((cur_pCisConn->txSduFifoWptr&(sduCisMng.tx_fifo_num-1))));
			sdu[20]++;
			smemcpy(p, sdu, MAX_TX_SDU_LEN);
			cur_pCisConn->txSduFifoWptr++;
		#endif
			//DBG_CHN15_TOGGLE;
		}
	}
#endif
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

	blc_ll_setAclMasterConnectionInterval(CONN_INTERVAL_31P25MS);

	rf_set_power_level_index (RF_POWER_P3dBm);

	//////////// LinkLayer Initialization  End /////////////////////////


	///// CIG Slave initialization //////////////
	blc_ll_initCisSlave_module();

	blc_ial_initSdu_module(); //IAL module register

	blc_ll_initCisSlaveParameters(app_cig_slave_param, APP_CIG_SLV_NUMBER);

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
									| 	HCI_LE_EVT_MASK_CIS_REQUESTED );


	u8 check_status = blc_controller_check_appBufferInitialization();
	if(check_status != BLE_SUCCESS){
		/* here user should set some log to know which application buffer incorrect*/
		write_log32(0x88880000 | check_status);
		while(1);
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
	blc_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	blc_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
	blc_ll_setAdvParam(ADV_INTERVAL_30MS, ADV_INTERVAL_30MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
	blc_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable

	#if 0
		blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
		blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
	#endif
	////////////////////////////////////////////////////////////////////////////////////////////////

#if (TEST_IAL_EN)

	for(u16 i=0; i< ISO_DL_LEN; i++)
	{
		sdu[20+i] = i;
	}

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
#if (APP_AUDIO_USB_ENABLE)
	app_usb_init();
	//plic_set_priority(IRQ11_USB_ENDPOINT, 0);
#endif
	my_dump_str_data(APP_DUMP_EN, "app_init_success!", NULL, 0);
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
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
_attribute_no_inline_ void main_loop (void)
{
	////////////////////////////////////// BLE entry /////////////////////////////////
	blc_sdk_main_loop();

#if (APP_AUDIO_USB_ENABLE)
	app_usb_handler();
#endif
#if (LE_AUDIO_DEMO_ENABLE)
	app_audio_handler();
#endif
	////////////////////////////////////// UI entry /////////////////////////////////
	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard (0,0, 0);
	#endif

	#if (TEST_IAL_EN&&0)
		if(1 && clock_time_exceed(sdu_tick, CIS_INTERVEEL_MS * 1000)){
			sdu_tick = clock_time();
			app_timer_test_irq_proc();
		}
	#endif
}





