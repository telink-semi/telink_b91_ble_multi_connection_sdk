/********************************************************************************************************
 * @file	app_buffer.h
 *
 * @brief	This is the header file for BLE SDK
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
#ifndef APP_BUFFER_H_
#define APP_BUFFER_H_

#include "tl_common.h"

#include "app_config.h"



/********************* USB_DEBUG_LOG FIFO allocation, Begin *******************************/

#if (APP_DUMP_EN)
	extern my_fifo_t print_fifo;
	extern	u8 	print_fifo_b[];
#endif

/******************** USB_DEBUG_LOG FIFO allocation, End ***********************************/




/********************* ACL connection LinkLayer TX & RX data FIFO allocation, Begin *******************************/
#define ACL_CONN_MAX_RX_OCTETS			251//27
#define ACL_SLAVE_MAX_TX_OCTETS			251//27
#define ACL_MASTER_MAX_TX_OCTETS		251//27


/**
 * @brief	LE_ACL_Data_Packet_Length, refer to BLE SPEC "7.8.2 LE Read Buffer Size command"
 * usage limitation:
 * 1. only used for BLE controller project
 * 2. must be an integer multiple of 4, such as 32,80,200...
 * 3. must greater than maximum of ACL_SLAVE_MAX_TX_OCTETS, ACL_MASTER_MAX_TX_OCTETS
 * 4. can not exceed 252(for maximum tx_octets when DLE used)
 */
#define LE_ACL_DATA_PACKET_LENGTH		(252)//(28)


#define ACL_RX_FIFO_SIZE		    CAL_LL_ACL_RX_FIFO_SIZE(ACL_CONN_MAX_RX_OCTETS) //64
#define ACL_RX_FIFO_NUM			    8

#define ACL_SLAVE_TX_FIFO_SIZE		CAL_LL_ACL_TX_FIFO_SIZE(ACL_SLAVE_MAX_TX_OCTETS) //48
#define ACL_SLAVE_TX_FIFO_NUM		9

#define ACL_MASTER_TX_FIFO_SIZE		CAL_LL_ACL_TX_FIFO_SIZE(ACL_MASTER_MAX_TX_OCTETS) //48
#define ACL_MASTER_TX_FIFO_NUM		9

extern	u8	app_acl_rxfifo[];
extern	u8	app_acl_mstTxfifo[];
extern	u8	app_acl_slvTxfifo[];


/***************************** HCI TX & RX data FIFO allocation, Begin *********************************************/
#define HCI_MAX_TX_SIZE				251//max2(ACL_SLAVE_MAX_TX_OCTETS, ACL_MASTER_MAX_TX_OCTETS) //support common tx max

#define HCI_TX_FIFO_SIZE			HCI_FIFO_SIZE(HCI_MAX_TX_SIZE)//80
#define HCI_TX_FIFO_NUM				8

#define HCI_RX_FIFO_SIZE			HCI_FIFO_SIZE(ACL_CONN_MAX_RX_OCTETS)//80//((1 + 4 + 300 + 16) & ~15)//HCI_FIFO_SIZE(ACL_CONN_MAX_RX_OCTETS)
#define HCI_RX_FIFO_NUM				8

#define HCI_RX_ACL_FIFO_SIZE		CALCULATE_HCI_ACL_DATA_FIFO_SIZE(LE_ACL_DATA_PACKET_LENGTH)
#define HCI_RX_ACL_FIFO_NUM			8

extern	u8	app_hci_rxfifo[];
extern	u8	app_hci_txfifo[];
extern	u8	app_hci_rxAclfifo[];


//HCI UART variables
#define UART_DATA_LEN    			HCI_TX_FIFO_SIZE

typedef struct{
    unsigned int  len;
    unsigned char data[UART_DATA_LEN];
}uart_data_t;

/****************************** HCI TX & RX data FIFO allocation, End *********************************************/




#endif /* APP_BUFFER_H_ */
