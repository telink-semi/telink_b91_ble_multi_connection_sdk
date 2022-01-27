/********************************************************************************************************
 * @file	app_att.h
 *
 * @brief	This is the header file for BLE SDK
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
#ifndef BLM_ATT_H_
#define BLM_ATT_H_

#include "tl_common.h"

#if (LE_AUDIO_DEMO_ENABLE)

#include "stack/ble/profile/audio/audio_cap.h"

// PACS
#define APP_AUDIO_SUPPORTED_FREQUENCY    (BLC_AUDIO_FREQUENCY_FLAG_8000|BLC_AUDIO_FREQUENCY_FLAG_16000)
#define APP_AUDIO_SUPPORTED_DURATIONS    (BLC_AUDIO_DURATION_FLAG_7_5|BLC_AUDIO_DURATION_FLAG_10|BLC_AUDIO_DURATION_FLAG_7_5_PERFERRED)
#define APP_AUDIO_CHANNEL_COUNTS         (BLC_AUDIO_CHANNEL_COUNTS_1|BLC_AUDIO_CHANNEL_COUNTS_2)
#define APP_AUDIO_FRAME_OCTETS_MIN       0x0010 // 16
#define APP_AUDIO_FRAME_OCTETS_MAX       0x00F0 // 240
#define APP_AUDIO_PREFERRED_CONTEXTS     BLC_AUDIO_CONTEXT_TYPE_CONVERSATIONAL
#define APP_AUDIO_STREAMING_CONTEXTS     (BLC_AUDIO_CONTEXT_TYPE_CONVERSATIONAL|BLC_AUDIO_CONTEXT_TYPE_MEDIA)
#define APP_AUDIO_LOCATION               (BLC_AUDIO_LOCATION_FLAG_FL|BLC_AUDIO_LOCATION_FLAG_FR)
#define APP_AUDIO_SUPPORTED_CONTEXTS     (BLC_AUDIO_CONTEXT_TYPE_CONVERSATIONAL|BLC_AUDIO_CONTEXT_TYPE_MEDIA)

// CSIS
#define APP_AUDIO_CSIS_SIZE              2 // Coordinated Device Count: >= 2
#define APP_AUDIO_CSIS_RANK              1 // Device Number: 1~APP_AUDIO_CSIS_SIZE
#define APP_AUDIO_CSIS_SIRK              {0x54,0x65,0x6C,0x69,0x6E,0x6B,0x20,0x4D,0x75,0x73,0x69,0x63,0x00,0x00,0x00,0x00}

// MICS
#define APP_AUDIO_MIC_VOLUME             20
#define APP_AUDIO_MIC_VOLUME_MIN         0
#define APP_AUDIO_MIC_VOLUME_MAX         100
#define APP_AUDIO_MIC_VOLUME_STEP        1

// VCS
#define APP_AUDIO_VCP_VOLUME             (3*BLC_AUDIO_VCP_VOLUME_STEP)
#define APP_AUDIO_VCP_VOLUME_STEP        BLC_AUDIO_VCP_VOLUME_STEP
#define APP_AUDIO_VOCS0_LOCATION         BLC_AUDIO_LOCATION_FLAG_FL


#if (BLC_AUDIO_MCS_ENABLE)
	#define APP_GMCS_CCID    0x01
	#define APP_GTBS_CCID    0x11
	
	#define MCP_CP_SUPPORTED_OPCODE_DEFAULT     0x001FFFFF //Supported All
#endif

#if (BLC_AUDIO_TBS_ENABLE)
	#define TBP_CP_SUPPORTED_OPCODE_DEFAULT     0x0003 //Bit0:Local Hold; Bit1:Join
#endif

///////////////////////////////////// slave-role ATT service HANDLER define ///////////////////////////////////////
typedef enum
{
	ATT_H_START = 0,

	//// Gap 1////
	/**********************************************************************************************/
	GenericAccess_PS_H, 					//UUID: 2800, 	VALUE: uuid 1800
	GenericAccess_DeviceName_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	GenericAccess_DeviceName_DP_H,			//UUID: 2A00,   VALUE: device name
	GenericAccess_Appearance_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	GenericAccess_Appearance_DP_H,			//UUID: 2A01,	VALUE: appearance
	CONN_PARAM_CD_H,						//UUID: 2803, 	VALUE:  			Prop: Read
	CONN_PARAM_DP_H,						//UUID: 2A04,   VALUE: connParameter


	//// gatt 8////
	/**********************************************************************************************/
	GenericAttribute_PS_H,					//UUID: 2800, 	VALUE: uuid 1801
	GenericAttribute_ServiceChanged_CD_H,	//UUID: 2803, 	VALUE:  			Prop: Indicate
	GenericAttribute_ServiceChanged_DP_H,   //UUID:	2A05,	VALUE: service change
	GenericAttribute_ServiceChanged_CCB_H,	//UUID: 2902,	VALUE: serviceChangeCCC


	//// device information 12////
	/**********************************************************************************************/
	DeviceInformation_PS_H,					//UUID: 2800, 	VALUE: uuid 180A
	DeviceInformation_pnpID_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	DeviceInformation_pnpID_DP_H,			//UUID: 2A50,	VALUE: PnPtrs


	//// HID 15////
	/**********************************************************************************************/
	HID_PS_H, 								//UUID: 2800, 	VALUE: uuid 1812

	//include
	HID_INCLUDE_H,							//UUID: 2802, 	VALUE: include

	//protocol
	HID_PROTOCOL_MODE_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	HID_PROTOCOL_MODE_DP_H,					//UUID: 2A4E,	VALUE: protocolMode

	//boot keyboard input report
	HID_BOOT_KB_REPORT_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_BOOT_KB_REPORT_INPUT_DP_H,			//UUID: 2A22, 	VALUE: bootKeyInReport
	HID_BOOT_KB_REPORT_INPUT_CCB_H,			//UUID: 2902, 	VALUE: bootKeyInReportCCC

	//boot keyboard output report
	HID_BOOT_KB_REPORT_OUTPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_BOOT_KB_REPORT_OUTPUT_DP_H,		    //UUID: 2A32, 	VALUE: bootKeyOutReport

	//consume report in
	HID_CONSUME_REPORT_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_CONSUME_REPORT_INPUT_DP_H,			//UUID: 2A4D, 	VALUE: reportConsumerIn
	HID_CONSUME_REPORT_INPUT_CCB_H,			//UUID: 2902, 	VALUE: reportConsumerInCCC
	HID_CONSUME_REPORT_INPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_CONSUMER, TYPE_INPUT

	//keyboard report in
	HID_NORMAL_KB_REPORT_INPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_NORMAL_KB_REPORT_INPUT_DP_H,		//UUID: 2A4D, 	VALUE: reportKeyIn
	HID_NORMAL_KB_REPORT_INPUT_CCB_H,		//UUID: 2902, 	VALUE: reportKeyInInCCC
	HID_NORMAL_KB_REPORT_INPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT

	//keyboard report out
	HID_NORMAL_KB_REPORT_OUTPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_NORMAL_KB_REPORT_OUTPUT_DP_H,  		//UUID: 2A4D, 	VALUE: reportKeyOut
	HID_NORMAL_KB_REPORT_OUTPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_OUTPUT

	// report map
	HID_REPORT_MAP_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read
	HID_REPORT_MAP_DP_H,					//UUID: 2A4B, 	VALUE: reportKeyIn
	HID_REPORT_MAP_EXT_REF_H,				//UUID: 2907 	VALUE: extService

	//hid information
	HID_INFORMATION_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read
	HID_INFORMATION_DP_H,					//UUID: 2A4A 	VALUE: hidInformation

	//control point
	HID_CONTROL_POINT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: write_without_rsp
	HID_CONTROL_POINT_DP_H,					//UUID: 2A4C 	VALUE: controlPoint


	//// battery service 42////
	/**********************************************************************************************/
	BATT_PS_H, 								//UUID: 2800, 	VALUE: uuid 180f
	BATT_LEVEL_INPUT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	BATT_LEVEL_INPUT_DP_H,					//UUID: 2A19 	VALUE: batVal
	BATT_LEVEL_INPUT_CCB_H,					//UUID: 2902, 	VALUE: batValCCC


	//// Ota 46////
	/**********************************************************************************************/
	OTA_PS_H, 								//UUID: 2800, 	VALUE: telink ota service uuid
	OTA_CMD_OUT_CD_H,						//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	OTA_CMD_OUT_DP_H,						//UUID: telink ota uuid,  VALUE: otaData
	OTA_CMD_OUT_DESC_H,						//UUID: 2901, 	VALUE: otaName


