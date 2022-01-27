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
#include "app.h"
#include "app_att.h"
#include "app_ui.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "stack/ble/profile/audio/audio_cap.h"


#if (LE_AUDIO_DEMO_ENABLE)




typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  u16 latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  u16 timeout;
} gap_periConnectParams_t;

static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

static const u16 extReportRefUUID = GATT_UUID_EXT_REPORT_REF;

static const u16 reportRefUUID = GATT_UUID_REPORT_REF;

//static const u16 characterPresentFormatUUID = GATT_UUID_CHAR_PRESENT_FORMAT;

static const u16 userdesc_UUID	= GATT_UUID_CHAR_USER_DESC;

static const u16 serviceChangeUUID = GATT_UUID_SERVICE_CHANGE;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

static const u16 my_secondaryServiceUUID = GATT_UUID_SECONDARY_SERVICE;

static const u16 my_characterUUID = GATT_UUID_CHARACTER;

static const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;

static const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;

static const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

static const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;

static const u16 my_appearanceUIID = GATT_UUID_APPEARANCE;

static const u16 my_periConnParamUUID = GATT_UUID_PERI_CONN_PARAM;

static const u16 my_appearance = GAP_APPEARE_UNKNOWN;

static const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;

static const gap_periConnectParams_t my_periConnParameters = {20, 40, 0, 1000};

static u16 serviceChangeVal[2] = {0};

static u8 serviceChangeCCC[2] = {0,0};

static const u8 my_devName[] = { 'c','i','s','_','s','l','v' };

static const u8 my_PnPtrs [] = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};

//////////////////////// Battery /////////////////////////////////////////////////
static const u16 my_batServiceUUID        = SERVICE_UUID_BATTERY;
static const u16 my_batCharUUID       	  = CHARACTERISTIC_UUID_BATTERY_LEVEL;
static u8 batteryValueInCCC[2];
static u8 my_batVal[1] 	= {99};

//////////////////////// HID /////////////////////////////////////////////////////

static const u16 my_hidServiceUUID        = SERVICE_UUID_HUMAN_INTERFACE_DEVICE;

//static const u16 hidServiceUUID           = SERVICE_UUID_HUMAN_INTERFACE_DEVICE;
static const u16 hidProtocolModeUUID      = CHARACTERISTIC_UUID_HID_PROTOCOL_MODE;
static const u16 hidReportUUID            = CHARACTERISTIC_UUID_HID_REPORT;
static const u16 hidReportMapUUID         = CHARACTERISTIC_UUID_HID_REPORT_MAP;
static const u16 hidbootKeyInReportUUID   = CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT;
static const u16 hidbootKeyOutReportUUID  = CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT;
//static const u16 hidbootMouseInReportUUID = CHARACTERISTIC_UUID_HID_BOOT_MOUSE_INPUT;
static const u16 hidinformationUUID       = CHARACTERISTIC_UUID_HID_INFORMATION;
static const u16 hidCtrlPointUUID         = CHARACTERISTIC_UUID_HID_CONTROL_POINT;
static const u16 hidIncludeUUID           = GATT_UUID_INCLUDE;

static const u8 protocolMode 			  = DFLT_HID_PROTOCOL_MODE;

// Key in Report characteristic variables
static u8 reportKeyIn[8];
static u8 reportKeyInCCC[2];
// HID Report Reference characteristic descriptor, key input
static const u8 reportRefKeyIn[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_INPUT };

// Key out Report characteristic variables
static u8 reportKeyOut[1];
static const u8 reportRefKeyOut[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_OUTPUT };

// Consumer Control input Report
static u8 reportConsumerControlIn[2];
static u8 reportConsumerControlInCCC[2];
static const u8 reportRefConsumerControlIn[2] =
			 { HID_REPORT_ID_CONSUME_CONTROL_INPUT, HID_REPORT_TYPE_INPUT };

// Boot Keyboard Input Report
static u8 bootKeyInReport;
static u8 bootKeyInReportCCC[2];

// Boot Keyboard Output Report
static u8 bootKeyOutReport;

// HID Information characteristic
static const u8 hidInformation[] =
{
  U16_LO(0x0111), U16_HI(0x0111),             // bcdHID (USB HID version)
  0x00,                                       // bCountryCode
  0x01                                        // Flags
};

// HID Control Point characteristic
static u8 controlPoint;

// HID Report Map characteristic
// Keyboard report descriptor (using format for Boot interface descriptor)

static const u8 reportMap[] =
{
	//keyboard report in
	0x05, 0x01,     // Usage Pg (Generic Desktop)
	0x09, 0x06,     // Usage (Keyboard)
	0xA1, 0x01,     // Collection: (Application)
	0x85, HID_REPORT_ID_KEYBOARD_INPUT,     // Report Id (keyboard)
				  //
	0x05, 0x07,     // Usage Pg (Key Codes)
	0x19, 0xE0,     // Usage Min (224)  VK_CTRL:0xe0
	0x29, 0xE7,     // Usage Max (231)  VK_RWIN:0xe7
	0x15, 0x00,     // Log Min (0)
	0x25, 0x01,     // Log Max (1)
				  //
				  // Modifier byte
	0x75, 0x01,     // Report Size (1)   1 bit * 8
	0x95, 0x08,     // Report Count (8)
	0x81, 0x02,     // Input: (Data, Variable, Absolute)
				  //
				  // Reserved byte
	0x95, 0x01,     // Report Count (1)
	0x75, 0x08,     // Report Size (8)
	0x81, 0x01,     // Input: (static constant)

	//keyboard output
	//5 bit led ctrl: NumLock CapsLock ScrollLock Compose kana
	0x95, 0x05,    //Report Count (5)
	0x75, 0x01,    //Report Size (1)
	0x05, 0x08,    //Usage Pg (LEDs )
	0x19, 0x01,    //Usage Min
	0x29, 0x05,    //Usage Max
	0x91, 0x02,    //Output (Data, Variable, Absolute)
	//3 bit reserved
	0x95, 0x01,    //Report Count (1)
	0x75, 0x03,    //Report Size (3)
	0x91, 0x01,    //Output (static constant)

	// Key arrays (6 bytes)
	0x95, 0x06,     // Report Count (6)
	0x75, 0x08,     // Report Size (8)
	0x15, 0x00,     // Log Min (0)
	0x25, 0xF1,     // Log Max (241)
	0x05, 0x07,     // Usage Pg (Key Codes)
	0x19, 0x00,     // Usage Min (0)
	0x29, 0xf1,     // Usage Max (241)
	0x81, 0x00,     // Input: (Data, Array)

	0xC0,            // End Collection

	//consumer report in
	0x05, 0x0C,   // Usage Page (Consumer)
	0x09, 0x01,   // Usage (Consumer Control)
	0xA1, 0x01,   // Collection (Application)
	0x85, HID_REPORT_ID_CONSUME_CONTROL_INPUT,   //     Report Id
	0x75,0x10,     //global, report size 16 bits
	0x95,0x01,     //global, report count 1
	0x15,0x01,     //global, min  0x01
	0x26,0x8c,0x02,  //global, max  0x28c
	0x19,0x01,     //local, min   0x01
	0x2a,0x8c,0x02,  //local, max    0x28c
	0x81,0x00,     //main,  input data varible, absolute
	0xc0,        //main, end collection

};

// HID External Report Reference Descriptor for report map
static u16 extServiceUUID;



/////////////////////////////////////////////////////////
//static const u8 my_AudioUUID[16]   	= {TELINK_AUDIO_UUID_SERVICE};
//static const u8 my_MicUUID[16]			= {TELINK_MIC_DATA};
//static const u8 my_SpeakerUUID[16]		= {TELINK_SPEAKER_DATA};
static const u8 my_OtaUUID[16]			= {TELINK_SPP_DATA_OTA};
static const u8 my_OtaServiceUUID[16]	= {TELINK_OTA_UUID_SERVICE};



static u8 my_OtaData 		= 0x00;

//static const u8  my_MicName[] = {'M', 'i', 'c'};
//static const u8  my_SpeakerName[] = {'S', 'p', 'e', 'a', 'k', 'e', 'r'};
static const u8  my_OtaName[] = {'O', 'T', 'A'};


// Include attribute (Battery service)
static const u16 include[3] = {BATT_PS_H, BATT_LEVEL_INPUT_CCB_H, SERVICE_UUID_BATTERY};


//// GAP attribute values
static const u8 my_devNameCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GenericAccess_DeviceName_DP_H), U16_HI(GenericAccess_DeviceName_DP_H),
	U16_LO(GATT_UUID_DEVICE_NAME), U16_HI(GATT_UUID_DEVICE_NAME)
};
static const u8 my_appearanceCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(GenericAccess_Appearance_DP_H), U16_HI(GenericAccess_Appearance_DP_H),
	U16_LO(GATT_UUID_APPEARANCE), U16_HI(GATT_UUID_APPEARANCE)
};
static const u8 my_periConnParamCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
	U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM)
};


//// GATT attribute values
static const u8 my_serviceChangeCharVal[5] = {
	CHAR_PROP_INDICATE,
	U16_LO(GenericAttribute_ServiceChanged_DP_H), U16_HI(GenericAttribute_ServiceChanged_DP_H),
	U16_LO(GATT_UUID_SERVICE_CHANGE), U16_HI(GATT_UUID_SERVICE_CHANGE)
};


//// device Information  attribute values
static const u8 my_PnCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(DeviceInformation_pnpID_DP_H), U16_HI(DeviceInformation_pnpID_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PNP_ID), U16_HI(CHARACTERISTIC_UUID_PNP_ID)
};


//// HID attribute values
static const u8 my_hidProtocolModeCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_PROTOCOL_MODE_DP_H), U16_HI(HID_PROTOCOL_MODE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE), U16_HI(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE)
};
static const u8 my_hidbootKeyInReporCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(HID_BOOT_KB_REPORT_INPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT)
};
static const u8 my_hidbootKeyOutReporCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_BOOT_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_OUTPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT)
};
static const u8 my_hidReportCCinCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(HID_CONSUME_REPORT_INPUT_DP_H), U16_HI(HID_CONSUME_REPORT_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYinCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(HID_NORMAL_KB_REPORT_INPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYoutCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_NORMAL_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_OUTPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportMapCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(HID_REPORT_MAP_DP_H), U16_HI(HID_REPORT_MAP_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT_MAP), U16_HI(CHARACTERISTIC_UUID_HID_REPORT_MAP)
};
static const u8 my_hidinformationCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(HID_INFORMATION_DP_H), U16_HI(HID_INFORMATION_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_INFORMATION), U16_HI(CHARACTERISTIC_UUID_HID_INFORMATION)
};
static const u8 my_hidCtrlPointCharVal[5] = {
	CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_CONTROL_POINT_DP_H), U16_HI(HID_CONTROL_POINT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_HID_CONTROL_POINT)
};


//// Battery attribute values
static const u8 my_batCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(BATT_LEVEL_INPUT_DP_H), U16_HI(BATT_LEVEL_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_BATTERY_LEVEL), U16_HI(CHARACTERISTIC_UUID_BATTERY_LEVEL)
};


//// OTA attribute values
static const u8 my_OtaCharVal[19] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(OTA_CMD_OUT_DP_H), U16_HI(OTA_CMD_OUT_DP_H),
	TELINK_SPP_DATA_OTA,
};




#if (BLC_AUDIO_PACS_ENABLE)
int app_att_audio_pacpRead(u16 connHandle, void *p);
int app_att_audio_pacpWrite(u16 connHandle, void *p);
#endif
#if (BLC_AUDIO_ASCS_ENABLE)
int app_att_audioAscpRead(u16 connHandle, void *p);
int app_att_audioAscpWrite(u16 connHandle, void *p);
#endif 
#if (BLC_AUDIO_CSIS_ENABLE)
int app_att_audioCsipRead(u16 connHandle, void *p);
int app_att_audioCsipWrite(u16 connHandle, void *p);
#endif 
#if (BLC_AUDIO_MICS_ENABLE)
int app_att_audioMicpRead(u16 connHandle, void *p);
int app_att_audioMicpWrite(u16 connHandle, void *p);
#endif 
#if (BLC_AUDIO_VCS_ENABLE)
int app_att_audioVcpRead(u16 connHandle, void *p);
int app_att_audioVcpWrite(u16 connHandle, void *p);
#endif 


////////////////////////////////////// PSCS /////////////////////////////////////////////////////
#if (BLC_AUDIO_PACS_ENABLE)
static const u16 my_PacsServiceUUID = SERVICE_UUID_PUBLISHED_AUDIO_CAPABILITIES;
static const u16 my_PacsSinkPacUUID = CHARACTERISTIC_UUID_PACS_SINK_PAC;
static const u8 my_PacsSinkPacChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read, Optional:Notify
	U16_LO(PACS_SINK_PAC_DP_H), U16_HI(PACS_SINK_PAC_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PACS_SINK_PAC), U16_HI(CHARACTERISTIC_UUID_PACS_SINK_PAC),
};
static u8 my_PacsSinkPacValue[] = {
	1, //recordNum

	// Recocrd[0]
	BLC_AUDIO_CODECID_LC3, 0x00, 0x00, 0x00, 0x00, // Codec_ID: format, companyID, vendorID
	//   Capabilities
	16, //Codec_Specific_Capabilities_Length
	//     Supported_Sampling_Frequencies
	0x03, BLC_AUDIO_CAPTYPE_SUP_SAMPLE_FREQUENCY, (APP_AUDIO_SUPPORTED_FREQUENCY & 0xFF), (APP_AUDIO_SUPPORTED_FREQUENCY & 0xFF00) >> 8,
	//     Supported_Frame_Durations
	0x02, BLC_AUDIO_CAPTYPE_SUP_FRAME_DURATION, APP_AUDIO_SUPPORTED_DURATIONS,
	//     Audio_Channel_Counts
	0x02, BLC_AUDIO_CAPTYPE_SUP_CHANNELS_COUNTS, APP_AUDIO_CHANNEL_COUNTS,
	//     Supported_Octets_Per_Codec_Frame
	0x05, BLC_AUDIO_CAPTYPE_SUP_FRAME_OCTETS, (APP_AUDIO_FRAME_OCTETS_MIN & 0xFF), (APP_AUDIO_FRAME_OCTETS_MIN & 0xFF00) >> 8, (APP_AUDIO_FRAME_OCTETS_MAX & 0xFF), (APP_AUDIO_FRAME_OCTETS_MAX & 0xFF00) >> 8,
	//   Metadata
	0x04, //Metadata_Length
	//     Preferred_Audio_Contexts
	0x03, BLC_AUDIO_METATYPE_PREFERRED_CONTEXTS, (APP_AUDIO_PREFERRED_CONTEXTS & 0xFF), (APP_AUDIO_PREFERRED_CONTEXTS & 0xFF00) >> 8,
};
u16 my_PacsSinkPacCCC = 0;

