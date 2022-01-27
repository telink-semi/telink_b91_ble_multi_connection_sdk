
#include "hci_port.h"
#include "compiler.h"
#include "hci_tr_api.h"

#include "hci_tr.h"


static u8 rxBuf[128];

void HCI_InitUart(void)
{
	HCI_Tr_InitUart(1, rxBuf, sizeof(rxBuf));
	HCI_Tr_TimeInit(100);
}

void HCI_Tr_InitUart(u8 isDmaMode, u8 *rxBuf, u32 byteNum)
{
	u16 div = 0;
	u8 bwpc = 0;

	uart_reset(HCI_TR_UART_ID);

	uart_set_pin(HCI_TR_TX_PIN, HCI_TR_RX_PIN);//UART0_TX_PB2,UART0_RX_PB3
	//uart_set_pin(UART0_TX_PA3,UART0_RX_PA4);

	uart_cal_div_and_bwpc(HCI_TR_BAUDRATE, sys_clk.pclk*1000*1000, &div, &bwpc);
	uart_init(HCI_TR_UART_ID, div, bwpc, UART_PARITY_NONE, UART_STOP_BIT_ONE);

	if(isDmaMode)
	{
		uart_set_dma_rx_timeout(HCI_TR_UART_ID, bwpc, 12, UART_BW_MUL3);//[!!important] //UART_BW_MUL2

		uart_set_rx_dma_config(HCI_TR_UART_ID, UART_DMA_CHN_RX);
		uart_set_tx_dma_config(HCI_TR_UART_ID, UART_DMA_CHN_TX);

		//uart_clr_tx_done(HCI_TR_UART_ID);

		dma_clr_irq_mask(UART_DMA_CHN_TX, TC_MASK|ABT_MASK|ERR_MASK);//disable UART DMA TX IRQ
		dma_clr_irq_mask(UART_DMA_CHN_RX, TC_MASK|ABT_MASK|ERR_MASK);//disable UART DMA RX IRQ
	}

	uart_set_irq_mask(HCI_TR_UART_ID, UART_RXDONE_MASK);//enable UART RX IRQ
	//uart_set_irq_mask(HCI_TR_UART_ID, UART_TXDONE_MASK);//enable UART RX IRQ
	uart_clr_irq_mask(HCI_TR_UART_ID, UART_TXDONE_MASK);
	plic_interrupt_enable(HCI_TR_UART_IRQn);        //enable UART global IRQ

	uart_receive_dma(HCI_TR_UART_ID, rxBuf, byteNum);   //set UART DMA RX buffer.
}

_attribute_ram_code_
void HCI_UART_IRQHandler(void)
{
#if 1
	HCI_Tr_UartIRQHandler();

#else//for test
    if(uart_get_irq_status(HCI_TR_UART_ID, UART_RXDONE)) //A0-SOC can't use RX-DONE status,so this interrupt can only used in A1-SOC.
    {
		if((uart_get_irq_status(HCI_TR_UART_ID, UART_RX_ERR)))
    	{
    		uart_clr_irq_status(HCI_TR_UART_ID, UART_CLR_RX);//it will clear rx_fifo and rx_err_status,rx_done_irq.
    	}

		/* Get the length of Rx data */
		u32 rxLen = 0;
    	if(((reg_uart_status1(HCI_TR_UART_ID) & FLD_UART_RBCNT) % 4)==0){
    		rxLen = 4 * (0xffffff - reg_dma_size(UART_DMA_CHN_RX));
    	}
    	else{
    		rxLen = 4 * (0xffffff - reg_dma_size(UART_DMA_CHN_RX)-1) + (reg_uart_status1(HCI_TR_UART_ID) & FLD_UART_RBCNT) % 4;
    	}

    	if(rxLen && !uart_tx_is_busy(HCI_TR_UART_ID)){
    		uart_send_dma(HCI_TR_UART_ID, rxBuf, rxLen);
    	}

    	/* Clear RxDone state */
    	uart_clr_irq_status(HCI_TR_UART_ID, UART_CLR_RX);
    	uart_receive_dma(HCI_TR_UART_ID, rxBuf, sizeof(rxBuf));//[!!important - must]
    }

    if(uart_get_irq_status(HCI_TR_UART_ID, UART_TXDONE))
	{
	    uart_clr_tx_done(HCI_TR_UART_ID);
	}
#endif
}


_attribute_ram_code_
void HCI_TIMER_IRQHandler(void)
{
	HCI_Tr_TimerIRQHandler();
}

