/********************************************************************************************************
 * @file	app_audio.c
 *
 * @brief	This is the source file for BLE SDK
 *
 * @author	BLE GROUP
 * @date	2021.01
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
#include "app_config.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#if (__PROJECT_B91_AUDIO_BIS_BCST__)
#if (LE_AUDIO_DEMO_ENABLE)
#include "app_buffer.h"
#include "app_audio.h"
#include "app_lc3.h"


#define APP_AUDIO_INPUT_DMIC      0
#define APP_AUDIO_INPUT_LINEIN    1
#define APP_AUDIO_INPUT_SOURCE    APP_AUDIO_INPUT_DMIC


u8 gAppAudioOutSDU[APP_AUDIO_COMP_BUFF_LEN+40] = {
		0,0,0,0,
		0,0,
		0,0,

		/* HCI ISO data header (4B) */
		0x20, 0x40,  //handle
		U16_LO(APP_AUDIO_COMP_BUFF_LEN+8), U16_HI(APP_AUDIO_COMP_BUFF_LEN+8),

		/*** ISO SDU Load ***/

		/* ISO Data header(8B) */
		0,0,0,0,	//time-stamp (TS_flag == 1)
		1,0,        //packet SN
		U16_LO(APP_AUDIO_COMP_BUFF_LEN), U16_HI(APP_AUDIO_COMP_BUFF_LEN),  //sdu_len

		/* ISO Data */
		1,2,3,4,5,6,7,8,9,10,
};


#define APP_AUDIO_MIC_BUFFER_SIZE    (APP_AUDIO_FRAME_BYTES << 1)

u8	 gAppAudioBuffer[APP_AUDIO_MIC_BUFFER_SIZE];
bool gAppAudioEnable = false;
bool gAppAudioIsSend = false;

extern u8  app_bisBcstNum;
extern u16 app_bisBcstHandle[APP_BIS_NUM_IN_PER_BIG_BCST];

int app_audio_init(void)
{
	#if (APP_AUDIO_BIS_INTERVEL == 10)
	app_lc3_init(APP_LC3_INTERVAL_10MS);
	#else
	app_lc3_init(APP_LC3_INTERVAL_7_5MS);
	#endif

//	gAppAudioEnable = false;
	gAppAudioIsSend = true;

	app_audio_enable(true);
	#if (APP_AUDIO_DMIC_ENABLE)
		#if (APP_AUDIO_INPUT_SOURCE == APP_AUDIO_INPUT_DMIC)
		audio_set_dmic_pin(DMIC_GROUPD_D4_DAT_D5_D6_CLK);// DMIC_GROUPB_B2_DAT_B3_B4_CLK
		audio_init(DMIC_IN_TO_BUF_TO_LINE_OUT, AUDIO_16K, MONO_BIT_16);
		#elif (APP_AUDIO_INPUT_SOURCE == APP_AUDIO_INPUT_LINEIN)
		audio_init(LINE_IN_TO_BUF_TO_LINE_OUT, AUDIO_16K, MONO_BIT_16);
		#endif
	audio_rx_dma_chain_init(DMA2, (u16*)gAppAudioBuffer, APP_AUDIO_MIC_BUFFER_SIZE);
	audio_tx_dma_chain_init(DMA3, (u16*)gAppAudioBuffer, APP_AUDIO_MIC_BUFFER_SIZE);
	#endif
	
	return 0;
}
void app_audio_deinit(void)
{
	
}

int app_audio_enable(bool enable)
{
	if(enable){
		
		gAppAudioEnable = true;
	}else{
		gAppAudioEnable = false;
		audio_clk_en(0,0);
		audio_rx_dma_dis();
		audio_codec_adc_power_down();
	}
	return 0;
}


void app_audio_handler(void)
{
	app_audio_send_handler();
}

void app_audio_send_handler(void)
{
	s16 *ps;
	static u16 sRawDataRptr = 0;
	u16 micWptr = (audio_get_rx_dma_wptr(DMA2) - (u32)gAppAudioBuffer) >> 1;
	u16 l = (micWptr >= sRawDataRptr) ? (micWptr-sRawDataRptr) : 0xffff;
	
	if(l < APP_AUDIO_FRAME_SAMPLE) return;

	if(!gAppAudioEnable || !gAppAudioIsSend) return;

	ps = ((s16*)gAppAudioBuffer) + sRawDataRptr;
	
	//app_usb_fill_buffer((u8*)ps);
	app_lc3_encode((u8*)ps, APP_AUDIO_FRAME_BYTES, gAppAudioOutSDU+20);
	
	//my_dump_str_data(APP_DUMP_EN,"lc3 encode", &ret, 1);
	DBG_CHN8_TOGGLE;

	//send BIS PDU dft: unframed PDU, framed PDU TODO: latter
	for(int i = 0; i < app_bisBcstNum; i++){

//		u16 bis_idx = app_bisBcstHandle[i] & BLT_BIS_IDX_MSK;
//		ll_bis_t *pBis = (ll_bis_t *) (global_pBis + bis_idx);
//		ll_big_bcst_t* pBig = global_pBigBcst + pBis->big_idx;
//
//		if(pBig->framing==0){
//			blc_ial_bis_splitSdu2UnframedPdu(app_bisBcstHandle[i], (iso_data_packet_t*)gAppAudioOutSDU);
//		}
//		else{
//			extern iso_sdu_mng_t sduBisMng;
//			u8 *p = (sduBisMng.tx_fifo_b + sduBisMng.max_tx_size * \
//										(bis_idx*sduBisMng.tx_fifo_num + (pBis->txSduFifoWptr&(sduBisMng.tx_fifo_num-1))));
//			smemcpy(p, gAppAudioOutSDU, APP_AUDIO_COMP_BUFF_LEN+40);
//			pBis->txSduFifoWptr++;
//		}
		//my_dump_str_data(APP_DUMP_EN,"sdu2pdu", &ret, 1);
	}

	sRawDataRptr = sRawDataRptr ? 0 : APP_AUDIO_FRAME_SAMPLE;
}


#endif //LE_AUDIO_DEMO_ENABLE
#endif //#if (__PROJECT_B91_AUDIO_BIS_BCST__)