static const u16 my_PacsSinkLocationUUID = CHARACTERISTIC_UUID_PACS_SINK_AUDIO_LOCATION;
static const u8 my_PacsSinkLocationChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE, //Mandatory:Read, Optional:Notify,Write
	U16_LO(PACS_SINK_LOCATION_DP_H), U16_HI(PACS_SINK_LOCATION_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PACS_SINK_AUDIO_LOCATION), U16_HI(CHARACTERISTIC_UUID_PACS_SINK_AUDIO_LOCATION),
};
u32 my_sinkLocationValue = APP_AUDIO_LOCATION;
u16 my_sinkLocationCCC = 0;

static const u16 my_PacsSourcePacUUID = CHARACTERISTIC_UUID_PACS_SOURCE_PAC;
static const u8 my_PacsSourcePacChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read, Optional:Notify
	U16_LO(PACS_SOURCE_PAC_DP_H), U16_HI(PACS_SOURCE_PAC_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PACS_SOURCE_PAC), U16_HI(CHARACTERISTIC_UUID_PACS_SOURCE_PAC),
};
u8 *my_pPacsSourcePacValue = my_PacsSinkPacValue;
u16 my_PacsSourcePacCCC = 0;

static const u16 my_PacsSourceLocationUUID = CHARACTERISTIC_UUID_PACS_SOURCE_AUDIO_LOCATION;
static const u8 my_PacsSourceLocationChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE, //Mandatory:Read, Optional:Notify,Write
	U16_LO(PACS_SOURCE_LOCATION_DP_H), U16_HI(PACS_SOURCE_LOCATION_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PACS_SOURCE_AUDIO_LOCATION), U16_HI(CHARACTERISTIC_UUID_PACS_SOURCE_AUDIO_LOCATION),
};
u32 my_sourceLocationValue = APP_AUDIO_LOCATION;
u16 my_sourceLocationCCC = 0;

static const u16 my_PacsAvailableContextUUID = CHARACTERISTIC_UUID_PACS_AVAILABLE_AUDIO_CONTEXT;
static const u8 my_PacsAvailableContextChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read,Notify Optional:
	U16_LO(PACS_AVAILABLE_CONTEXT_DP_H), U16_HI(PACS_AVAILABLE_CONTEXT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PACS_AVAILABLE_AUDIO_CONTEXT), U16_HI(CHARACTERISTIC_UUID_PACS_AVAILABLE_AUDIO_CONTEXT),
};
u8 my_AvailableContextValue[] = {
	(APP_AUDIO_SUPPORTED_CONTEXTS&0xFF), (APP_AUDIO_SUPPORTED_CONTEXTS&0xFF00)>>8, //sinkContext
	(APP_AUDIO_SUPPORTED_CONTEXTS&0xFF), (APP_AUDIO_SUPPORTED_CONTEXTS&0xFF00)>>8, //sourceContext
};
u16 my_AvailableContextCCC = 0;

static const u16 my_PacsSuppertedContextUUID = CHARACTERISTIC_UUID_PACS_SUPPORTED_AUDIO_CONTEXT;
static const u8 my_PacsSuppertedContextChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read,Notify Optional:
	U16_LO(PACS_SUPPORTED_CONTEXT_DP_H), U16_HI(PACS_SUPPORTED_CONTEXT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PACS_SUPPORTED_AUDIO_CONTEXT), U16_HI(CHARACTERISTIC_UUID_PACS_SUPPORTED_AUDIO_CONTEXT),
};
u8 my_SuppertedContextValue[] = {
	(APP_AUDIO_SUPPORTED_CONTEXTS&0xFF), (APP_AUDIO_SUPPORTED_CONTEXTS&0xFF00)>>8, //sinkContext
	(APP_AUDIO_SUPPORTED_CONTEXTS&0xFF), (APP_AUDIO_SUPPORTED_CONTEXTS&0xFF00)>>8, //sourceContext
};
u16 my_SuppertedContextCCC = 0;
#endif //BLC_AUDIO_PACS_ENABLE

////////////////////////////////////// ASCS /////////////////////////////////////////////////////
#if (BLC_AUDIO_ASCS_ENABLE)
static const u16 my_AscsServiceUUID = SERVICE_UUID_AUDIO_STREAM_CONTROL;
static const u16 my_AscsAse0UUID = CHARACTERISTIC_UUID_ASCS_ASE;
static const u8 my_AscsAse0Char[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(ASCS_ASE0_DP_H), U16_HI(ASCS_ASE0_DP_H),
	U16_LO(CHARACTERISTIC_UUID_ASCS_ASE), U16_HI(CHARACTERISTIC_UUID_ASCS_ASE),
};
u8 my_AscsAse0Value[2] = {0x01, 0x00};
u16 my_AscsAse0CCC = 0;

static const u16 my_AscsAse1UUID = CHARACTERISTIC_UUID_ASCS_ASE;
static const u8 my_AscsAse1Char[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(ASCS_ASE1_DP_H), U16_HI(ASCS_ASE1_DP_H),
	U16_LO(CHARACTERISTIC_UUID_ASCS_ASE), U16_HI(CHARACTERISTIC_UUID_ASCS_ASE),
};
u8 my_AscsAse1Value[2] = {0x02, 0x00};
u16 my_AscsAse1CCC = 0;

static const u16 my_AscsCtrlUUID = CHARACTERISTIC_UUID_ASCS_ASE_CONTROL_PPOINT;
static const u8 my_AscsCtrlChar[5] = {
	CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
	U16_LO(ASCS_CTRL_DP_H), U16_HI(ASCS_CTRL_DP_H),
	U16_LO(CHARACTERISTIC_UUID_ASCS_ASE), U16_HI(CHARACTERISTIC_UUID_ASCS_ASE),
};
u8 my_AscsCtrlValue[2] = {0x00, 0x00};
u16 my_AscsCtrlCCC = 0;

#endif //BLC_AUDIO_ASCS_ENABLE

////////////////////////////////////// CSIS /////////////////////////////////////////////////////
#if (BLC_AUDIO_CSIS_ENABLE)
static const u16 my_CsisServiceUUID = SERVICE_UUID_COPRDINATED_SET_IDENTIFICATION;
static const u16 my_CsisSIRKUUID = CHARACTERISTIC_UUID_CSIS_SIRK;
static const u8 my_CsisSIRKChar[5] = {
	CHAR_PROP_READ,
	U16_LO(CSIS_SIRK_DP_H), U16_HI(CSIS_SIRK_DP_H),
	U16_LO(CHARACTERISTIC_UUID_CSIS_SIRK), U16_HI(CHARACTERISTIC_UUID_CSIS_SIRK),
};
u8 my_CsisSIRKValue[16] = APP_AUDIO_CSIS_SIRK;

static const u16 my_CsisSizeUUID = CHARACTERISTIC_UUID_CSIS_SIZE;
static const u8 my_CsisSizeChar[5] = {
	CHAR_PROP_READ,
	U16_LO(CSIS_SIZE_DP_H), U16_HI(CSIS_SIZE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_CSIS_SIZE), U16_HI(CHARACTERISTIC_UUID_CSIS_SIZE),
};
u8 my_CsisSizeValue[1] = {APP_AUDIO_CSIS_SIZE};

static const u16 my_CsisLockUUID = CHARACTERISTIC_UUID_CSIS_LOCK;
static const u8 my_CsisLockChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY,
	U16_LO(CSIS_LOCK_DP_H), U16_HI(CSIS_LOCK_DP_H),
	U16_LO(CHARACTERISTIC_UUID_CSIS_LOCK), U16_HI(CHARACTERISTIC_UUID_CSIS_LOCK),
};
u8 my_CsisLockValue[1] = {0x01}; //0x01--Unlock, 0x02--Locked
u16 my_CsisLockCCC = 0;

static const u16 my_CsisRankUUID = CHARACTERISTIC_UUID_CSIS_RANK;
static const u8 my_CsisRankChar[5] = {
	CHAR_PROP_READ,
	U16_LO(CSIS_RANK_DP_H), U16_HI(CSIS_RANK_DP_H),
	U16_LO(CHARACTERISTIC_UUID_CSIS_RANK), U16_HI(CHARACTERISTIC_UUID_CSIS_RANK),
};
u8 my_CsisRankValue[1] = {APP_AUDIO_CSIS_RANK};

#endif //BLC_AUDIO_CSIS_ENABLE

////////////////////////////////////// MICS /////////////////////////////////////////////////////
#if (BLC_AUDIO_MICS_ENABLE)
static const u16 my_MicsServiceUUID = SERVICE_UUID_MICROPHONE_CONTROL;

#if (BLC_AUDIO_MICS_AICS_ENABLE)
static const u16 my_MicsAicsIncludeUUID = GATT_UUID_INCLUDE;
static const u16 my_MicsAicsInclude[3] = {MICS_AICS_PS_H, MICS_AICS_INPUT_DESCRIP_DP_H, SERVICE_UUID_AUDIO_INPUT_CONTROL};
#endif //BLC_AUDIO_AICS_ENABLE

static const u16 my_MicsMuteUUID = CHARACTERISTIC_UUID_MICS_MUTE;
static const u8 my_MicsMuteChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY,
	U16_LO(MICS_MUTE_DP_H), U16_HI(MICS_MUTE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_MICS_MUTE), U16_HI(CHARACTERISTIC_UUID_MICS_MUTE),
};
//u8 my_MicsMuteValue[1] = {BLC_AUDIO_MUTE_NOT_MUTED};
u8 my_MicsMuteValue[1] = {BLC_AUDIO_MUTE_DISABLED};
u16 my_MicsMuteCCC = 0;

//   AICS
#if (BLC_AUDIO_MICS_AICS_ENABLE)
static const u16 my_MicsAicsServiceUUID = SERVICE_UUID_AUDIO_INPUT_CONTROL;
static const u16 my_MicsAicsInputStateUUID = CHARACTERISTIC_UUID_AICS_INPUT_STATE;
static const u8 my_MicsAicsInputStateChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(MICS_AICS_INPUT_STATE_DP_H), U16_HI(MICS_AICS_INPUT_STATE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_STATE), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_STATE),
};
u8 my_MicsAicsInputStateValue[4] = {
	APP_AUDIO_MIC_VOLUME, //Gain Setting
	BLC_AUDIO_MUTE_NOT_MUTED, //Mute
	BLC_AUDIO_GAIN_MODE_MANUAL, //Gain Mode
	0, //Change Counter
	};
u16 my_MicsAicsInputStateCCC = 0;

static const u16 my_MicsAicsGainSettingUUID = CHARACTERISTIC_UUID_AICS_GAIN_SETTING;
static const u8 my_MicsAicsGainSettingChar[5] = {
	CHAR_PROP_READ,
	U16_LO(MICS_AICS_GAIN_SETTING_DP_H), U16_HI(MICS_AICS_GAIN_SETTING_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_GAIN_SETTING), U16_HI(CHARACTERISTIC_UUID_AICS_GAIN_SETTING),
};
u8 my_MicsAicsGainSettingValue[3] = {
	APP_AUDIO_MIC_VOLUME_STEP,
	APP_AUDIO_MIC_VOLUME_MIN,
	APP_AUDIO_MIC_VOLUME_MAX,
	};

static const u16 my_MicsAicsInputTypeUUID = CHARACTERISTIC_UUID_AICS_INPUT_TYPE;
static const u8 my_MicsAicsInputTypeChar[5] = {
	CHAR_PROP_READ,
	U16_LO(MICS_AICS_INPUT_TYPE_DP_H), U16_HI(MICS_AICS_INPUT_TYPE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_TYPE), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_TYPE),
};
u8 my_MicsAicsInputTypeValue[1] = {BLC_AUDIO_INPUT_TYPE_ANALOG_CONNECTOR};

static const u16 my_MicsAicsInputStatusUUID = CHARACTERISTIC_UUID_AICS_INPUT_STATUS;
static const u8 my_MicsAicsInputStatusChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(MICS_AICS_INPUT_STATUS_DP_H), U16_HI(MICS_AICS_INPUT_STATUS_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_STATUS), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_STATUS),
};
u8 my_MicsAicsInputStatusValue[1] = {BLC_AUDIO_INPUT_STATE_INACTIVE};
u16 my_MicsAicsInputStatusCCC = 0;

static const u16 my_MicsAicsInputControlUUID = CHARACTERISTIC_UUID_AICS_INPUT_CONTROL;
static const u8 my_MicsAicsInputControlChar[5] = {
	CHAR_PROP_WRITE,
	U16_LO(MICS_AICS_INPUT_CONTROL_DP_H), U16_HI(MICS_AICS_INPUT_CONTROL_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_CONTROL), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_CONTROL),
};
u8 my_MicsAicsInputControlValue[1] = {0};

static const u16 my_MicsAicsInputDescripUUID = CHARACTERISTIC_UUID_AICS_INPUT_DESCRIP;
static const u8 my_MicsAicsInputDescripChar[5] = {
	CHAR_PROP_READ|CHAR_PROP_NOTIFY|CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(MICS_AICS_INPUT_DESCRIP_DP_H), U16_HI(MICS_AICS_INPUT_DESCRIP_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_DESCRIP), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_DESCRIP),
};
u8 my_MicsAicsInputDescripValue[] = {'T','L','I','N','K','-','M','I','C'};
u16 my_MicsAicsInputDescripCCC = 0;
#endif //BLC_AUDIO_AICS_ENABLE

