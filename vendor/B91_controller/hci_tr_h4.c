/********************************************************************************************************
 * @file	hci_tr_h4.c
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
#include "hci_tr_h4.h"
#include "hci_tr.h"
#include "drivers.h"
#include "stack/ble/controller/ble_controller.h"
#include "hci_tr_api.h"
#include "hci_tr_def.h"

#include "hci_port.h"

/******************************************************************************/
/*********    UART Normal Mode                                    *************/
/******************************************************************************/
#if !HCI_H4_DMA_MODE_EN
/*! HCI Transmit State */
enum{
	HCI_TR_STATE_INIT   = 0x00,
	HCI_TR_STATE_IDLE   = 0x01,
	HCI_TR_STATE_RCVING = 0x02,
	HCI_TR_STATE_RCVED  = 0x03,
	HCI_TR_STATE_ERR    = 0x04,
};

/*!  HCI Transmit main control block */
typedef struct{
	u8            *pCurRxBuf;
	u16            rxCnt;
	u16            frameHeadLen;
	u16            paramLen;
	u8             trState;
	u8             trType;
	hci_fifo_t     *rxFifo;
}HciTrCB_t;

static HciTrCB_t hciTrCb;

void HCI_Tr_H4Init(hci_fifo_t *pRxFifo)
{
	hciTrCb.rxFifo = pRxFifo;

	hciTrCb.pCurRxBuf = NULL;
	hciTrCb.rxCnt = 0;

	hciTrCb.frameHeadLen = 0;
	hciTrCb.paramLen = 0;

	hciTrCb.trState = HCI_TR_STATE_INIT;
	hciTrCb.trType = HCI_TR_TYPE_NONE;

	/* Timer configuration. */
	HCI_Tr_TimeInit(5);

	/* UART configuration. */
	uart_gpio_set(HCI_TR_TX_PIN, HCI_TR_RX_PIN);
	uart_reset();

#if (MCU_CORE_TYPE == MCU_CORE_825x)
	//baud rate: 115200
	#if(CLOCK_SYS_CLOCK_HZ == 16000000)
		uart_init(9, 13, PARITY_NONE, STOP_BIT_ONE);
	#elif(CLOCK_SYS_CLOCK_HZ == 24000000)
		uart_init(12, 15, PARITY_NONE, STOP_BIT_ONE);
	#elif(CLOCK_SYS_CLOCK_HZ == 32000000)
		uart_init(30, 8, PARITY_NONE, STOP_BIT_ONE);//uart_init_baudrate();
	#elif(CLOCK_SYS_CLOCK_HZ == 48000000)
		uart_init(25, 15, PARITY_NONE, STOP_BIT_ONE);
	#endif
#else ///8278 use 1000000 for xiaomi project.
	//baud rate: 1000000
	#if(CLOCK_SYS_CLOCK_HZ == 16000000)
		uart_init(0, 15, PARITY_NONE, STOP_BIT_ONE);
	#elif(CLOCK_SYS_CLOCK_HZ == 24000000)
		uart_init(1, 11, PARITY_NONE, STOP_BIT_ONE);
	#elif(CLOCK_SYS_CLOCK_HZ == 32000000)
		uart_init(1, 15, PARITY_NONE, STOP_BIT_ONE);//uart_init_baudrate();
	#elif(CLOCK_SYS_CLOCK_HZ == 48000000)
		uart_init(2, 15, PARITY_NONE, STOP_BIT_ONE);
	#endif
#endif

	/* UART Rx use Normal mode, UART Tx use DMA mode. */
	uart_dma_enable(0, 1);
	irq_set_mask(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_TX, 1);
	uart_irq_enable(1, 0);
	uart_ndma_irq_triglevel(1,1);
}

u8 rxIndex = 0;
static u8 HCI_Tr_ReceiveByte(void)
{
	u8 data = REG_ADDR8(0x90 + rxIndex);
	rxIndex++;
	rxIndex = rxIndex & 0x03;
	return data;
}

