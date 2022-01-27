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
#include "app_audio.h"
#include "app_buffer.h"
#include "app_config.h"
#include "app_ui.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app_att.h"


#if (__PROJECT_B91_AUDIO_BIS_BCST__)




#define 	TEST_IAL_EN					(!LE_AUDIO_DEMO_ENABLE)
#define		BIS_INTERVEEL_MS			(10) //LC3 10ms
#define 	ISO_DL_LEN					(40) //LC3 10ms-> 40B
#define 	UNFRAMDE_PACKET_TEST_EN		(0)
#define 	BIS_ENCRYPTION_EN			(0)


int	master_smp_pending = 0; 		// SMP: security & encryption;




const u8	tbl_advData[] = {
	 9,  DT_COMPLETE_LOCAL_NAME, 				'b','i','s','_','b','c','s', 't',
	 2,	 DT_FLAGS, 								0x05, 					// BLE limited discoverable mode and BR/EDR not supported
	 3,  DT_APPEARANCE, 						0x80, 0x01, 			// 384, Generic Remote Control, Generic category
	 5,  DT_INCOMPLT_LIST_16BIT_SERVICE_UUID,	0x12, 0x18, 0x0F, 0x18,	// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 9,  DT_COMPLETE_LOCAL_NAME, 				'b','i','s','_','b','c','s', 't',
};

