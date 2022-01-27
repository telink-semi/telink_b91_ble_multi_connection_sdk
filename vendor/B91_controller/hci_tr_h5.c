/********************************************************************************************************
 * @file	hci_tr_h5.c
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
#include "hci_tr_h5.h"
#include "hci_tr_api.h"
#include "hci_tr_def.h"
#include "hci_slip.h"
#include "hci_tr.h"
#include "hci_h5.h"

#include "hci_port.h"

/*! Backup buffer define. */
static u8 h5TrBackupBuf[HCI_H5_TR_RX_BUF_SIZE];

/*! HCI H5 transport Rx buffer define. */
static u8 h5TrRxBuf[HCI_H5_TR_RX_BUF_SIZE * HCI_H5_TR_RX_BUF_NUM];

/*! HCI H5 transport Rx Fifo define. */
static hci_fifo_t h5TrRxFifo = {
	HCI_H5_TR_RX_BUF_SIZE,
	HCI_H5_TR_RX_BUF_NUM,
	HCI_H5_TR_RX_BUF_NUM - 1,
	0,
	0,
	&h5TrRxBuf[0],
};

/*! HCI H5 transport main control block. */
typedef struct{
	hci_fifo_t            *pRxFifo;
	HciH5PacketHandler_t   HCI_Tr_SlipHandler;
	u8                    *pBackupBuf;
	u8                     backupCnt;
	u8                     recvd1;
	u16                    recvd2;
}HciH5TrCb_t;

static HciH5TrCb_t hciH5TrCb;

void HCI_Tr_DefaultSlipHandler(u8 *pPacket, u32 len)
{
}

/**
 * @brief : H5 protocol initializaiton.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5Init(void)
{
	hciH5TrCb.pRxFifo    = &h5TrRxFifo;
	hciH5TrCb.pBackupBuf = h5TrBackupBuf;
	hciH5TrCb.backupCnt  = 0;

	hciH5TrCb.HCI_Tr_SlipHandler = HCI_Tr_DefaultSlipHandler;

	/* Timer configuration. */
	HCI_Tr_TimeInit(5000);

	/* UART configuration. */
	HCI_Tr_InitUart(1, (hciH5TrCb.pRxFifo->p+4),  hciH5TrCb.pRxFifo->size - 4);
}

/**
 * @brief : Register slip handler
 * @param : func    Pointer point to handler function.
 * @param : none.
 */
void HCI_Tr_H5RegisterSlipHandler(HciH5PacketHandler_t func)
{
	ASSERT(func != NULL, HCI_TR_ERR_INVALID_PARAM);

	hciH5TrCb.HCI_Tr_SlipHandler = func;
}

/**
 * @brief : Find slip delimiiter
 * @param : pPacket     Pointer point to slip packet buffer.
 * @param : len         the length of data.
 * @param : none.
 */
static u8 *HCI_Tr_H5FindSlipFlag(u8 *pPacket, u32 len)
{
	u8 *pBuf = pPacket;
	while(len)
	{
		u8 slipFlag = pBuf[0];
		if(slipFlag != 0xC0){
			pBuf++;
			len--;
			continue;
		}
		return pBuf;// pointer point to 0xC0
	}
	return NULL;
}

int HCI_Tr_H5BackupHandler(u8 *pPacket, u32 len)
{
	u8 *pBuf = hciH5TrCb.pBackupBuf;
	u16 backupLen = hciH5TrCb.backupCnt;

	if(backupLen == 0)
	{
		ASSERT(len < HCI_H5_TR_RX_BUF_SIZE, HCI_TR_ERR_TR_BACKUP_BUF);

		memcpy(pBuf, pPacket, len);
		hciH5TrCb.backupCnt += len;
		HCI_Tr_TimeEnable();

		TR_TRACK_INFO("Backup start: ");
		HCI_TRACK_DATA(pBuf, hciH5TrCb.backupCnt);
		return len;
	}

	u16 cnt = 0;
	while(len)
	{
		if(*pPacket == 0xC0)
		{
			ASSERT(hciH5TrCb.backupCnt+1 < HCI_H5_TR_RX_BUF_SIZE, HCI_TR_ERR_TR_BACKUP_BUF);

			memcpy(pBuf + hciH5TrCb.backupCnt, pPacket, 1);
			hciH5TrCb.backupCnt += 1;
			cnt++;

			TR_TRACK_INFO("Rx data2: ");
			HCI_TRACK_DATA(hciH5TrCb.pBackupBuf, hciH5TrCb.backupCnt);

			hciH5TrCb.HCI_Tr_SlipHandler(hciH5TrCb.pBackupBuf, hciH5TrCb.backupCnt);

			hciH5TrCb.backupCnt = 0;
			HCI_Tr_TimeDisable();
			return cnt;
		}
		else
		{
			ASSERT(hciH5TrCb.backupCnt+1 < HCI_H5_TR_RX_BUF_SIZE, HCI_TR_ERR_TR_BACKUP_BUF);

			memcpy(pBuf + hciH5TrCb.backupCnt, pPacket, 1);//TODO: Check len
			hciH5TrCb.backupCnt += 1;
			cnt++;
			HCI_Tr_TimeEnable();
		}

		pPacket++;
		len--;
	}

	return cnt;
}