#endif //BLC_AUDIO_MICS_ENABLE

////////////////////////////////////// VCS /////////////////////////////////////////////////////
#if (BLC_AUDIO_VCS_ENABLE)
static const u16 my_VcsServiceUUID = SERVICE_UUID_VOLUME_CONTROL;
#if (BLC_AUDIO_AICS_ENABLE)
static const u16 my_VcsAicsIncludeUUID = GATT_UUID_INCLUDE;
static const u16 my_VcsAicsInclude[3] = {VCS_AICS_PS_H, VCS_AICS_INPUT_DESCRIP_CCC_H, SERVICE_UUID_AUDIO_INPUT_CONTROL};
#endif
#if (BLC_AUDIO_VOCS_ENABLE)
static const u16 my_VcsVocsIncludeUUID = GATT_UUID_INCLUDE;
static const u16 my_VcsVocsInclude[3] = {VOCS0_PS_H, VOCS0_DESCRIPT_CCC_H, SERVICE_UUID_VOLUME_OFFSET_CONTROL};
#endif
static const u16 my_VcsStateUUID = CHARACTERISTIC_UUID_VCS_STATE;
static const u8 my_VcsStateChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read,Notify; Optional:
	U16_LO(VCS_STATE_DP_H), U16_HI(VCS_STATE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_VCS_STATE), U16_HI(CHARACTERISTIC_UUID_VCS_STATE),
};
u8 my_VcsStateValue[3] = {APP_AUDIO_VCP_VOLUME, 0, 1};
u16 my_VcsStateCCC = 0;

static const u16 my_VcsControlPointUUID = CHARACTERISTIC_UUID_VCS_CONTROL_POINT;
static const u8 my_VcsControlPointChar[5] = {
	CHAR_PROP_WRITE, //Mandatory:Write, Optional:
	U16_LO(VCS_CONTROL_POINT_DP_H), U16_HI(VCS_CONTROL_POINT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_VCS_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_VCS_CONTROL_POINT),
};
u8 my_VcsControlPointValue = 0;

static const u16 my_VcsFlagsUUID = CHARACTERISTIC_UUID_VCS_FLAGS;
static const u8 my_VcsFlagsChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY, //Mandatory:Read,Notify; Optional:
	U16_LO(VCS_FLAGS_DP_H), U16_HI(VCS_FLAGS_DP_H),
	U16_LO(CHARACTERISTIC_UUID_VCS_FLAGS), U16_HI(CHARACTERISTIC_UUID_VCS_FLAGS),
};
u8 my_VcsFlagsValue = 0;
u16 my_VcsFlagsCCC = 0;

#if (BLC_AUDIO_VCS_AICS_ENABLE)
static const u16 my_VcsAicsServiceUUID = SERVICE_UUID_AUDIO_INPUT_CONTROL;
static const u16 my_VcsAicsInputStateUUID = CHARACTERISTIC_UUID_AICS_INPUT_STATE;
static const u8 my_VcsAicsInputStateChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(VCS_AICS_INPUT_STATE_DP_H), U16_HI(VCS_AICS_INPUT_STATE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_STATE), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_STATE),
};
u8 my_VcsAicsInputStateValue[4] = {
	APP_AUDIO_MIC_VOLUME, //Gain Setting
	BLC_AUDIO_MUTE_NOT_MUTED, //Mute
	BLC_AUDIO_GAIN_MODE_MANUAL, //Gain Mode
	0, //Change Counter
	};
u16 my_VcsAicsInputStateCCC = 0;

static const u16 my_VcsAicsGainSettingUUID = CHARACTERISTIC_UUID_AICS_GAIN_SETTING;
static const u8 my_VcsAicsGainSettingChar[5] = {
	CHAR_PROP_READ,
	U16_LO(VCS_AICS_GAIN_SETTING_DP_H), U16_HI(VCS_AICS_GAIN_SETTING_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_GAIN_SETTING), U16_HI(CHARACTERISTIC_UUID_AICS_GAIN_SETTING),
};
u8 my_VcsAicsGainSettingValue[3] = {
	APP_AUDIO_MIC_VOLUME_STEP,
	APP_AUDIO_MIC_VOLUME_MIN,
	APP_AUDIO_MIC_VOLUME_MAX,
	};

static const u16 my_VcsAicsInputTypeUUID = CHARACTERISTIC_UUID_AICS_INPUT_TYPE;
static const u8 my_VcsAicsInputTypeChar[5] = {
	CHAR_PROP_READ,
	U16_LO(VCS_AICS_INPUT_TYPE_DP_H), U16_HI(VCS_AICS_INPUT_TYPE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_TYPE), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_TYPE),
};
u8 my_VcsAicsInputTypeValue[1] = {BLC_AUDIO_INPUT_TYPE_ANALOG_CONNECTOR};

static const u16 my_VcsAicsInputStatusUUID = CHARACTERISTIC_UUID_AICS_INPUT_STATUS;
static const u8 my_VcsAicsInputStatusChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(VCS_AICS_INPUT_STATUS_DP_H), U16_HI(VCS_AICS_INPUT_STATUS_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_STATUS), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_STATUS),
};
u8 my_VcsAicsInputStatusValue[1] = {BLC_AUDIO_INPUT_STATE_INACTIVE};
u16 my_VcsAicsInputStatusCCC = 0;

static const u16 my_VcsAicsInputControlUUID = CHARACTERISTIC_UUID_AICS_INPUT_CONTROL;
static const u8 my_VcsAicsInputControlChar[5] = {
	CHAR_PROP_WRITE,
	U16_LO(VCS_AICS_INPUT_CONTROL_DP_H), U16_HI(VCS_AICS_INPUT_CONTROL_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_CONTROL), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_CONTROL),
};
u8 my_VcsAicsInputControlValue[1] = {0};

static const u16 my_VcsAicsInputDescripUUID = CHARACTERISTIC_UUID_AICS_INPUT_DESCRIP;
static const u8 my_VcsAicsInputDescripChar[5] = {
	CHAR_PROP_READ|CHAR_PROP_NOTIFY|CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(VCS_AICS_INPUT_DESCRIP_DP_H), U16_HI(VCS_AICS_INPUT_DESCRIP_DP_H),
	U16_LO(CHARACTERISTIC_UUID_AICS_INPUT_DESCRIP), U16_HI(CHARACTERISTIC_UUID_AICS_INPUT_DESCRIP),
};
u8 my_VcsAicsInputDescripValue[] = {'T','L','I','N','K','-','S','P','K'};
u16 my_VcsAicsInputDescripCCC = 0;
#endif //BLC_AUDIO_VCS_AICS_ENABLE

#if (BLC_AUDIO_VOCS_ENABLE)
static const u16 my_Vocs0ServiceUUID = SERVICE_UUID_VOLUME_OFFSET_CONTROL;
//Offset State
static const u16 my_Vocs0StateUUID = CHARACTERISTIC_UUID_VOCS_STATE;
static const u8 my_Vocs0StateChar[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(VOCS0_STATE_DP_H), U16_HI(VOCS0_STATE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_VOCS_STATE), U16_HI(CHARACTERISTIC_UUID_VOCS_STATE),
};
u8 my_Vocs0StateValue[3] = {0x00, 0x00, 0x00}; //offset(word), counter(byte)
u16 my_Vocs0StateCCC = 0;
//Audio Location
static const u16 my_Vocs0LocationUUID = CHARACTERISTIC_UUID_VOCS_LOCATION;
static const u8 my_Vocs0LocationChar[5] = {
	CHAR_PROP_READ,
	U16_LO(VOCS0_LOCATION_DP_H), U16_HI(VOCS0_LOCATION_DP_H),
	U16_LO(CHARACTERISTIC_UUID_VOCS_LOCATION), U16_HI(CHARACTERISTIC_UUID_VOCS_LOCATION),
};
u32 my_Vocs0LocationValue = APP_AUDIO_VOCS0_LOCATION;
u16 my_Vocs0LocationCCC = 0;
//Volume Offset Control Point
static const u16 my_Vocs0ControlUUID = CHARACTERISTIC_UUID_VOCS_CONTROL;
static const u8 my_Vocs0ControlChar[5] = {
	CHAR_PROP_WRITE,
	U16_LO(VOCS0_CONTROL_DP_H), U16_HI(VOCS0_CONTROL_DP_H),
	U16_LO(CHARACTERISTIC_UUID_VOCS_CONTROL), U16_HI(CHARACTERISTIC_UUID_VOCS_CONTROL),
};
u8 my_Vocs0ControlValue[3] = {};
//Audio Output Description
static const u16 my_Vocs0DescriptUUID = CHARACTERISTIC_UUID_VOCS_DESCRIPT;
static const u8 my_Vocs0DescriptChar[5] = {
	CHAR_PROP_READ|CHAR_PROP_NOTIFY|CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(VOCS0_DESCRIPT_DP_H), U16_HI(VOCS0_DESCRIPT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_VOCS_DESCRIPT), U16_HI(CHARACTERISTIC_UUID_VOCS_DESCRIPT),
};
u8 my_Vocs0DescriptValue[] = {'L','e','f','t',' ','S','p','e','a','k','e','r'};
u16 my_Vocs0DescriptCCC = 0;
#endif //BLC_AUDIO_VOCP_ENABLE

#endif //BLC_AUDIO_VCS_ENABLE



#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
	////////////////////////////////////// RAAS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_RAAS_ENABLE)
	int app_att_audioRaasRead(u16 connHandle, void *p);
	int app_att_audioRaasWrite(u16 connHandle, void *p);
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
		BLC_AUDIO_CONTEXT_TYPE_MEDIA, 0x00,//Content_Type
		2, //Number of CCIDs
		0x00,0x01,//List of CCIDs
		
		// Configured Audio Routes[1]
		0x02, //Route ID
		BLC_AUDIO_CONTEXT_TYPE_CONVERSATIONAL, //Content_Type
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
	#endif //#if (BLC_AUDIO_RAAS_ENABLE)
#endif //#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
	////////////////////////////////////// GMCS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_MCS_ENABLE)
	int app_att_audioMcsRead(u16 connHandle, void *p);
	int app_att_audioMcsWrite(u16 connHandle, void *p);
	static const u16 my_GmcsServiceUUID = SERVICE_UUID_GENERIC_MEDIA_CONTROL;
	//    Media Player Name
	static const u16 my_GmcsPlayerNameUUID = CHARACTERISTIC_UUID_MCS_PLAYER_NAME;
	static const u8 my_GmcsPlayerNameChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GMCS_PLAYER_NAME_DP_H), U16_HI(GMCS_PLAYER_NAME_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_PLAYER_NAME), U16_HI(CHARACTERISTIC_UUID_MCS_PLAYER_NAME),
	};
	static u8 my_GmcsPlayerNameValue[] = {'t','l','i','n','k','-','M','a','d','i','a', };