_attribute_ram_code_
void HCI_Tr_H4RxHandler(u8 data)
{
	u8 res = data; //HCI_Tr_ReceiveByte();

	hci_fifo_t *pFifo = hciTrCb.rxFifo;

	switch(hciTrCb.trState)
	{
	case HCI_TR_STATE_INIT:
		//hciTrCb.trState = HCI_TR_STATE_IDLE;
		break;

	case HCI_TR_STATE_IDLE:
	{
		HCI_Tr_TimeEnable();
		hciTrCb.trType = HCI_TR_TYPE_NONE;
		hciTrCb.rxCnt = 0;

		switch(res)
		{
		case HCI_TR_TYPE_CMD:
			hciTrCb.frameHeadLen = HCI_CMD_HEAD_LEN;//3;
			break;
		case HCI_TR_TYPE_ACL:
			hciTrCb.frameHeadLen = HCI_ACL_HEAD_LEN;//4;
			break;
		case HCI_TR_TYPE_EVENT:
			//hciTrCb.frameHeadLen = HCI_EVT_HEAD_LEN;//2;
			//break;
		case HCI_TR_TYPE_SCO:
			//hciTrCb.frameHeadLen = HCI_SCO_HEAD_LEN;//3;
			//break;
		default:
			hciTrCb.frameHeadLen = 0;
			return;
		}

		if(pFifo->wptr - pFifo->rptr >= pFifo->num){
			return; //have no memory.
		}

		hciTrCb.pCurRxBuf = pFifo->p + (pFifo->wptr & (pFifo->num-1)) * pFifo->size;
		hciTrCb.pCurRxBuf[hciTrCb.rxCnt++] = res;

		hciTrCb.trType   = res;
		hciTrCb.paramLen = 0;
		hciTrCb.trState  = HCI_TR_STATE_RCVING;
		break;
	}
	case HCI_TR_STATE_RCVING:
	{
		HCI_Tr_TimeEnable();

		hciTrCb.pCurRxBuf[hciTrCb.rxCnt++] = res;

		if(hciTrCb.frameHeadLen == hciTrCb.rxCnt - 1)
		{
			u8 *p = NULL;
			if(hciTrCb.trType == HCI_TR_TYPE_ACL){
				p = hciTrCb.pCurRxBuf + hciTrCb.rxCnt - 2;
				hciTrCb.paramLen = (p[1]<<8) + p[0];
			}
			else if(hciTrCb.trType == HCI_TR_TYPE_CMD){
				p = hciTrCb.pCurRxBuf + hciTrCb.rxCnt - 1;
				hciTrCb.paramLen = p[0];
			}
			else{
				/* We will never be here */
				hciTrCb.paramLen = 0xff00;
				hciTrCb.trState = HCI_TR_STATE_IDLE;
				HCI_Tr_TimeDisable();
			}

			if(hciTrCb.paramLen == 0){
				//array_printf(hciTrCb.pCurRxBuf, hciTrCb.rxCnt);
				pFifo->wptr++;
				hciTrCb.trState = HCI_TR_STATE_IDLE;
				HCI_Tr_TimeDisable();
				break;
			}
			else if(hciTrCb.paramLen + hciTrCb.rxCnt > hciTrCb.rxFifo->size){
				hciTrCb.trState = HCI_TR_STATE_IDLE;
				HCI_Tr_TimeDisable();
				break;
			}
		}

		if(hciTrCb.paramLen != 0)
		{
			if(hciTrCb.rxCnt == 1 + hciTrCb.frameHeadLen + hciTrCb.paramLen){
				//array_printf(hciTrCb.pCurRxBuf, hciTrCb.rxCnt);
				pFifo->wptr++;
				hciTrCb.trState = HCI_TR_STATE_IDLE;
				HCI_Tr_TimeDisable();
			}
			else if(hciTrCb.rxCnt > 1 + hciTrCb.frameHeadLen + hciTrCb.paramLen){
				hciTrCb.trState = HCI_TR_STATE_IDLE;
				HCI_Tr_TimeDisable();
			}
		}
		break;
	}

	case HCI_TR_STATE_ERR:

	default:
		hciTrCb.trState = HCI_TR_STATE_IDLE;
		HCI_Tr_TimeDisable();
		break;
	}
}

