/********************************************************************************************************
 * @file	app.c
 *
 * @brief	This is the source file for BLE SDK
 *
 * @author	BLE GROUP
 * @date	2020.06
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *          
 *          Redistribution and use in source and binary forms, with or without
 *          modification, are permitted provided that the following conditions are met:
 *          
 *              1. Redistributions of source code must retain the above copyright
 *              notice, this list of conditions and the following disclaimer.
 *          
 *              2. Unless for usage inside a TELINK integrated circuit, redistributions 
 *              in binary form must reproduce the above copyright notice, this list of 
 *              conditions and the following disclaimer in the documentation and/or other
 *              materials provided with the distribution.
 *          
 *              3. Neither the name of TELINK, nor the names of its contributors may be 
 *              used to endorse or promote products derived from this software without 
 *              specific prior written permission.
 *          
 *              4. This software, with or without modification, must only be used with a
 *              TELINK integrated circuit. All other usages are subject to written permission
 *              from TELINK and different commercial license may apply.
 *
 *              5. Licensee shall be solely responsible for any claim to the extent arising out of or 
 *              relating to such deletion(s), modification(s) or alteration(s).
 *         
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *          DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 *          DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *         
 *******************************************************************************************************/
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "vendor/common/blt_common.h"

#include "app_buffer.h"


#include "hci_port.h"
#include  "hci_tr_api.h"

#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_ALL
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_40MS




//////////////////////////////////////////////////////////////////////////////
//	Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8	tbl_advData[] = {
	 0x05, 0x09, 't', 'H', 'C', 'I',
	 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
};

const u8	tbl_scanRsp [] = {
	 0x07, 0x09, 't', 'H', 'C', 'I', '0', '1',	//scan name " tHCI01"
};


/////////////////////////////////////blc_register_hci_handler for spp//////////////////////////////
int rx_from_uart_cb (void)//UART data send to Master,we will handler the data as CMD or DATA
{
	return 0;
}

int tx_to_uart_cb (void)
{
	return 0;
}

#if LL_FEATURE_ENABLE_LE_EXTENDED_ADVERTISING

/** Number of Supported Advertising Sets, no exceed "ADV_SETS_NUMBER_MAX" */
#define	APP_ADV_SETS_NUMBER						   				3 //Max support up to 3 ADV sets

/** Maximum Advertising Data Length,   (if legacy ADV, max length 31 bytes is enough) */
#define APP_MAX_LENGTH_ADV_DATA									320//2048//1664//1024

/** Maximum Scan Response Data Length, (if legacy ADV, max length 31 bytes is enough) */
#define APP_MAX_LENGTH_SCAN_RESPONSE_DATA		  				320//2048//1664//1024

_attribute_ble_data_retention_	u8  app_advSet_buffer[ADV_SET_PARAM_LENGTH * APP_ADV_SETS_NUMBER];

_attribute_ble_data_retention_	u8 	app_advData_buffer[APP_MAX_LENGTH_ADV_DATA * APP_ADV_SETS_NUMBER];

_attribute_ble_data_retention_	u8 	app_scanRspData_buffer[APP_MAX_LENGTH_SCAN_RESPONSE_DATA * APP_ADV_SETS_NUMBER];
#endif

///////////////////////////////////////////

/**
 * @brief      use initialization
 * @param[in]  none.
 * @return     none.
 */
void user_init_normal(void)
{
	/* random number generator must be initiated here( in the beginning of user_init_nromal).
	 * When deepSleep retention wakeUp, no need initialize again */
	random_generator_init();

//////////////////////////// BLE stack Initialization  Begin //////////////////////////////////

	/* for 512K Flash, flash_sector_mac_address equals to 0x76000
	 * for 1M   Flash, flash_sector_mac_address equals to 0xFF000 */
	u8  mac_public[6];
	u8  mac_random_static[6];
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);

#if (APP_DUMP_EN)
	my_usb_init(0x120, &print_fifo);
	usb_set_pin_en ();
#endif

	//////////// Controller Initialization  Begin /////////////////////////
	blc_ll_initBasicMCU();

	blc_ll_initStandby_module(mac_public);						   //mandatory

    blc_ll_initLegacyAdvertising_module(); 	//adv module: 		 mandatory for BLE slave,

    blc_ll_initLegacyScanning_module(); 	//scan module: 		 mandatory for BLE master

	blc_ll_initInitiating_module();			//initiate module: 	 mandatory for BLE master

	blc_ll_initAclConnection_module();
	blc_ll_initAclMasterRole_module();
	blc_ll_initAclSlaveRole_module();

