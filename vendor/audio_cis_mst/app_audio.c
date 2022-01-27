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
#include "app_buffer.h"
#include "app_audio.h"
#include "app_att.h"
#include "app_lc3.h"


#define APP_AUDIO_INPUT_DMIC      0
#define APP_AUDIO_INPUT_LINEIN    1
#define APP_AUDIO_INPUT_SOURCE    APP_AUDIO_INPUT_DMIC

static int app_audio_eventCB(u16 connHandle, u16 evtID, u16 dataLen, u8 *pData);
void app_audio_ase_start(void);

_attribute_ble_data_retention_  int		audio_sdp_finish[MASTER_MAX_NUM] = {0};


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

u8 gAppAudioBuffer[APP_AUDIO_MIC_BUFFER_SIZE];
bool gAppAudioEnable = false;
bool gAppAudioIsSend = false;

extern u16 app_cisConnHandle[TEST_CIS_COUNT];
extern u16 app_aclConnHandle[TEST_CIS_COUNT];

u8 gAppAudioConn0Ase[2] = {0};
u8 gAppAudioConn1Ase[2] = {0};

u8 cis_id[] = {0, 1, 2, 3};

u8 cisCreateDoing = 0;
u8 cisCreatePending = 0;

int app_audio_init(void)
{
	#if (APP_AUDIO_CIS_INTERVEL == 10)
	app_lc3_init(APP_LC3_INTERVAL_10MS);
	#else
	app_lc3_init(APP_LC3_INTERVAL_7_5MS);
	#endif

	blc_audio_init();
	app_audio_gatt_init();
	blc_audio_regEventCBByRole(BLC_AUDIO_ROLE_CLIENT, app_audio_eventCB);

	gAppAudioEnable = false;
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


void app_audio_acl_connect(u16 aclHandle, u8 *pPkt, bool isReconn)
{
	my_dump_str_data(APP_DUMP_EN, "acl_connect:", (u8*)&aclHandle, 2);
	if(isReconn){
		blc_audio_setConnState(aclHandle, BLC_AUDIO_ROLE_CLIENT, BLC_AUDIO_STATE_RECONNECT);
	}else{
		blc_audio_setConnState(aclHandle, BLC_AUDIO_ROLE_CLIENT, BLC_AUDIO_STATE_CONNECT);
	}
}
void app_audio_acl_disconn(u16 aclHandle, u8 *pPkt)
{
	my_dump_str_data(APP_DUMP_EN, "acl_disconn:", (u8*)&aclHandle, 2);
	blc_audio_setConnState(aclHandle, BLC_AUDIO_ROLE_CLIENT, BLC_AUDIO_STATE_DISCONN);
	if(aclHandle != 0 && app_aclConnHandle[0] == aclHandle){
		gAppAudioConn0Ase[0] = 0; gAppAudioConn0Ase[1] = 0;
	}
	if(aclHandle != 0 && app_aclConnHandle[1] == aclHandle){
		gAppAudioConn1Ase[0] = 0; gAppAudioConn1Ase[1] = 0;
	}
	cisCreateDoing = false;
	cisCreatePending = 0;
}
void app_audio_cis_connect(u16 cisHandle, u8 *pPkt)
{
	u16 aclHandle = blc_audio_getConnHandle(cisHandle, BLC_AUDIO_ROLE_CLIENT);
	if(aclHandle == 0) return;
	
	my_dump_str_data(APP_DUMP_EN, "cis_connect:", (u8*)&cisHandle, 2);
	#if (BLC_AUDIO_ASCS_ENABLE)
	blc_audio_ascpCisConnectEvt(aclHandle, cisHandle, pPkt);
	#endif
	if(cisHandle != 0 && app_cisConnHandle[0] == cisHandle){
		gAppAudioConn0Ase[0] = 0; gAppAudioConn0Ase[1] = 0;
	}
	if(cisHandle != 0 && app_cisConnHandle[1] == cisHandle){
		gAppAudioConn1Ase[0] = 0; gAppAudioConn1Ase[1] = 0;
	}
	cisCreateDoing = false;
}
void app_audio_cis_disconn(u16 cisHandle, u8 *pPkt)
{
	u16 aclHandle = blc_audio_getConnHandle(cisHandle, BLC_AUDIO_ROLE_CLIENT);
	if(aclHandle == 0) return;
	
	my_dump_str_data(APP_DUMP_EN, "cis_disconn:", (u8*)&cisHandle, 2);
	#if (BLC_AUDIO_ASCS_ENABLE)
	blc_audio_ascpCisDisconnEvt(aclHandle, cisHandle, pPkt);
	#endif
	cisCreateDoing = false;
}




int  app_audio_gattHandler(u16 connHandle, u8 *pkt)
{
	return blc_audio_gattHandler(connHandle, pkt);
}

extern int		startCISTest;
void app_audio_handler(void)
{
	#if 1//(BLC_AUDIO_ASCS_ENABLE)
	if(startCISTest && audio_sdp_finish[1]){
		if(app_aclConnHandle[0] != 0 && app_cisConnHandle[0] == 0 && app_aclConnHandle[1] != 0 && app_cisConnHandle[1] == 0){
			if(gAppAudioConn0Ase[0] != 0){
				my_dump_str_data(APP_DUMP_EN,"blc_audio_ascpEnableAse", 0, 0);
				blc_audio_ascpEnableAse(app_aclConnHandle[0], gAppAudioConn0Ase[0]);
			}
		}
//		if(app_aclConnHandle[1] != 0 && app_cisConnHandle[1] == 0){
//			if(gAppAudioConn1Ase[0] != 0){
//				blc_audio_ascpEnableAse(app_aclConnHandle[1], gAppAudioConn1Ase[0]);
//			}
//		}


		startCISTest = 0;
		audio_sdp_finish[1] = 0;
	}
	#endif
	
	blc_audio_handler();
	app_audio_send_handler();

	if(cisCreateDoing){
		return;
	}
	if(cisCreatePending){
		cisCreatePending = 0;
		app_audio_ase_start();
	}
}

void app_audio_send_handler(void)
{
	s16 *ps;
	static u16 sRawDataRptr = 0;
	u16 micWptr = (audio_get_rx_dma_wptr(DMA2) - (u32)gAppAudioBuffer) >> 1;
	u16 l = (micWptr >= sRawDataRptr) ? (micWptr-sRawDataRptr) : 0xffff;
	
	if(l < APP_AUDIO_FRAME_SAMPLE) return;

//	extern u8 app_cis_estab_cnt;
//	if(!app_cis_estab_cnt) return;

//	if(!gAppAudioEnable || !gAppAudioIsSend) return;//todo cis haven't data, because of gAppAudioIsSend=FALSE

	ps = ((s16*)gAppAudioBuffer) + sRawDataRptr;
	//app_usb_fill_buffer((u8*)ps);
	app_lc3_encode((u8*)ps, APP_AUDIO_FRAME_BYTES, gAppAudioOutSDU+20);

	#if (TEST_CIS_COUNT >= 1)
		if(app_cisConnHandle[0] != 0){
			blc_ial_cis_splitSdu2UnframedPdu(app_cisConnHandle[0], (iso_data_packet_t*)gAppAudioOutSDU);
		}
	#endif

	#if (TEST_CIS_COUNT >= 2)
		if(app_cisConnHandle[1] != 0){
			blc_ial_cis_splitSdu2UnframedPdu(app_cisConnHandle[1], (iso_data_packet_t*)gAppAudioOutSDU);
		}
	#endif

	sRawDataRptr = sRawDataRptr ? 0 : APP_AUDIO_FRAME_SAMPLE;
}

void app_audio_ase_start(void)
{
	if(app_aclConnHandle[0] != 0 && app_cisConnHandle[0] == 0 && audio_sdp_finish[app_aclConnHandle[0] & 0x0f] == 1){
		if(gAppAudioConn0Ase[0] != 0){
			my_dump_str_data(APP_DUMP_EN, "[APP] ACL connect0 - ASE0 start", app_aclConnHandle, 2);
			blc_audio_ascpEnableAse(app_aclConnHandle[0], gAppAudioConn0Ase[0]);
			audio_sdp_finish[app_aclConnHandle[0] & 0x0f] = 0;
			cisCreateDoing = true;
			return;
		}
	}

	if(app_aclConnHandle[1] != 0 && app_cisConnHandle[1] == 0 && audio_sdp_finish[app_aclConnHandle[1] & 0x0f] == 1){
		if(gAppAudioConn1Ase[0] != 0){
			my_dump_str_data(APP_DUMP_EN, "[APP] ACL connect1- ASE0 start", &app_aclConnHandle[1], 2);
			int res = blc_audio_ascpEnableAse(app_aclConnHandle[1], gAppAudioConn1Ase[0]);
			my_dump_str_data(APP_DUMP_EN, "[APP] ASE res", &res, 4);
			audio_sdp_finish[app_aclConnHandle[1] & 0x0f] = 0;
			cisCreateDoing = true;
			return;
		}
	}
}


#if (BLC_AUDIO_ASCS_ENABLE)
static void app_audio_ascpInitCfg(blc_audio_ascpServiceReadyEvt_t *pEvt);
#endif 
static int app_audio_eventCB(u16 connHandle, u16 evtID, u16 dataLen, u8 *pData)
{
	if(evtID == BLC_AUDIO_EVTID_SDP_OVER){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_SDP_OVER", NULL, 0);

		if(connHandle & 0x0080){
			audio_sdp_finish[connHandle & 0x0f] = 1;
		}

		if(cisCreateDoing == 0){
			app_audio_ase_start();
		}else{
			cisCreatePending = 1;
		}
	}else if(evtID == BLC_AUDIO_EVTID_ASE_ENABLE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_ENABLE", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_ASE_UPDATE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_UPDATE", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_ASE_DISABLE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_DISABLE", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_ASE_RELEASE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_ASE_RELEASE", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_RECV_START){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_RECV_START", NULL, 0);
		if(dataLen == sizeof(blc_audio_aseStartEvt_t)){
			//blc_audio_aseStartEvt_t *pEvt = (blc_audio_aseStartEvt_t*)pData;
			gAppAudioIsSend = true;
		}
	}else if(evtID == BLC_AUDIO_EVTID_RECV_STOP){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_RECV_STOP", NULL, 0);
		if(dataLen == sizeof(blc_audio_aseStopEvt_t)){
			u8 i;
			for(i=0; i<TEST_CIS_COUNT; i++){
				if(app_cisConnHandle[i] != 0) break;
			}
			if(i == TEST_CIS_COUNT){
				gAppAudioIsSend = false;
			}
		}
	}else if(evtID == BLC_AUDIO_EVTID_MICS_AICS_STATE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_MICS_AICS_STATE", NULL, 0);
	}else if(evtID == BLC_AUDIO_EVTID_VCS_STATE){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIO_EVTID_VCS_STATE", NULL, 0);
	}else if(evtID == BLC_AUDIOC_EVTID_MCP_CTRL){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOC_EVTID_MCP_CTRL", NULL, 0);
	}else if(evtID == BLC_AUDIOC_EVTID_PACS_READY){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOC_EVTID_PACS_READY", NULL, 0);
	}else if(evtID == BLC_AUDIOC_EVTID_ASCS_READY){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOC_EVTID_ASCS_READY", NULL, 0);
		#if (BLC_AUDIO_ASCS_ENABLE)
		if(dataLen == sizeof(blc_audio_ascpServiceReadyEvt_t)){
			app_audio_ascpInitCfg((blc_audio_ascpServiceReadyEvt_t*)pData);
		}
		#endif
	}else if(evtID == BLC_AUDIOC_EVTID_MICS_READY){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOC_EVTID_MICS_READY", NULL, 0);
		if(dataLen == sizeof(blc_audio_micpServiceReadyEvt_t)){
			blc_audio_micpServiceReadyEvt_t *pEvt;
			pEvt = (blc_audio_micpServiceReadyEvt_t*)pData;
			my_dump_str_data(APP_DUMP_EN, "Vcp: aicsCount:", &pEvt->aicsCount, 1);
		}
	}else if(evtID == BLC_AUDIOC_EVTID_VCS_READY){
		my_dump_str_data(APP_DUMP_EN, "BLC_AUDIOC_EVTID_VCS_READY", NULL, 0);
		if(dataLen == sizeof(blc_audio_vcpServiceReadyEvt_t)){
			blc_audio_vcpServiceReadyEvt_t *pEvt;
			pEvt = (blc_audio_vcpServiceReadyEvt_t*)pData;
			{
				u8 count[2] = {pEvt->aicsCount, pEvt->vocsCount};
				my_dump_str_data(APP_DUMP_EN, "aicsCount-vocsCount:", count, 2);
			}
		}
	}
	
	return 0;
}
#if (BLC_AUDIO_ASCS_ENABLE)
static void app_audio_ascpInitCfg(blc_audio_ascpServiceReadyEvt_t *pEvt)
{
	int ret;
	blc_audio_aseConfig_t aseCfg;

	aseCfg.cigID = CIG_ID_0;
	aseCfg.cisID = cis_id[pEvt->aclHandle & 0x03];
	memset(aseCfg.codecId, 0, 5);
	aseCfg.codecId[0] = BLC_AUDIO_CODECID_LC3;
	aseCfg.direction = BLC_AUDIO_DIRECTION_SOURCE;
	aseCfg.frequency = BLC_AUDIO_FREQUENCY_CFG_16000;
	aseCfg.duration = BLC_AUDIO_DURATION_CFG_10;
	aseCfg.context = BLC_AUDIO_CONTEXT_TYPE_CONVERSATIONAL;
	aseCfg.frameOcts = 140;
	aseCfg.location = BLC_AUDIO_LOCATION_FLAG_FL;
	
	if(pEvt->aseCount >= 1){
		aseCfg.direction = BLC_AUDIO_DIRECTION_SOURCE;
		ret = blc_audio_ascpSetAseCfg(pEvt->aclHandle, pEvt->aseID[0], &aseCfg);
		if(ret != BLC_AUDIO_SUCCESS){
			//printf("Set Ase[%d] Fail!\r\n", pEvt->aseID[0]);
		}else{
			//printf("Set Ase[%d] Succ!\r\n", pEvt->aseID[0]);
		}
		
		if(pEvt->aclHandle != 0 && app_aclConnHandle[0] == pEvt->aclHandle){
			gAppAudioConn0Ase[0] = pEvt->aseID[0];
			my_dump_str_data(APP_DUMP_EN,"[APP] SDP -ACL connect0- ASE0", gAppAudioConn0Ase, 1);
		}
		if(pEvt->aclHandle != 0 && app_aclConnHandle[1] == pEvt->aclHandle){
			gAppAudioConn1Ase[0] = pEvt->aseID[0];
			my_dump_str_data(APP_DUMP_EN,"[APP] SDP - ACL connect1 - ASE0", gAppAudioConn1Ase, 1);
		}
	}
	if(pEvt->aseCount >= 2){
		aseCfg.direction = BLC_AUDIO_DIRECTION_SINK;
		ret = blc_audio_ascpSetAseCfg(pEvt->aclHandle, pEvt->aseID[1], &aseCfg);
		if(ret != BLC_AUDIO_SUCCESS){
			//printf("Set Ase[%d] Fail!\r\n", pEvt->aseID[1]);
		}else{
			//printf("Set Ase[%d] Succ!\r\n", pEvt->aseID[1]);
		}
		
		if(pEvt->aclHandle != 0 && app_aclConnHandle[0] == pEvt->aclHandle){
			gAppAudioConn0Ase[1] = pEvt->aseID[1];
			my_dump_str_data(APP_DUMP_EN,"[APP] SDP -ACL connect0- ASE1", gAppAudioConn0Ase+1, 1)
		}
		if(pEvt->aclHandle != 0 && app_aclConnHandle[1] == pEvt->aclHandle){
			gAppAudioConn1Ase[1] = pEvt->aseID[1];
			my_dump_str_data(APP_DUMP_EN,"[APP] SDP -ACL connect1- ASE1", gAppAudioConn1Ase+1, 1);
		}
	}
}
#endif //#if (BLC_AUDIO_ASCS_ENABLE)


#endif //LE_AUDIO_DEMO_ENABLE