_attribute_ram_code_
void HCI_Tr_H4IRQHandler(void)
{
	/* UART Normal Rx IRQ */
	if(REG_ADDR8(0x9d) & 0x08)
	{
		for(int i=0; i<((REG_ADDR8(0x9c)) & 0x0f); i++)
		{
			u8 res = HCI_Tr_ReceiveByte();
			HCI_Tr_RxHandler(res);
		}
		REG_ADDR8(0x9d) = 0x08;
	}

	/* UART DMA Tx IRQ */
	if(reg_dma_irq_status & BIT(1)) {
		reg_dma_irq_status = BIT(1);//clear
	}

	/* Timer timeout IRQ */
	if(reg_tmr_sta & BIT(0)){
		reg_tmr_sta = BIT(0);
		HCI_Tr_TimeDisable();
		hciTrCb.trState = HCI_TR_STATE_IDLE;
	}
}
#endif /* End of HCI_H4_DMA_MODE_EN */


/******************************************************************************/
/*********    UART DMA Mode                                       *************/
/******************************************************************************/
#if HCI_H4_DMA_MODE_EN

u8 h4TrBackupBuf[HCI_H4_TR_RX_BUF_SIZE];
u8 h4TrRxBuf[HCI_H4_TR_RX_BUF_SIZE * HCI_H4_TR_RX_BUF_NUM];

static hci_fifo_t h4TrRxFifo = {
	HCI_H4_TR_RX_BUF_SIZE,
	HCI_H4_TR_RX_BUF_NUM,
	HCI_H4_TR_RX_BUF_NUM - 1,
	0,0,
	&h4TrRxBuf[0],
};

/*!  HCI Transmit main control block */
typedef struct{
	hci_fifo_t   *pHciRxFifo;  /*!< Point to HCI rx fifo. */
	hci_fifo_t   *pRxFifo;     /*!< Point to H4 rx fifo.  */
	u8           *pBackUpBuf;
	u8            isBackup;
	u8            backupCnt;
	u8            recvd1;
	u8            recvd2;
}HciH4TrCb_t;

static HciH4TrCb_t hciH4TrCB;

/**
 * @brief  : HCI Transport initialization.
 * @param  : pFifo    Pointer point to hci rx FIFO.
 * @return : None.
 */
void HCI_Tr_H4Init(hci_fifo_t *pHciRxFifo)
{
	hciH4TrCB.pHciRxFifo = pHciRxFifo;
	hciH4TrCB.pRxFifo    = &h4TrRxFifo;
	hciH4TrCB.pBackUpBuf = h4TrBackupBuf;
	hciH4TrCB.isBackup   = 0;
	hciH4TrCB.backupCnt  = 0;

	/* Timer configuration. */
	HCI_Tr_TimeInit(1000);

	/* UART configuration. */
	HCI_Tr_InitUart(1, (hciH4TrCB.pRxFifo->p + 4), hciH4TrCB.pRxFifo->size - 4);//4 for dma len
}

/**
 * @brief  : HCI Transmit backup handler
 * @param  : pPacket    Pointer point to data buffer.
 * @param  : len        The length of data.
 * @return : Returns the number of bytes processed.
 */
