/********************************************************************************************************
 * @file	app_att.c
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
#include "app_att.h"


#if (LE_AUDIO_DEMO_ENABLE)



extern u16 gAppmCisHandle;
extern u16 gAppmConnHandle;


int app_att_audioRaasWrite(u16 connHandle, void *p);
int app_att_audioGmcsWrite(u16 connHandle, void *p);


////////////////////////////////////// RAAS /////////////////////////////////////////////////////
static const u16 my_RaasServiceUUID = SERVICE_UUID_ROUTING_ACTIVE_AUDIO;
//    Selectable Audio Route Endpoint List
static const u16 my_RaasSelectableAREsUUID = CHARACTERISTIC_UUID_RAAS_SELECTABLE_ARE;
static const u8 my_RaasSelectableAREsChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read, Optional:Notify
	U16_LO(RAAS_SELECTABLE_ARES_DP_H), U16_HI(RAAS_SELECTABLE_ARES_DP_H),
	U16_LO(CHARACTERISTIC_UUID_RAAS_SELECTABLE_ARE), U16_HI(CHARACTERISTIC_UUID_RAAS_SELECTABLE_ARE),
};
static u8 my_RaasSelectableAREsValue[] = {
	//Selectable AREP[0]
	0x03, //AREP Identifier
	BIT(0), //Features: Bit 0-Output, Bit 1-Input
	0x0B, //Friendly Name Length
	't','l','i','n','k','-','M','a','d','i','a', //Friendly Name

	//Selectable AREP[1]
	0x04, //AREP Identifier
	BIT(1)|BIT(0), //Features: Bit 0-Output, Bit 1-Input
	0x0A, //Friendly Name Length
	't','l','i','n','k','-','c','a','l','l', //Friendly Name
};
u16 my_RaasSelectableAREsCCC = 0;
//    Configured Audio Routes List
static const u16 my_RaasConfiguredAREsUUID = CHARACTERISTIC_UUID_RAAS_CONFIGURED_ARE;
static const u8 my_RaasConfiguredAREsChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read, Optional:Notify
	U16_LO(RAAS_CONFIGURED_ARES_DP_H), U16_HI(RAAS_CONFIGURED_ARES_DP_H),
	U16_LO(CHARACTERISTIC_UUID_RAAS_CONFIGURED_ARE), U16_HI(CHARACTERISTIC_UUID_RAAS_CONFIGURED_ARE),
};
static u8 my_RaasConfiguredAREsValue[] = {
	// Configured Audio Routes[0]
	0x01, //Route ID
	1, //Number of Outputs
	0x03, //AREP Identifiers for Output[j]
	0, //Number of Inputs
	0x00,//Server Broadcast 
	
	// Configured Audio Routes[1]
	0x02, //Route ID
	1, //Number of Outputs
	0x04, //AREP Identifiers for Output[j]
	0x01, //Number of Inputs
	0x04, //AREP Identifiers for Input[k]
	0x00,//Server Broadcast 
};
u16 my_RaasConfiguredAREsCCC = 0;
//    Configured Audio Routes Content Information List
static const u16 my_RaasConfiguredContentUUID = CHARACTERISTIC_UUID_RAAS_CONFIGURED_CONTENT;
static const u8 my_RaasConfiguredContentChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read, Optional:Notify
	U16_LO(RAAS_CONFIGURED_CONTENT_DP_H), U16_HI(RAAS_CONFIGURED_CONTENT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_RAAS_CONFIGURED_CONTENT), U16_HI(CHARACTERISTIC_UUID_RAAS_CONFIGURED_CONTENT),
};
static u8 my_RaasConfiguredContentValue[] = {
	// Configured Audio Routes[0]
	0x01, //Route ID
	AUDIO_CONTEXT_TYPE_MEDIA, 0x00,//Content_Type
	2, //Number of CCIDs
	0x00,0x01,//List of CCIDs
	
	// Configured Audio Routes[1]
	0x02, //Route ID
	AUDIO_CONTEXT_TYPE_CONVERSATIONAL, //Content_Type
	0x01, //Number of CCIDs
	0x00, //List of CCIDs
};
u16 my_RaasConfiguredContentCCC = 0;
//    Modify Audio Routing Control Point
static const u16 my_RaasModifyAreUUID = CHARACTERISTIC_UUID_RAAS_MODIFY_ARC;
static const u8 my_RaasModifyAreChar[5] = {
	CHAR_PROP_WRITE | CHAR_PROP_NOTIFY, //Mandatory:Read, Optional:Notify
	U16_LO(RAAS_MODIFY_ARES_DP_H), U16_HI(RAAS_MODIFY_ARES_DP_H),
	U16_LO(CHARACTERISTIC_UUID_RAAS_MODIFY_ARC), U16_HI(CHARACTERISTIC_UUID_RAAS_MODIFY_ARC),
};
static u8 my_RaasModifyAreValue[] = {0x00};
u16 my_RaasModifyAreCCC = 0;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;
static const u16 my_characterUUID = GATT_UUID_CHARACTER;
static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;


////////////////////////////////////// GMCS /////////////////////////////////////////////////////
static const u16 my_GmcsServiceUUID = SERVICE_UUID_GENERIC_MEDIA_CONTROL;
//    Media Player Name
static const u16 my_GmcsPlayerNameUUID = CHARACTERISTIC_UUID_MCS_PLAYER_NAME;
static const u8 my_GmcsPlayerNameChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GMCS_PLAYER_NAME_DP_H), U16_HI(GMCS_PLAYER_NAME_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_PLAYER_NAME), U16_HI(CHARACTERISTIC_UUID_MCS_PLAYER_NAME),
};
static u8 my_GmcsPlayerNameValue[] = {'t','l','i','n','k','-','M','a','d','i','a', };
u16 my_GmcsPlayerNameCCC = 0;
//    Track Changed
static const u16 my_GmcsTrackChangedUUID = CHARACTERISTIC_UUID_MCS_TRACK_CHANGED;
static const u8 my_GmcsTrackChangedChar[5] = {
	CHAR_PROP_NOTIFY,
	U16_LO(GMCS_TRACK_CHANGED_DP_H), U16_HI(GMCS_TRACK_CHANGED_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_TRACK_CHANGED), U16_HI(CHARACTERISTIC_UUID_MCS_TRACK_CHANGED),
};
u16 my_GmcsTrackChangedCCC = 0;
//    Track Title
static const u16 my_GmcsTrackTitleUUID = CHARACTERISTIC_UUID_MCS_TRACK_TITLE;
static const u8 my_GmcsTrackTitleChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GMCS_TRACK_TITLE_DP_H), U16_HI(GMCS_TRACK_TITLE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_TRACK_TITLE), U16_HI(CHARACTERISTIC_UUID_MCS_TRACK_TITLE),
};
static u8 my_GmcsTrackTitleValue[] = {'M','u','s','i','c','1'};
u16 my_GmcsTrackTitleCCC = 0;
//    Track Duration
static const u16 my_GmcsTrackDurationUUID = CHARACTERISTIC_UUID_MCS_TRACK_DURATION;
static const u8 my_GmcsTrackDurationChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GMCS_TRACK_DURATION_DP_H), U16_HI(GMCS_TRACK_DURATION_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_TRACK_DURATION), U16_HI(CHARACTERISTIC_UUID_MCS_TRACK_DURATION),
};
static u32 my_GmcsTrackDurationValue = 0xFFFFFFFF; // unit:10ms, 0xFFFFFFFF means unknown
u16 my_GmcsTrackDurationCCC = 0;
//    Track Position
static const u16 my_GmcsTrackPositionUUID = CHARACTERISTIC_UUID_MCS_TRACK_POSITION;
static const u8 my_GmcsTrackPositionChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GMCS_TRACK_POSITION_DP_H), U16_HI(GMCS_TRACK_POSITION_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_TRACK_POSITION), U16_HI(CHARACTERISTIC_UUID_MCS_TRACK_POSITION),
};
static u32 my_GmcsTrackPositionValue = 0xFFFFFFFF; // unit:10ms, 0xFFFFFFFF means UNAVAILABLE 
u16 my_GmcsTrackPositionCCC = 0;
//    Media State
static const u16 my_GmcsMediaStateUUID = CHARACTERISTIC_UUID_MCS_MEDIA_STATE;
static const u8 my_GmcsMediaStateChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GMCS_MEDIA_STATE_DP_H), U16_HI(GMCS_MEDIA_STATE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_MEDIA_STATE), U16_HI(CHARACTERISTIC_UUID_MCS_MEDIA_STATE),
};
static u8 my_GmcsMediaStateValue = MCP_MEDIA_STATE_INACTIVE; 
u16 my_GmcsMediaStateCCC = 0;
//    Media Control Point
static const u16 my_GmcsControlPointUUID = CHARACTERISTIC_UUID_MCS_MEDIA_CONTROL_POINT;
static const u8 my_GmcsControlPointChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(GMCS_CONTROL_POINT_DP_H), U16_HI(GMCS_CONTROL_POINT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_MEDIA_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_MCS_MEDIA_CONTROL_POINT),
};
static u8 my_GmcsControlPointValue = 0x00; 
u16 my_GmcsControlPointCCC = 0;
//    Media Control Point Opcodes Supported
static const u16 my_GmcsSupportedOpcodesUUID = CHARACTERISTIC_UUID_MCS_OPCODES_SUPPORTED;
static const u8 my_GmcsSupportedOpcodesChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GMCS_SUPPORTED_OPCODE_DP_H), U16_HI(GMCS_SUPPORTED_OPCODE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_OPCODES_SUPPORTED), U16_HI(CHARACTERISTIC_UUID_MCS_OPCODES_SUPPORTED),
};
static u32 my_GmcsSupportedOpcodesValue = MCP_CTRL_SUPPORTED_OPCODE_DEFAULT; 
u16 my_GmcsSupportedOpcodesCCC = 0;
//    Content Control ID (CCID)
static const u16 my_GmcsContentControlUUID = CHARACTERISTIC_UUID_MCS_CONTENT_CONTROL_ID;
static const u8 my_GmcsContentControlChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GMCS_SUPPORTED_OPCODE_DP_H), U16_HI(GMCS_SUPPORTED_OPCODE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MCS_CONTENT_CONTROL_ID), U16_HI(CHARACTERISTIC_UUID_MCS_CONTENT_CONTROL_ID),
};
static u8 my_GmcsCCIDValue = APP_GMCS_CCID; 
u16 my_GmcsContentControlCCC = 0;


////////////////////////////////////// GTBS /////////////////////////////////////////////////////
static const u16 my_GtbsServiceUUID = SERVICE_UUID_GENERIC_TELEPHONE_BEARER;
//    Bearer Provider Name
static const u16 my_GtbsProviderNameUUID = CHARACTERISTIC_UUID_TBS_PROVIDER_NAME;
static const u8 my_GtbsProviderNameChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GTBS_PROVIDER_NAME_DP_H), U16_HI(GTBS_PROVIDER_NAME_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_PROVIDER_NAME), U16_HI(CHARACTERISTIC_UUID_TBS_PROVIDER_NAME),
};
static u8 my_GtbsProviderNameValue[] = {'t','l','i','n','k','-','P','h','o','n','e', };
u16 my_GtbsProviderNameCCC = 0;
//    Bearer Uniform Caller Identifier (UCI)
static const u16 my_GtbsUCIUUID = CHARACTERISTIC_UUID_TBS_CALLER_IDENTIFIER;
static const u8 my_GtbsUCIChar[5] = {
	CHAR_PROP_READ,
	U16_LO(GTBS_CALLER_IDENTIFIER_DP_H), U16_HI(GTBS_CALLER_IDENTIFIER_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_CALLER_IDENTIFIER), U16_HI(CHARACTERISTIC_UUID_TBS_CALLER_IDENTIFIER),
};
static u8 my_GtbsUCIValue[] = {'u','n','0','0','0'}; ///un000 to un999 -- Unknown, Refer https://www.bluetooth.com/specifications/assigned-numbers/uniform-caller-identifiers.
//    Bearer Technology
static const u16 my_GtbsTechUUID = CHARACTERISTIC_UUID_TBS_TECHNOLOGY;
static const u8 my_GtbsTechChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GTBS_TECHNOLOGY_DP_H), U16_HI(GTBS_TECHNOLOGY_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_TECHNOLOGY), U16_HI(CHARACTERISTIC_UUID_TBS_TECHNOLOGY),
};
static u8 my_GtbsTechValue[] = {0x01, 0x00}; ///Refer https://www.bluetooth.com/specifications/assigned-numbers
u16 my_GtbsTechCCC = 0;
//    Bearer URI Schemes Supported List
static const u16 my_GtbsURIListUUID = CHARACTERISTIC_UUID_TBS_URI_LIST;
static const u8 my_GtbsURIListChar[5] = {
	CHAR_PROP_READ,
	U16_LO(GTBS_URI_LIST_CD_H), U16_HI(GTBS_URI_LIST_CD_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_URI_LIST), U16_HI(CHARACTERISTIC_UUID_TBS_URI_LIST),
};
static u8 my_GtbsURIListValue[] = {'t','e','l',',','s','k','y','p','e'}; ///
//    Bearer List Current Calls
static const u16 my_GtbsCurCallListUUID = CHARACTERISTIC_UUID_TBS_CURRENT_CALL_LIST;
static const u8 my_GtbsCurCallListChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GTBS_CURRENT_CALL_LIST_DP_H), U16_HI(GTBS_CURRENT_CALL_LIST_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_CURRENT_CALL_LIST), U16_HI(CHARACTERISTIC_UUID_TBS_CURRENT_CALL_LIST),
};
static u8 my_GtbsCurCallListValue[] = {
	//_Item_Length, Call_Index, Call_State, Call_Flags, Call_URI[i]
	0x06,0x00,0x00,0x00,'t','e','l',
	0x08,0x01,0x00,0x00,'s','k','y','p','e'
};
u16 my_GtbsCurCallListCCC = 0;
//    Content Control ID (CCID)
static const u16 my_GtbsContentControlUUID = CHARACTERISTIC_UUID_TBS_CCID;
static const u8 my_GtbsContentControlChar[5] = {
	CHAR_PROP_READ,
	U16_LO(GTBS_CCID_DP_H), U16_HI(GTBS_CCID_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_CCID), U16_HI(CHARACTERISTIC_UUID_TBS_CCID),
};
static u8 my_GtbsCCIDValue = APP_GTBS_CCID; 
//    Status Flags
static const u16 my_GtbsStatusUUID = CHARACTERISTIC_UUID_TBS_STATUS_FLAGS;
static const u8 my_GtbsStatusChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GTBS_STATUS_FLAGS_DP_H), U16_HI(GTBS_STATUS_FLAGS_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_STATUS_FLAGS), U16_HI(CHARACTERISTIC_UUID_TBS_STATUS_FLAGS),
};
static u8 my_GtbsStatusValue[] = {0x00};
u16 my_GtbsStatusCCC = 0;
//    Call State
static const u16 my_GtbsStateUUID = CHARACTERISTIC_UUID_TBS_CALL_STATE;
static const u8 my_GtbsStateChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GTBS_CALL_STATE_DP_H), U16_HI(GTBS_CALL_STATE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_CALL_STATE), U16_HI(CHARACTERISTIC_UUID_TBS_CALL_STATE),
};
static u8 my_GtbsStateValue[] = {
	//Call_Index[i], State[i], Call_Flags[i] 
	0x00, 0x00, 0x00,
	0x01, 0x00, 0x00,
};
u16 my_GtbsStateCCC = 0;
//    Call Control Point
static const u16 my_GtbsCallCtrlUUID = CHARACTERISTIC_UUID_TBS_CALL_CONTROL_POINT;
static const u8 my_GtbsCallCtrlChar[5] = {
	CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
	U16_LO(GTBS_CALL_CONTROL_POINT_DP_H), U16_HI(GTBS_CALL_CONTROL_POINT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_CALL_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_TBS_CALL_CONTROL_POINT),
};
static u8 my_GtbsCallCtrlValue[] = {0x00};
u16 my_GtbsCallCtrlCCC = 0;
//    Call Control Point Optional Opcodes
static const u16 my_GtbsCtrlOpcodesUUID = CHARACTERISTIC_UUID_TBS_CCP_OPTIONAL_OPCODES;
static const u8 my_GtbsCtrlOpcodesChar[5] = {
	CHAR_PROP_READ,
	U16_LO(GTBS_CCP_OPTIONAL_OPCODES_DP_H), U16_HI(GTBS_CCP_OPTIONAL_OPCODES_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_CCP_OPTIONAL_OPCODES), U16_HI(CHARACTERISTIC_UUID_TBS_CCP_OPTIONAL_OPCODES),
};
static u8 my_GtbsCtrlOpcodesValue[] = {0x00, 0x00};
//    Termination Reason
static const u16 my_GtbsTermReasonUUID = CHARACTERISTIC_UUID_TBS_TERMINATIONO_REASON;
static const u8 my_GtbsTermReasonChar[5] = {
	CHAR_PROP_NOTIFY,
	U16_LO(GTBS_CCP_OPTIONAL_OPCODES_DP_H), U16_HI(GTBS_CCP_OPTIONAL_OPCODES_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_TERMINATIONO_REASON), U16_HI(CHARACTERISTIC_UUID_TBS_TERMINATIONO_REASON),
};
static u8 my_GtbsTermReasonValue[] = {
	//Call_Index, Reason_Code 
	0x00, 0x00,
};
u16 my_GtbsTermReasonCCC = 0;
//    Incoming Call
static const u16 my_GtbsIncomingCallUUID = CHARACTERISTIC_UUID_TBS_INCOMING_CALL;
static const u8 my_GtbsIncomingCallChar[5] = {
	CHAR_PROP_NOTIFY,
	U16_LO(GTBS_INCOMING_CALL_DP_H), U16_HI(GTBS_INCOMING_CALL_DP_H),
	U16_LO(CHARACTERISTIC_UUID_TBS_INCOMING_CALL), U16_HI(CHARACTERISTIC_UUID_TBS_INCOMING_CALL),
};
static u8 my_GtbsIncomingCallValue[] = {
	//Call_Index, URI (variable) 
	0x01, 's','k','y','p','e'
};
u16 my_GtbsIncomingCallCCC = 0;



static const attribute_t my_Attributes[] = {
	{APP_ATT_END_H - 1, 0,0,0,0,0},	// total num of attribute
	
	////////////////////////////////////// RAAS /////////////////////////////////////////////////////
	{13,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_RaasServiceUUID), 0},
	//			Selectable Audio Route Endpoint List
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_RaasSelectableAREsChar),(u8*)(&my_characterUUID), (u8*)(my_RaasSelectableAREsChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_RaasSelectableAREsValue),(u8*)(&my_RaasSelectableAREsUUID), my_RaasSelectableAREsValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_RaasSelectableAREsCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_RaasSelectableAREsCCC), 0},
	//			Configured Audio Routes List
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_RaasConfiguredAREsChar),(u8*)(&my_characterUUID), (u8*)(my_RaasConfiguredAREsChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_RaasConfiguredAREsValue),(u8*)(&my_RaasConfiguredAREsUUID), my_RaasConfiguredAREsValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_RaasConfiguredAREsCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_RaasConfiguredAREsCCC), 0},
	//			Configured Audio Routes Content Information List
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_RaasConfiguredContentChar),(u8*)(&my_characterUUID), (u8*)(my_RaasConfiguredContentChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_RaasConfiguredContentValue),(u8*)(&my_RaasConfiguredContentUUID), my_RaasConfiguredContentValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_RaasConfiguredContentCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_RaasConfiguredContentCCC), 0},
	//          Modify Audio Routing Control Point
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_RaasModifyAreChar),(u8*)(&my_characterUUID), (u8*)(my_RaasModifyAreChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_RaasModifyAreValue),(u8*)(&my_RaasModifyAreUUID), my_RaasModifyAreValue, app_att_audioRaasWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_RaasModifyAreCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_RaasModifyAreCCC), 0},
	
	////////////////////////////////////// GMCS /////////////////////////////////////////////////////
	{27,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_GmcsServiceUUID), 0},
	//			Media Player Name
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerNameChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsPlayerNameChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerNameValue),(u8*)(&my_GmcsPlayerNameUUID), my_GmcsPlayerNameValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsPlayerNameCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsPlayerNameCCC), 0},
	//          Track Changed
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsTrackChangedChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsTrackChangedChar), 0},
	{0,ATT_PERMISSIONS_READ,2,0,(u8*)(&my_GmcsTrackChangedUUID), NULL, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsTrackChangedCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsTrackChangedCCC), 0},
	//          Track Title
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsTrackTitleChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsTrackTitleChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsTrackTitleValue),(u8*)(&my_GmcsTrackTitleUUID), my_GmcsTrackTitleValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsTrackTitleCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsTrackTitleCCC), 0},	
	//          Track Duration
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsTrackDurationChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsTrackDurationChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsTrackDurationValue),(u8*)(&my_GmcsTrackDurationUUID), (u8*)(&my_GmcsTrackDurationValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsTrackDurationCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsTrackDurationCCC), 0},	
	//          Track Position
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsTrackPositionChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsTrackPositionChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsTrackPositionValue),(u8*)(&my_GmcsTrackPositionUUID), (u8*)(&my_GmcsTrackPositionValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsTrackPositionCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsTrackPositionCCC), 0},	
	//          Media State
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsMediaStateChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsMediaStateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsMediaStateValue),(u8*)(&my_GmcsMediaStateUUID), (u8*)(&my_GmcsMediaStateValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsMediaStateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsMediaStateCCC), 0},	
	//          Media Control Point
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsControlPointChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsControlPointChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsControlPointValue), (u8*)(&my_GmcsControlPointUUID), (u8*)(&my_GmcsControlPointValue), app_att_audioGmcsWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsControlPointCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsControlPointCCC), 0},	
	//          Media Control Point Opcodes Supported
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSupportedOpcodesChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsSupportedOpcodesChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSupportedOpcodesValue),(u8*)(&my_GmcsSupportedOpcodesUUID), (u8*)(&my_GmcsSupportedOpcodesValue), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsSupportedOpcodesCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsSupportedOpcodesCCC), 0},	
	//          Content Control ID (CCID)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsContentControlChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsContentControlChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsCCIDValue),(u8*)(&my_GmcsContentControlUUID), (u8*)(&my_GmcsCCIDValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsContentControlCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsContentControlCCC), 0},	

	
	////////////////////////////////////// GMCS /////////////////////////////////////////////////////
	{23,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_GtbsServiceUUID), 0},
	//	  Bearer Provider Name	
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsProviderNameChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsProviderNameChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsProviderNameValue),(u8*)(&my_GtbsProviderNameUUID), (u8*)(&my_GtbsProviderNameValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsProviderNameCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsProviderNameCCC), 0},	
	//	  Bearer Uniform Caller Identifier (UCI)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsUCIChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsUCIChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsUCIValue),(u8*)(&my_GtbsUCIUUID), (u8*)(&my_GtbsUCIValue), 0, 0},
	//	  Bearer Technology
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsTechChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsTechChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsTechValue),(u8*)(&my_GtbsTechUUID), (u8*)(&my_GtbsTechValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsTechCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsTechCCC), 0},	
	//	  Bearer URI Schemes Supported List
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsURIListChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsURIListChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsURIListValue),(u8*)(&my_GtbsURIListUUID), (u8*)(&my_GtbsURIListValue), 0, 0},
	//	  Bearer List Current Calls
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCurCallListChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsCurCallListChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCurCallListValue),(u8*)(&my_GtbsCurCallListUUID), (u8*)(&my_GtbsCurCallListValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsCurCallListCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsCurCallListCCC), 0},	
	//	  Content Control ID (CCID)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsContentControlChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsContentControlChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCCIDValue),(u8*)(&my_GtbsContentControlUUID), (u8*)(&my_GtbsCCIDValue), 0, 0},
	//	  Status Flags
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStatusChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsStatusChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStatusValue),(u8*)(&my_GtbsStatusUUID), (u8*)(&my_GtbsStatusValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsStatusCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsStatusCCC), 0},	
	//	  Call State
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStateChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsStateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStateValue),(u8*)(&my_GtbsStateUUID), (u8*)(&my_GtbsStateValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsStateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsStateCCC), 0},	
	//	  Call Control Point
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCallCtrlChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsCallCtrlChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsCallCtrlValue),(u8*)(&my_GtbsCallCtrlUUID), (u8*)(&my_GtbsCallCtrlValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsCallCtrlCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsCallCtrlCCC), 0},	
	//	  Call Control Point Optional Opcodes
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCtrlOpcodesChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsCtrlOpcodesChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCtrlOpcodesValue),(u8*)(&my_GtbsCtrlOpcodesUUID), (u8*)(&my_GtbsCtrlOpcodesValue), 0, 0},
	//	  Termination Reason
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsTermReasonChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsTermReasonChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsTermReasonValue),(u8*)(&my_GtbsTermReasonUUID), (u8*)(&my_GtbsTermReasonValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsTermReasonCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsTermReasonCCC), 0},	
	//	  Incoming Call
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsIncomingCallChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsIncomingCallChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsIncomingCallValue),(u8*)(&my_GtbsIncomingCallUUID), (u8*)(&my_GtbsIncomingCallValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsIncomingCallCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsIncomingCallCCC), 0},	
};



void my_gatt_init(void)
{
	bls_att_setAttributeTable((u8*)my_Attributes);	
}
void app_audio_gatt_init(void)
{
	int ret;
	
	ret = 0;
	#if (BLC_AUDIO_RAAS_ENABLE)
	ret = blc_audio_setServiceByRole(BLC_AUDIO_ROLE_CLIENT, BLC_AUDIO_SERVICE_RAAS, (attribute_t*)&my_Attributes[RAAS_PS_H]);
	#endif //(BLC_AUDIO_RAAS_ENABLE)
	#if (BLC_AUDIO_MCS_ENABLE)
	ret = blc_audio_setServiceByRole(BLC_AUDIO_ROLE_CLIENT, BLC_AUDIO_SERVICE_MCS, (attribute_t*)&my_Attributes[GMCS_PS_H]);
	#endif //(BLC_AUDIO_MCS_ENABLE)
	#if (BLC_AUDIO_TBS_ENABLE)
	ret = blc_audio_setServiceByRole(BLC_AUDIO_ROLE_CLIENT, BLC_AUDIO_SERVICE_TBS, (attribute_t*)&my_Attributes[GTBS_PS_H]);
	#endif //(BLC_AUDIO_TBS_ENABLE)
	
	if(ret != 0){
		
	}
	//printf("app_audio_gatt_init: %d\r\n", ret);
}

int app_att_audioRaasWrite(u16 connHandle, void *p)
{
	
	return 0;
}
int app_att_audioGmcsWrite(u16 connHandle, void *p)
{
	
	return 0;
}


#endif
