/********************************************************************************************************
 * @file	audio_pacp.h
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
#ifndef _AUDIO_PACP_H_
#define _AUDIO_PACP_H_

#if (BLC_AUDIO_PACS_ENABLE)


typedef enum{
	BLT_AUDIO_PACP_FLAG_NONE           = 0x0000,
	BLT_AUDIO_PACP_FLAG_CONTEXT_CHANGE = 0x0001,
}BLT_AUDIO_PACP_FLAGS_ENUM;


typedef enum{
	BLT_AUDIO_PACP_RECORD_TYPE_SINK    = 0x00,
	BLT_AUDIO_PACP_RECORD_TYPE_SOURCE  = 0x01,
}BLT_PACP_RECORD_TYPE_ENUM;


typedef struct{
	u8 counts;
	u8 duration;
	u16 frequency;
	u16 minOctets;
	u16 maxOctets;
	u16 preferredContext;
}blt_audio_pacpRecordParam_t;

int blc_audio_pacpAttRead(u16 connHandle, void* p);
int blc_audio_pacpAttWrite(u16 connHandle, void* p);


u8 blt_audio_mallocContext(blt_audio_handle_t *pHandle, u8 type, u16 context);
u8 blt_audio_freeContext(blt_audio_handle_t *pHandle, u8 type, u16 context);

u16 blt_audio_getAvalibleContext(blt_audio_handle_t *pHandle, u8 type);
void blt_audio_setAvalibleContext(blt_audio_handle_t *pHandle, u8 type, u16 context);

u8 *blt_audio_getRecord(blt_audio_handle_t *pHandle, u8 type, u8 *pCodecId);
u8 blt_audio_getRecordParam(blt_audio_handle_t *pHandle, u8 type, u8 *pCodecId, blt_audio_pacpRecordParam_t *pParam);


#endif //#if (BLC_AUDIO_PACS_ENABLE)

#endif //_AUDIO_PACP_H_

