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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "stack/ble/profile/audio/audio_cap.h"

#include "app_config.h"
#if (LE_AUDIO_DEMO_ENABLE)
#include "app_att.h"
#include "app_lc3.h"
#include "app_usb.h"
#include "app_audio.h"


#define AUDIO_TO_USB_ENABLE     (1 && APP_AUDIO_USB_ENABLE)
#define AUDIO_TO_SPK_ENABLE     (1)

#define APP_AUDIO_MIC_BUFFER_SIZE    (APP_AUDIO_FRAME_BYTES)
#define APP_AUDIO_SPK_EXTEND_SIZE    (32*3) //3ms, <APP_AUDIO_CIS_INTERVEL

extern u16 app_aclConnHandle;
extern u16 app_cisConnHandle;

static int app_audio_eventCB(u16 connHandle, u16 evtID, u16 dataLen, u8 *pData);

u32 gAppAudioDelayTimer = 0;
u8 gAppAudioBuffer[APP_AUDIO_MIC_BUFFER_SIZE+APP_AUDIO_SPK_EXTEND_SIZE]; //Extended 2ms
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

volatile u32 AAAA_audios_test01 = 0;
volatile u32 AAAA_audios_test02 = 0;
volatile u32 AAAA_audios_test03 = 0;
volatile u32 AAAA_audios_test04 = 0;
volatile u32 AAAA_audios_test05 = 0;
volatile u32 AAAA_audios_test06 = 0;
volatile u32 AAAA_audios_test07 = 0;
volatile u32 AAAA_audios_test08 = 0;

extern iso_data_packet_t* blc_isoal_popCisSdu(u16 cis_connHandle);

void app_audio_init(void)
{
	gAppAudioEnable = false;
	
	#if (APP_AUDIO_CIS_INTERVEL == 10)
	app_lc3_init(APP_LC3_INTERVAL_10MS);
	#else
	app_lc3_init(APP_LC3_INTERVAL_7_5MS);
	#endif

	blc_audio_init();
	app_audio_gatt_init();
	blc_audio_regEventCBByRole(BLC_AUDIO_ROLE_SERVER, app_audio_eventCB);
	
	
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
	if(enable) gAppAudioEnable = true;
	else gAppAudioEnable = false;

	if(enable){
		gAppAudioEnable = true;
		
	}else{
		gAppAudioEnable = false;
		#if (AUDIO_TO_SPK_ENABLE)
//			unsigned int r=core_interrupt_disable();
			audio_clk_en(0,0);
			audio_rx_dma_dis();
			audio_codec_adc_power_down();
//			core_restore_interrupt(r);
		#endif
	}
		
	return 0;
}


void app_audio_acl_connect(u16 aclHandle, u8 *pPkt, bool isReconn)
{
	my_dump_str_data(APP_DUMP_EN, "acl_connect:", (u8*)&aclHandle, 2);
	
	if(isReconn){
		blc_audio_setConnState(aclHandle, BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_STATE_RECONNECT);
	}else{
		blc_audio_setConnState(aclHandle, BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_STATE_CONNECT);
	}
	
}
void app_audio_acl_disconn(u16 aclHandle, u8 *pPkt)
{
	my_dump_str_data(APP_DUMP_EN, "acl_disconn:", (u8*)&aclHandle, 2);
	gAppAudioIsRecv = false;
	gAppAudioDelayTimer = 0;
	blc_audio_setConnState(aclHandle, BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_STATE_DISCONN);
}
void app_audio_cis_request(u16 aclHandle, u16 cisHandle, u8 *pPkt)
{
	my_dump_str_data(APP_DUMP_EN, "cis_request:", (u8*)&cisHandle, 2);
	#if (BLC_AUDIO_ASCS_ENABLE)
	blc_audio_ascpCisRequestEvt(aclHandle, cisHandle, pPkt);
	#endif
}
void app_audio_cis_connect(u16 cisHandle, u8 *pPkt)
{
	u16 aclHandle = blc_audio_getConnHandle(cisHandle, BLC_AUDIO_ROLE_SERVER);
	if(aclHandle == 0) return;
	
	my_dump_str_data(APP_DUMP_EN, "cis_connect:", (u8*)&cisHandle, 2);
	gAppAudioDelayTimer = 0;
	#if (BLC_AUDIO_ASCS_ENABLE)
	blc_audio_ascpCisConnectEvt(aclHandle, cisHandle, pPkt);
	#endif
}
void app_audio_cis_disconn(u16 cisHandle, u8 *pPkt)
{
	u16 aclHandle = blc_audio_getConnHandle(cisHandle, BLC_AUDIO_ROLE_SERVER);
	if(aclHandle == 0) return;
	
	my_dump_str_data(APP_DUMP_EN, "cis_disconn:", (u8*)&cisHandle, 2);
	gAppAudioIsRecv = false;
	gAppAudioDelayTimer = 0;
	#if (BLC_AUDIO_ASCS_ENABLE)
	blc_audio_ascpCisDisconnEvt(aclHandle, cisHandle, pPkt);
	#endif
}