//PACS_PS_H  50
	//// PACS ////
	/**********************************************************************************************/
	#if (BLC_AUDIO_PACS_ENABLE)
	PACS_PS_H, //Support at least one of SINK or SOURCE, context is Mandatory
	PACS_SINK_PAC_CD_H, //Mandatory:Read, Optional:Notify
	PACS_SINK_PAC_DP_H,
	PACS_SINK_PAC_CCC_H,
	PACS_SINK_LOCATION_CD_H, //Mandatory:Read, Optional:Notify,Write
	PACS_SINK_LOCATION_DP_H,
	PACS_SINK_LOCATION_CCC_H,
	PACS_SOURCE_PAC_CD_H, //Mandatory:Read, Optional:Notify
	PACS_SOURCE_PAC_DP_H,
	PACS_SOURCE_PAC_CCC_H,
	PACS_SOURCE_LOCATION_CD_H, //Mandatory:Read, Optional:Notify,Write
	PACS_SOURCE_LOCATION_DP_H,
	PACS_SOURCE_LOCATION_CCC_H,
	PACS_AVAILABLE_CONTEXT_CD_H, //Mandatory:Read,Notify Optional:
	PACS_AVAILABLE_CONTEXT_DP_H,
	PACS_AVAILABLE_CONTEXT_CCC_H,
	PACS_SUPPORTED_CONTEXT_CD_H, //Mandatory:Read, Optional:Notify
	PACS_SUPPORTED_CONTEXT_DP_H,
	PACS_SUPPORTED_CONTEXT_CCC_H,
	#endif //(BLC_AUDIO_PACS_ENABLE)
