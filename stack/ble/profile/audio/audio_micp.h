/********************************************************************************************************
 * @file	audio_micp.h
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
#ifndef _AUDIO_MICP_H_
#define _AUDIO_MICP_H_

#if (BLC_AUDIO_MICP_ENABLE)




typedef enum{
	BLT_AUDIO_MICP_OPCODE_SET_GAIN       = 0x01,
	BLT_AUDIO_MICP_OPCODE_UNMUTE         = 0x02,
	BLT_AUDIO_MICP_OPCODE_MUTE           = 0x03,
	BLT_AUDIO_MICP_OPCODE_MANUAL_MODE    = 0x04,
	BLT_AUDIO_MICP_OPCODE_AUTOMATIC_MODE = 0x05,
}BLT_AUDIO_MICP_OPCODE_ENUM;

typedef enum{
	BLT_AUDIO_MICP_ERRCODE_MUTE_DISABLED      = 0x80,
	BLT_AUDIO_MICP_ERRCODE_VALUE_OUT_OF_RANGE = 0x81,
}BLT_AUDIO_MICP_ERRCODE_ENUM;

typedef enum{
	BLT_MICP_AICS_OPCODE_SET_GAIN_VALUE  = 0x01,
	BLT_MICP_AICS_OPCODE_UNMUTE          = 0x02,
	BLT_MICP_AICS_OPCODE_MUTE            = 0x03,
	BLT_MICP_AICS_OPCODE_MANU_GAIN_MODE  = 0x04,
	BLT_MICP_AICS_OPCODE_AUTO_GAIN_MODE  = 0x05,
}BLT_MICP_AICS_OPCODE_ENUM;
typedef enum{
	BLT_MICP_AICS_ERRCODE_INVALID_CHANGE_COUNTER  = 0x80,
	BLT_MICP_AICS_ERRCODE_OPCODE_NOT_SUPPORTED    = 0x81,
	BLT_MICP_AICS_ERRCODE_MUTE_DISABLED           = 0x82,
	BLT_MICP_AICS_ERRCODE_VALUE_OUT_OF_RANGE      = 0x83,
	BLT_MICP_AICS_ERRCODE_MODE_CHANGE_NOT_ALLOWED = 0x84,
}BLT_MICP_AICS_ERRCODE_ENUM;


typedef enum{
	BLT_AUDIO_MICP_SDP_FLAG_NONE  = 0x00,
	BLT_AUDIO_MICP_SDP_FLAG_FIRST = 0x80,
	BLT_AUDIO_MICP_SDP_FLAG_INIT  = 0x40,
	BLT_AUDIO_MICP_SDP_FLAG_AICS  = 0x20,
	BLT_AUDIO_MICP_SDP_FLAG_VOCS  = 0x10,
	BLT_AUDIO_MICP_SDP_FLAG_READY = 0x08,
	
	BLT_AUDIO_MICP_SDP_FLAG_PVAL  = 0x04,
	BLT_AUDIO_MICP_SDP_FLAG_SVAL  = 0x02,
}BLT_AUDIO_MICP_SDP_FLAGS_ENUM;

typedef enum{
	BLT_AUDIO_MICP_FLAG_NONE         = 0x00,
	BLT_AUDIO_MICP_FLAG_MUTE_CHANGE  = 0x01,
	BLT_AUDIO_MICP_FLAG_AICS_CHANGE  = 0x02,
}BLT_AUDIO_MICP_FLAGS_ENUM;
typedef enum{
	BLT_AUDIO_MICP_AICS_FLAG_NONE          = 0x00,
	BLT_AUDIO_MICP_AICS_FLAG_STATE_CHANGE  = 0x01,
	BLT_AUDIO_MICP_AICS_FLAG_STATUS_CHANGE = 0x02,
}BLT_AUDIO_MICP_AICS_FLAGS_ENUM;



int blc_audio_micpAttRead(u16 connHandle, void *p);
int blc_audio_micpAttWrite(u16 connHandle, void *p);

int blc_audio_micpSetMute(u16 connHandle, u8 mute);


static int  blt_audio_micpInit(blt_audio_handle_t *pHandle);
static int  blt_audio_micpGattIn(blt_audio_handle_t *pHandle, u8 *pPkt);
static int  blt_audio_micpSetServ(blt_audio_handle_t *pHandle, attribute_t *pService);
static void blt_audio_micpSetConn(blt_audio_handle_t *pHandle, BLC_AUDIO_STATE_ENUM state);
static void blt_audio_micpProcess(blt_audio_handle_t *pHandle);

static int blt_audio_micpSetMicsServ(blt_audio_handle_t *pHandle, attribute_t *pService);
static int blt_audio_micpWriteMics(blt_audio_handle_t *pHandle, u8 dataLen, u8 *pData);
#if (BLC_AUDIO_MICS_AICS_ENABLE)
static int blt_audio_micpSetAicsServ(blt_audio_handle_t *pHandle, attribute_t *pService);
static int blt_audio_micpWriteAics(blt_audio_handle_t *pHandle, blt_audio_aics_t *pAics, u8 opcode, u8 dataLen, u8 *pData);
#endif




#endif //#if (BLC_AUDIO_MICP_ENABLE)

#endif //_AUDIO_MICP_H_