static int app_audio_eventCB(u16 connHandle, u16 evtID, u16 dataLen, u8 *pData)
{
	if(evtID == BLC_AUDIO_EVTID_SDP_OVER){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_SDP_OVER\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_ASE_ENABLE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_ENABLE\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_ASE_UPDATE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_UPDATE\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_ASE_DISABLE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_DISABLE\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_ASE_RELEASE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_RELEASE\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_RECV_START){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_RECV_START\r\n", NULL, 0);
		gAppAudioIsRecv = true;
		gAppAudioDelayTimer = clock_time()|1;
		app_audio_enable(true);
	}else if(evtID == BLC_AUDIO_EVTID_RECV_STOP){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_RECV_STOP\r\n", NULL, 0);
		gAppAudioIsRecv = false;
		gAppAudioDelayTimer = 0;
		//app_audio_enable(false);
	}else if(evtID == BLC_AUDIO_EVTID_MICS_AICS_STATE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_MICS_AICS_STATE\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_VCS_STATE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_VCS_STATE\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIOS_EVTID_MCS_READY){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOS_EVTID_MCS_READY\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIOS_EVTID_MCS_NOTY){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOS_EVTID_MCS_NOTY\r\n", NULL, 0);
	}else if(evtID == BLC_AUDIOS_EVTID_MCS_STATE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOS_EVTID_MCS_STATE\r\n", NULL, 0);
	}
	
	return 0;
}


int app_audio_gattHandler(u16 connHandle, u8 *pkt)
{
	return blc_audio_gattHandler(connHandle, pkt);
}

void app_audio_handler(void)
{
	blc_audio_handler();
	
	if(gAppAudioIsRecv && gAppAudioIsPlay && gAppAudioDelayTimer){
		if(gAppAudioDelayTimer != 0 && clock_time_exceed(gAppAudioDelayTimer, 50000)){
			gAppAudioDelayTimer = 0;
		}
		return;
	}
	if(gAppAudioIsRecv &&!gAppAudioIsPlay){
		iso_data_packet_t *pPkt = blc_isoal_popCisSdu(app_cisConnHandle);
		if(pPkt != NULL){
			u8 inLen;
			int ret;
			#if (APP_AUDIO_CIS_INTERVEL == 10)
			inLen = 40;
			#else
			inLen = 30;
			#endif
			if(pPkt->iso_dl_len == inLen+8){
				ret = app_lc3_decode((u8*)(pPkt->data+8), inLen, gAppAudioBuffer);
			}else{
				ret = app_lc3_decode(NULL, inLen, gAppAudioBuffer);
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
		}
	}
	#if (AUDIO_TO_USB_ENABLE)
	if(gAppAudioUsbIsPlay && app_usb_fill_buffer(gAppAudioBuffer)){
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
#endif
