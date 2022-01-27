/********************************************************************************************************
 * @file	audio_config.h
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
#ifndef _AUDIO_CONFIG_H_
#define _AUDIO_CONFIG_H_


#define BLC_AUDIO_PROFILE_EN           (1 && LE_AUDIO_ENABLE)

#if (BLC_AUDIO_PROFILE_EN)

#define BLC_AUDIO_DEBUG_ENABLE         1

#define BLC_AUDIO_SERVER_ENABLE        1
#define BLC_AUDIO_CLIENT_ENABLE        1


#define BLC_AUDIO_BSINK_ENABLE         1
#define BLC_AUDIO_BSOURCE_ENABLE       1
#define BLC_AUDIO_ASSISTANT_ENABLE     1
#define BLC_AUDIO_DELEGATOR_ENABLE     1

//Assistant
//Delegator

#if (!BLC_AUDIO_SERVER_ENABLE && !BLC_AUDIO_CLIENT_ENABLE)
	#error "Server & Client must be supported one or more!"
#endif
#if BLC_AUDIO_SERVER_ENABLE
	#define BLC_AUDIO_SERVER_COUNT     2
#else
	#define BLC_AUDIO_SERVER_COUNT     0
#endif
#if BLC_AUDIO_SERVER_ENABLE
	#define BLC_AUDIO_CLIENT_COUNT     2
#else
	#define BLC_AUDIO_CLIENT_COUNT     0
#endif
#define BLC_AUDIO_HANDLE_COUNT         (BLC_AUDIO_SERVER_COUNT+BLC_AUDIO_CLIENT_COUNT)


#define BLC_AUDIO_ROLE_SWICH_ENABLE                  0
#define BLC_AUDIO_MCP_TBP_RAAP_ROLE_SWICH_ENABLE     1
#define BLC_AUDIO_OTP_ROLE_SWICH_ENABLE              (1 && BLC_AUDIO_ROLE_SWICH_ENABLE)
#define BLC_AUDIO_MCP_ROLE_SWICH_ENABLE              (1 && BLC_AUDIO_ROLE_SWICH_ENABLE)
#define BLC_AUDIO_TBP_ROLE_SWICH_ENABLE              (1 && BLC_AUDIO_ROLE_SWICH_ENABLE)
#define BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE             (1 && BLC_AUDIO_ROLE_SWICH_ENABLE)
#define BLC_AUDIO_MCS_EXTEND_SUPPORT_ENABLE          1
#define BLC_AUDIO_TBS_EXTEND_SUPPORT_ENABLE          1
#define BLC_AUDIO_OTS_EXTEND_SUPPORT_ENABLE          1


#define BLC_AUDIO_PACP_ENABLE          1
#define BLC_AUDIO_ASCP_ENABLE          1
#define BLC_AUDIO_BASP_ENABLE          0
#define BLC_AUDIO_CSIP_ENABLE          1
#define BLC_AUDIO_RAAP_ENABLE          1
#define BLC_AUDIO_MICP_ENABLE          0
#define BLC_AUDIO_VCP_ENABLE           1
#define BLC_AUDIO_MCP_ENABLE           1
#define BLC_AUDIO_TBP_ENABLE           1
#define BLC_AUDIO_OTP_ENABLE           (1 && BLC_AUDIO_MCP_ENABLE)

#define BLC_AUDIO_AICS_ENABLE          1
#define BLC_AUDIO_PACS_ENABLE          (1 && BLC_AUDIO_PACP_ENABLE)
#define BLC_AUDIO_ASCS_ENABLE          (1 && BLC_AUDIO_ASCP_ENABLE)
#define BLC_AUDIO_BASS_ENABLE          (1 && BLC_AUDIO_BASP_ENABLE)
#define BLC_AUDIO_CSIS_ENABLE          (1 && BLC_AUDIO_CSIP_ENABLE)
#define BLC_AUDIO_RAAS_ENABLE          (1 && BLC_AUDIO_RAAP_ENABLE)
#define BLC_AUDIO_MICS_ENABLE          (1 && BLC_AUDIO_MICP_ENABLE)
#define BLC_AUDIO_MICS_AICS_ENABLE     (0 && (BLC_AUDIO_MICS_ENABLE && BLC_AUDIO_AICS_ENABLE))
#define BLC_AUDIO_VCS_ENABLE           (1 && BLC_AUDIO_VCP_ENABLE)
#define BLC_AUDIO_VCS_AICS_ENABLE      (1 && (BLC_AUDIO_VCS_ENABLE && BLC_AUDIO_AICS_ENABLE))
#define BLC_AUDIO_VOCS_ENABLE          (1 && BLC_AUDIO_VCS_ENABLE)
#define BLC_AUDIO_MCS_ENABLE           (1 && BLC_AUDIO_MCP_ENABLE)
#define BLC_AUDIO_TBS_ENABLE           (1 && BLC_AUDIO_TBP_ENABLE)
#define BLC_AUDIO_OTS_ENABLE           (1 && BLC_AUDIO_OTP_ENABLE)


#define BLC_AUDIO_CLIENT_SDP_ENABLE    (1 && (BLC_AUDIO_CLIENT_ENABLE && (BLC_AUDIO_MCP_ENABLE || BLC_AUDIO_TBP_ENABLE || BLC_AUDIO_RAAP_ENABLE)))
#define BLC_AUDIO_SERVER_SDP_ENABLE    (1 && BLC_AUDIO_SERVER_ENABLE)
#define BLC_AUDIO_EATT_ENABLE          (0 && (BLC_AUDIO_CLIENT_SDP_ENABLE || BLC_AUDIO_SERVER_SDP_ENABLE))
#define BLC_AUDIO_SDP_ENABLE           (BLC_AUDIO_CLIENT_SDP_ENABLE || BLC_AUDIO_SERVER_SDP_ENABLE)



#if (BLC_AUDIO_PACS_ENABLE == 0 && BLC_AUDIO_ASCS_ENABLE)
	#error "You should enable PACS befor using ASCS!"
#endif

#if (BLC_AUDIO_ASCS_ENABLE)
	#define BLC_AUDIO_AES_COUNT        4
	#define BLC_AUDIO_ASE_PER_HANDLE   2
	#if (BLC_AUDIO_AES_COUNT == 0)
	#error "Audio Ase Count should not be zero!"
	#endif
#endif

#if (BLC_AUDIO_AICS_ENABLE)
	#if (BLC_AUDIO_MICS_AICS_ENABLE)
		#define BLC_AUDIO_MICS_AICS_COUNT    1
	#endif
	#if (BLC_AUDIO_VCS_AICS_ENABLE)
		#define BLC_AUDIO_VCS_AICS_COUNT     1
	#endif
	#define BLC_AUDIO_AICS_DESC_ENABLE    1
	#if (BLC_AUDIO_AICS_DESC_ENABLE)
		#define BLC_AUDIO_AICS_DESC_SIZE  32
	#endif
#endif

#if (BLC_AUDIO_VCP_ENABLE)
	#define BLC_AUDIO_VCP_VOLUME_MAX     255
	#define BLC_AUDIO_VCP_VOLUME_STEP    10
#endif
#if (BLC_AUDIO_VOCS_ENABLE)
	#define BLC_AUDIO_VCS_VOCS_COUNT     2
	#define BLC_AUDIO_VOCS_DESC_ENABLE    1
	#if (BLC_AUDIO_VOCS_DESC_ENABLE)
		#define BLC_AUDIO_VOCS_DESC_SIZE      32
	#endif
#endif



#if (BLC_AUDIO_TBS_ENABLE)
	#define BLC_AUDIO_TBS_CALL_COUNT     3
	#define BLC_AUDIO_TBS_URL_MAX_LEN    16
	#define BLC_AUDIO_TBS_NAME_MAX_LEN   16
#endif


#if (BLC_AUDIO_ASCS_ENABLE)
	#define BLC_AUDIO_CFG_INTERVAL_MIN             0x001D4C // 7.5ms // Range: 0x0000FF-0xFFFFF    Units:us
	#define BLC_AUDIO_CFG_INTERVAL_MAX             0x0186A0 // 100ms // Range: 0x0000FF-0xFFFFFF   Units:us
	#define BLC_AUDIO_CFG_INTERVAL_TYPE            0x003A98 // 15ms
	#define BLC_AUDIO_CFG_FRAMING_SUPPORTED        0x03 // BIT(0)-Unframe, BIT(1)-frame
	#define BLC_AUDIO_CFG_FRAMING_TYPE             0x00 // 0x00-Unframe, 0x01-frame ISOAL PDUs
	#define BLC_AUDIO_CFG_PHY_SUPPORTED            0x03 // BIT(0)-1M, BIT(1)-2M, BIT(2)-Coded
	#define BLC_AUDIO_CFG_PHY_TYPE                 0x01 // BIT(0)-1M, BIT(1)-2M, BIT(2)-Coded
	#define BLC_AUDIO_CFG_SDU_MAX                  240 // Range: 0x0000-0x0FFF
	//#define BLC_AUDIO_CFG_SDU_TYPE                 240 // Range: 0x0000-0x0FFF
	#define BLC_AUDIO_CFG_RETRANSMIT_NUM_MIN       0
	#define BLC_AUDIO_CFG_RETRANSMIT_NUM_MAX       15
	#define BLC_AUDIO_CFG_RETRANSMIT_NUM_TYPE      3 // Range: 0x00-0x0F
	#define BLC_AUDIO_CFG_TRANSPORT_LATENCY_MAX    80 // Range: 0x0005-0x0FA0    Units:ms
	//#define BLC_AUDIO_CFG_TRANSPORT_LATENCY_TYPE   40
	#define BLC_AUDIO_CFG_PRESENTATION_DELAY_MIN   0x002EE0 // 12ms // Range: 0x000000-0xFFFFFF,   Units:us
	#define BLC_AUDIO_CFG_PRESENTATION_DELAY_MAX   0x004E20 // 20ms // Range: 0x000000-0xFFFFFF,   Units:us
	#define BLC_AUDIO_CFG_PRESENTATION_DELAY_TYPE  0x003A98 // 15ms
#endif //BLC_AUDIO_ASCP_ENABLE



#endif //BLC_AUDIO_PROFILE_EN

#endif //_AUDIO_C ?I_'