// 69-ASCS_PS_H
	//// ASCS ////
	/**********************************************************************************************/
	#if (BLC_AUDIO_ASCS_ENABLE)
	ASCS_PS_H, //Support at least one of "ASE", one "ASE Control Point"
	ASCS_ASE0_CD_H, //Mandatory:Read,Notify; Optional:
	ASCS_ASE0_DP_H,
	ASCS_ASE0_CCC_H,
	ASCS_ASE1_CD_H, //Mandatory:Read,Notify; Optional:
	ASCS_ASE1_DP_H,
	ASCS_ASE1_CCC_H,
	ASCS_CTRL_CD_H, //Mandatory:WriteWithoutResponse,Notify;  Optional:
	ASCS_CTRL_DP_H,
	ASCS_CTRL_CCC_H,
	#endif //(BLC_AUDIO_ASCS_ENABLE)
// 79-	CSIS_PS_H
	//// CSIS ////
	/**********************************************************************************************/
	#if (BLC_AUDIO_CSIS_ENABLE)
	CSIS_PS_H, //Support at least one of SINK or SOURCE, context is Mandatory
	CSIS_SIRK_CD_H, //M Mandatory:Read, Optional:Notify
	CSIS_SIRK_DP_H,
	CSIS_SIZE_CD_H, //O Mandatory:Read, Optional:Notify
	CSIS_SIZE_DP_H,
	CSIS_LOCK_CD_H, //O Mandatory:Read, Optional:Notify
	CSIS_LOCK_DP_H,
	CSIS_LOCK_CCC_H,
	CSIS_RANK_CD_H, //O Mandatory:Read, Optional:
	CSIS_RANK_DP_H,
	#endif //(BLC_AUDIO_PACS_ENABLE)
	