#if LL_FEATURE_ENABLE_LE_EXTENDED_ADVERTISING
	blc_ll_initExtendedAdvertising_module();
	blc_ll_initExtendedAdvSetBuffer(app_advSet_buffer, APP_ADV_SETS_NUMBER);
	blc_ll_initExtAdvDataBuffer(app_advData_buffer, APP_MAX_LENGTH_ADV_DATA);
	blc_ll_initExtScanRspDataBuffer(app_scanRspData_buffer, APP_MAX_LENGTH_SCAN_RESPONSE_DATA);
#endif

	blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_MASTER_MAX_TX_OCTETS, ACL_SLAVE_MAX_TX_OCTETS);

	/* all ACL connection share same RX FIFO */
	if(blc_ll_initAclConnRxFifo(app_acl_rxfifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM) != BLE_SUCCESS)	{  	while(1); 	}
	/* ACL Master TX FIFO */
	if(blc_ll_initAclConnMasterTxFifo(app_acl_mstTxfifo, ACL_MASTER_TX_FIFO_SIZE, ACL_MASTER_TX_FIFO_NUM, MASTER_MAX_NUM) != BLE_SUCCESS) { while(1); }
	/* ACL Slave TX FIFO */
	if(blc_ll_initAclConnSlaveTxFifo(app_acl_slvTxfifo, ACL_SLAVE_TX_FIFO_SIZE, ACL_SLAVE_TX_FIFO_NUM, SLAVE_MAX_NUM) != BLE_SUCCESS)	{ while(1); }


	//blc_ll_setMaxConnectionNumber( MASTER_MAX_NUM, SLAVE_MAX_NUM);
	blc_ll_setMaxConnectionNumber( 1, 1 );
	blc_ll_setAclMasterConnectionInterval(CONN_INTERVAL_10MS);
	blc_ll_setCreateConnectionTimeout(50000);//[!!!important]

	rf_set_power_level_index (RF_POWER_INDEX_P2p79dBm);

	blc_ll_initChannelSelectionAlgorithm_2_feature();
	blc_ll_init2MPhyCodedPhy_feature();



	//////////// HCI Initialization  Begin /////////////////////////
	/* HCI RX FIFO */
	if(blc_ll_initHciRxFifo(app_hci_rxfifo, HCI_RX_FIFO_SIZE, HCI_RX_FIFO_NUM) != BLE_SUCCESS)	{  while(1); }
	/* HCI TX FIFO */
	if(blc_ll_initHciTxFifo(app_hci_txfifo, HCI_TX_FIFO_SIZE, HCI_TX_FIFO_NUM) != BLE_SUCCESS) { while(1); }
	/* HCI RX ACL FIFO */
	if(blc_ll_initHciAclDataFifo(app_hci_rxAclfifo, HCI_RX_ACL_FIFO_SIZE, HCI_RX_ACL_FIFO_NUM) != BLE_SUCCESS)	{ while(1); }


	/* HCI Data && Event */
	blc_hci_registerControllerDataHandler (blc_hci_sendACLData2Host);
	blc_hci_registerControllerEventHandler(blc_hci_send_data); //controller hci event to host all processed in this func

	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);
	//bluetooth low energy(LE) event, all enable
	blc_hci_le_setEventMask_cmd( 0xFFFFFFFF );
	blc_hci_le_setEventMask_2_cmd( 0x7FFFFFFF );


	u8 check_status = blc_controller_check_appBufferInitialization();
	if(check_status != BLE_SUCCESS){
		/* here user should set some log to know which application buffer incorrect*/
		write_log32(0x88880000 | check_status);
		while(1);
	}

	////// ADV and Scan CONFIG  //////////
#if 0
	blc_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	blc_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
	blc_ll_setAdvParam(ADV_INTERVAL_30MS, ADV_INTERVAL_30MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
	blc_ll_setAdvEnable(0);  //ADV disable

	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_200MS, SCAN_WINDOW_200MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_DISABLE, DUP_FILTER_DISABLE);
#endif

	/* HCI configuration */
	HCI_Tr_Init();
	blc_register_hci_handler(HCI_Tr_RxHandlerCback, HCI_Tr_TxHandlerCback);
}


void user_init_deepRetn(void)
{

}

void main_loop (void)
{
	HCI_Tr_Poll();

	////////////////////////////////////// BLE entry /////////////////////////////////
	blc_sdk_main_loop();

#if 0
	static u32 tickLoop = 1;
	if(tickLoop && clock_time_exceed(tickLoop, 100000)){
		tickLoop = clock_time();
		gpio_toggle(GPIO_LED_WHITE);
	}
#endif

#if (APP_DUMP_EN)
	myudb_usb_handle_irq ();
#endif
	////////////////////////////////////// UI entry /////////////////////////////////
}
