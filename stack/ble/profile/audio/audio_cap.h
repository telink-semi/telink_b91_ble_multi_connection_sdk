/********************************************************************************************************
 * @file	audio_cap.h
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
#ifndef _AUDIO_CAP_H_
#define _AUDIO_CAP_H_

#include "stack/ble/profile/audio/audio_config.h"

#if (BLC_AUDIO_PROFILE_EN)
#include "stack/ble/host/attr/att.h"
#include "stack/ble/profile/audio/audio_define.h"
#include "stack/ble/profile/audio/audio_struct.h"

int  blc_audio_init(void);

bool blc_audio_isBusy(u16 connHandle);
bool blc_audio_sdpIsBusy(u16 connHandle);

bool blc_audio_setConnState(u16 connHandle, BLC_AUDIO_ROLE_ENUM role, BLC_AUDIO_STATE_ENUM state);

void blc_audio_regEventCB(u8 connHandle, BlcAudioEventCB evtCB);
void blc_audio_regEventCBByRole(u8 role, BlcAudioEventCB evtCB);
void blc_audio_regEventCBForAll(BlcAudioEventCB evtCB);

int  blc_audio_setService(u8 connHandle, BLC_AUDIO_SERVICE_ENUM srvType, attribute_t *pService);
int  blc_audio_setServiceByRole(u8 role, BLC_AUDIO_SERVICE_ENUM srvType, attribute_t *pService);

int  blc_audio_gattHandler(u16 connHandle, u8 *pkt);

void blc_audio_handler(void);


extern u16 blc_audio_getConnHandle(u16 cisHandle, u8 role);

#if (BLC_AUDIO_PACS_ENABLE)
	extern int blc_audio_pacpAttRead(u16 connHandle, void* p);
	extern int blc_audio_pacpAttWrite(u16 connHandle, void* p);
#endif

#if (BLC_AUDIO_ASCS_ENABLE)
	extern int blc_audio_ascpAttRead(u16 connHandle, void *p);
	extern int blc_audio_ascpAttWrite(u16 connHandle, void *p);
	extern int blc_audio_ascpCisRequestEvt(u16 connHandle, u16 cisHandle, u8 *pPkt);
	extern int blc_audio_ascpCisConnectEvt(u16 connHandle, u16 cisHandle, u8 *pPkt);
	extern int blc_audio_ascpCisDisconnEvt(u16 connHandle, u16 cisHandle, u8 *pPkt);
	#if (BLC_AUDIO_CLIENT_ENABLE)
	extern int blc_audio_ascpRemoveCIGByAse(u16 connHandle, u8 aseID);
	extern int blc_audio_ascpSetAseCfg(u16 connHandle, u8 aseID, blc_audio_aseConfig_t *pCfg);
	extern int blc_audio_ascpSetAllAseCfg(u16 connHandle, blc_audio_aseConfig_t *pCfg);
	extern int blc_audio_ascpEnableAse(u16 connHandle, u8 aseID);
	extern int blc_audio_ascpDisableAse(u16 connHandle, u8 aseID);
	extern int blc_audio_ascpReleaseAse(u16 connHandle, u8 aseID);
	extern int blc_audio_ascpSendCtrlData(u16 connHandle, u8 aseID, u8 *pData, u16 dataLen);
	#endif
	extern int blc_audio_ascpRelease(u16 connHandle, u8 aseID);
#endif //#if (BLC_AUDIO_ASCS_ENABLE)

#if (BLC_AUDIO_CSIS_ENABLE)
	extern int blc_audio_csipAttRead(u16 connHandle, void* p);
	extern int blc_audio_csipAttWrite(u16 connHandle, void* p);
	extern int blc_audio_csipSetLock(u16 connHandle, bool isLock);
#endif

#if (BLC_AUDIO_RAAP_ENABLE)
	extern int blc_audio_raapAttRead(u16 connHandle, void* p);
	extern int blc_audio_raapAttWrite(u16 connHandle, void* p);
#endif

#if (BLC_AUDIO_MICS_ENABLE)
	extern int blc_audio_micpAttRead(u16 connHandle, void *p);
	extern int blc_audio_micpAttWrite(u16 connHandle, void *p);
	extern int blc_audio_micpSetMute(u16 connHandle, u8 mute);
#endif

#if (BLC_AUDIO_VCS_ENABLE)
	extern int blc_audio_vcpAttRead(u16 connHandle, void* p);
	extern int blc_audio_vcpAttWrite(u16 connHandle, void* p);
	extern int blc_audio_vcpSetVolume(u16 connHandle, u8 volume);
	extern int blc_audio_vcpSetMute(u16 connHandle, u8 mute);
	extern int blc_audio_vcpSetCounter(u16 connHandle, u8 counter);
	#if (BLC_AUDIO_VOCS_ENABLE)
	extern int blc_audio_vcpVocsSetCounter(u16 connHandle, u8 counter, int num);
	extern int blc_audio_vcpVocsSetAllCounter(u16 connHandle, u8 counter);
	extern int blc_audio_vcpVocsSetLocation(u16 connHandle, u32 location, int num);
	extern int blc_audio_vcpVocsSetDescrible(u16 connHandle, u8 *pDesc, u8 length, int num);
	#endif
	#if (BLC_AUDIO_VCS_AICS_ENABLE)
	extern int blc_audio_vcpAicsSetCounter(u16 connHandle, u8 counter, int num);
	extern int blc_audio_vcpAicsSetAllCounter(u16 connHandle, u8 counter);
	extern int blc_audio_vcpAicsSetMute(u16 connHandle, u8 mute, int num);
	extern int blc_audio_vcpAicsSetAllMute(u16 connHandle, u8 mute);
	extern int blc_audio_vcpAicsSetGainMode(u16 connHandle, u8 gainMode, int num);
	extern int blc_audio_vcpAicsSetAllGainMode(u16 connHandle, u8 gainMode);
	extern int blc_audio_vcpAicsSetDescrible(u16 connHandle, u8 *pDesc, u8 length, int num);
	#endif
#endif

#if (BLC_AUDIO_MCS_ENABLE)
	extern int blc_audio_mcpAttRead(u16 connHandle, void* p);
	extern int blc_audio_mcpAttWrite(u16 connHandle, void* p);
	extern int blc_audio_mcpEnable(u16 connHandle);
	extern int blc_audio_mcpDisable(u16 connHandle);
	extern int blc_audio_mcpSendCtrl(u16 connHandle, u8 opcode, u8 *pData, u16 dataLen);
	extern int blc_audio_mcpSetMediaState(u16 connHandle, u8 state, bool isNoty);
	extern int blt_audio_mcpSetTrackPosition(u16 connHandle, u32 value, bool isNoty);
#endif

#if (BLC_AUDIO_TBS_ENABLE)
	extern int blc_audio_tbpAttRead(u16 connHandle, void* p);
	extern int blc_audio_tbpAttWrite(u16 connHandle, void* p);
	extern int blc_audio_tbpEnable(u16 connHandle);
	extern int blc_audio_tbpDisable(u16 connHandle);
	extern int blc_audio_tbpSendCtrl(u16 connHandle, u8 opcode, u8 *pData, u8 dataLen);
	extern int blc_audio_tbpAccept(u16 connHandle, u8 index);
	extern int blc_audio_tbpTerminal(u16 connHandle, u8 index);
	extern int blc_audio_tbpAlerting(u16 connHandle, u8 callIndex);;
	extern int blc_audio_tbpCallHold(u16 connHandle, u8 index);
	extern int blc_audio_tbpCallRetrieve(u16 connHandle, u8 index);
	extern int blc_audio_tbpAddIncomingCall(u16 connHandle, u8 *pURI, u8 URILen, u8 *pName, u8 nameLen);
	extern int blc_audio_tbpSetSignalStrength(u16 connHandle, u8 value);
	extern int blc_audio_tbpSetProvideName(u16 connHandle, u8 *pName, u8 nameLen);
	extern int blc_audio_tbpSetBearerTechnology(u16 connHandle, u8 value);
	extern int blc_audio_tbpSetStatusFlags(u16 connHandle, u16 value);
#endif

#if (BLC_AUDIO_OTS_ENABLE)
	extern int blc_audio_otpAttRead(u16 connHandle, void* p);
	extern int blc_audio_otpAttWrite(u16 connHandle, void* p);
	extern int blc_audio_otpSetObjectName(u16 connHandle, u8 *pName, u16 nameLen);
	extern int blc_audio_otpSendActionCtrl(u16 connHandle, u8 opcode, u8 *pData, u8 dataLen);
	extern int blc_audio_otpSendListCtrl(u16 connHandle, u8 opcode, u8 *pData, u8 dataLen);
#endif


#endif //#if (BLC_AUDIO_PROFILE_EN)

#endif //_AUDIO_CAP_H_