//	u8 my_GmcsPlayerNameValue[33] = {
//		't','l','i','n','k','-','M','a','d','i','a', 
//		't','l','i','n','k','-','M','a','d','i','a',
//		't','l','i','n','k','-','M','a','d','i','a',
//	};
	u16 my_GmcsPlayerNameCCC = 0;
	//    Media Player Icon Object ID: Mandatory-Read; Optional-
	static const u16 my_GmcsPlayerIconIDUUID = CHARACTERISTIC_UUID_MCS_ICON_OBJECT_ID;
	static const u8 my_GmcsPlayerIconIDChar[5] = {
		CHAR_PROP_READ,
		U16_LO(GMCS_PLAYER_ICON_ID_DP_H), U16_HI(GMCS_PLAYER_ICON_ID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_ICON_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_ICON_OBJECT_ID),
	};
	static u8 my_GmcsPlayerIconIDValue[] = {0x01,0x00,0x00,0x00,0x00,0x00}; //UINT48
	//    Media Player Icon URL: Mandatory-Read; Optional-
	static const u16 my_GmcsPlayerIconURLUUID = CHARACTERISTIC_UUID_MCS_ICON_URL;
	static const u8 my_GmcsPlayerIconURLChar[5] = {
		CHAR_PROP_READ,
		U16_LO(GMCS_PLAYER_ICON_URL_DP_H), U16_HI(GMCS_PLAYER_ICON_URL_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_ICON_URL), U16_HI(CHARACTERISTIC_UUID_MCS_ICON_URL),
	};
	static u8 my_GmcsPlayerIconURLValue[] = {'t','l','i','n','k','-','U','R','L','0','1', };
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
//	u8 my_GmcsTrackTitleValue[42] = {
//		'M','u','s','i','c','1',
//		'M','u','s','i','c','1',
//		'M','u','s','i','c','1',
//		'M','u','s','i','c','1',
//		'M','u','s','i','c','1',
//		'M','u','s','i','c','1',
//		'M','u','s','i','c','1',
//	};
	u16 my_GmcsTrackTitleCCC = 0;
	//    Track Duration
	static const u16 my_GmcsTrackDurationUUID = CHARACTERISTIC_UUID_MCS_TRACK_DURATION;
	static const u8 my_GmcsTrackDurationChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GMCS_TRACK_DURATION_DP_H), U16_HI(GMCS_TRACK_DURATION_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_TRACK_DURATION), U16_HI(CHARACTERISTIC_UUID_MCS_TRACK_DURATION),
	};
	static u32 my_GmcsTrackDurationValue = 50; //0xFFFFFFFF; // unit:10ms, 0xFFFFFFFF means unknown
	u16 my_GmcsTrackDurationCCC = 0;
	//    Track Position
	static const u16 my_GmcsTrackPositionUUID = CHARACTERISTIC_UUID_MCS_TRACK_POSITION;
	static const u8 my_GmcsTrackPositionChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GMCS_TRACK_POSITION_DP_H), U16_HI(GMCS_TRACK_POSITION_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_TRACK_POSITION), U16_HI(CHARACTERISTIC_UUID_MCS_TRACK_POSITION),
	};
	static u32 my_GmcsTrackPositionValue = 10; //0; //0xFFFFFFFF; // unit:10ms, 0xFFFFFFFF means UNAVAILABLE 
	u16 my_GmcsTrackPositionCCC = 0;
	//    Playback Speed: Mandatory-Read, Write,Write Without Response; Optional-Notify
	static const u16 my_GmcsPlaybackSpeedUUID = CHARACTERISTIC_UUID_MCS_PLAYBACK_SPEED;
	static const u8 my_GmcsPlaybackSpeedChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GMCS_PLAYBACK_SPEED_DP_H), U16_HI(GMCS_PLAYBACK_SPEED_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_PLAYBACK_SPEED), U16_HI(CHARACTERISTIC_UUID_MCS_PLAYBACK_SPEED),
	};
	static u8 my_GmcsPlaybackSpeedValue = 0; // 0=1, 64=2, 127=3.957, -64=0.5, -128=0.25
	u16 my_GmcsPlaybackSpeedCCC = 0;
	//    Seeking Speed: Mandatory-Read; Optional-Notify
	static const u16 my_GmcsSeekingSpeedUUID = CHARACTERISTIC_UUID_MCS_SEEKING_SPEED;
	static const u8 my_GmcsSeekingSpeedChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GMCS_SEEKING_SPEED_DP_H), U16_HI(GMCS_SEEKING_SPEED_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_SEEKING_SPEED), U16_HI(CHARACTERISTIC_UUID_MCS_SEEKING_SPEED),
	};
	static u8 my_GmcsSeekingSpeedValue = 0; // 0=not seeking, >0=Fast rewind, <0=Fast forward
	u16 my_GmcsSeekingSpeedCCC = 0;
	//    Current Track Segments Object ID: Mandatory-Read; Optional-
	static const u16 my_GmcsCurTrackSegmIDUUID = CHARACTERISTIC_UUID_MCS_SEGMENTS_OBJECT_ID;
	static const u8 my_GmcsCurTrackSegmIDChar[5] = {
		CHAR_PROP_READ,
		U16_LO(GMCS_CUR_TRACK_SEGM_ID_DP_H), U16_HI(GMCS_CUR_TRACK_SEGM_ID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_SEGMENTS_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_SEGMENTS_OBJECT_ID),
	};
	static u8 my_GmcsCurTrackSegmIDValue[] = {0x02,0x00,0x00,0x00,0x00,0x00}; //UINT48
	//    Current Track Object ID: Mandatory-Read, Write, Write Without Response; Optional-Notify
	static const u16 my_GmcsCurTrackIDUUID = CHARACTERISTIC_UUID_MCS_CURRENT_OBJECT_ID;
	static const u8 my_GmcsCurTrackIDChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GMCS_CUR_TRACK_ID_DP_H), U16_HI(GMCS_CUR_TRACK_ID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_CURRENT_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_CURRENT_OBJECT_ID),
	};
	static u8 my_GmcsCurTrackIDValue[] = {0x03,0x00,0x00,0x00,0x00,0x00}; //UINT48
	u16 my_GmcsCurTrackIDCCC = 0;
	//    Next Track Object ID: Mandatory-Read, Write, Write Without Response; Optional-Notify
	static const u16 my_GmcsNextTrackIDUUID = CHARACTERISTIC_UUID_MCS_NEXT_OBJECT_ID;
	static const u8 my_GmcsNextTrackIDChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GMCS_NEXT_TRACK_ID_DP_H), U16_HI(GMCS_NEXT_TRACK_ID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_NEXT_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_NEXT_OBJECT_ID),
	};
	static u8 my_GmcsNextTrackIDValue[] = {0x04,0x00,0x00,0x00,0x00,0x00}; //UINT48
	u16 my_GmcsNextTrackIDCCC = 0;
	//    Parent Group Object ID: Mandatory-Read; Optional-Notify
	static const u16 my_GmcsParentGroupIDUUID = CHARACTERISTIC_UUID_MCS_PARENT_GOUP_OBJECT_ID;
	static const u8 my_GmcsParentGroupIDChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GMCS_PARENT_GROUP_ID_DP_H), U16_HI(GMCS_PARENT_GROUP_ID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_PARENT_GOUP_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_PARENT_GOUP_OBJECT_ID),
	};
	static u8 my_GmcsParentGroupIDValue[] = {0x05,0x00,0x00,0x00,0x00,0x00}; //UINT48
	u16 my_GmcsParentGroupIDCCC = 0;
	//    Current Group Object ID: Mandatory-Read, Write, Write Without Response; Optional-Notify
	static const u16 my_GmcsCurrentGroupIDUUID = CHARACTERISTIC_UUID_MCS_CURRENT_GOUP_OBJECT_ID;
	static const u8 my_GmcsCurrentGroupIDChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GMCS_CURRENT_GROUP_ID_DP_H), U16_HI(GMCS_CURRENT_GROUP_ID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_CURRENT_GOUP_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_CURRENT_GOUP_OBJECT_ID),
	};
	static u8 my_GmcsCurrentGroupIDValue[] = {0x05,0x00,0x00,0x00,0x00,0x00}; //UINT48
	u16 my_GmcsCurrentGroupIDCCC = 0;
	//    Playing Order: Mandatory-Read, Write, Write Without Response; Optional-Notify
	static const u16 my_GmcsPlayingOrderUUID = CHARACTERISTIC_UUID_MCS_PLAYING_ORDER;
	static const u8 my_GmcsPlayingOrderChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GMCS_PLAYING_ORDER_DP_H), U16_HI(GMCS_PLAYING_ORDER_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_PLAYING_ORDER), U16_HI(CHARACTERISTIC_UUID_MCS_PLAYING_ORDER),
	};
	static u8 my_GmcsPlayingOrderValue[] = {0x01}; //MCS v1.0 P22: 0x01-Single once,A single track is played once; there is no next track.
	u16 my_GmcsPlayingOrderCCC = 0;
	//    Playing Orders Supported: Mandatory-Read; Optional-
	static const u16 my_GmcsPlayingOrderSuppUUID = CHARACTERISTIC_UUID_MCS_PLAYING_ORDER_SUPPEORTED;
	static const u8 my_GmcsPlayingOrderSuppChar[5] = {
		CHAR_PROP_READ,
		U16_LO(GMCS_PLAYING_ORDER_DP_H), U16_HI(GMCS_PLAYING_ORDER_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_PLAYING_ORDER_SUPPEORTED), U16_HI(CHARACTERISTIC_UUID_MCS_PLAYING_ORDER_SUPPEORTED),
	};
	static u8 my_GmcsPlayingOrderSuppValue[] = {0xFF, 0x07}; // 16-bits
	//    Media State
	static const u16 my_GmcsMediaStateUUID = CHARACTERISTIC_UUID_MCS_MEDIA_STATE;
	static const u8 my_GmcsMediaStateChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GMCS_MEDIA_STATE_DP_H), U16_HI(GMCS_MEDIA_STATE_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_MEDIA_STATE), U16_HI(CHARACTERISTIC_UUID_MCS_MEDIA_STATE),
	};
	static u8 my_GmcsMediaStateValue = BLC_AUDIO_MCP_MEDIA_STATE_PLAYING;//BLC_AUDIO_MCP_MEDIA_STATE_INACTIVE; 
	u16 my_GmcsMediaStateCCC = 0;
	//    Media Control Point
	static const u16 my_GmcsControlPointUUID = CHARACTERISTIC_UUID_MCS_MEDIA_CONTROL_POINT;
	static const u8 my_GmcsControlPointChar[5] = {
		CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
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
	static u32 my_GmcsSupportedOpcodesValue = MCP_CP_SUPPORTED_OPCODE_DEFAULT; 
	u16 my_GmcsSupportedOpcodesCCC = 0;
	//    Search Control Point: Mandatory-Write, Write Without Response,Notify; Optional-
	static const u16 my_GmcsSearchCtrPointUUID = CHARACTERISTIC_UUID_MCS_SEARCH_CONTROL_POINT;
	static const u8 my_GmcsSearchCtrPointChar[5] = {
		CHAR_PROP_NOTIFY | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GMCS_SEARCH_CONTROL_POINT_DP_H), U16_HI(GMCS_SEARCH_CONTROL_POINT_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_SEARCH_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_MCS_SEARCH_CONTROL_POINT),
	};
	static u8 my_GmcsSearchCtrPointValue[] = {
			0x06, 0x01, 'T', 'r', 'a', 'c', 'k', //0x01-Track Name
			0x06, 0x04, 'G', 'r', 'o', 'u', 'p', //0x04-Group Name
		}; //Length+Type+Parameter   max:64 octets.
	u16 my_GmcsSearchCtrPointCCC = 0;
	//    Search Results Object ID: Mandatory-Read, Notify; Optional-
	static const u16 my_GmcsSearchResultObjIDUUID = CHARACTERISTIC_UUID_MCS_SEARCH_RESULTS_OBJECT_ID;
	static const u8 my_GmcsSearchResultObjIDChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GMCS_SEARCH_RESULT_OBJID_DP_H), U16_HI(GMCS_SEARCH_RESULT_OBJID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_SEARCH_RESULTS_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_SEARCH_RESULTS_OBJECT_ID),
	};
	static u8 my_GmcsSearchResultObjIDValue[] = {0x04,0x00,0x00,0x00,0x00,0x00}; //UINT48
	u16 my_GmcsSearchResultObjIDCCC = 0;
	//    Content Control ID (CCID)
	static const u16 my_GmcsContentControlUUID = CHARACTERISTIC_UUID_MCS_CONTENT_CONTROL_ID;
	static const u8 my_GmcsContentControlChar[5] = {
		CHAR_PROP_READ,
		U16_LO(GMCS_SUPPORTED_OPCODE_DP_H), U16_HI(GMCS_SUPPORTED_OPCODE_DP_H),
		U16_LO(CHARACTERISTIC_UUID_MCS_CONTENT_CONTROL_ID), U16_HI(CHARACTERISTIC_UUID_MCS_CONTENT_CONTROL_ID),
	};
	static u8 my_GmcsCCIDValue = APP_GMCS_CCID; 
	#endif //#if (BLC_AUDIO_MCS_ENABLE)
#endif //#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
	////////////////////////////////////// GTBS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_TBS_ENABLE)
	int app_att_audioTbsRead(u16 connHandle, void *p);
	int app_att_audioTbsWrite(u16 connHandle, void *p);
	static const u16 my_GtbsServiceUUID = SERVICE_UUID_GENERIC_TELEPHONE_BEARER;
	//    Bearer Provider Name
	static const u16 my_GtbsProviderNameUUID = CHARACTERISTIC_UUID_TBS_PROVIDER_NAME;
	static const u8 my_GtbsProviderNameChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_PROVIDER_NAME_DP_H), U16_HI(GTBS_PROVIDER_NAME_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_PROVIDER_NAME), U16_HI(CHARACTERISTIC_UUID_TBS_PROVIDER_NAME),
	};
	u8 my_GtbsProviderNameValue[] = {
		't','l','i','n','k','-','P','h','o','n','e', 
	};
//	u8 my_GtbsProviderNameValue[] = {
//		't','l','i','n','k','-','P','h','o','n','e','-','P','h','o','n','e','-','P','h','o','n','e','-','P','h','o','n','e','-','P','h','o','n','e',
//	};
	u16 my_GtbsProviderNameCCC = 0;
	//    Bearer Uniform Caller Identifier (UCI)
	static const u16 my_GtbsUCIUUID = CHARACTERISTIC_UUID_TBS_CALLER_IDENTIFIER;
	static const u8 my_GtbsUCIChar[5] = {
		CHAR_PROP_READ,
		U16_LO(GTBS_UNIFORM_CALLER_IDENTIFIER_DP_H), U16_HI(GTBS_UNIFORM_CALLER_IDENTIFIER_DP_H),
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
	static u8 my_GtbsTechValue[] = {0x01}; // 0x01-3G, 0x02-4G //Refer https://www.bluetooth.com/specifications/assigned-numbers
	u16 my_GtbsTechCCC = 0;
	//    Bearer URI Schemes Supported List
	static const u16 my_GtbsURIListUUID = CHARACTERISTIC_UUID_TBS_URI_LIST;
	static const u8 my_GtbsURIListChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_URI_SCHEMES_SUUP_LIST_DP_H), U16_HI(GTBS_URI_SCHEMES_SUUP_LIST_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_URI_LIST), U16_HI(CHARACTERISTIC_UUID_TBS_URI_LIST),
	};
	u8 my_GtbsURIListValue[] = {'t','e','l',',','s','k','y','p','e'}; ///
	u16 my_GtbsURIListCCC = 0;
	//    Bearer Signal Strength 
	static const u16 my_GtbsSignalStrengthUUID = CHARACTERISTIC_UUID_TBS_SIGNAL_STRENGTH;
	static const u8 my_GtbsSignalStrengthChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_SIGNAL_STRENGTH_DP_H), U16_HI(GTBS_SIGNAL_STRENGTH_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_SIGNAL_STRENGTH), U16_HI(CHARACTERISTIC_UUID_TBS_SIGNAL_STRENGTH),
	};
	static u8 my_GtbsSignalStrengthValue = 100; ///0 to 100, or 255. Values 101 to 254 are RFU. 100 indicates the maximum signal strength
	u16 my_GtbsSignalStrengthCCC = 0;
	//    Bearer Signal Strength Reporting Interval
	static const u16 my_GtbsSignalStrengthReportUUID = CHARACTERISTIC_UUID_TBS_SIGNAL_REPORT_INTERVAL;
	static const u8 my_GtbsSignalStrengthReportChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
		U16_LO(GTBS_SIGNAL_STRENGTH_REPORT_DP_H), U16_HI(GTBS_SIGNAL_STRENGTH_REPORT_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_SIGNAL_REPORT_INTERVAL), U16_HI(CHARACTERISTIC_UUID_TBS_SIGNAL_REPORT_INTERVAL),
	};
	static u8 my_GtbsSignalStrengthReportValue = 100; ///0 to 255, Unit:s
	u16 my_GtbsSignalStrengthReportCCC = 0;
	//    Bearer List Current Calls
	static const u16 my_GtbsCurCallListUUID = CHARACTERISTIC_UUID_TBS_CURRENT_CALL_LIST;
	static const u8 my_GtbsCurCallListChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_CURRENT_CALL_LIST_DP_H), U16_HI(GTBS_CURRENT_CALL_LIST_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_CURRENT_CALL_LIST), U16_HI(CHARACTERISTIC_UUID_TBS_CURRENT_CALL_LIST),
	};
	u8 my_GtbsCurCallListValue[] = {
		//_Item_Length, Call_Index, Call_State, Call_Flags, Call_URI[i]
		0x06,0x00,0x00,0x00,'t','e','l',
		0x08,0x01,0x00,0x00,'s','k','y','p','e'
	};
