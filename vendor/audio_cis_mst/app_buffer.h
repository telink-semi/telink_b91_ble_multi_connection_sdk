/********************************************************************************************************
 * @file	app_buffer.h
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
#ifndef APP_BUFFER_H_
#define APP_BUFFER_H_

#include "app_config.h"
#include "tl_common.h"




/**
 * @brief	connMaxRxOctets
 * refer to BLE SPEC "4.5.10 Data PDU length management" & "2.4.2.21 LL_LENGTH_REQ and LL_LENGTH_RSP"
 * usage limitation:
 * 1. should be in range of 27 ~ 251
 */
#define ACL_CONN_MAX_RX_OCTETS			27


/**
 * @brief	connMaxTxOctets
 * refer to BLE SPEC "4.5.10 Data PDU length management" & "2.4.2.21 LL_LENGTH_REQ and LL_LENGTH_RSP"
 *  in this SDK, we separate this value into 2 parts: slaveMaxTxOctets and masterMaxTxOctets,
 *  for purpose to save some SRAM costed by when slave and master use different connMaxTxOctets.
 * usage limitation:
 * 1. slaveMaxTxOctets and masterMaxTxOctets should be in range of 27 ~ 251
 * 2. for CIS, ll_cis_req(36Byte), ACL_CONN_MAX_RX_OCTETS must >= 36
 */
#define ACL_MASTER_MAX_TX_OCTETS		36
#define ACL_SLAVE_MAX_TX_OCTETS			27




/********************* ACL connection LinkLayer TX & RX data FIFO allocation, Begin ************************************************/
/**
 * @brief	ACL RX buffer size & number
 *  		ACL RX buffer is shared by all connections to hold LinkLayer RF RX data.
 * usage limitation for ACL_RX_FIFO_SIZE:
 * 1. should be greater than or equal to (connMaxRxOctets + 21)
 * 2. should be be an integer multiple of 16 (16 Byte align)
 * 3. user can use formula:  size = CAL_LL_ACL_RX_FIFO_SIZE(connMaxRxOctets)
 * usage limitation for ACL_RX_FIFO_NUM:
 * 1. must be: 2^n, (power of 2)
 * 2. at least 4; recommended value: 8, 16
 */
#define ACL_RX_FIFO_SIZE				48  // ACL_CONN_MAX_RX_OCTETS + 21, then 16 Byte align
#define ACL_RX_FIFO_NUM					8	// must be: 2^n


/**
 * @brief	ACL TX buffer size & number
 *  		ACL MASTER TX buffer is shared by all master connections to hold LinkLayer RF TX data.
*			ACL SLAVE  TX buffer is shared by all slave  connections to hold LinkLayer RF TX data.
 * usage limitation for ACL_MASTER_TX_FIFO_SIZE & ACL_SLAVE_TX_FIFO_SIZE:
 * 1. should be greater than or equal to (connMaxTxOctets + 10)
 * 2. should be be an integer multiple of 16 (16 Byte align)
 * 3. user can use formula:  size = CAL_LL_ACL_TX_FIFO_SIZE(connMaxTxOctets)
 * usage limitation for ACL_MASTER_TX_FIFO_NUM & ACL_SLAVE_TX_FIFO_NUM:
 * 1. must be: (2^n) + 1, (power of 2, then add 1)
 * 2. at least 9; recommended value: 9, 17, 33; other value not allowed.
 * usage limitation for size * (number - 1)
 * 1. (ACL_MASTER_TX_FIFO_SIZE * (ACL_MASTER_TX_FIFO_NUM - 1)) must be less than 4096 (4K)
 *    (ACL_SLAVE_TX_FIFO_SIZE  * (ACL_SLAVE_TX_FIFO_NUM - 1))  must be less than 4096 (4K)
 *    so when ACL TX FIFO size equal to or bigger than 256, ACL TX FIFO number can only be 9(can not use 17 or 33), cause 256*(17-1)=4096
 */
#define ACL_MASTER_TX_FIFO_SIZE			48	// ACL_MASTER_MAX_TX_OCTETS + 10, then 16 Byte align
#define ACL_MASTER_TX_FIFO_NUM			9	// must be: (2^n) + 1

#define ACL_SLAVE_TX_FIFO_SIZE			48  // ACL_MASTER_MAX_TX_OCTETS + 10, then 16 Byte align
#define ACL_SLAVE_TX_FIFO_NUM			9   // must be: (2^n) + 1




extern	u8	app_acl_rxfifo[];
extern	u8	app_acl_mstTxfifo[];
extern	u8	app_acl_slvTxfifo[];
/******************** ACL connection LinkLayer TX & RX data FIFO allocation, End ***************************************************/



/***************** ACL connection L2CAP layer MTU TX & RX data FIFO allocation, Begin ********************************/

/*Note:
 * if support LE Secure Connections, L2CAP buffer must >= 72.([64+6]+3)/4*4), 4B align.
 * MTU Buff size = Extra_Len(6)+ ATT_MTU_MAX
 *  1. should be greater than or equal to (ATT_MTU + 6)
 *  2. should be be an integer multiple of 4 (4 Byte align)
 */
#if(UI_AUDIO_ENABLE)
#define ATT_MTU_MASTER_RX_MAX_SIZE  144 //associated with MIC_ADPCM_FRAME_SIZE
#else
#if (LE_AUDIO_ENABLE)
#define ATT_MTU_MASTER_RX_MAX_SIZE  64
#else
#define ATT_MTU_MASTER_RX_MAX_SIZE  23
#endif
#endif

