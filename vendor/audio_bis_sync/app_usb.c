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
#include "app_config.h"
#include "app_usb.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"


#if (__PROJECT_B91_AUDIO_BIS_SYNC__)
#if (APP_AUDIO_USB_ENABLE)



extern void usb_handle_irq(void);


#define AUDIO_USB_BUFFER_LENGTH      APP_AUDIO_FRAME_BYTES//(AUDIO_ADPCM_BUFFER_SIZE<<2)

#define AUDIO_SAMPLE_RATE            MIC_SAMPLE_RATE
#define	AUDIO_USB_UNIT_LENGTH        ((AUDIO_SAMPLE_RATE/1000) << 1)
#define	AUDIO_USB_UNIT_COUNT         (AUDIO_USB_BUFFER_LENGTH/AUDIO_USB_UNIT_LENGTH)

u8 sAppAudioDataSerial;
u8 *sAppAudioDataPtr = 0;
u8 gAudioUsbBuffer0[AUDIO_USB_BUFFER_LENGTH];
u8 gAudioUsbBuffer1[AUDIO_USB_BUFFER_LENGTH];
volatile u8 gAudioUsbCount = 0;
u8 gAudioUsbRptr = 0;
u8 gAudioUsbWptr = 0;


volatile u32 AAAA_usb_test001 = 0;
volatile u32 AAAA_usb_test002 = 0;
volatile u32 AAAA_usb_test003 = 0;

void app_usb_init(void)
{
	//set USB ID
	REG_ADDR8(0x1401f4) = 0x65;
	REG_ADDR16(0x1401fe) = 0x08d0;
	REG_ADDR8(0x1401f4) = 0x00;
	
	//////////////// config USB ISO IN/OUT interrupt /////////////////
	reg_usb_ep_irq_mask = BIT(7);			//audio in interrupt enable
	plic_interrupt_enable(IRQ11_USB_ENDPOINT);
	plic_set_priority(IRQ11_USB_ENDPOINT, 1);
	reg_usb_ep6_buf_addr = 0x80;
	reg_usb_ep7_buf_addr = 0x60;
	reg_usb_ep_max_size = (256 >> 3);
	
	app_usb_enable(true);
	
	gAudioUsbCount = 0;
	gAudioUsbRptr = 0;
	gAudioUsbRptr = 0;
}

void app_usb_enable(bool enable)
{
	if(enable) usb_dp_pullup_en(1);
	else usb_dp_pullup_en(0);
}

void app_usb_handler(void)
{
	usb_handle_irq();
}


bool app_usb_fill_buffer(u8 *pData)
{
	if(gAudioUsbCount == 2) return false;
	
	if(gAudioUsbWptr == 0){
		memcpy(gAudioUsbBuffer0, pData, AUDIO_USB_BUFFER_LENGTH);
		gAudioUsbWptr = 1;
	}else{
		memcpy(gAudioUsbBuffer1, pData, AUDIO_USB_BUFFER_LENGTH);
		gAudioUsbWptr = 0;
	}
	
	gAudioUsbCount ++;
	
	return true;
}
u8 *app_usb_get_buffer(void)
{
	if(gAudioUsbCount == 0) return NULL;

	if(gAudioUsbRptr == 0){
		gAudioUsbRptr = 1;
		return gAudioUsbBuffer0;
	}else{
		gAudioUsbRptr = 0;
		return gAudioUsbBuffer1;
	}

}

_attribute_ram_code_ void app_usb_send_handler(void)
{
	reg_usb_ep7_ptr = 0;
	
	if(sAppAudioDataSerial == 0){
		sAppAudioDataPtr = app_usb_get_buffer();
	}
	if(sAppAudioDataPtr == 0){
#if 0
		u8 i;
		for(i=0; i<AUDIO_USB_UNIT_LENGTH; i++){
			reg_usb_ep7_dat = 0x00;
		}
#endif
		return;
	}
	#if 1
	{
		u8 i;
		u8 *pData;
		u8 unitCount;

			
		unitCount = AUDIO_USB_UNIT_COUNT;
		pData = sAppAudioDataPtr+(sAppAudioDataSerial*AUDIO_USB_UNIT_LENGTH);
		for(i=0; i<AUDIO_USB_UNIT_LENGTH; i++){
			reg_usb_ep7_dat = pData[i];
		}
		sAppAudioDataSerial ++;
		if(sAppAudioDataSerial == unitCount){
			sAppAudioDataSerial = 0;
			gAudioUsbCount --;
		}
	}
	#endif
}


_attribute_ram_code_ void  app_usb_irq_handler(void)
{
	if(reg_usb_irq & BIT(7)){
		reg_usb_irq = BIT(7);
		app_usb_send_handler();
		reg_usb_ep7_ctrl = BIT(0);
	}
}


#endif
#endif //#if (__PROJECT_B91_AUDIO_BIS_SYNC__)