//	u8 my_GtbsCurCallListValue[] = {
//		//_Item_Length, Call_Index, Call_State, Call_Flags, Call_URI[i]
//		0x06,0x00,0x00,0x00,'t','e','l',
//		0x08,0x01,0x00,0x00,'s','k','y','p','e',
//		0x08,0x02,0x00,0x00,'s','k','y','p','2',
//		0x08,0x03,0x00,0x00,'s','k','y','p','3',
//		0x08,0x04,0x00,0x00,'s','k','y','p','4',
//		0x08,0x05,0x00,0x00,'s','k','y','p','5',
//	};
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
	static u8 my_GtbsStatusValue[] = {0x00, 0x00};
	u16 my_GtbsStatusCCC = 0;
	//    Incoming Call Target Bearer URI
	static const u16 my_GtbsIncomingCallURIUUID = CHARACTERISTIC_UUID_TBS_INCOMING_CALL_URI;
	static const u8 my_GtbsIncomingCallURIChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_INCOMING_CALL_URI_DP_H), U16_HI(GTBS_INCOMING_CALL_URI_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_INCOMING_CALL_URI), U16_HI(CHARACTERISTIC_UUID_TBS_INCOMING_CALL_URI),
	};
	u8 my_GtbsIncomingCallURIValue[] = {
		0x01, //Call_Index
		't','e','l',':','+','8','6','2','1','2','0','2','8','1','1','1','8' //URI
	};
//	u8 my_GtbsIncomingCallURIValue[] = {
//		0x01, //Call_Index
//		't','e','l',':','+','8','6','2','1','2','0','2','8','1','1','1','8','8','6','2','1','2','0','2','8','1','1','1','8' //URI
//	};
	u16 my_GtbsIncomingCallURICCC = 0;
	//    Call State
	static const u16 my_GtbsStateUUID = CHARACTERISTIC_UUID_TBS_CALL_STATE;
	static const u8 my_GtbsStateChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_CALL_STATE_DP_H), U16_HI(GTBS_CALL_STATE_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_CALL_STATE), U16_HI(CHARACTERISTIC_UUID_TBS_CALL_STATE),
	};
	u8 my_GtbsStateValue[] = {
		//Call_Index[i], State[i], Call_Flags[i] 
		0x00, 0x00, 0x00,
		0x01, 0x00, 0x00,
	};
//	u8 my_GtbsStateValue[] = {
//		//Call_Index[i], State[i], Call_Flags[i] 
//		0x00, 0x00, 0x00,
//		0x01, 0x00, 0x00,
//		0x02, 0x00, 0x00,
//		0x03, 0x00, 0x00,
//		0x04, 0x00, 0x00,
//		0x05, 0x00, 0x00,
//		0x06, 0x00, 0x00,
//		0x07, 0x00, 0x00,
//		0x08, 0x00, 0x00,
//		0x09, 0x00, 0x00,
//		0x0A, 0x00, 0x00,
//	};
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
	static u16 my_GtbsCtrlOpcodesValue = TBP_CP_SUPPORTED_OPCODE_DEFAULT;
	//    Termination Reason
	static const u16 my_GtbsTermReasonUUID = CHARACTERISTIC_UUID_TBS_TERMINATIONO_REASON;
	static const u8 my_GtbsTermReasonChar[5] = {
		CHAR_PROP_NOTIFY,
		U16_LO(GTBS_TERMINATIONO_REASON_DP_H), U16_HI(GTBS_TERMINATIONO_REASON_DP_H),
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
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_INCOMING_CALL_DP_H), U16_HI(GTBS_INCOMING_CALL_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_INCOMING_CALL), U16_HI(CHARACTERISTIC_UUID_TBS_INCOMING_CALL),
	};
	u8 my_GtbsIncomingCallValue[] = {
		//Call_Index, URI (variable) 
		0x01, 's','k','y','p','e'
	};
//	u8 my_GtbsIncomingCallValue[] = {
//		//Call_Index, URI (variable) 
//		0x01, 's','k','y','p','e','s','k','y','p','e','s','k','y','p','e','s','k','y','p','e','s','k','y','p','e',
//	};
	u16 my_GtbsIncomingCallCCC = 0;
	//    Call Friendly Name
	static const u16 my_GtbsCallFriendlyNameUUID = CHARACTERISTIC_UUID_TBS_CALL_FREIENDLY_NAME;
	static const u8 my_GtbsCallFriendlyNameChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_NOTIFY,
		U16_LO(GTBS_CALL_FRIENDLY_NAME_DP_H), U16_HI(GTBS_CALL_FRIENDLY_NAME_DP_H),
		U16_LO(CHARACTERISTIC_UUID_TBS_CALL_FREIENDLY_NAME), U16_HI(CHARACTERISTIC_UUID_TBS_CALL_FREIENDLY_NAME),
	};
	u8 my_GtbsCallFriendlyNameValue[] = {
		'f','r','i','e','n','d','l','y'
	};
//	u8 my_GtbsCallFriendlyNameValue[] = {
//		'f','r','i','e','n','d','l','y',
//		'f','r','i','e','n','d','l','y',
//		'f','r','i','e','n','d','l','y',
//		'f','r','i','e','n','d','l','y',
//		'f','r','i','e','n','d','l','y',
//		'f','r','i','e','n','d','l','y',
//	};
	u16 my_GtbsCallFriendlyNameCCC = 0;
	#endif //#if (BLC_AUDIO_TBS_ENABLE)
#endif //#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)	
	////////////////////////////////////// OTS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_OTS_ENABLE)
	int app_att_audioOtsRead(u16 connHandle, void *p);
	int app_att_audioOtsWrite(u16 connHandle, void *p);
	static const u16 my_OtsServiceUUID = SERVICE_UUID_OBJECT_TRANSFER;
	//    OTS Feature    Mandatory:Read; Optional:
	static const u16 my_OtsFeatureUUID = CHARACTERISTIC_UUID_OTS_FEATURE;
	static const u8 my_OtsFeatureChar[5] = {
		CHAR_PROP_READ,
		U16_LO(OTS_FEATURE_DP_H), U16_HI(OTS_FEATURE_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_FEATURE), U16_HI(CHARACTERISTIC_UUID_OTS_FEATURE),
	};
	u8 my_OtsFeatureValue[] = {
		0xFF, 0x03, 0x00, 0x00, //OACP Features Field, all supported. <OTS_V10.pdf> P16
		0x0F, 0x00, 0x00, 0x00, //OLCP Features Field, all supported. <OTS_V10.pdf> P17
	};
	//    Object Name    Mandatory:Read; Optional:Write
	static const u16 my_OtsObjectNameUUID = CHARACTERISTIC_UUID_OTS_OBJECT_NAME;
	static const u8 my_OtsObjectNameChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_WRITE,
		U16_LO(OTS_OBJECT_NAME_DP_H), U16_HI(OTS_OBJECT_NAME_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_NAME), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_NAME),
	};
	u8 my_OtsObjectNameValue[] = {
		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
//		'T','e','l','i','n','k',
	};
	//    Object Type    Mandatory:Read; Optional:
	static const u16 my_OtsObjectTypeUUID = CHARACTERISTIC_UUID_OTS_OBJECT_TYPE;
	static const u8 my_OtsObjectTypeChar[5] = {
		CHAR_PROP_READ,
		U16_LO(OTS_OBJECT_TYPE_DP_H), U16_HI(OTS_OBJECT_TYPE_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_TYPE), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_TYPE),
	};
	u8 my_OtsObjectTypeValue[] = {
		//U16_LO(CHARACTERISTIC_UUID_MCS_CURRENT_GOUP_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_MCS_CURRENT_GOUP_OBJECT_ID), //UUID:The Object Type characteristic returns its associated value when read.
		U16_LO(0x7FB1), U16_HI(0x7FB1), 
	};
	//    Object Size    Mandatory:Read; Optional:
	static const u16 my_OtsObjectSizeUUID = CHARACTERISTIC_UUID_OTS_OBJECT_SIZE;
	static const u8 my_OtsObjectSizeChar[5] = {
		CHAR_PROP_READ,
		U16_LO(OTS_OBJECT_SIZE_DP_H), U16_HI(OTS_OBJECT_SIZE_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_SIZE), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_SIZE),
	};
	u8 my_OtsObjectSizeValue[] = {
		0x010, 0x00, 0x00, 0x00, //Current Size Field
		0x020, 0x00, 0x00, 0x00, //Allocated Size Field
	};
	//    Object First-Created    Mandatory:Read; Optional:Write
	static const u16 my_OtsFirstCreatedUUID = CHARACTERISTIC_UUID_OTS_OBJECT_FIRST_CREATED;
	static const u8 my_OtsFirstCreatedChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_WRITE,
		U16_LO(OTS_OBJECT_FIRST_CREATED_DP_H), U16_HI(OTS_OBJECT_FIRST_CREATED_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_FIRST_CREATED), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_FIRST_CREATED),
	};
	u8 my_OtsFirstCreatedValue[] = {
		0xE5, 0x07, 0x04, 0x15, 0x09, 0x04, 0x2E,  //2021-01-21 09:04:46
	};
	//    Object Last-Modified    Mandatory:Read; Optional:Write
	static const u16 my_OtsLastCreatedUUID = CHARACTERISTIC_UUID_OTS_OBJECT_LAST_CREATED;
	static const u8 my_OtsLastCreatedChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_WRITE,
		U16_LO(OTS_OBJECT_LAST_CREATED_DP_H), U16_HI(OTS_OBJECT_LAST_CREATED_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_LAST_CREATED), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_LAST_CREATED),
	};
	u8 my_OtsLastCreatedValue[] = {
		0xE5, 0x07, 0x04, 0x15, 0x09, 0x08, 0x15, //2021-01-21 09:08:21
	};
	//    Object ID    Mandatory:Read; Optional:
	static const u16 my_OtsObjectIDUUID = CHARACTERISTIC_UUID_OTS_OBJECT_ID;
	static const u8 my_OtsObjectIDChar[5] = {
		CHAR_PROP_READ,
		U16_LO(OTS_OBJECT_ID_DP_H), U16_HI(OTS_OBJECT_ID_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_ID), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_ID),
	};
	u8 my_OtsObjectIDValue[] = {
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00,  //0x000000000100 to 0xFFFFFFFFFFFF.
	};
	//    Object Properties    Mandatory:Read; Optional:Write
	static const u16 my_OtsObjectPropertiesUUID = CHARACTERISTIC_UUID_OTS_OBJECT_PROPERTIES;
	static const u8 my_OtsObjectPropertiesChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_WRITE,
		U16_LO(OTS_OBJECT_PROPERTIES_DP_H), U16_HI(OTS_OBJECT_PROPERTIES_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_PROPERTIES), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_PROPERTIES),
	};
	u8 my_OtsObjectPropertiesValue[] = {
		0xFF, 0x00, 0x00, 0x00, //Object Properties, all supported. <OTS_V10.pdf> P22
	};
	//    Object Action Control Point    Mandatory:Write,Indicate; Optional:
	static const u16 my_OtsActionControlPointUUID = CHARACTERISTIC_UUID_OTS_OBJECT_ACTION_CONTROL_POINT;
	static const u8 my_OtsActionControlPointChar[5] = {
		CHAR_PROP_WRITE | CHAR_PROP_INDICATE,
		U16_LO(OTS_OBJECT_ACTION_CP_DP_H), U16_HI(OTS_OBJECT_ACTION_CP_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_ACTION_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_ACTION_CONTROL_POINT),
	};
	u8 my_OtsActionControlPointValue[] = { 0x00, };
	u16 my_OtsActionControlPointCCC = 0;
	//    Object List Control Point    Mandatory:Write,Indicate; Optional:
	static const u16 my_OtsListControlPointUUID = CHARACTERISTIC_UUID_OTS_OBJECT_LIST_CONTROL_POINT;
	static const u8 my_OtsListControlPointChar[5] = {
		CHAR_PROP_WRITE | CHAR_PROP_INDICATE,
		U16_LO(OTS_OBJECT_LIST_CP_DP_H), U16_HI(OTS_OBJECT_LIST_CP_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_LIST_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_LIST_CONTROL_POINT),
	};
	u8 my_OtsListControlPointValue[] = { 0x00, };
	u16 my_OtsListControlPointCCC = 0;
	//    Object List Filter    Mandatory:Read,Write; Optional:
	static const u16 my_OtsObjectListFilterUUID = CHARACTERISTIC_UUID_OTS_OBJECT_LIST_FILTER;
	static const u8 my_OtsObjectListFilterChar[5] = {
		CHAR_PROP_READ | CHAR_PROP_WRITE,
		U16_LO(OTS_OBJECT_LIST_FILTER_DP_H), U16_HI(OTS_OBJECT_LIST_FILTER_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_LIST_FILTER), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_LIST_FILTER),
	};
	u8 my_OtsObjectListFilterValue[] = { //ItemList: FilterValue(1Byte)+Size(1Byte)+String, 0x00--No Filter (everything passes)
		0x01, 0x06, 'T','e','l','i','n','k', //0x01-Name Starts With
		0x02, 0x06, 'T','e','l','i','n','k', //0x02-Name Ends With
//		0x03, 0x06, 'T','e','l','i','n','k', //0x03-Name Contains
//		0x04, 0x06, 'T','e','T','T','T','T', //0x04-Name is Exactly
	};
	//    Object Changed    Mandatory:Indicate; Optional:
	static const u16 my_OtsObjectChangedUUID = CHARACTERISTIC_UUID_OTS_OBJECT_CHANGED;
	static const u8 my_OtsObjectChangedChar[5] = {
		CHAR_PROP_INDICATE,
		U16_LO(OTS_OBJECT_CHANGED_DP_H), U16_HI(OTS_OBJECT_CHANGED_DP_H),
		U16_LO(CHARACTERISTIC_UUID_OTS_OBJECT_CHANGED), U16_HI(CHARACTERISTIC_UUID_OTS_OBJECT_CHANGED),
	};
	u8 my_OtsObjectChangedValue[] = { 
		0x00, //Flags
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, //Object ID
	};
	u16 my_OtsObjectChangedCCC = 0;
	#endif