// 89-MICS_AICS_PS_H
	#if (BLC_AUDIO_MICS_ENABLE)
	#if BLC_AUDIO_MICS_AICS_ENABLE
	MICS_AICS_PS_H,
	MICS_AICS_INPUT_STATE_CP_H, //M Mandatory:Read,Notify; Optional:
	MICS_AICS_INPUT_STATE_DP_H,
	MICS_AICS_INPUT_STATE_CCC_H,
	MICS_AICS_GAIN_SETTING_CP_H, //M Mandatory:Read; Optional:
	MICS_AICS_GAIN_SETTING_DP_H,
	MICS_AICS_INPUT_TYPE_CP_H, //M Mandatory:Read; Optional:
	MICS_AICS_INPUT_TYPE_DP_H,
	MICS_AICS_INPUT_STATUS_CP_H, //M Mandatory:Read,Notify; Optional:
	MICS_AICS_INPUT_STATUS_DP_H,
	MICS_AICS_INPUT_STATUS_CCC_H,
	MICS_AICS_INPUT_CONTROL_CP_H, //M Mandatory:Write; Optional:
	MICS_AICS_INPUT_CONTROL_DP_H,
	MICS_AICS_INPUT_DESCRIP_CP_H, //M Mandatory:Read; Optional:Write Without Response, Notify
	MICS_AICS_INPUT_DESCRIP_DP_H,
	MICS_AICS_INPUT_DESCRIP_CCC_H,
	#endif
	MICS_PS_H,
	#if BLC_AUDIO_MICS_AICS_ENABLE
	MICS_INCLUDE_H,
	#endif
	MICS_MUTE_CP_H, //M Mandatory:Read,Write,Indicate;  Optional:
	MICS_MUTE_DP_H,
	#endif

	//// VCS ////
	/**********************************************************************************************/
// 109 -- VCS_AICS_PS_H
	#if (BLC_AUDIO_VCS_ENABLE)
	#if BLC_AUDIO_VCS_AICS_ENABLE
	VCS_AICS_PS_H,
	VCS_AICS_INPUT_STATE_CP_H, //M Mandatory:Read,Notify; Optional:
	VCS_AICS_INPUT_STATE_DP_H,
	VCS_AICS_INPUT_STATE_CCC_H,
	VCS_AICS_GAIN_SETTING_CP_H, //M Mandatory:Read; Optional:
	VCS_AICS_GAIN_SETTING_DP_H,
	VCS_AICS_INPUT_TYPE_CP_H, //M Mandatory:Read; Optional:
	VCS_AICS_INPUT_TYPE_DP_H,
	VCS_AICS_INPUT_STATUS_CP_H, //M Mandatory:Read,Notify; Optional:
	VCS_AICS_INPUT_STATUS_DP_H,
	VCS_AICS_INPUT_STATUS_CCC_H,
	VCS_AICS_INPUT_CONTROL_CP_H, //M Mandatory:Write; Optional:
	VCS_AICS_INPUT_CONTROL_DP_H,
	VCS_AICS_INPUT_DESCRIP_CP_H, //M Mandatory:Read,Notify; Optional:Write Without Response, Notify
	VCS_AICS_INPUT_DESCRIP_DP_H,
	VCS_AICS_INPUT_DESCRIP_CCC_H,
	#endif
	#if BLC_AUDIO_VOCS_ENABLE
	VOCS0_PS_H, // 125
	VOCS0_STATE_CD_H, //M Mandatory:Read,Notify; Optional:
	VOCS0_STATE_DP_H,
	VOCS0_STATE_CCC_H,
	VOCS0_LOCATION_CD_H, //M Mandatory:Read; Optional:Write Without Response, Notify
	VOCS0_LOCATION_DP_H,
	VOCS0_LOCATION_CCC_H,
	VOCS0_CONTROL_CD_H, //M Mandatory:Write; Optional:
	VOCS0_CONTROL_DP_H,
	VOCS0_DESCRIPT_CD_H, //M Mandatory:Read; Optional:Write Without Response, Notify
	VOCS0_DESCRIPT_DP_H,
	VOCS0_DESCRIPT_CCC_H,
	#endif
	VCS_PS_H,
	#if BLC_AUDIO_VCS_AICS_ENABLE
	VCS_AICS_INCLUDE_H,
	#endif
	#if BLC_AUDIO_VOCS_ENABLE
	VCS_VOCS_INCLUDE_H,
	#endif
	VCS_STATE_CD_H, //M Mandatory:Read,Notify; Optional:
	VCS_STATE_DP_H, // 141
	VCS_STATE_CCC_H,
	VCS_CONTROL_POINT_CD_H, //M Mandatory:Write; Optional:
	VCS_CONTROL_POINT_DP_H,
	VCS_FLAGS_CD_H,  //M Mandatory:Read,Notify; Optional:
	VCS_FLAGS_DP_H,  // 146
	VCS_FLAGS_CCC_H,
	#endif //(BLC_AUDIO_VCS_ENABLE)

