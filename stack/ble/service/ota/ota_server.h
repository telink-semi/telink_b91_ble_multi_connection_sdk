/********************************************************************************************************
 * @file	ota_server.h
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

#ifndef STACK_BLE_SERVICE_OTA_OTA_SERVER_H_
#define STACK_BLE_SERVICE_OTA_OTA_SERVER_H_




/**
 * @brief	OTA start command callback declaration
 */
typedef void (*ota_startCb_t)(void);

/**
 * @brief	OTA version callback declaration
 */
typedef void (*ota_versionCb_t)(void);

/**
 * @brief		OTA result indicate callback declaration
 * @param[in]   result - OTA result
 */
typedef void (*ota_resIndicateCb_t)(int result);





/**
 * @brief      this function is used for user to initialize OTA server module.
 * 			   //attention: this API must called before any other OTA relative settings.
 * @param	   none
 * @return     none
 */
void blc_ota_initOtaServer_module(void);





/**
 * @brief      This function is used to set OTA new firmware storage address on Flash.
 * 			   note: this function must be called before "sys_init" or "cpu_wakeup_init".
 * @param[in]  firmware_size_k - set the firmware size. i.e. OTA erase flash size.note: unit is 1K(1024B)
 * @param[in]  new_fw_addr - new firmware storage address, 1.choose from multiple boot address
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ota_setFirmwareSizeAndBootAddress(int firmware_size_k, multi_boot_addr_e new_fw_addr);




/**
 * @brief      This function is used to set OTA firmware version number.
 * 			   if user use version compare to decide if OTA update, should use this API to set firmware version.
 * @param[in]  version_number - firmware version number
 * @return     none
 */
void blc_ota_setFirmwareVersionNumber(u16 version_number);










/**
 * @brief      This function is used to register OTA start command callback.
 * 			   when local device receive OTA command  "CMD_OTA_START" and  "CMD_OTA_START_EXT"  form peer device,
 * 			   after checking all parameters are correct, local device will enter OTA update and trigger OTA start command callback.
 * @param[in]  cb - callback function
 * @return     none
 */
void blc_ota_registerOtaStartCmdCb(ota_startCb_t cb);






/**
 * @brief      This function is used to register OTA version command callback
 * 			   when local device receive OTA command  "CMD_OTA_VERSION", trigger this callback.
 * @param[in]  cb - callback function
 * @return     none
 */
void blc_ota_registerOtaFirmwareVersionReqCb(ota_versionCb_t cb);





/**
 * @brief      This function is used to register OTA result indication callback
 * @param[in]  cb - callback function
 * @return     none
 */
void blc_ota_registerOtaResultIndicationCb(ota_resIndicateCb_t cb);




/**
 * @brief      This function is used to set OTA whole process timeout value
 * 			   if not set, default value is 30 S
 * @param[in]  timeout_second - timeout value, unit: S, should in range of 4 ~ 250
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ota_setOtaProcessTimeout(int timeout_second);



/**
 * @brief      This function is used to set OTA packet interval timeout value
 * 			   if not set, default value is 5 S
 * @param[in]  timeout_ms - timeout value, unit: mS, should in range of 1 ~ 20
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ota_setOtaDataPacketTimeout(int timeout_second);



/**
 * @brief      This function is used to write OTA data to flash
 * @param[in]  connHandle - ACL connection handle
 * @return     p - GATT data buffer pointer of write_req or write_cmd
 */
int	otaWrite(u16 connHandle, void * p);







#endif /* STACK_BLE_SERVICE_OTA_OTA_SERVER_H_ */