#endif //#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)

// TM : to modify
static const attribute_t my_Attributes[] = {

	{ATT_END_H - 1, 0,0,0,0,0},	// total num of attribute


	// 0001 - 0007  gap
	{7,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gapServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharVal),(u8*)(&my_characterUUID), (u8*)(my_devNameCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devName), (u8*)(&my_devNameUUID), (u8*)(my_devName), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharVal),(u8*)(&my_characterUUID), (u8*)(my_appearanceCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_appearance), (u8*)(&my_appearanceUIID), 	(u8*)(&my_appearance), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamCharVal),(u8*)(&my_characterUUID), (u8*)(my_periConnParamCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_periConnParameters),(u8*)(&my_periConnParamUUID), 	(u8*)(&my_periConnParameters), 0},


	// 0008 - 000b gatt
	{4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gattServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_serviceChangeCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_serviceChangeCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (serviceChangeVal), (u8*)(&serviceChangeUUID), 	(u8*)(&serviceChangeVal), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof (serviceChangeCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0},


	// 000c - 000e  device Information Service
	{3,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_devServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PnCharVal),(u8*)(&my_characterUUID), (u8*)(my_PnCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_PnPtrs),(u8*)(&my_PnPUUID), (u8*)(my_PnPtrs), 0},


	/////////////////////////////////// 4. HID Service /////////////////////////////////////////////////////////
	// 000f
	//{27, ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_hidServiceUUID), 0},
	{HID_CONTROL_POINT_DP_H - HID_PS_H + 1, ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_hidServiceUUID), 0},

	// 0010  include battery service
	{0,ATT_PERMISSIONS_READ,2,sizeof(include),(u8*)(&hidIncludeUUID), 	(u8*)(include), 0},

	// 0011 - 0012  protocol mode
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidProtocolModeCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidProtocolModeCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2, sizeof(protocolMode),(u8*)(&hidProtocolModeUUID), 	(u8*)(&protocolMode), 0},	//value

	// 0013 - 0015  boot keyboard input report (char-val-client)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyInReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyInReporCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(bootKeyInReport),(u8*)(&hidbootKeyInReportUUID), 	(u8*)(&bootKeyInReport), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(bootKeyInReportCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(bootKeyInReportCCC), 0},	//value

	// 0016 - 0017   boot keyboard output report (char-val)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyOutReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyOutReporCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2, sizeof(bootKeyOutReport), (u8*)(&hidbootKeyOutReportUUID), 	(u8*)(&bootKeyOutReport), 0},	//value


	// 0018 - 001b. consume report in: 4 (char-val-client-ref)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportCCinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportCCinCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2, sizeof(reportConsumerControlIn),(u8*)(&hidReportUUID), 	(u8*)(reportConsumerControlIn), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportConsumerControlInCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(reportConsumerControlInCCC), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefConsumerControlIn),(u8*)(&reportRefUUID), 	(u8*)(reportRefConsumerControlIn), 0},	//value

	// 001c - 001f . keyboard report in : 4 (char-val-client-ref)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYinCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2, sizeof(reportKeyIn),(u8*)(&hidReportUUID), 	(u8*)(reportKeyIn), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyInCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(reportKeyInCCC), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyIn),(u8*)(&reportRefUUID), 	(u8*)(reportRefKeyIn), 0},	//value

	// 0020 - 0022 . keyboard report out: 3 (char-val-ref)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYoutCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYoutCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyOut),(u8*)(&hidReportUUID), 	(u8*)(reportKeyOut), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyOut),(u8*)(&reportRefUUID), 	(u8*)(reportRefKeyOut), 0},	//value


	// 0023 - 0025 . report map: 3
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportMapCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportMapCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(reportMap),(u8*)(&hidReportMapUUID), 	(u8*)(reportMap), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(extServiceUUID),(u8*)(&extReportRefUUID), 	(u8*)(&extServiceUUID), 0},	//value

	// 0026 - 0027 . hid information: 2
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidinformationCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidinformationCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2, sizeof(hidInformation),(u8*)(&hidinformationUUID), 	(u8*)(hidInformation), 0},	//value

	// 0028 - 0029 . control point: 2
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidCtrlPointCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidCtrlPointCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_WRITE,2, sizeof(controlPoint),(u8*)(&hidCtrlPointUUID), 	(u8*)(&controlPoint), 0},	//value

	////////////////////////////////////// Battery Service /////////////////////////////////////////////////////
	// 002a - 002d
	{4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_batServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_batCharVal),(u8*)(&my_characterUUID), (u8*)(my_batCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_batVal),(u8*)(&my_batCharUUID), (u8*)(my_batVal), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(batteryValueInCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(batteryValueInCCC), 0},	//value

	////////////////////////////////////// OTA /////////////////////////////////////////////////////
	// 002e - 0031
	{4,ATT_PERMISSIONS_READ, 2,16,(u8*)(&my_primaryServiceUUID),  (u8*)(&my_OtaServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ, 2, sizeof(my_OtaCharVal),(u8*)(&my_characterUUID), (u8*)(my_OtaCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,16,sizeof(my_OtaData),(u8*)(&my_OtaUUID), (&my_OtaData), &otaWrite, 0},			//value
	{0,ATT_PERMISSIONS_READ, 2,sizeof (my_OtaName),(u8*)(&userdesc_UUID), (u8*)(my_OtaName), 0},


	////////////////////////////////////// PSCS /////////////////////////////////////////////////////
#if (BLC_AUDIO_PACS_ENABLE)
	// 58~
	{19,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_PacsServiceUUID), 0},
	//          Sink PAC: Mandatory:Read, Optional:Notify
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsSinkPacChar),(u8*)(&my_characterUUID), (u8*)(my_PacsSinkPacChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsSinkPacValue),(u8*)(&my_PacsSinkPacUUID), my_PacsSinkPacValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_PacsSinkPacCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_PacsSinkPacCCC), 0},
	//          Sink Location: Mandatory:Read, Optional:Notify,Write
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsSinkLocationChar),(u8*)(&my_characterUUID), (u8*)(my_PacsSinkLocationChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_sinkLocationValue),(u8*)(&my_PacsSinkLocationUUID), (u8*)(&my_sinkLocationValue), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_sinkLocationCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_sinkLocationCCC), 0},
	//          Source PAC: Mandatory:Read, Optional:Notify
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsSourcePacChar),(u8*)(&my_characterUUID), (u8*)(my_PacsSourcePacChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsSinkPacValue),(u8*)(&my_PacsSourcePacUUID), my_PacsSinkPacValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_PacsSourcePacCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_PacsSourcePacCCC), 0},
	//          Source Location: Mandatory:Read, Optional:Notify,Write
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsSourceLocationChar),(u8*)(&my_characterUUID), (u8*)(my_PacsSourceLocationChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_sourceLocationValue),(u8*)(&my_PacsSourceLocationUUID), (u8*)(&my_sourceLocationValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_sourceLocationCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_sourceLocationCCC), 0},
	//          Available Context: Mandatory:Read,Notify Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsAvailableContextChar),(u8*)(&my_characterUUID), (u8*)(my_PacsAvailableContextChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_AvailableContextValue),(u8*)(&my_PacsAvailableContextUUID), my_AvailableContextValue, &app_att_audio_pacpWrite, &app_att_audio_pacpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_AvailableContextCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_AvailableContextCCC), 0},
	//          Supperted Context: Mandatory:Read,Notify Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PacsSuppertedContextChar),(u8*)(&my_characterUUID), (u8*)(my_PacsSuppertedContextChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_SuppertedContextValue),(u8*)(&my_PacsSuppertedContextUUID), my_SuppertedContextValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_SuppertedContextCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_SuppertedContextCCC), 0},
#endif //BLC_AUDIO_PACS_ENABLE

	////////////////////////////////////// ASCS /////////////////////////////////////////////////////
#if (BLC_AUDIO_ASCS_ENABLE)
	//
	{10,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_AscsServiceUUID), 0},
	//			ASE0:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_AscsAse0Char),(u8*)(&my_characterUUID), (u8*)(my_AscsAse0Char), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_AscsAse0Value),(u8*)(&my_AscsAse0UUID), my_AscsAse0Value, 0, &app_att_audioAscpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_AscsAse0CCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_AscsAse0CCC), 0},
	//			ASE1:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_AscsAse1Char),(u8*)(&my_characterUUID), (u8*)(my_AscsAse1Char), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_AscsAse1Value),(u8*)(&my_AscsAse1UUID), my_AscsAse1Value, 0, &app_att_audioAscpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_AscsAse1CCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_AscsAse1CCC), 0},
	//			Ctrl:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_AscsCtrlChar),(u8*)(&my_characterUUID), (u8*)(my_AscsCtrlChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_AscsCtrlValue),(u8*)(&my_AscsCtrlUUID), my_AscsCtrlValue, &app_att_audioAscpWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_AscsCtrlCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_AscsCtrlCCC), 0},
#endif //BLC_AUDIO_CSIS_ENABLE

	////////////////////////////////////// CSIS /////////////////////////////////////////////////////
#if (BLC_AUDIO_CSIS_ENABLE)
	//
	{10,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_CsisServiceUUID), 0},
	//          SIRK:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_CsisSIRKChar),(u8*)(&my_characterUUID), (u8*)(my_CsisSIRKChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_CsisSIRKValue),(u8*)(&my_CsisSIRKUUID), my_CsisSIRKValue, 0, &app_att_audioCsipRead},
	//          SIZE:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_CsisSizeChar),(u8*)(&my_characterUUID), (u8*)(my_CsisSizeChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_CsisSizeValue),(u8*)(&my_CsisSizeUUID), my_CsisSizeValue, 0, 0},
	//          LOCK:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_CsisLockChar),(u8*)(&my_characterUUID), (u8*)(my_CsisLockChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_CsisLockValue),(u8*)(&my_CsisLockUUID), my_CsisLockValue, &app_att_audioCsipWrite, &app_att_audioCsipRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_CsisLockCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_CsisLockCCC), 0},
	//          RANK:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_CsisRankChar),(u8*)(&my_characterUUID), (u8*)(my_CsisRankChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_CsisRankValue),(u8*)(&my_CsisRankUUID), my_CsisRankValue, 0, &app_att_audioCsipRead},
#endif //BLC_AUDIO_CSIS_ENABLE

////////////////////////////////////// MICS /////////////////////////////////////////////////////
#if (BLC_AUDIO_MICS_ENABLE)
	#if  BLC_AUDIO_MICS_AICS_ENABLE
	// AICS
	{16,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_secondaryServiceUUID), (u8*)(&my_MicsAicsServiceUUID), 0},
	//          Input State:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputStateChar),(u8*)(&my_characterUUID), (u8*)(my_MicsAicsInputStateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputStateValue),(u8*)(&my_MicsAicsInputStateUUID), my_MicsAicsInputStateValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicsAicsInputStateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_MicsAicsInputStateCCC), 0},
	//          Gain Setting:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsGainSettingChar),(u8*)(&my_characterUUID), (u8*)(my_MicsAicsGainSettingChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsGainSettingValue),(u8*)(&my_MicsAicsGainSettingUUID), my_MicsAicsGainSettingValue, 0, 0},
	//          Input Type
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputTypeChar),(u8*)(&my_characterUUID), (u8*)(my_MicsAicsInputTypeChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputTypeValue),(u8*)(&my_MicsAicsInputTypeUUID), my_MicsAicsInputTypeValue, 0, 0},
	//          Input Status
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputStatusChar),(u8*)(&my_characterUUID), (u8*)(my_MicsAicsInputStatusChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputStatusValue),(u8*)(&my_MicsAicsInputStatusUUID), my_MicsAicsInputStatusValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicsAicsInputStatusCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_MicsAicsInputStatusCCC), 0},
	//          Input Control 
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputControlChar),(u8*)(&my_characterUUID), (u8*)(my_MicsAicsInputControlChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_MicsAicsInputControlValue),(u8*)(&my_MicsAicsInputControlUUID), my_MicsAicsInputControlValue, app_att_audioMicpWrite, 0},
	//          Input Description
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInputDescripChar),(u8*)(&my_characterUUID), (u8*)(my_MicsAicsInputDescripChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicsAicsInputDescripValue),(u8*)(&my_MicsAicsInputDescripUUID), my_MicsAicsInputDescripValue, app_att_audioMicpWrite, app_att_audioMicpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicsAicsInputDescripCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_MicsAicsInputDescripCCC), 0},
	#endif
	//  MICS
	#if BLC_AUDIO_MICS_AICS_ENABLE
	{5,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID),(u8*)(&my_MicsServiceUUID), 0},
	#else
	{4,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID),(u8*)(&my_MicsServiceUUID), 0},
	#endif
	#if BLC_AUDIO_MICS_AICS_ENABLE //Inclue
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsAicsInclude),(u8*)(&my_MicsAicsIncludeUUID),(u8*)(my_MicsAicsInclude), 0},
	#endif
	//          Mute
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_MicsMuteChar),(u8*)(&my_characterUUID), (u8*)(my_MicsMuteChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicsMuteValue),(u8*)(&my_MicsMuteUUID), my_MicsMuteValue, &app_att_audioMicpWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicsMuteCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_MicsMuteCCC), 0},
#endif //BLC_AUDIO_MICS_ENABLE

	////////////////////////////////////// VCS /////////////////////////////////////////////////////