#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
	//// RAAS 1:13////
	/**********************************************************************************************/
	#if (BLC_AUDIO_RAAS_ENABLE)
	RAAS_PS_H,
	RAAS_SELECTABLE_ARES_CD_H, //M Mandatory:Read,Notify; Optional:
	RAAS_SELECTABLE_ARES_DP_H,
	RAAS_SELECTABLE_ARES_CCC_H,
	RAAS_CONFIGURED_ARES_CD_H, //M Mandatory:Read,Notify; Optional:
	RAAS_CONFIGURED_ARES_DP_H,
	RAAS_CONFIGURED_ARES_CCC_H,
	RAAS_CONFIGURED_CONTENT_CD_H, //M Mandatory:Read,Notify; Optional:
	RAAS_CONFIGURED_CONTENT_DP_H,
	RAAS_CONFIGURED_CONTENT_CCC_H,
	RAAS_MODIFY_ARES_CD_H, //M Mandatory:Write,Notify; Optional:
	RAAS_MODIFY_ARES_DP_H,
	RAAS_MODIFY_ARES_CCC_H,
	#endif //#if (BLC_AUDIO_RAAS_ENABLE)
#endif //#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
	//// GMCS 14:27////
	/**********************************************************************************************/
	#if (BLC_AUDIO_MCS_ENABLE)
	GMCS_PS_H, // 14
	GMCS_PLAYER_NAME_CD_H,  //M Mandatory:Read; Optional:Notify
	GMCS_PLAYER_NAME_DP_H,
	GMCS_PLAYER_NAME_CCC_H,
	GMCS_PLAYER_ICON_ID_CD_H,  //M Mandatory:Read; Optional:
	GMCS_PLAYER_ICON_ID_DP_H,
	GMCS_PLAYER_ICON_URL_CD_H, //M Mandatory:Read; Optional:
	GMCS_PLAYER_ICON_URL_DP_H,
	GMCS_TRACK_CHANGED_CD_H, //M Mandatory:Notify; Optional:
	GMCS_TRACK_CHANGED_DP_H,
	GMCS_TRACK_CHANGED_CCC_H,
	GMCS_TRACK_TITLE_CD_H, //M Mandatory:Read; Optional:Notify
	GMCS_TRACK_TITLE_DP_H,
	GMCS_TRACK_TITLE_CCC_H,
	GMCS_TRACK_DURATION_CD_H, //M Mandatory:Read; Optional:Notify
	GMCS_TRACK_DURATION_DP_H,
	GMCS_TRACK_DURATION_CCC_H,
	GMCS_TRACK_POSITION_CD_H, //M Mandatory:Read,Write,Write Without Response; Optional:Notify
	GMCS_TRACK_POSITION_DP_H,
	GMCS_TRACK_POSITION_CCC_H,
	GMCS_PLAYBACK_SPEED_CD_H, //M Mandatory:Read,Write,Write Without Response; Optional:Notify
	GMCS_PLAYBACK_SPEED_DP_H,
	GMCS_PLAYBACK_SPEED_CCC_H,
	GMCS_SEEKING_SPEED_CD_H, //M Mandatory:Read; Optional:Notify
	GMCS_SEEKING_SPEED_DP_H,
	GMCS_SEEKING_SPEED_CCC_H,
	GMCS_CUR_TRACK_SEGM_ID_CD_H, //M Mandatory:Read; Optional:
	GMCS_CUR_TRACK_SEGM_ID_DP_H,
	GMCS_CUR_TRACK_ID_CD_H, //M Mandatory:Read, Write, Write Without Response; Optional:Notify
	GMCS_CUR_TRACK_ID_DP_H,
	GMCS_CUR_TRACK_ID_CCC_H,
	GMCS_NEXT_TRACK_ID_CD_H, //M Mandatory:Read, Write, Write Without Response; Optional:Notify
	GMCS_NEXT_TRACK_ID_DP_H,
	GMCS_NEXT_TRACK_ID_CCC_H,
	GMCS_PARENT_GROUP_ID_CD_H, //M Mandatory:Read; Optional:Notify
	GMCS_PARENT_GROUP_ID_DP_H,
	GMCS_PARENT_GROUP_ID_CCC_H,
	GMCS_CURRENT_GROUP_ID_CD_H, //M Mandatory:Read, Write, Write Without Response; Optional:Notify
	GMCS_CURRENT_GROUP_ID_DP_H,
	GMCS_CURRENT_GROUP_ID_CCC_H,
	GMCS_PLAYING_ORDER_CD_H, //M Mandatory:Read, Write, Write Without Response; Optional:Notify
	GMCS_PLAYING_ORDER_DP_H,
	GMCS_PLAYING_ORDER_CCC_H,
	GMCS_PLAYING_ORDER_SUPP_CD_H, //M Mandatory:Read; Optional:
	GMCS_PLAYING_ORDER_SUPP_DP_H,
	GMCS_MEDIA_STATE_CD_H, //M Mandatory:Read,Notify; Optional:
	GMCS_MEDIA_STATE_DP_H,
	GMCS_MEDIA_STATE_CCC_H,
	GMCS_CONTROL_POINT_CD_H, //O Mandatory:Read,Write,Write Without Response,Notify; Optional:
	GMCS_CONTROL_POINT_DP_H,
	GMCS_CONTROL_POINT_CCC_H,
	GMCS_SUPPORTED_OPCODE_CD_H, //O Mandatory:Read,Notify; Optional:
	GMCS_SUPPORTED_OPCODE_DP_H,
	GMCS_SUPPORTED_OPCODE_CCC_H,
	GMCS_SEARCH_CONTROL_POINT_CD_H, //O Mandatory:Read,Notify; Optional:
	GMCS_SEARCH_CONTROL_POINT_DP_H,
	GMCS_SEARCH_CONTROL_POINT_CCC_H,
	GMCS_SEARCH_RESULT_OBJID_CD_H, //O Mandatory:Read,Notify; Optional:
	GMCS_SEARCH_RESULT_OBJID_DP_H,
	GMCS_SEARCH_RESULT_OBJID_CCC_H,
	GMCS_CCID_CD_H, //O Mandatory:Read; Optional:
	GMCS_CCID_DP_H,
	#endif //#if (BLC_AUDIO_MCS_ENABLE)
