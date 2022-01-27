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

#if (__PROJECT_B91_AUDIO_BIS_SYNC__)
#if (LE_AUDIO_DEMO_ENABLE)
#include "app_buffer.h"
#include "app_lc3.h"
#include "app_usb.h"
#include "app_audio.h"


#define AUDIO_TO_USB_ENABLE     		(1 && APP_AUDIO_USB_ENABLE)
#define AUDIO_TO_SPK_ENABLE     		(1)

#define APP_AUDIO_MIC_BUFFER_SIZE    	(APP_AUDIO_FRAME_BYTES)
#define APP_AUDIO_SPK_EXTEND_SIZE    	(32*3) //3ms, <APP_AUDIO_BIS_INTERVEL


u32  gAppAudioDelayTimer = 0;
u8   gAppAudioBuffer[APP_AUDIO_MIC_BUFFER_SIZE+APP_AUDIO_SPK_EXTEND_SIZE]; //Extended 2ms
bool gAppAudioEnable = false;
bool gAppAudioIsRecv = false;
bool gAppAudioIsPlay = false;
bool gAppAudioSpkIsPlay = false;
bool gAppAudioUsbIsPlay = false;


#if (AUDIO_TO_SPK_ENABLE)
#define APP_AUDIO_SPK_BUFFER_SIZE       2048
#define APP_AUDIO_SPK_FILL_SIZE         (APP_AUDIO_MIC_BUFFER_SIZE+APP_AUDIO_SPK_EXTEND_SIZE)
#define APP_AUDIO_SPK_JUDGE1_SIZE       (32*3) //5ms
#define APP_AUDIO_SPK_JUDGE2_SIZE       (32*4) //5ms

volatile u16 gAppAudioSpkWptr = 0;
volatile u16 gAppAudioSpkRptr = 0;
u8 gAppAudioSpkBuffer[APP_AUDIO_SPK_BUFFER_SIZE] = {0};

bool app_audio_spk_fill_buffer(u8 *pData, u16 dataLen);
#endif //AUDIO_TO_SPK_ENABLE

extern u8  app_bisSyncNum;
extern u16 app_bisSyncHandle[APP_BIS_NUM_IN_PER_BIG_SYNC];

extern iso_data_packet_t* blc_isoal_popBisSdu(u16 bis_connHandle);


void app_audio_init(void)
{
	gAppAudioEnable = false;
	
	#if (APP_AUDIO_BIS_INTERVEL == 10)
	app_lc3_init(APP_LC3_INTERVAL_10MS);
	#else
	app_lc3_init(APP_LC3_INTERVAL_7_5MS);
	#endif

	
	//app_audio_enable(true);

	#if (AUDIO_TO_SPK_ENABLE)
	gAppAudioSpkWptr = 0;
	gAppAudioSpkRptr = 0;
	audio_set_dmic_pin(DMIC_GROUPD_D4_DAT_D5_D6_CLK);// DMIC_GROUPB_B2_DAT_B3_B4_CLK
	audio_init(BUF_TO_LINE_OUT, AUDIO_16K, MONO_BIT_16);
	audio_tx_dma_chain_init(DMA3, (u16*)gAppAudioSpkBuffer, APP_AUDIO_SPK_BUFFER_SIZE);
	#endif
	
}


void app_audio_deinit(void)
{
	
}

int app_audio_enable(bool enable)
{
	if(enable){
		gAppAudioEnable = true;
		audio_tx_dma_en();	//TODO: Currently, it is temporarily resolved, directly close the DMA TX of the audio codec
							//Seems to meet the requirements
	}else{
		gAppAudioEnable = false;
		#if (AUDIO_TO_SPK_ENABLE)
		unsigned int r=core_interrupt_disable();
//		audio_clk_en(0,0);
//		audio_rx_dma_dis();
//		audio_codec_adc_power_down();
		audio_tx_dma_dis(); //TODO: Currently, it is temporarily resolved, directly close the DMA TX of the audio codec
							//Seems to meet the requirements
		core_restore_interrupt(r);
		#endif
	}
		
	return 0;
}

void app_audio_recv_start(void)
{
	gAppAudioIsRecv = true;
	gAppAudioDelayTimer = clock_time()|1;
	app_audio_enable(true);
}

void app_audio_recv_stop(void)
{
	gAppAudioIsRecv = false;
	gAppAudioDelayTimer = 0;
	app_audio_enable(false);
}