#if (BLC_AUDIO_VCS_ENABLE)
	#if  BLC_AUDIO_VCS_AICS_ENABLE
	// AICS
	{16,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_secondaryServiceUUID), (u8*)(&my_VcsAicsServiceUUID), 0},
	//          Input State:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputStateChar),(u8*)(&my_characterUUID), (u8*)(my_VcsAicsInputStateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputStateValue),(u8*)(&my_VcsAicsInputStateUUID), my_VcsAicsInputStateValue, 0, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_VcsAicsInputStateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_VcsAicsInputStateCCC), 0},
	//          Gain Setting:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsGainSettingChar),(u8*)(&my_characterUUID), (u8*)(my_VcsAicsGainSettingChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsGainSettingValue),(u8*)(&my_VcsAicsGainSettingUUID), my_VcsAicsGainSettingValue, 0, app_att_audioVcpRead},
	//          Input Type
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputTypeChar),(u8*)(&my_characterUUID), (u8*)(my_VcsAicsInputTypeChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputTypeValue),(u8*)(&my_VcsAicsInputTypeUUID), my_VcsAicsInputTypeValue, 0, app_att_audioVcpRead},
	//          Input Status
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputStatusChar),(u8*)(&my_characterUUID), (u8*)(my_VcsAicsInputStatusChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputStatusValue),(u8*)(&my_VcsAicsInputStatusUUID), my_VcsAicsInputStatusValue, 0, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_VcsAicsInputStatusCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_VcsAicsInputStatusCCC), 0},
	//          Input Control
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputControlChar),(u8*)(&my_characterUUID), (u8*)(my_VcsAicsInputControlChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_VcsAicsInputControlValue),(u8*)(&my_VcsAicsInputControlUUID), my_VcsAicsInputControlValue, app_att_audioVcpWrite, 0},
	//          Input Description
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInputDescripChar),(u8*)(&my_characterUUID), (u8*)(my_VcsAicsInputDescripChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_VcsAicsInputDescripValue),(u8*)(&my_VcsAicsInputDescripUUID), my_VcsAicsInputDescripValue, app_att_audioVcpWrite, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_VcsAicsInputDescripCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_VcsAicsInputDescripCCC), 0},
	#endif
	#if (BLC_AUDIO_VOCS_ENABLE)
	{12,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_secondaryServiceUUID), (u8*)(&my_Vocs0ServiceUUID), 0},
	//          Offset State:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_Vocs0StateChar),(u8*)(&my_characterUUID), (u8*)(my_Vocs0StateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_Vocs0StateValue),(u8*)(&my_Vocs0StateUUID), my_Vocs0StateValue, 0, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_Vocs0StateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_Vocs0StateCCC), 0},
	//          Audio Location:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_Vocs0LocationChar),(u8*)(&my_characterUUID), (u8*)(my_Vocs0LocationChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_Vocs0LocationValue),(u8*)(&my_Vocs0LocationUUID), (u8*)(&my_Vocs0LocationValue), app_att_audioVcpWrite, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_Vocs0LocationCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_Vocs0LocationCCC), 0},
	//          Volume Offset Control Point:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_Vocs0ControlChar),(u8*)(&my_characterUUID), (u8*)(my_Vocs0ControlChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_Vocs0ControlValue),(u8*)(&my_Vocs0ControlUUID), my_Vocs0ControlValue, app_att_audioVcpWrite, 0},
	//          Audio Output Description:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_Vocs0DescriptChar),(u8*)(&my_characterUUID), (u8*)(my_Vocs0DescriptChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_Vocs0DescriptValue),(u8*)(&my_Vocs0DescriptUUID), my_Vocs0DescriptValue, app_att_audioVcpWrite, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_Vocs0DescriptCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_Vocs0DescriptCCC), 0},
	#endif
	#if (BLC_AUDIO_VCS_AICS_ENABLE && BLC_AUDIO_VOCS_ENABLE)
	{11,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_VcsServiceUUID), 0},
	#elif (BLC_AUDIO_VCS_AICS_ENABLE || BLC_AUDIO_VOCS_ENABLE)
	{10,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_VcsServiceUUID), 0},
	#else
	{9,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_VcsServiceUUID), 0},
	#endif
	#if  BLC_AUDIO_VCS_AICS_ENABLE //Inclue
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsAicsInclude),(u8*)(&my_VcsAicsIncludeUUID),(u8*)(my_VcsAicsInclude), 0},
	#endif
	#if BLC_AUDIO_VOCS_ENABLE //Inclue
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsVocsInclude),(u8*)(&my_VcsVocsIncludeUUID),(u8*)(my_VcsVocsInclude), 0},
	#endif
	//          Volume State: Mandatory:Read,Notify; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsStateChar),(u8*)(&my_characterUUID), (u8*)(my_VcsStateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsStateValue),(u8*)(&my_VcsStateUUID), my_VcsStateValue, 0, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_VcsStateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_VcsStateCCC), 0},
	//          Volume Control Point: Mandatory:Write; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsControlPointChar),(u8*)(&my_characterUUID), (u8*)(my_VcsControlPointChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_VcsControlPointValue),(u8*)(&my_VcsControlPointUUID), (&my_VcsControlPointValue), app_att_audioVcpWrite, 0},
	//			Volume Flags: Mandatory:Read,Notify; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsFlagsChar),(u8*)(&my_characterUUID), (u8*)(my_VcsFlagsChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_VcsFlagsValue),(u8*)(&my_VcsFlagsUUID), (&my_VcsFlagsValue), 0, app_att_audioVcpRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_VcsFlagsCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_VcsFlagsCCC), 0},
#endif //BLC_AUDIO_VCS_ENABLE

#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
	////////////////////////////////////// RAAS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_RAAS_ENABLE)
	{RAAS_MODIFY_ARES_CCC_H-RAAS_PS_H+1,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_RaasServiceUUID), 0},
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
	#endif
#endif //#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
	////////////////////////////////////// GMCS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_MCS_ENABLE)
	{GMCS_CCID_DP_H-GMCS_PS_H+1,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_GmcsServiceUUID), 0},
	//			Media Player Name
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerNameChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsPlayerNameChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerNameValue),(u8*)(&my_GmcsPlayerNameUUID), my_GmcsPlayerNameValue, 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsPlayerNameCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsPlayerNameCCC), 0},
	//			Media Player Icon Object ID: Mandatory-Read; Optional-
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerIconIDChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsPlayerIconIDChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerIconIDValue),(u8*)(&my_GmcsPlayerIconIDUUID), my_GmcsPlayerIconIDValue, 0, 0},
	//			Media Player Icon URL: Mandatory-Read; Optional-
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerIconURLChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsPlayerIconURLChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayerIconURLValue),(u8*)(&my_GmcsPlayerIconURLUUID), my_GmcsPlayerIconURLValue, 0, 0},
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
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsTrackPositionValue),(u8*)(&my_GmcsTrackPositionUUID), (u8*)(&my_GmcsTrackPositionValue), app_att_audioMcsWrite, app_att_audioMcsRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsTrackPositionCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsTrackPositionCCC), 0},	
	//          Playback Speed
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlaybackSpeedChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsPlaybackSpeedChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsPlaybackSpeedValue),(u8*)(&my_GmcsPlaybackSpeedUUID), (u8*)(&my_GmcsPlaybackSpeedValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsPlaybackSpeedCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsPlaybackSpeedCCC), 0},	
	//          Seeking Speed
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSeekingSpeedChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsSeekingSpeedChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSeekingSpeedValue),(u8*)(&my_GmcsSeekingSpeedUUID), (u8*)(&my_GmcsSeekingSpeedValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsSeekingSpeedCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsSeekingSpeedCCC), 0},	
	//          Current Track Segments Object ID
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsCurTrackSegmIDChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsCurTrackSegmIDChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsCurTrackSegmIDValue),(u8*)(&my_GmcsCurTrackSegmIDUUID), (u8*)(&my_GmcsCurTrackSegmIDValue), 0, 0},
	//          Current Track Object ID
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsCurTrackIDChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsCurTrackIDChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsCurTrackIDValue),(u8*)(&my_GmcsCurTrackIDUUID), (u8*)(&my_GmcsCurTrackIDValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsCurTrackIDCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsCurTrackIDCCC), 0},
	//          Next Track Object ID
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsNextTrackIDChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsNextTrackIDChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsNextTrackIDValue),(u8*)(&my_GmcsNextTrackIDUUID), (u8*)(&my_GmcsNextTrackIDValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsNextTrackIDCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsNextTrackIDCCC), 0},
	//          Parent Group Object ID
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsParentGroupIDChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsParentGroupIDChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsParentGroupIDValue),(u8*)(&my_GmcsParentGroupIDUUID), (u8*)(&my_GmcsParentGroupIDValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsParentGroupIDCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsParentGroupIDCCC), 0},
	//          Current Group Object ID
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsCurrentGroupIDChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsCurrentGroupIDChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsCurrentGroupIDValue),(u8*)(&my_GmcsCurrentGroupIDUUID), (u8*)(&my_GmcsCurrentGroupIDValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsCurrentGroupIDCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsCurrentGroupIDCCC), 0},
	//          Playing Order
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayingOrderChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsPlayingOrderChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsPlayingOrderValue),(u8*)(&my_GmcsPlayingOrderUUID), (u8*)(&my_GmcsPlayingOrderValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsPlayingOrderCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsPlayingOrderCCC), 0},
	//          Playing Orders Supported
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsPlayingOrderSuppChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsPlayingOrderSuppChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsPlayingOrderSuppValue),(u8*)(&my_GmcsPlayingOrderSuppUUID), (u8*)(&my_GmcsPlayingOrderSuppValue), 0, 0},
	//          Media State
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsMediaStateChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsMediaStateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsMediaStateValue),(u8*)(&my_GmcsMediaStateUUID), (u8*)(&my_GmcsMediaStateValue), 0, app_att_audioMcsRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsMediaStateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsMediaStateCCC), 0},	
	//          Media Control Point
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsControlPointChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsControlPointChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_GmcsControlPointValue), (u8*)(&my_GmcsControlPointUUID), (u8*)(&my_GmcsControlPointValue), app_att_audioMcsWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsControlPointCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsControlPointCCC), 0},	
	//          Media Control Point Opcodes Supported
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSupportedOpcodesChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsSupportedOpcodesChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSupportedOpcodesValue),(u8*)(&my_GmcsSupportedOpcodesUUID), (u8*)(&my_GmcsSupportedOpcodesValue), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsSupportedOpcodesCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsSupportedOpcodesCCC), 0},	
	//          Search Control Point
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSearchCtrPointChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsSearchCtrPointChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_GmcsSearchCtrPointValue), (u8*)(&my_GmcsSearchCtrPointUUID), (u8*)(&my_GmcsSearchCtrPointValue), app_att_audioMcsWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsSearchCtrPointCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsSearchCtrPointCCC), 0},	
	//          Search Results Object ID
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSearchResultObjIDChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsSearchResultObjIDChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsSearchResultObjIDValue),(u8*)(&my_GmcsSearchResultObjIDUUID), (u8*)(&my_GmcsSearchResultObjIDValue), 0, app_att_audioMcsRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GmcsSearchResultObjIDCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GmcsSearchResultObjIDCCC), 0},	
	//          Content Control ID (CCID)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsContentControlChar),(u8*)(&my_characterUUID), (u8*)(my_GmcsContentControlChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GmcsCCIDValue),(u8*)(&my_GmcsContentControlUUID), (u8*)(&my_GmcsCCIDValue), 0, 0},
	#endif //#if (BLC_AUDIO_MCS_ENABLE)
#endif //#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
	////////////////////////////////////// GTBS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_TBS_ENABLE)
	{GTBS_CALL_FRIENDLY_NAME_CCC_H-GTBS_PS_H+1,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_GtbsServiceUUID), 0},
	//	  Bearer Provider Name	
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsProviderNameChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsProviderNameChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsProviderNameValue),(u8*)(&my_GtbsProviderNameUUID), (u8*)(&my_GtbsProviderNameValue), 0, app_att_audioTbsRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsProviderNameCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsProviderNameCCC), 0},	
	//	  Bearer Uniform Caller Identifier (UCI)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsUCIChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsUCIChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsUCIValue),(u8*)(&my_GtbsUCIUUID), (u8*)(&my_GtbsUCIValue), 0, 0},
	//	  Bearer Technology
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsTechChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsTechChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsTechValue),(u8*)(&my_GtbsTechUUID), (u8*)(&my_GtbsTechValue), 0, app_att_audioTbsRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsTechCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsTechCCC), 0},	
	//	  Bearer URI Schemes Supported List 
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsURIListChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsURIListChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsURIListValue),(u8*)(&my_GtbsURIListUUID), (u8*)(&my_GtbsURIListValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsURIListCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsURIListCCC), 0},	
	//    Bearer Signal Strength
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsSignalStrengthChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsSignalStrengthChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsSignalStrengthValue),(u8*)(&my_GtbsSignalStrengthUUID), (u8*)(&my_GtbsSignalStrengthValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsSignalStrengthCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsSignalStrengthCCC), 0},	
	//    Bearer Signal Strength Reporting Interval
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsSignalStrengthReportChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsSignalStrengthReportChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsSignalStrengthReportValue),(u8*)(&my_GtbsSignalStrengthReportUUID), (u8*)(&my_GtbsSignalStrengthReportValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsSignalStrengthReportCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsSignalStrengthReportCCC), 0},	
	//	  Bearer List Current Calls
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCurCallListChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsCurCallListChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCurCallListValue),(u8*)(&my_GtbsCurCallListUUID), (u8*)(&my_GtbsCurCallListValue), 0, app_att_audioTbsRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsCurCallListCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsCurCallListCCC), 0},	
	//	  Content Control ID (CCID)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsContentControlChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsContentControlChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCCIDValue),(u8*)(&my_GtbsContentControlUUID), (u8*)(&my_GtbsCCIDValue), 0, 0},
	//	  Status Flags
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStatusChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsStatusChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStatusValue),(u8*)(&my_GtbsStatusUUID), (u8*)(&my_GtbsStatusValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsStatusCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsStatusCCC), 0},	
	//    Incoming Call Target Bearer URI
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsIncomingCallURIChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsIncomingCallURIChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsIncomingCallURIValue),(u8*)(&my_GtbsIncomingCallURIUUID), (u8*)(&my_GtbsIncomingCallURIValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsIncomingCallURICCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsIncomingCallURICCC), 0},	
	//	  Call State
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStateChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsStateChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsStateValue),(u8*)(&my_GtbsStateUUID), (u8*)(&my_GtbsStateValue), 0, app_att_audioTbsRead},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsStateCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsStateCCC), 0},	
	//	  Call Control Point
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCallCtrlChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsCallCtrlChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_GtbsCallCtrlValue),(u8*)(&my_GtbsCallCtrlUUID), (u8*)(&my_GtbsCallCtrlValue), app_att_audioTbsWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsCallCtrlCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsCallCtrlCCC), 0},	
	//	  Call Control Point Optional Opcodes
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCtrlOpcodesChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsCtrlOpcodesChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCtrlOpcodesValue),(u8*)(&my_GtbsCtrlOpcodesUUID), (u8*)(&my_GtbsCtrlOpcodesValue), 0, 0},
	//	  Termination Reason
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsTermReasonChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsTermReasonChar), 0},
	{0,                  0x00,2,sizeof(my_GtbsTermReasonValue),(u8*)(&my_GtbsTermReasonUUID), (u8*)(&my_GtbsTermReasonValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsTermReasonCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsTermReasonCCC), 0},	
	//	  Incoming Call
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsIncomingCallChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsIncomingCallChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsIncomingCallValue),(u8*)(&my_GtbsIncomingCallUUID), (u8*)(&my_GtbsIncomingCallValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsIncomingCallCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsIncomingCallCCC), 0},	
	//    Call Friendly Name
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCallFriendlyNameChar),(u8*)(&my_characterUUID), (u8*)(my_GtbsCallFriendlyNameChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_GtbsCallFriendlyNameValue),(u8*)(&my_GtbsCallFriendlyNameUUID), (u8*)(&my_GtbsCallFriendlyNameValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_GtbsCallFriendlyNameCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_GtbsCallFriendlyNameCCC), 0},	
	#endif //#if (BLC_AUDIO_TBS_ENABLE)
