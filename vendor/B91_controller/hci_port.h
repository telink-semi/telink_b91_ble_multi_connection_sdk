
#ifndef _HCI_PORT_H
#define _HCI_PORT_H

#include "drivers.h"
#include "hci_tr_h4.h"
#include "types.h"

#define UART_DMA_CHN_RX  DMA2
#define UART_DMA_CHN_TX  DMA3

#define UART_DMA_RX_IRQ_CHN  FLD_DMA_CHANNEL2_IRQ
#define UART_DMA_TX_IRQ_CHN  FLD_DMA_CHANNEL3_IRQ

#define TIMER           TIMER0
#define TIMER_EN_BIT    FLD_TMR0_EN
#define TIMER_STAT_BIT  TMR_STA_TMR0

void HCI_InitUart(void);//for test

void HCI_Tr_InitUart(u8 isDmaMode, u8 *rxBuf, u32 byteNum);

static inline void HCI_Tr_TimeInit(u16 ms)
{
	timer_set_mode(TIMER, 0);
	timer_set_init_tick(TIMER, 0);
	timer_set_cap_tick(TIMER, ms*1000*sys_clk.pclk);
	plic_interrupt_enable(IRQ4_TIMER0);
	//timer_start(TIMER);
	reg_tmr_ctrl0 |= TIMER_EN_BIT;

#if !HCI_H4_DMA_MODE_EN
	reg_tmr_ctrl0 |= TIMER_EN_BIT;
#endif
}

static inline void HCI_Tr_TimeEnable(void)
{
	reg_tmr_tick(TIMER) = 0;
	reg_tmr_ctrl0 |= TIMER_EN_BIT;
}

static inline void HCI_Tr_TimeDisable(void)
{
	reg_tmr_tick(TIMER) = 0;
	reg_tmr_ctrl0 &= ~TIMER_EN_BIT;
}

#endif