#define	MTU_M_BUFF_SIZE_MAX			CAL_MTU_BUFF_SIZE(ATT_MTU_MASTER_RX_MAX_SIZE)

#if (LE_AUDIO_ENABLE)
#define ATT_MTU_SLAVE_RX_MAX_SIZE   64
#else
#define ATT_MTU_SLAVE_RX_MAX_SIZE   23
#endif
#define	MTU_S_BUFF_SIZE_MAX			CAL_MTU_BUFF_SIZE(ATT_MTU_SLAVE_RX_MAX_SIZE)


extern	u8 mtu_m_rx_fifo[];

extern	u8 mtu_s_rx_fifo[];
extern	u8 mtu_s_tx_fifo[];
/***************** ACL connection L2CAP layer MTU TX & RX data FIFO allocation, End **********************************/




/************************* CIS connection RX/TX ISO PDU buffer FIFO allocation, Begin ********************************/
/**
 * @brief	cisMaxTxPDU is the maximum number of data octets that can be carried in a CIS Data PDU
 * refer to BLE SPEC "2.6 ISOCHRONOUS PHYSICAL CHANNEL PDU"
 * usage limitation:
 * 1. should be in range of 0 ~ 251
 */
#define		CIS_CONN_MAX_TX_PDU			251
/**
 * @brief	cisMaxTxPDU is the maximum number of data octets that can be carried in a CIS Data PDU
 * refer to BLE SPEC "2.6 ISOCHRONOUS PHYSICAL CHANNEL PDU"
 * usage limitation:
 * 1. should be in range of 0 ~ 251
 */
#define 	CIS_CONN_MAX_RX_PDU			251


/**
 * @brief	CIS RX/TX buffer size & number
 *  		CIS RX/TX buffer is shared by all connections to hold LinkLayer RF RX/TX CIS data.
 * usage limitation for MAX_RX_ISO_FIFO_SIZE or MAX_TX_ISO_FIFO_SIZE:
 * 1. should be greater than or equal to XXX, TODO
 * 2. should be be an integer multiple of 4 (4 Byte align)
 * 3. user can use formula:  size = CIS_PDU_ALLIGN4_RXBUFF(cisMaxTxPDU)
 * usage limitation for ISO_RX_FIFO_NUM or ISO_TX_FIFO_NUM:
 * 1. must be: 2^n, (power of 2)
 * 2. at least 4; recommended value: 8, 16
 */

#define		MAX_RX_ISO_FIFO_SIZE		CIS_PDU_ALLIGN4_RXBUFF(CIS_CONN_MAX_RX_PDU)
#define		MAX_TX_ISO_FIFO_SIZE		CIS_PDU_ALLIGN4_TXBUFF(CIS_CONN_MAX_TX_PDU)

#define		ISO_RX_FIFO_NUM				(8)
#define		ISO_TX_FIFO_NUM				(8)


#define		MAX_RX_ISO_EVT_FIFO_SIZE	DATA_LENGTH_ALLIGN4(ISO_RX_EVENT_LENGTH)
#define		ISO_RX_EVT_FIFO_NUM			(16)


extern	u8	app_cis_rxfifo[];
extern	u8	app_cis_txfifo[];
extern	u8  app_cis_rxEvtfifo[];
/************************* CIS connection RX/TX ISO PDU buffer FIFO allocation, End **********************************/




/************************ CIS connection RX/TX ISOAL SDU buffer FIFO allocation, Begin *******************************/
#define		MAX_RX_SDU_LEN				IAL_SDU_ALLIGN4_BUFF(400)
#define		MAX_TX_SDU_LEN				IAL_SDU_ALLIGN4_BUFF(400)

#define		IAL_SDU_RX_FIFO_NUM			(8)
#define		IAL_SDU_TX_FIFO_NUM			(8)

extern	u8	bltSduRxFifo[];
extern	u8	bltSduTxFifo[];
/************************ CIS connection RX/TX ISOAL SDU buffer FIFO allocation, End  *******************************/




/********************************** CIS Master Global Data Definition, Begin *****************************************/
#define		TEST_CIS_COUNT				2	// The example supports 1 CIG MST and 2 CIS

#define		APP_CIG_MST_NUMBER			1	// Number of Supported CIG Mst number
#define		APP_CIG_SLV_NUMBER			0	// Number of Supported CIG Slv number
#define		APP_CIS_NUM_IN_PER_CIG_MST	2	// Number of Supported CISes in Per CIG Master
#define		APP_CIS_NUM_IN_PER_CIG_SLV	2	// Number of Supported CISes in Per CIG Slave

// The total number of CIS supported by CIG MST
#define		APP_CIS_NUM_IN_ALL_CIG_MST	(APP_CIG_MST_NUMBER * APP_CIS_NUM_IN_PER_CIG_MST)
// The total number of CIS supported by CIG SLV
#define		APP_CIS_NUM_IN_ALL_CIG_SLV	(APP_CIG_SLV_NUMBER * APP_CIS_NUM_IN_PER_CIG_SLV)
// The total number of CIS supported by CIG (Mst and Slv)
#define		APP_TOTAL_CIS_NUMBER		(APP_CIS_NUM_IN_ALL_CIG_MST + APP_CIS_NUM_IN_ALL_CIG_SLV)
/********************************** CIS Master Global Data Definition, end  *****************************************/




/********************************** CIS Master Global Data Definition, Begin *****************************************/
extern	u8	app_cig_param[];
extern	u8  app_cis_conn_param[];
/********************************** CIS Master Global Data Definition, End  *****************************************/




#endif /* APP_BUFFER_H_ */