/**
 * @brief : H5 transport rx handler.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5RxHandler(void)
{
	hci_fifo_t *pFifo = hciH5TrCb.pRxFifo;
	if(pFifo->wptr == pFifo->rptr){
		return;
	}

	u8 *pPacket = pFifo->p + (pFifo->rptr & pFifo->mask) * pFifo->size;
	u32 len = 0;
	BSTREAM_TO_UINT32(len, pPacket);

	TR_TRACK_INFO("TR Rx data: ");
	HCI_TRACK_DATA(pPacket, len);

	while(len)
	{
		if(hciH5TrCb.backupCnt){
			u8 res = HCI_Tr_H5BackupHandler(pPacket, len);
			len -= res;
			pPacket += res;
		}

		u8 *pBuf = NULL;
		u8 *pBuf2 = NULL;
		pBuf = HCI_Tr_H5FindSlipFlag(pPacket, len);
		if(pBuf)
		{
			u16 tmpLen = pBuf+1 - pPacket;
			pBuf2 = HCI_Tr_H5FindSlipFlag(pBuf+1, len - tmpLen);
			if(pBuf2 == NULL){
				HCI_Tr_H5BackupHandler(pBuf, len - (pBuf - pPacket));
				len = 0;
				continue;
			}
			else{
				if(pBuf2+1 - pBuf < 6){
					pPacket += (pBuf2 - pBuf);
					len -= (pBuf2 - pBuf);
					continue;
				}

				TR_TRACK_INFO("Rx data1: ");
				HCI_TRACK_DATA(pBuf, pBuf2 + 1 - pBuf);

				hciH5TrCb.HCI_Tr_SlipHandler(pBuf, pBuf2 + 1 - pBuf);

				pPacket += (pBuf2 + 1 - pBuf);
				len -= (pBuf2 + 1 - pBuf);
			}
		}
		else
		{
			len = 0;
			continue;
		}
	}

	if(len == 0){
		pFifo->rptr++;
	}
}

/**
 * @brief : H5 transport IRQ handler.
 * @param : none.
 * @param : none.
 */
_attribute_ram_code_
void HCI_Tr_H5UartIRQHandler(void)
{
	/* UART RX IRQ */
	if((uart_get_irq_status(HCI_TR_UART_ID, UART_RX_ERR))){
		uart_clr_irq_status(HCI_TR_UART_ID, UART_CLR_RX);//it will clear rx_fifo and rx_err_status,rx_done_irq.

		hci_fifo_t *pRxFifo = hciH5TrCb.pRxFifo;
		u8 *p = pRxFifo->p + (pRxFifo->wptr & pRxFifo->mask) * pRxFifo->size;
		uart_receive_dma(HCI_TR_UART_ID, (p+4), pRxFifo->size - 4);//[!!important]
	}
	else if(uart_get_irq_status(HCI_TR_UART_ID, UART_RXDONE)) //A0-SOC can't use RX-DONE status,so this interrupt can only used in A1-SOC.
	{
		/* Get the length of Rx data */
		u32 rxLen = 0;
		if(((reg_uart_status1(HCI_TR_UART_ID) & FLD_UART_RBCNT) % 4)==0){
			rxLen = 4 * (0xffffff - reg_dma_size(UART_DMA_CHN_RX));
		}else{
			rxLen = 4 * (0xffffff - reg_dma_size(UART_DMA_CHN_RX)-1) + (reg_uart_status1(HCI_TR_UART_ID) & FLD_UART_RBCNT) % 4;
		}

		/* Clear RxDone state */
		uart_clr_irq_status(HCI_TR_UART_ID, UART_CLR_RX);

		hci_fifo_t *pRxFifo = hciH5TrCb.pRxFifo;
		u8 *p = pRxFifo->p + (pRxFifo->wptr & pRxFifo->mask) * pRxFifo->size;

		//todo:
		if(pRxFifo->wptr - pRxFifo->rptr >= pRxFifo->num){
			uart_receive_dma(HCI_TR_UART_ID, (p+4), pRxFifo->size - 4);//[!!important]
			return; //have no memory.
		}

		if(rxLen)
		{
			UINT32_TO_BSTREAM(p, rxLen);

			pRxFifo->wptr++;
			p = pRxFifo->p + (pRxFifo->wptr & pRxFifo->mask) * pRxFifo->size;
		}
		uart_receive_dma(HCI_TR_UART_ID, (p+4), pRxFifo->size - 4);//[!!important]
	}

	/* UART TX IRQ */
	if(uart_get_irq_status(HCI_TR_UART_ID, UART_TXDONE))
	{
		//uart_clr_tx_done(HCI_TR_UART_ID);
	}
}

_attribute_ram_code_
void HCI_Tr_H5TimerIRQHandler(void)
{
	/* Timer timeout IRQ */
	if(reg_tmr_sta & TIMER_STAT_BIT){
		reg_tmr_sta = TIMER_STAT_BIT;
		HCI_Tr_TimeDisable();
		hciH5TrCb.backupCnt = 0;
	}
}