//_attribute_ram_code_
int HCI_Tr_H4BackUpHandler(u8 *pPacket, u32 len)
{
	u8 *pBuf = hciH4TrCB.pBackUpBuf;
	u8 backupLen = hciH4TrCB.backupCnt;

	if(backupLen == 0)
	{
		memcpy(pBuf, pPacket, len);
		hciH4TrCB.backupCnt = len;
		hciH4TrCB.isBackup = true;

		HCI_Tr_TimeEnable();
		return len;
	}

	u8 hciType = 0;
	BSTREAM_TO_UINT8(hciType, pBuf);

	u16 paramLen = 0;
	hci_fifo_t *pHciRxFifo = hciH4TrCB.pHciRxFifo;

	switch(hciType)
	{
	case HCI_TR_TYPE_CMD:
		if(backupLen >= 1 + HCI_CMD_HEAD_LEN)
		{
			pBuf += 2;//skip opcode
			BSTREAM_TO_UINT8(paramLen, pBuf);

			u16 cpyLen = 1 + HCI_CMD_HEAD_LEN + paramLen - hciH4TrCB.backupCnt;
			memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, min(cpyLen, len));
			hciH4TrCB.backupCnt += min(cpyLen, len);

			if(hciH4TrCB.backupCnt == 1 + HCI_CMD_HEAD_LEN + paramLen)
			{
				u8 *p = pHciRxFifo->p + (pHciRxFifo->wptr & (pHciRxFifo->num-1)) * pHciRxFifo->size;
				memcpy(p, hciH4TrCB.pBackUpBuf, hciH4TrCB.backupCnt);
				pHciRxFifo->wptr++;

				hciH4TrCB.isBackup = false;
				hciH4TrCB.backupCnt = 0;
				HCI_Tr_TimeDisable();
			}
			else
			{
				HCI_Tr_TimeEnable();
			}
			return min(cpyLen, len);
		}
		else
		{
			if(backupLen + len >= 1 + HCI_CMD_HEAD_LEN)
			{
				u16 cnt = 0;
				u16 cpyLen = 1 + HCI_CMD_HEAD_LEN - backupLen;
				memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, cpyLen);
				hciH4TrCB.backupCnt += cpyLen;
				pPacket += cpyLen;
				len -= cpyLen;
				cnt += cpyLen;

				pBuf += 2;//skip opcode
				BSTREAM_TO_UINT8(paramLen, pBuf);
				cpyLen = 1 + HCI_CMD_HEAD_LEN + paramLen - hciH4TrCB.backupCnt;
				memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, min(cpyLen, len));
				hciH4TrCB.backupCnt += min(cpyLen, len);
				cnt += min(cpyLen, len);

				if(hciH4TrCB.backupCnt == 1 + HCI_CMD_HEAD_LEN + paramLen){
					u8 *p = pHciRxFifo->p + (pHciRxFifo->wptr & (pHciRxFifo->num-1)) * pHciRxFifo->size;
					memcpy(p, hciH4TrCB.pBackUpBuf, hciH4TrCB.backupCnt);
					pHciRxFifo->wptr++;

					hciH4TrCB.isBackup = false;
					hciH4TrCB.backupCnt = 0;
					HCI_Tr_TimeDisable();
				}
				else{
					HCI_Tr_TimeEnable();
				}
				return cnt;
			}
			else
			{
				memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, len);
				hciH4TrCB.backupCnt += len;
				HCI_Tr_TimeEnable();
				return len;
			}
		}
		break;

	case HCI_TR_TYPE_ACL:
		if(backupLen >= 1 + HCI_ACL_HEAD_LEN)
		{
			pBuf += 2;//skip connHandle
			BSTREAM_TO_UINT16(paramLen, pBuf);
			u16 cpyLen = 1 + HCI_ACL_HEAD_LEN + paramLen - backupLen;
			memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, min(cpyLen, len));
			hciH4TrCB.backupCnt += min(cpyLen, len);

			if(hciH4TrCB.backupCnt == 1 + HCI_ACL_HEAD_LEN + paramLen)
			{
				HCI_Tr_TimeDisable();

				u8 *p = pHciRxFifo->p + (pHciRxFifo->wptr & (pHciRxFifo->num-1)) * pHciRxFifo->size;
				memcpy(p, hciH4TrCB.pBackUpBuf, hciH4TrCB.backupCnt);
				pHciRxFifo->wptr++;

				hciH4TrCB.isBackup = false;
				hciH4TrCB.backupCnt = 0;
			}
			else
			{
				HCI_Tr_TimeEnable();
			}
			return min(cpyLen, len);
		}
		else
		{
			if(backupLen + len >= 1 + HCI_ACL_HEAD_LEN)
			{
				u16 cnt = 0;
				u16 cpyLen = 1 + HCI_ACL_HEAD_LEN - backupLen;
				memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, cpyLen);
				hciH4TrCB.backupCnt += cpyLen;
				pPacket += cpyLen;
				len -= cpyLen;
				cnt += cpyLen;

				pBuf += 2;//skip connHandle
				BSTREAM_TO_UINT16(paramLen, pBuf);
				cpyLen = 1 + HCI_ACL_HEAD_LEN + paramLen - hciH4TrCB.backupCnt;
				memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, min(cpyLen, len));
				hciH4TrCB.backupCnt += min(cpyLen, len);
				cnt += min(cpyLen, len);

				if(hciH4TrCB.backupCnt == 1 + HCI_ACL_HEAD_LEN + paramLen){
					u8 *p = pHciRxFifo->p + (pHciRxFifo->wptr & (pHciRxFifo->num-1)) * pHciRxFifo->size;
					memcpy(p, hciH4TrCB.pBackUpBuf, hciH4TrCB.backupCnt);
					pHciRxFifo->wptr++;

					hciH4TrCB.isBackup = false;
					hciH4TrCB.backupCnt = 0;
					HCI_Tr_TimeDisable();
				}
				else{
					HCI_Tr_TimeEnable();
				}
				return cnt;
			}
			else
			{
				memcpy(hciH4TrCB.pBackUpBuf + hciH4TrCB.backupCnt, pPacket, len);
				hciH4TrCB.backupCnt += len;
				HCI_Tr_TimeEnable();
				return len;
			}
		}
		break;

	case HCI_TR_TYPE_EVENT:
	case HCI_TR_TYPE_SCO:
	default:
		return len;
	}

	/* we will never be here */
	return 0;
}