#endif //#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)
	////////////////////////////////////// OTS /////////////////////////////////////////////////////
	#if (BLC_AUDIO_OTS_ENABLE)
//	OTS_PS_H, //
	{OTS_OBJECT_CHANGED_CCC_H-OTS_PS_H+1,ATT_PERMISSIONS_RDWR,2,2,(u8*)(&my_primaryServiceUUID), (u8*)(&my_OtsServiceUUID), 0},
	//    OTS Feature    Mandatory:Read; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsFeatureChar),(u8*)(&my_characterUUID), (u8*)(my_OtsFeatureChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsFeatureValue),(u8*)(&my_OtsFeatureUUID), (u8*)(&my_OtsFeatureValue), 0, 0},
	//    Object Name    Mandatory:Read; Optional:Write
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectNameChar),(u8*)(&my_characterUUID), (u8*)(my_OtsObjectNameChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsObjectNameValue),(u8*)(&my_OtsObjectNameUUID), (u8*)(&my_OtsObjectNameValue), app_att_audioOtsWrite, app_att_audioOtsRead},
	//    Object Type    Mandatory:Read; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectTypeChar),(u8*)(&my_characterUUID), (u8*)(my_OtsObjectTypeChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectTypeValue),(u8*)(&my_OtsObjectTypeUUID), (u8*)(&my_OtsObjectTypeValue), 0, 0},
	//    Object Size    Mandatory:Read; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectSizeChar),(u8*)(&my_characterUUID), (u8*)(my_OtsObjectSizeChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectSizeValue),(u8*)(&my_OtsObjectSizeUUID), (u8*)(&my_OtsObjectSizeValue), 0, 0},
	//    Object First-Created    Mandatory:Read; Optional:Write
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsFirstCreatedChar),(u8*)(&my_characterUUID), (u8*)(my_OtsFirstCreatedChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsFirstCreatedValue),(u8*)(&my_OtsFirstCreatedUUID), (u8*)(&my_OtsFirstCreatedValue), app_att_audioOtsWrite, app_att_audioOtsRead},
	//    Object Last-Modified    Mandatory:Read; Optional:Write
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsLastCreatedChar),(u8*)(&my_characterUUID), (u8*)(my_OtsLastCreatedChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsLastCreatedValue),(u8*)(&my_OtsLastCreatedUUID), (u8*)(&my_OtsLastCreatedValue), app_att_audioOtsWrite, app_att_audioOtsRead},
	//    Object ID    Mandatory:Read; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectIDChar),(u8*)(&my_characterUUID), (u8*)(my_OtsObjectIDChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectIDValue),(u8*)(&my_OtsObjectIDUUID), (u8*)(&my_OtsObjectIDValue), 0, 0},
	//    Object Properties    Mandatory:Read; Optional:Write
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectPropertiesChar),(u8*)(&my_characterUUID), (u8*)(my_OtsObjectPropertiesChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsObjectPropertiesValue),(u8*)(&my_OtsObjectPropertiesUUID), (u8*)(&my_OtsObjectPropertiesValue), app_att_audioOtsWrite, app_att_audioOtsRead},
	//    Object Action Control Point    Mandatory:Write,Indicate; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsActionControlPointChar),(u8*)(&my_characterUUID), (u8*)(my_OtsActionControlPointChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_OtsActionControlPointValue),(u8*)(&my_OtsActionControlPointUUID), (u8*)(&my_OtsActionControlPointValue), app_att_audioOtsWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsActionControlPointCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_OtsActionControlPointCCC), 0},
	//    Object List Control Point    Mandatory:Write,Indicate; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsListControlPointChar),(u8*)(&my_characterUUID), (u8*)(my_OtsListControlPointChar), 0},
	{0,ATT_PERMISSIONS_WRITE,2,sizeof(my_OtsListControlPointValue),(u8*)(&my_OtsListControlPointUUID), (u8*)(&my_OtsListControlPointValue), app_att_audioOtsWrite, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsListControlPointCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_OtsListControlPointCCC), 0},
	//    Object List Filter    Mandatory:Read,Write; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectListFilterChar),(u8*)(&my_characterUUID), (u8*)(my_OtsObjectListFilterChar), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsObjectListFilterValue),(u8*)(&my_OtsObjectListFilterUUID), (u8*)(&my_OtsObjectListFilterValue), app_att_audioOtsWrite, app_att_audioOtsRead},
	//    Object Changed    Mandatory:Indicate; Optional:
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_OtsObjectChangedChar),(u8*)(&my_characterUUID), (u8*)(my_OtsObjectChangedChar), 0},
	{0,                0x00,2,sizeof(my_OtsObjectChangedValue),(u8*)(&my_OtsObjectChangedUUID), (u8*)(&my_OtsObjectChangedValue), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtsObjectChangedCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&my_OtsObjectChangedCCC), 0},
	#endif
#endif //#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)
};


void my_gatt_init (void)
{
	bls_att_setAttributeTable((u8 *)my_Attributes);
}

void app_audio_gatt_init(void)
{
	int ret;

	//bls_att_setAttributeTable((u8 *)my_Attributes);

	ret = 0;
	#if (BLC_AUDIO_PACS_ENABLE)
		ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_PACS, (attribute_t*)&my_Attributes[PACS_PS_H]);
	#endif //(BLC_AUDIO_PACS_ENABLE)
	#if (BLC_AUDIO_ASCS_ENABLE)
		ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_ASCS, (attribute_t*)&my_Attributes[ASCS_PS_H]);
	#endif //(BLC_AUDIO_ASCS_ENABLE)
	#if (BLC_AUDIO_CSIS_ENABLE)
		ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_CSIS, (attribute_t*)&my_Attributes[CSIS_PS_H]);
	#endif //(BLC_AUDIO_CSIS_ENABLE)
	#if (BLC_AUDIO_MICS_ENABLE)
		ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_MICS, (attribute_t*)&my_Attributes[MICS_PS_H]); //The MICS must be set before the AICSs
		#if (BLC_AUDIO_MICS_AICS_ENABLE)
			ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_MICS, (attribute_t*)&my_Attributes[MICS_AICS_PS_H]);
		#endif
	#endif //(BLC_AUDIO_MICS_ENABLE)
	#if (BLC_AUDIO_VCS_ENABLE)
		ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_VCS, (attribute_t*)&my_Attributes[VCS_PS_H]);
	#if (BLC_AUDIO_VCS_AICS_ENABLE)
		ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_VCS, (attribute_t*)&my_Attributes[VCS_AICS_PS_H]);
	#endif
	#if (BLC_AUDIO_VOCS_ENABLE)
		ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_VCS, (attribute_t*)&my_Attributes[VOCS0_PS_H]);
	#endif
	#endif //(BLC_AUDIO_VCS_ENABLE)

	#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
		#if (BLC_AUDIO_RAAS_ENABLE)
			ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_RAAS, (attribute_t*)&my_Attributes[RAAS_PS_H]);
		#endif //(BLC_AUDIO_RAAS_ENABLE)
	#endif //#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
	#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
		#if (BLC_AUDIO_MCS_ENABLE)
			ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_MCS, (attribute_t*)&my_Attributes[GMCS_PS_H]);
		#endif //(BLC_AUDIO_MCS_ENABLE)
	#endif //#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
	#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
		#if (BLC_AUDIO_TBS_ENABLE)
			ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_TBS, (attribute_t*)&my_Attributes[GTBS_PS_H]);
		#endif //(BLC_AUDIO_TBS_ENABLE)
	#endif //#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
	#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)
		#if (BLC_AUDIO_OTS_ENABLE)
			ret += blc_audio_setServiceByRole(BLC_AUDIO_ROLE_SERVER, BLC_AUDIO_SERVICE_OTS, (attribute_t*)&my_Attributes[OTS_PS_H]);
		#endif //(BLC_AUDIO_TBS_ENABLE)
	#endif //#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)
	if(ret != 0){
		
	}
	//printf("app_audios_gatt_init: %d\r\n", ret);
}



#if (BLC_AUDIO_PACS_ENABLE)
int app_att_audio_pacpRead(u16 connHandle, void *p)
{
	if(blc_audio_pacpAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
		return true;
	}else{
		return false;
	}
}
int app_att_audio_pacpWrite(u16 connHandle, void *p)
{
	return blc_audio_pacpAttWrite(connHandle, p);
}
#endif //BLC_AUDIO_PACS_ENABLE

#if (BLC_AUDIO_ASCS_ENABLE)
int app_att_audioAscpRead(u16 connHandle, void *p)
{
	if(blc_audio_ascpAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
		return true;
	}else{
		return false;
	}
}
int app_att_audioAscpWrite(u16 connHandle, void *p)
{
	return blc_audio_ascpAttWrite(connHandle, p);
}
#endif //BLC_AUDIO_PACS_ENABLE


#if (BLC_AUDIO_CSIS_ENABLE)
int app_att_audioCsipRead(u16 connHandle, void *p)
{
	if(blc_audio_csipAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
		return true;
	}else{
		return false;
	}
}
int app_att_audioCsipWrite(u16 connHandle, void *p)
{
	return blc_audio_csipAttWrite(connHandle, p);
}
#endif //BLC_AUDIO_CSIS_ENABLE

#if (BLC_AUDIO_MICS_ENABLE)
int app_att_audioMicpRead(u16 connHandle, void *p)
{
	if(blc_audio_micpAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
		return true;
	}else{
		return false;
	}
}
int app_att_audioMicpWrite(u16 connHandle, void *p)
{
	return blc_audio_micpAttWrite(connHandle, p);
}
#endif //BLC_AUDIO_MICS_ENABLE

#if (BLC_AUDIO_VCS_ENABLE)
int app_att_audioVcpRead(u16 connHandle, void *p)
{
	if(blc_audio_vcpAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
		return true;
	}else{
		return false;
	}
}
int app_att_audioVcpWrite(u16 connHandle, void *p)
{
	return blc_audio_vcpAttWrite(connHandle, p);
}
#endif //AUDIO_VCP_ENBALE

#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
	#if (BLC_AUDIO_RAAS_ENABLE)
		int app_att_audioRaasRead(u16 connHandle, void *p)
		{
			if(blc_audio_raapAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
				return true;
			}else{
				return false;
			}
		}
		int app_att_audioRaasWrite(u16 connHandle, void *p)
		{
			return blc_audio_raapAttWrite(connHandle, p);
		}
	#endif //#if (BLC_AUDIO_RAAS_ENABLE)
#endif //#if (BLC_AUDIO_RAAP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)
	#if (BLC_AUDIO_MCS_ENABLE)
		int app_att_audioMcsRead(u16 connHandle, void *p)
		{
			if(blc_audio_mcpAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
				return true;
			}else{
				return false;
			}
		}
		int app_att_audioMcsWrite(u16 connHandle, void *p)
		{
			return blc_audio_mcpAttWrite(connHandle, p);
		}
	#endif
#endif //#if (BLC_AUDIO_MCP_ROLE_SWICH_ENABLE)	
#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
	#if (BLC_AUDIO_TBS_ENABLE)
		int app_att_audioTbsRead(u16 connHandle, void *p)
		{
			if(blc_audio_tbpAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
				return true;
			}else{
				return false;
			}
		}
		int app_att_audioTbsWrite(u16 connHandle, void *p)
		{
			return blc_audio_tbpAttWrite(connHandle, p);
		}
	#endif
#endif //#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)
#if (BLC_AUDIO_OTP_ROLE_SWICH_ENABLE)
	#if (BLC_AUDIO_OTS_ENABLE)
		int app_att_audioOtsRead(u16 connHandle, void *p)
		{
			if(blc_audio_otpAttRead(connHandle, p) == BLC_AUDIO_SUCCESS){
				return true;
			}else{
				return false;
			}
		}
		int app_att_audioOtsWrite(u16 connHandle, void *p)
		{
			return blc_audio_otpAttWrite(connHandle, p);
		}
	#endif
#endif //#if (BLC_AUDIO_TBP_ROLE_SWICH_ENABLE)

#endif