#endif //#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
	//// GTBS 28:60////
	/**********************************************************************************************/
	#if (BLC_AUDIO_TBS_ENABLE)
	GTBS_PS_H, // 28
	GTBS_PROVIDER_NAME_CD_H,       //M Mandatory:Read, Notify;   Optional:
	GTBS_PROVIDER_NAME_DP_H,
	GTBS_PROVIDER_NAME_CCC_H,
	GTBS_UNIFORM_CALLER_IDENTIFIER_CD_H,   //M Mandatory:Read;   Optional:
	GTBS_UNIFORM_CALLER_IDENTIFIER_DP_H,
	GTBS_TECHNOLOGY_CD_H,          //M Mandatory:Read, Notify;   Optional:
	GTBS_TECHNOLOGY_DP_H,
	GTBS_TECHNOLOGY_CCC_H,
	GTBS_URI_SCHEMES_SUUP_LIST_CD_H, //M Mandatory:Read;   Optional:Notify
	GTBS_URI_SCHEMES_SUUP_LIST_DP_H,
	GTBS_URI_SCHEMES_SUUP_LIST_CCC_H,
	GTBS_SIGNAL_STRENGTH_CD_H,     //M Mandatory:Read, Notify;   Optional:
	GTBS_SIGNAL_STRENGTH_DP_H,
	GTBS_SIGNAL_STRENGTH_CCC_H,
	GTBS_SIGNAL_STRENGTH_REPORT_CD_H, //M Mandatory:Read, Write, Write Without Response;   Optional:
	GTBS_SIGNAL_STRENGTH_REPORT_DP_H,
	GTBS_SIGNAL_STRENGTH_REPORT_CCC_H,
	GTBS_CURRENT_CALL_LIST_CD_H,   //M Mandatory:Read, Notify;   Optional:
	GTBS_CURRENT_CALL_LIST_DP_H,
	GTBS_CURRENT_CALL_LIST_CCC_H,
	GTBS_CCID_CD_H,                //M Mandatory:Read;   Optional:
	GTBS_CCID_DP_H,
	GTBS_STATUS_FLAGS_CD_H,        //M Mandatory:Read, Notify;   Optional:
	GTBS_STATUS_FLAGS_DP_H,
	GTBS_STATUS_FLAGS_CCC_H,
	GTBS_INCOMING_CALL_URI_CD_H,        //M Mandatory:Read, Notify;   Optional:
	GTBS_INCOMING_CALL_URI_DP_H,
	GTBS_INCOMING_CALL_URI_CCC_H,
	GTBS_CALL_STATE_CD_H,          //M Mandatory:Read, Notify;   Optional:
	GTBS_CALL_STATE_DP_H,
	GTBS_CALL_STATE_CCC_H,
	GTBS_CALL_CONTROL_POINT_CD_H,  //M Mandatory:Write, Write Without Response, Notify;   Optional:
	GTBS_CALL_CONTROL_POINT_DP_H,
	GTBS_CALL_CONTROL_POINT_CCC_H,
	GTBS_CCP_OPTIONAL_OPCODES_CD_H, //M Mandatory:Read;   Optional:
	GTBS_CCP_OPTIONAL_OPCODES_DP_H,
	GTBS_TERMINATIONO_REASON_CD_H,  //M Mandatory:Notify;   Optional:
	GTBS_TERMINATIONO_REASON_DP_H,
	GTBS_TERMINATIONO_REASON_CCC_H,
	GTBS_INCOMING_CALL_CD_H,        //M Mandatory:Read, Notify;   Optional:
	GTBS_INCOMING_CALL_DP_H,
	GTBS_INCOMING_CALL_CCC_H,
	GTBS_CALL_FRIENDLY_NAME_CD_H,   //M Mandatory:Read, Notify;   Optional:
	GTBS_CALL_FRIENDLY_NAME_DP_H,
	GTBS_CALL_FRIENDLY_NAME_CCC_H,
	#endif //#if (BLC_AUDIO_TBS_ENABLE)