/**
 * @brief  : HCI Transmit RX handler
 * @param  : none.
 * @return : none.
 */
void HCI_Tr_H4RxHandler(void)
{
	hci_fifo_t *pRxFifo = hciH4TrCB.pRxFifo;
	if(pRxFifo->wptr == pRxFifo->rptr){
		return;//have no data
	}

	u8 *pPacket = pRxFifo->p + (pRxFifo->rptr & pRxFifo->mask) * pRxFifo->size;
	u32 len = 0;
	BSTREAM_TO_UINT32(len, pPacket);//DMA mode length

	hci_fifo_t *pHciRxFifo = hciH4TrCB.pHciRxFifo;

	while(len)
	{
		if(hciH4TrCB.isBackup){
			u8 res = HCI_Tr_H4BackUpHandler(pPacket, len);
			pPacket += res;
			len -= res;

			if(len == 0){
				continue;
			}
		}

		u8 *pBuf = pPacket;
		u8 hciType = 0;
		BSTREAM_TO_UINT8(hciType, pBuf);
		u16 paramLen = 0;
		u16 pktLen = 0;

		switch(hciType)
		{
		case HCI_TR_TYPE_CMD:
			if(len < 1 + HCI_CMD_HEAD_LEN)
			{
				HCI_Tr_H4BackUpHandler(pPacket, len);

				len = 0;
				continue;
			}

			pBuf += 2; //skip opcode
		    BSTREAM_TO_UINT8(paramLen, pBuf);
		    pktLen = 1 + HCI_CMD_HEAD_LEN + paramLen;
			break;

		case HCI_TR_TYPE_ACL:
			if(len < 1 + HCI_ACL_HEAD_LEN)
			{
				HCI_Tr_H4BackUpHandler(pPacket, len);

				len = 0;
				continue;
			}
			pBuf += 2; //skip connHandle
		    BSTREAM_TO_UINT16(paramLen, pBuf);
		    pktLen = 1 + HCI_ACL_HEAD_LEN + paramLen;
			break;

		case HCI_TR_TYPE_EVENT:
			if(len < 1 + HCI_EVT_HEAD_LEN)
			{
				len = 0;
				continue;
			}
			pBuf += 1; //skip evtCode
		    BSTREAM_TO_UINT8(paramLen, pBuf);
		    pktLen = 1 + HCI_EVT_HEAD_LEN + paramLen;
		    len -= pktLen;
		    pktLen = 0;
		    continue;

		case HCI_TR_TYPE_SCO:
			if(len < 1 + HCI_SCO_HEAD_LEN)
			{
				len = 0;
				continue;
			}
			pBuf += 2; //skip connHandler
		    BSTREAM_TO_UINT8(paramLen, pBuf);
		    pktLen = 1 + HCI_SCO_HEAD_LEN + paramLen;
		    len -= pktLen;
		    pktLen = 0;
		    continue;

		default:
			len = 0;
			continue;
		}

	    if(pktLen && len >= pktLen)
	    {
	    	//if(pFifo->wptr - pFifo->rptr >= pFifo->num){
	    	//	return; //have no memory.
	    	//}

	    	 /* Copy packet to HCI Rx buffer */
			 u8 *p = pHciRxFifo->p + (pHciRxFifo->wptr & (pHciRxFifo->num-1)) * pHciRxFifo->size;
			 memcpy(p, pPacket, pktLen);
			 pHciRxFifo->wptr++;

			 len -= pktLen;
			 pPacket += pktLen;
	    }
	    else if(pktLen && len < pktLen)
	    {
			HCI_Tr_H4BackUpHandler(pPacket, len);

			len = 0;
			continue;
	    }
	}/* while(len) */


	if(len == 0){
		pRxFifo->rptr++;
	}
}