u8	app_bisBcstNum = 0;
u16 app_bisBcstHandle[APP_BIS_NUM_IN_PER_BIG_BCST] = {0};


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
			#if (BLE_MASTER_SMP_ENABLE)
				master_smp_pending = pConnEvt->connHandle; // this connection need SMP
			#else

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

	dev_char_info_delete_by_connhandle(pCon->connHandle);

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
			//------hci le event: le create BIG complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CREATE_BIG_COMPLETE)	// create BIG complete
			{
				hci_le_createBigCompleteEvt_t* pEvt = (hci_le_createBigCompleteEvt_t*)p;

				if(pEvt->status == BLE_SUCCESS){
					my_dump_str_data(APP_DUMP_EN,"Create BIG complete succeed", 0, 0);

					app_bisBcstNum = pEvt->numBis;
					my_dump_str_data(APP_DUMP_EN,"app_bisBcstNum", &app_bisBcstNum, 1);

					for(int i = 0; i < pEvt->numBis; i++){
						app_bisBcstHandle[i] = pEvt->bisHandles[i];
						my_dump_str_data(APP_DUMP_EN,"app_bisBcstHandle", &app_bisBcstHandle[i], 2);
					}
				}
				else{
					my_dump_str_data(APP_DUMP_EN,"Create BIG complete failed", &pEvt->status, 1);
				}
			}
			//------hci le event: le terminate BIG complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_TERMINATE_BIG_COMPLETE)	// terminate  BIG complete
			{
				hci_le_terminateBigCompleteEvt_t* pEvt = (hci_le_terminateBigCompleteEvt_t*)p;
				my_dump_str_data(APP_DUMP_EN,"terminate  BIG complete", &pEvt->bigHandle, 1);
				my_dump_str_data(APP_DUMP_EN,"		terminate  reason", &pEvt->reason, 1);
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
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{

		}
		break;

		case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
		{
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

	if( dev_char_get_conn_role_by_connhandle(connHandle) == LL_ROLE_MASTER)   //GATT data for Master
	{
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

		ll_bis_t* cur_pBis = NULL;
		ll_big_bcst_t* pBig = NULL;

		for(int bis_idx=0; bis_idx<bltBisMng.maxNum_bisTotal; bis_idx++)
		{
			cur_pBis = (ll_bis_t *) (global_pBis + bis_idx);
			pBig =(ll_big_bcst_t *) (global_pBigBcst + cur_pBis->big_idx);



			if((cur_pBis->bis_occupied)&&(pBig->cmd_status == BIG_CREATE_COMPLETE)&& (cur_pBis->curSendPldNum>100))//(send_cnt<10))//
			{
				send_cnt ++;
				*(u32*)&sdu[20] = send_cnt;
				DBG_CHN13_TOGGLE;
			#if	(UNFRAMDE_PACKET_TEST_EN)
				blc_ial_bis_splitSdu2UnframedPdu(cur_pBis->bis_handle, (iso_data_packet_t*)sdu);
			#else
				extern iso_sdu_mng_t sduBisMng;
				u8 *p = (sduBisMng.tx_fifo_b + sduBisMng.max_tx_size * \
											(bis_idx*sduBisMng.tx_fifo_num + (cur_pBis->txSduFifoWptr&(sduBisMng.tx_fifo_num-1))));

//				sdu[20]++;
				smemcpy(p, sdu, MAX_TX_SDU_LEN);
				cur_pBis->txSduFifoWptr++;
//
//				my_dump_str_data(DBG_IAL_EN,"txSduFifoWptr", &cur_pBis->txSduFifoWptr, 1);
//				my_dump_str_u32s(DBG_IAL_EN,"txSduFifo",cur_pBis->txSduFifoWptr,cur_pBis->txSduFifoRptr,0,0);
			#endif
				//DBG_CHN13_TOGGLE;
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

	////////////// 2M or coded PHY register ///////////////
	blc_ll_init2MPhyCodedPhy_feature(); //if use 2M or Coded PHY
	//////////////// CSA#2 feature register ///////////////
	blc_ll_initChannelSelectionAlgorithm_2_feature(); //Periodic ADV must used!!! //ISO features must support CSA#2

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



	//////////// HCI Initialization  Begin /////////////////////////
	blc_hci_registerControllerDataHandler (blc_l2cap_pktHandler);

	blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(	HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
								|	HCI_LE_EVT_MASK_ADVERTISING_REPORT \
								|	HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE  \
								|	HCI_LE_EVT_MASK_CREATE_BIG_COMPLETE \
								|	HCI_LE_EVT_MASK_TERMINATE_BIG_COMPLETE );
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


	//////////// Extended ADV Initialization  Begin ///////////////////////
	blc_ll_initExtendedAdvertising_module();
	blc_ll_initExtendedAdvSetBuffer(app_advSet_buffer, APP_ADV_SETS_NUMBER);
	blc_ll_initExtAdvDataBuffer(app_advData_buffer, APP_MAX_LENGTH_ADV_DATA);
	blc_ll_initExtScanRspDataBuffer(app_scanRspData_buffer, APP_MAX_LENGTH_SCAN_RESPONSE_DATA);
	u32  my_adv_interval_min = ADV_INTERVAL_80MS;
	u32  my_adv_interval_max = ADV_INTERVAL_90MS;
	// Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
	blc_ll_setExtAdvParam( ADV_HANDLE0, 		ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED, my_adv_interval_min, 			my_adv_interval_max,
						   BLT_ENABLE_ADV_ALL,	OWN_ADDRESS_PUBLIC, 										    BLE_ADDR_PUBLIC, 				NULL,
						   ADV_FP_NONE,  		TX_POWER_3dBm,												   	BLE_PHY_1M, 					0,
						   BLE_PHY_1M, 	 		PRIVATE_EXT_FILTER_SPECIFIC_SID, 								0);
	for(int i=0;i<1024;i++){
		testAdvData[i] = i;
	}

#if(LL_FEATURE_PRIVATE_BIS_SYNC_RECEIVER)
	blc_ll_enPrivExtAdvForBigBcst(ADV_HANDLE0);	/// For private purposes ///
#endif
	blc_ll_setExtAdvData( ADV_HANDLE0,   sizeof(tbl_advData), (u8 *)tbl_advData);
//	blc_ll_setExtAdvData( ADV_HANDLE0, 	 200, testAdvData);
//	blc_ll_setExtAdvData( ADV_HANDLE0, 	 400, testAdvData);
//	blc_ll_setExtAdvData( ADV_HANDLE0, 	 600, testAdvData);
//	blc_ll_setExtAdvData( ADV_HANDLE0,  1024, testAdvData);
	blc_ll_setExtAdvEnable( BLC_ADV_ENABLE, ADV_HANDLE0, 0 , 0);
	//////////////// Extended ADV Initialization End ///////////////////////




	//////////// Periodic Adv Initialization  Begin ////////////////////////
	blc_ll_initPeriodicAdvertising_module();
	blc_ll_initPeriodicAdvParamBuffer(app_perdAdvSet_buffer, APP_PER_ADV_SETS_NUMBER);
	blc_ll_initPeriodicAdvDataBuffer(app_perdAdvData_buffer, APP_MAX_LENGTH_PER_ADV_DATA);
	u32  my_per_adv_itvl_min = PERADV_INTERVAL_100MS;
	u32  my_per_adv_itvl_max = PERADV_INTERVAL_100MS;
	blc_ll_setPeriodicAdvParam( ADV_HANDLE0, my_per_adv_itvl_min, my_per_adv_itvl_max, PERD_ADV_PROP_MASK_TX_POWER_INCLUDE);
	blc_ll_setPeriodicAdvData( ADV_HANDLE0, 20, testAdvData);
	blc_ll_setPeriodicAdvEnable( BLC_ADV_ENABLE, ADV_HANDLE0);
	//////////////// Periodic Adv Initialization End ///////////////////////




	/////////////////// BIS BCST initialization ////////////////////////
	blc_ll_initBigBroadcast_module();

	blc_ial_initSdu_module(); //IAL module register

	blc_ll_initBigBcstParameters(app_bigBcstParam, APP_BIG_BCST_NUMBER);

	blc_ll_InitBisConnectionlessParameters(app_bisToatlParam, APP_BIS_NUM_IN_ALL_BIG_BCST, APP_BIS_NUM_IN_ALL_BIG_SYNC);

	blc_setHostFeatureISOChannel_en(LL_FEATURE_ENABLE);

	/* BIS TX buffer init */
	blc_ll_initBisTxFifo(app_bisBcstTxfifo, MAX_TX_ISO_FIFO_SIZE, ISO_TX_FIFO_NUM);
	/* IAL SDU buff init */
	blc_ial_initBisSduBuff(NULL, 0, 0, bltSduTxFifo, MAX_TX_SDU_LEN, IAL_SDU_TX_FIFO_NUM);


	u8 big_create_buffer[sizeof(hci_le_createBigParamsTest_t)];
	hci_le_createBigParamsTest_t* pBigCreateTstParam = (hci_le_createBigParamsTest_t*)big_create_buffer;
	pBigCreateTstParam->big_handle = 0;									/* Used to identify the BIG */
	pBigCreateTstParam->adv_handle = ADV_HANDLE0;								/* Used to identify the periodic advertising train */
	pBigCreateTstParam->num_bis = min(1, APP_BIS_NUM_IN_PER_BIG_BCST);			/* Total number of BISes in the BIG */
	pBigCreateTstParam->sdu_intvl[0] = U32_BYTE0(1000*BIS_INTERVEEL_MS); 		/* The interval, in microseconds, of periodic SDUs */
	pBigCreateTstParam->sdu_intvl[1] = U32_BYTE1(1000*BIS_INTERVEEL_MS);
	pBigCreateTstParam->sdu_intvl[2] = U32_BYTE2(1000*BIS_INTERVEEL_MS);
	pBigCreateTstParam->iso_intvl = (1000*BIS_INTERVEEL_MS)/1250; //X*1.25ms;	/* The time between consecutive BIG anchor points */
	pBigCreateTstParam->nse = 6;												/* The total number of subevents in each interval of each BIS in the BIG */
	pBigCreateTstParam->max_sdu = ISO_DL_LEN;									/* Maximum size of an SDU, in octets */
	pBigCreateTstParam->max_pdu = ISO_DL_LEN; 									/* Maximum size, in octets, of payload */
	pBigCreateTstParam->phy = PHY_PREFER_1M;									/* The transmitter PHY of packets */
	pBigCreateTstParam->packing = PACK_SEQUENTIAL;
	pBigCreateTstParam->framing = UNFRAMDE_PACKET_TEST_EN ? UNFRAMED : FRAMED;
	pBigCreateTstParam->bn  = 3;												/* The number of new payloads in each interval for each BIS */
	pBigCreateTstParam->irc = 1;												/* The number of times the scheduled payload(s) are transmitted in a given event*/
	pBigCreateTstParam->pto = 2;												/* Offset used for pre-transmissions */
	pBigCreateTstParam->enc = BIS_ENCRYPTION_EN;								/* Encryption flag */
//	generateRandomNum(16, pBigCreateTstParam->broadcast_code);					/* The code used to derive the session key that is used to encrypt and decrypt BIS payloads */
	smemset(pBigCreateTstParam->broadcast_code, 0, 16);							/* TK: all zeros, just like JustWorks TODO: LE security mode 3, here use LE security mode 3 level2 */
	ble_sts_t status = blc_hci_le_createBigParamsTest(pBigCreateTstParam);
	my_dump_str_data(APP_DUMP_EN,"user_init", &status, 1);
	//////////////// BIS BCST Initialization End ///////////////////////


	u8 check_status = blc_controller_check_appBufferInitialization();
	if(check_status != BLE_SUCCESS){
		/* here user should set some log to know which application buffer incorrect*/
		write_log32(0x88880000 | check_status);
		while(1){
			myudb_usb_handle_irq();
		}
	}


	#if (TEST_IAL_EN)
		for(u16 i=0; i< ISO_DL_LEN; i++)
		{
			sdu[20+i] = i;
		}

		#if (1)//Attention: AES_CCM_Encryption in IRQ, AES_CCM_Decryption in main_loop maybe overlap!!!
			   //At present, the current CIS_CCM_NONCE is maintained before calling AES_CCM_ENC_V2, and
			   //the saved CIS_CCM_NONCE is restored after the encryption is over.
			/* So here you can open the macro for testing. Currently, the method of polling and sending ISO DATA in main_loop is used. */
			plic_set_priority(IRQ4_TIMER0, 1);
			timer_set_init_tick(TIMER0, 0);
			plic_interrupt_enable(IRQ4_TIMER0);
			timer_set_mode(TIMER0, TIMER_MODE_SYSCLK);
			timer_set_cap_tick(TIMER0, sys_clk.pclk*BIS_INTERVEEL_MS*1000);
			timer_start(TIMER0);
		#endif
	#endif

	#if (LE_AUDIO_DEMO_ENABLE)
		app_audio_init();
	#endif

	my_dump_str_data(APP_DUMP_EN,"user_init end", 0, 0);
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


	////////////////////////////////////// UI entry /////////////////////////////////
	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard (0,0, 0);
	#endif


	#if (LE_AUDIO_DEMO_ENABLE)
		app_audio_handler();
	#endif

}

#endif