#endif //#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)
	//// OTS ////
	/**********************************************************************************************/
	#if (BLC_AUDIO_OTS_ENABLE)
	OTS_PS_H, //
	
	OTS_FEATURE_CD_H, //M Mandatory:Read; Optional:
	OTS_FEATURE_DP_H,
	OTS_OBJECT_NAME_CD_H, //M Mandatory:Read; Optional:Write
	OTS_OBJECT_NAME_DP_H,
	OTS_OBJECT_TYPE_CD_H, //M Mandatory:Read; Optional:
	OTS_OBJECT_TYPE_DP_H,
	OTS_OBJECT_SIZE_CD_H, //M Mandatory:Read; Optional:
	OTS_OBJECT_SIZE_DP_H,
	OTS_OBJECT_FIRST_CREATED_CD_H, //O Mandatory:Read; Optional:Write
	OTS_OBJECT_FIRST_CREATED_DP_H,
	OTS_OBJECT_LAST_CREATED_CD_H,  //O Mandatory:Read; Optional:Write
	OTS_OBJECT_LAST_CREATED_DP_H,
	OTS_OBJECT_ID_CD_H, //O Mandatory:Read; Optional:
	OTS_OBJECT_ID_DP_H,
	OTS_OBJECT_PROPERTIES_CD_H, //M Mandatory:Read; Optional:Write
	OTS_OBJECT_PROPERTIES_DP_H,
	OTS_OBJECT_ACTION_CP_CD_H, //M Mandatory:Write,Indicate; Optional:
	OTS_OBJECT_ACTION_CP_DP_H,
	OTS_OBJECT_ACTION_CP_CCC_H,
	OTS_OBJECT_LIST_CP_CD_H, //O Mandatory:Write,Indicate; Optional:
	OTS_OBJECT_LIST_CP_DP_H,
	OTS_OBJECT_LIST_CP_CCC_H,
	OTS_OBJECT_LIST_FILTER_CD_H, //O Mandatory:Read,Write; Optional:
	OTS_OBJECT_LIST_FILTER_DP_H,
	OTS_OBJECT_CHANGED_CD_H, //O Mandatory:Indicate; Optional:
	OTS_OBJECT_CHANGED_DP_H,
	OTS_OBJECT_CHANGED_CCC_H,
	#endif
#endif //#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)

	
	ATT_END_H,

}ATT_HANDLE;



void my_gatt_init(void);
void app_audio_gatt_init(void);



#endif
#endif /* BLM_ATT_H_ */