bool isUartStable = false;

_attribute_ram_code_
void HCI_Tr_H4UartIRQHandler(void)
{
	/* UART TX IRQ */
	if(uart_get_irq_status(HCI_TR_UART_ID, UART_TXDONE))
	{
		//uart_clr_tx_done(HCI_TR_UART_ID);
	}

	/* UART RX err IRQ */
	if((uart_get_irq_status(HCI_TR_UART_ID, UART_RX_ERR))){
		uart_clr_irq_status(HCI_TR_UART_ID, UART_CLR_RX);//it will clear rx_fifo and rx_err_status,rx_done_irq.

		hci_fifo_t *pRxFifo = hciH4TrCB.pRxFifo;
		u8 *p = pRxFifo->p + (pRxFifo->wptr & pRxFifo->mask) * pRxFifo->size;
		uart_receive_dma(HCI_TR_UART_ID, (p+4), pRxFifo->size - 4);//[!!important]
	}
	/* UART RX IRQ */
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

		hci_fifo_t *pRxFifo = hciH4TrCB.pRxFifo;
		u8 *p = pRxFifo->p + (pRxFifo->wptr & pRxFifo->mask) * pRxFifo->size;

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
}

/**
 * @brief : H4 IRQ handler.
 * @param : none.
 * @return: none
 */
_attribute_ram_code_
void HCI_Tr_H4TimerIRQHandler(void)
{
	/* Timer timeout IRQ */
	if(reg_tmr_sta & TIMER_STAT_BIT){
		reg_tmr_sta = TIMER_STAT_BIT;
		HCI_Tr_TimeDisable();
		hciH4TrCB.isBackup = false;
		hciH4TrCB.backupCnt = 0;
	}
}


#endif /* End of HCI_H4_DMA_MODE_EN */