void app_audio_handler(void)
{
	if(!gAppAudioIsRecv){
		return;
	}

	if(gAppAudioIsRecv && gAppAudioIsPlay && gAppAudioDelayTimer){
		if(gAppAudioDelayTimer != 0 && clock_time_exceed(gAppAudioDelayTimer, 50000)){
			gAppAudioDelayTimer = 0;
		}
		return;
	}
	
	if(gAppAudioIsRecv && !gAppAudioIsPlay){
		for(int i = 0; i < app_bisSyncNum; i++){
			iso_data_packet_t *pPkt = blc_isoal_popBisSdu(app_bisSyncHandle[i]); //TODO  qinghua re-check
			if(pPkt != NULL){

				//my_dump_str_data(APP_DUMP_EN, "***** Get SDU *****", ((u8*)pPkt+8), 4+pPkt->iso_dl_len);

				u8 inLen;
				int ret;
				#if (APP_AUDIO_BIS_INTERVEL == 10)
				inLen = 40;
				#else
				inLen = 30;
				#endif
				u8 isoDlHdrLen = (pPkt->ts ? HCI_ISO_LOAD_HDR_LEN_MAX : HCI_ISO_LOAD_HDR_LEN_MIN);
				if(pPkt->iso_dl_len == inLen + isoDlHdrLen){
					ret = app_lc3_decode((u8*)(pPkt->data + isoDlHdrLen), inLen, gAppAudioBuffer);
					DBG_CHN8_TOGGLE;
				}else{
					ret = app_lc3_decode(NULL, inLen, gAppAudioBuffer);
					DBG_CHN11_TOGGLE;
				}
				if(ret == 0){ //success
					gAppAudioIsPlay = true;
					#if (AUDIO_TO_USB_ENABLE)
					gAppAudioUsbIsPlay = true;
					#endif
					#if (AUDIO_TO_SPK_ENABLE)
					{
						u16 i;
						u8 *pData = gAppAudioBuffer+APP_AUDIO_MIC_BUFFER_SIZE;
						for(i=0; i<APP_AUDIO_SPK_EXTEND_SIZE; i++){
							pData[i] = gAppAudioBuffer[APP_AUDIO_MIC_BUFFER_SIZE-i-1];
						}
					}
					gAppAudioSpkIsPlay = true;
					#endif
				}
				else{
					my_dump_str_data(APP_DUMP_EN, "lc3_decode err", &ret, 4);
				}
			}
			else{
				//my_dump_str_data(APP_DUMP_EN, "Get BIS SDU NULL", 0, 0);
			}
		}
	}
	#if (AUDIO_TO_USB_ENABLE)
	if(gAppAudioUsbIsPlay && app_usb_fill_buffer(gAppAudioBuffer)){
		DBG_CHN9_TOGGLE;
		gAppAudioUsbIsPlay = false;
	}
	#endif
	
	#if (AUDIO_TO_SPK_ENABLE)
	if(gAppAudioSpkIsPlay && app_audio_spk_fill_buffer(gAppAudioBuffer, APP_AUDIO_SPK_FILL_SIZE)){
		gAppAudioSpkIsPlay = false;
	}
	#endif

	if(gAppAudioIsPlay){
		#if (AUDIO_TO_USB_ENABLE || AUDIO_TO_SPK_ENABLE)
			#if (AUDIO_TO_USB_ENABLE)
			if(!gAppAudioUsbIsPlay) gAppAudioIsPlay = false;
			#endif
			#if (AUDIO_TO_SPK_ENABLE)
			if(!gAppAudioSpkIsPlay) gAppAudioIsPlay = false;
			#endif
		#else
			gAppAudioIsPlay = true;
		#endif
	}
}

#if (AUDIO_TO_SPK_ENABLE)
bool app_audio_spk_fill_buffer(u8 *pData, u16 dataLen)
{
	u16 unUsed;
	u16 offset;
	volatile u16 wptr;
	volatile u16 rptr;
	
	
	if(pData == NULL || dataLen == 0 || (dataLen & 0x01)) return true;

	wptr = gAppAudioSpkWptr;
	rptr = ((audio_get_tx_dma_rptr(DMA3)-(unsigned int)gAppAudioSpkBuffer));

	gAppAudioSpkRptr = rptr;
	
	if(rptr > wptr) unUsed = rptr-wptr;
	else unUsed = APP_AUDIO_SPK_BUFFER_SIZE+rptr-wptr;

	if((rptr > wptr && unUsed < APP_AUDIO_SPK_JUDGE1_SIZE)){
		
	}
	if((rptr > wptr && unUsed < APP_AUDIO_SPK_JUDGE1_SIZE)
		|| (wptr > rptr && unUsed < APP_AUDIO_SPK_JUDGE2_SIZE 
		&& rptr < APP_AUDIO_SPK_JUDGE1_SIZE
		&& wptr > APP_AUDIO_SPK_BUFFER_SIZE/2)){
		unUsed = APP_AUDIO_SPK_BUFFER_SIZE;
	}
	if(unUsed <= dataLen) return false;
	
	if(unUsed == APP_AUDIO_SPK_BUFFER_SIZE){
		gAppAudioSpkWptr = rptr+32;
	}
	if(gAppAudioSpkWptr >= APP_AUDIO_SPK_BUFFER_SIZE){
		gAppAudioSpkWptr = 0;
	}

	if(gAppAudioSpkWptr+dataLen >  APP_AUDIO_SPK_BUFFER_SIZE){
		offset = APP_AUDIO_SPK_BUFFER_SIZE-gAppAudioSpkWptr;
	}else{
		offset = dataLen;
	}
	memcpy(gAppAudioSpkBuffer+gAppAudioSpkWptr, pData, offset);
	if(offset < dataLen){
		memcpy(gAppAudioSpkBuffer, pData+offset, dataLen-offset);
	}

	gAppAudioSpkWptr += APP_AUDIO_MIC_BUFFER_SIZE;
	if(gAppAudioSpkWptr >= APP_AUDIO_SPK_BUFFER_SIZE){
		gAppAudioSpkWptr -= APP_AUDIO_SPK_BUFFER_SIZE;
	}
	
	return true;
}
#endif
#endif //#if (__PROJECT_B91_AUDIO_BIS_SYNC__)

#endif
