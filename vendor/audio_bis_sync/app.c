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
#include "app_usb.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "app_att.h"

#if (__PROJECT_B91_AUDIO_BIS_SYNC__)


int	big_sync_create = 0;
u8	app_bisSyncNum = 0;
u16 app_bisSyncHandle[APP_BIS_NUM_IN_PER_BIG_SYNC] = {0};
u16 app_sync_handle = 0xFFFF;
u8  app_ExtAdvId = 0xff;



u32	tick_legadv_rpt = 0;
/**
 * @brief      LE Extended Advertising report event handler
 * @param[in]  p - Pointer point to event parameter buffer.
 * @return
 */
int app_le_ext_adv_report_event_handle(u8 *p, int evt_data_len)
{


	hci_le_extAdvReportEvt_t *pExtAdvRpt = (hci_le_extAdvReportEvt_t *)p;

	int offset = 0;

	extAdvEvt_info_t *pExtAdvInfo = NULL;


	for(int i=0; i<pExtAdvRpt->num_reports ; i++)
	{

		pExtAdvInfo = (extAdvEvt_info_t *)(pExtAdvRpt->advEvtInfo + offset);
		offset += (EXTADV_INFO_LENGTH + pExtAdvInfo->data_length);

		/* Legacy */
		if(pExtAdvInfo->event_type & EXTADV_RPT_EVT_MASK_LEGACY)
		{
			if(clock_time_exceed(tick_legadv_rpt, 4000 * 1000)){
				//my_dump_str_data(APP_DUMP_EN, "leg adv rpt", pExtAdvInfo, EXTADV_INFO_LENGTH + pExtAdvInfo->data_length);
				tick_legadv_rpt = clock_time();
			}
		}
		/* Extended */
		else
		{
			if(pExtAdvInfo->perd_adv_inter != PERIODIC_ADV_INTER_NO_PERIODIC_ADV){

				#if 1
					if(app_ExtAdvId==pExtAdvInfo->advertising_sid)
						return 0;

					my_dump_str_data(APP_DUMP_EN, "creat PAD", &pExtAdvInfo->advertising_sid, 1);
					u8 status = blc_ll_periodicAdvertisingCreateSync(SYNC_ADV_SPECIFY | REPORTING_INITIALLY_EN, pExtAdvInfo->advertising_sid, \
																	 pExtAdvInfo->address_type, pExtAdvInfo->address, 0, SYNC_TIMEOUT_2S, 0);

					if(status != BLE_SUCCESS){
						my_dump_str_data(APP_DUMP_EN, "PAD SYNC fail", &status, 1);
					}
					else{
						my_dump_str_data(APP_DUMP_EN, "PAD SYNC start" , 0, 0);
						//blc_ll_setExtScanEnable( 0, DUP_FILTER_DISABLE, SCAN_DURATION_CONTINUOUS, SCAN_WINDOW_CONTINUOUS);
					}
				#endif
			}

		}
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
int AA_dbg_adv_rpt = 0;
u32	tick_adv_rpt = 0;

int app_controller_event_callback (u32 h, u8 *p, int n)
{
	if (h &HCI_FLAG_EVENT_BT_STD)		//Controller HCI event
	{
		u8 evtCode = h & 0xff;

		//------------ disconnect -------------------------------------
		if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
		{
		}
		else if(evtCode == HCI_EVT_LE_META)  //LE Event
		{
			u8 subEvt_code = p[0];

			//------hci le event: le connection complete event---------------------------------
			if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)	// connection complete
			{
			}
			//--------hci le event: le adv report event ----------------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// Report Leg ADV packet
			{
				#if 1  //debug, print ADV report number every 5 seconds
					event_adv_report_t *pa = (event_adv_report_t *)p;
					//s8 rssi = pa->data[pa->len];

					AA_dbg_adv_rpt ++;
					if(clock_time_exceed(tick_adv_rpt, 5000000)){
						my_dump_str_data(APP_DUMP_EN, "Leg_Advertising_Report", pa->mac, 6);
						tick_adv_rpt = clock_time();
					}
				#endif
			}
			else if(subEvt_code == HCI_SUB_EVT_LE_EXTENDED_ADVERTISING_REPORT) // Report Ext ADV packet
			{
				//Obtain AUX_SYNC_IND PDU, Step1: HCI_LE_Periodic_Advertising_Create_Sync
				app_le_ext_adv_report_event_handle(p, n);
			}
			else if(subEvt_code == HCI_SUB_EVT_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED)
			{
				//Step2: After receiving HCI_LE_Periodic_Advertising_Sync_Established Event(With Sync_Handle)
				hci_le_periodicAdvSyncEstablishedEvt_t *pEvt = (hci_le_periodicAdvSyncEstablishedEvt_t*)p;

				app_ExtAdvId = pEvt->advSID;
				my_dump_str_data(APP_DUMP_EN,"PAD ESTABLISHED, syncHandle", &pEvt->syncHandle, 2);
			}
			else if(subEvt_code == HCI_SUB_EVT_LE_PERIODIC_ADVERTISING_SYNC_LOST)
			{
				hci_le_periodicAdvSyncLostEvt_t *pEvt = (hci_le_periodicAdvSyncLostEvt_t*)p;
				app_ExtAdvId = 0xff;
				my_dump_str_data(APP_DUMP_EN,"PAD SYNC lost", &pEvt->syncHandle, 2);
			}
			else if(subEvt_code == HCI_SUB_EVT_LE_PERIODIC_ADVERTISING_REPORT)
			{
				//Step3: After receiving HCI_LE_Periodic_Advertising_Report Event
//				hci_le_periodicAdvReportEvt_t *pEvt = (hci_le_periodicAdvReportEvt_t*)p;
//				my_dump_str_data(APP_DUMP_EN,"Received HCI_LE_Periodic_Advertising_Report Event, syncHandle", &pEvt->syncHandle, 2);
			}
			else if(subEvt_code == HCI_SUB_EVT_LE_BIGINFO_ADVERTISING_REPORT){

				//Step4: After receiving HCI_LE_BIGInfo_Advertising_Report Event
				hci_le_bigInfoAdvReportEvt_t *pEvt = (hci_le_bigInfoAdvReportEvt_t *)p;

				if(app_sync_handle == pEvt->syncHandle){
					//my_dump_str_data(APP_DUMP_EN,"Report same sync_handle, skip big_create_sync", 0, 0);
					return 0;
				}

				if(!big_sync_create){

					my_dump_str_data(APP_DUMP_EN,"BIGInfo_Advertising_Report", p, sizeof(hci_le_bigInfoAdvReportEvt_t));

					//Step5: Send HCI_LE_BIG_Create_Sync(Sync_Handle)
					u8 bigCreateSyncBuff[sizeof(hci_le_bigCreateSyncParams_t)  + 4];
					hci_le_bigCreateSyncParams_t* pBigCreateSyncParam = (hci_le_bigCreateSyncParams_t*)bigCreateSyncBuff;
					pBigCreateSyncParam->big_handle = 0;			 /* Used to identify the BIG */
					pBigCreateSyncParam->sync_handle = pEvt->syncHandle; /* Identifier of the periodic advertising train */
					pBigCreateSyncParam->enc = pEvt->enc; 				 /* Encryption flag */
					memset(pBigCreateSyncParam->broadcast_code, 0, 16);	 /* TK: all zeros, just like JustWorks TODO: LE security mode 3, here use LE security mode 3 level2 */
					pBigCreateSyncParam->mse = pEvt->nse;				 /* The Controller can schedule reception of any number of subevents up to NSE */
					pBigCreateSyncParam->big_sync_timeout = 10*pEvt->IsoItvl*1250/10000; /* Synchronization timeout for the BIG */
					pBigCreateSyncParam->num_bis = pEvt->numBis;		 /* Total number of BISes to synchronize */
					foreach(i, pBigCreateSyncParam->num_bis){
						pBigCreateSyncParam->bis[i] = i+1;				 /* List of indices of BISes */
					}

					ble_sts_t status = blc_hci_le_bigCreateSync(pBigCreateSyncParam);
					my_dump_str_data(APP_DUMP_EN,"Start LE BIG Create Sync, status", &status, 1);
					my_dump_str_data(APP_DUMP_EN,"		Sync handle", &pEvt->syncHandle, 2);

					big_sync_create = !status; //0 avoid repeat big create sync

					if(!status){
						app_sync_handle = pEvt->syncHandle;
					}
				}
			}
			//------- hci le event: le big sync established event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_BIG_SYNC_ESTABLILSHED)	// create BIG complete
			{
				hci_le_bigSyncEstablishedEvt_t* pEvt = (hci_le_bigSyncEstablishedEvt_t*)p;
				my_dump_str_data(APP_DUMP_EN,"le big sync established event", pEvt, sizeof(hci_le_bigSyncEstablishedEvt_t)-2+pEvt->numBis*2);

				if(pEvt->status == BLE_SUCCESS){
					my_dump_str_data(APP_DUMP_EN,"BIG Sync Establish succeed", 0, 0);

					app_bisSyncNum = pEvt->numBis;
					my_dump_str_data(APP_DUMP_EN,"	BIS num", &app_bisSyncNum, 1);

					for(int i = 0; i < app_bisSyncNum; i++){
						app_bisSyncHandle[i] = pEvt->bisHandles[i];
						my_dump_str_data(APP_DUMP_EN,"	Use BIS handle i", &app_bisSyncHandle[i], 2);
					}

					#if (LE_AUDIO_DEMO_ENABLE)
						app_audio_recv_start();
					#endif

					#if (UI_LED_ENABLE)
						gpio_write(GPIO_LED_GREEN, 1);
					#endif
				}
				else{
					my_dump_str_data(APP_DUMP_EN,"BIG Sync Establish failed", &pEvt->status, 1);

					app_sync_handle = 0xFFFF;

					#if (LE_AUDIO_DEMO_ENABLE)
						app_audio_recv_stop();
					#endif

					#if (UI_LED_ENABLE)
						gpio_write(GPIO_LED_GREEN, 0);
					#endif
				}
			}
			//------------- hci le event: le big sync lost event -------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_BIG_SYNC_LOST)
			{
				hci_le_bigSyncLostEvt_t* pEvt = (hci_le_bigSyncLostEvt_t*)p;
				my_dump_str_data(APP_DUMP_EN,"BIG Sync lost, bigHandle", &pEvt->bigHandle, 1);
				my_dump_str_data(APP_DUMP_EN,"		       lost reason", &pEvt->reason, 1);

				big_sync_create = 0;
				app_sync_handle = 0xFFFF;

				ble_sts_t status = blc_ll_bigTerminateSync(pEvt->bigHandle);
				my_dump_str_data(APP_DUMP_EN, "LE Terminate BIG Sync: status", &status, 1);

				blc_ll_setScanEnable (BLC_SCAN_DISABLE, DUP_FILTER_DISABLE);
				my_dump_str_data(APP_DUMP_EN, "Scan disable", &big_sync_create, 1);

				#if (UI_LED_ENABLE)
					gpio_write(GPIO_LED_GREEN, 0);
				#endif

				#if (LE_AUDIO_DEMO_ENABLE)
					app_audio_recv_stop();
				#endif
			}
		}
	}


	return 0;

}



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


#if (LL_FEATURE_ENABLE_LE_EXTENDED_SCAN)
	blc_ll_initExtendedScanning_module();
#endif

#if (LL_FEATURE_ENABLE_SYNCHRONIZED_RECEIVER)
    blc_ll_initPeriodicAdvertisingSynchronization_module();
#endif



	////////////// 2M or coded PHY register ///////////////
	blc_ll_init2MPhyCodedPhy_feature(); //if use 2M or Coded PHY
	//////////////// CSA#2 feature register ///////////////
	blc_ll_initChannelSelectionAlgorithm_2_feature(); //Periodic ADV must used!!! //ISO features must support CSA#2

	blc_ll_setMaxConnectionNumber( MASTER_MAX_NUM, SLAVE_MAX_NUM);

	rf_set_power_level_index (RF_POWER_P3dBm);


	//////////// LinkLayer Initialization  End /////////////////////////



	//////////// HCI Initialization  Begin /////////////////////////
	blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func


	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(    HCI_LE_EVT_MASK_EXTENDED_ADVERTISING_REPORT \
								|	HCI_LE_EVT_MASK_PERIODIC_ADVERTISING_SYNC_ESTABLISHED \
								|	HCI_LE_EVT_MASK_PERIODIC_ADVERTISING_REPORT \
								|	HCI_LE_EVT_MASK_PERIODIC_ADVERTISING_SYNC_LOST \
								|	HCI_LE_EVT_MASK_BIG_SYNC_ESTABLILSHED  \
								|	HCI_LE_EVT_MASK_BIG_SYNC_LOST );

	blc_hci_le_setEventMask_2_cmd(	HCI_LE_EVT_MASK_2_BIGINFO_ADVERTISING_REPORT );
	//////////// HCI Initialization  End /////////////////////////



	//////////////////////////// User Configuration for BLE application ////////////////////////////

	/////////////////// BIS SYNC initialization ////////////////////////

#if(LL_FEATURE_PRIVATE_BIS_SYNC_RECEIVER)
	blc_ll_enPrivLegScanForBigBync();	/// For private purposes ///
#elif(LL_FEATURE_NORMAL_BIS_SYNC_RECEIVER)
	blc_ll_enScanForBigBync();
#endif

	blc_ll_initBisSynchronize_module();

	blc_ial_initSdu_module(); //IAL module register

	blc_ll_initBigSyncParameters(app_bigSyncParam, APP_BIG_SYNC_NUMBER);

	blc_ll_InitBisConnectionlessParameters(app_bisToatlParam, APP_BIS_NUM_IN_ALL_BIG_BCST, APP_BIS_NUM_IN_ALL_BIG_SYNC);

	blc_setHostFeatureISOChannel_en(LL_FEATURE_ENABLE);

	/* BIS RX buffer init */
	blc_ll_initBisRxFifo(app_bisSyncRxfifo, MAX_RX_ISO_FIFO_SIZE, ISO_RX_FIFO_NUM, APP_BIG_SYNC_NUMBER);

	/* IAL SDU buff init */
	blc_ial_initBisSduBuff(bltSduRxFifo, MAX_RX_SDU_LEN, IAL_SDU_RX_FIFO_NUM, NULL, 0, 0);
	//////////////// BIS SYNC Initialization End ///////////////////////


	u8 check_status = blc_controller_check_appBufferInitialization();
	if(check_status != BLE_SUCCESS){
		/* here user should set some log to know which application buffer incorrect*/
		write_log32(0x88880000 | check_status);
		while(1){
			myudb_usb_handle_irq();
		}
	}


#if (LL_FEATURE_ENABLE_LE_EXTENDED_SCAN)
	blc_ll_setExtScanParam( OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY, SCAN_PHY_1M, \
							SCAN_TYPE_PASSIVE,  SCAN_INTERVAL_100MS,   SCAN_INTERVAL_100MS, \
							0, 					0, 					   0);

	blc_ll_setExtScanEnable( BLC_SCAN_ENABLE, DUP_FILTER_DISABLE, SCAN_DURATION_CONTINUOUS, SCAN_WINDOW_CONTINUOUS);
#endif

	#if (LE_AUDIO_DEMO_ENABLE)
		app_audio_init();
	#endif

	#if (APP_AUDIO_USB_ENABLE)
		app_usb_init();
		plic_set_priority(IRQ11_USB_ENDPOINT, 1); //ZB_RF, STIME use the highest IRQ priority
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

	#if (APP_AUDIO_USB_ENABLE)
		app_usb_handler();
	#endif

}

#endif



