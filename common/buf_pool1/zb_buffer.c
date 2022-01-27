/********************************************************************************************************
 * @file	zb_buffer.c
 *
 * @brief	This is the source file for BLE SDK
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
/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
PURPOSE: Zigbee packet buffers pool
*/
#include "common/compiler.h"
#include "zb_buffer.h"

/*! \addtogroup buf */
/*! @{ */


/*
 * the buffer for zigbee stack  initilization
 *
 * */
void tl_zbBufferInit(void){
	memset((u8 *)&g_mPool, 0, sizeof(zb_buf_pool_t));

	zb_buf_t *p = g_mPool.head = &g_mPool.pool[0];

	for(s32 i = 0; i < ZB_BUF_POOL_SIZE - 1; i++){
		p->next = (p + 1);
		p++;
	}
	p->next = NULL;
}


/*
 * allocate a buffer directly
 * return success, if the pool has a empty buffer,
 * if not, return failure
 *
 * */
#if ZB_BUFFER_DEBUG
static _attribute_ram_code_ zb_buf_t *my_zb_buf_get(u16 line)
#else
static _attribute_ram_code_ zb_buf_t *zb_buf_get(void)
#endif
{
	zb_buf_t *buf = NULL;

	u8 r = irq_disable();
	if(g_mPool.usedNum < ZB_BUF_POOL_SIZE)
	{
		if((((u32)(g_mPool.head) < (u32)(&g_mPool.pool[0])) && (g_mPool.head != NULL)) ||
			((u32)(g_mPool.head) > (u32)(&g_mPool.pool[ZB_BUF_POOL_SIZE - 1]))) {
			//ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_BUFFER_EXCEPTION);
		}

		buf = g_mPool.head; 
		if(buf){
			g_mPool.head = buf->next;
			buf->next = NULL;
			memset(&buf->hdr, 0, sizeof(buf->hdr));
			memset(buf->buf, 0, ZB_BUF_SIZE);
			g_mPool.usedNum++;
			buf->hdr.used = 1;
#if ZB_BUFFER_DEBUG
			u32 idx = buf->allocCnt % ZB_BUFF_DBG_NUM;
			buf->allocInfo[idx].allocLine = line;
#endif
			buf->allocCnt++;
		}else{
			if(g_mPool.usedNum < ZB_BUF_POOL_SIZE){
				//ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_BUFFER_EXCEPTION);
			}
		}
	}
	irq_restore(r);

	if(!buf){
		/* diagnostics packet buffer allocate failures */
		//g_sysDiags.packetBufferAllocateFailures++;
	}

	return buf;
}


inline u8 *tl_phyRxBufTozbBuf(u8 *p){
	return (p - RX_ZBBUF_OFFSET);
}

inline u8 *tl_zbBufToPhyRxBuf(u8 *p){
	return (p + RX_ZBBUF_OFFSET);
}

/*
 * allocate a buffer directly for ZB stack
 * return success, if the pool has a empty buffer,
 * if not, return failure
 *
 * */
#if ZB_BUFFER_DEBUG
zb_buf_t *my_zb_buf_allocate(u16 line){
	return my_zb_buf_get(line);
}
_attribute_ram_code_ u8 *tl_getRxBuf(){
#if 0
	if (g_mPool.usedNum > 8){
		return NULL;

	}
#endif
	u8 *buf = (u8 *)my_zb_buf_get(90);
	if(buf){
		return TL_INBUF_TO_RXBUF(buf);
	}
	return NULL;

}
#else
_attribute_ram_code_ zb_buf_t *zb_buf_allocate(){
	return zb_buf_get();
}

_attribute_ram_code_ u8 *tl_getRxBuf(){
	u8 *buf = (u8 *)zb_buf_get();
	if(buf){
		return TL_INBUF_TO_RXBUF(buf);
	}
	return NULL;
}

#endif

/*
 * free the buffer
 * */
volatile u32 T_zbBufDbg = 0;
volatile u32 T_zbBufFreeDbg = 0;
#if ZB_BUFFER_DEBUG
volatile u32 T_zbBufFreeDbgLine = 0;
volatile u32 T_zbBufFreeDbgIdx = 0;
volatile u32 T_zbBufFreeDbgIdx1 = 0;
u8 my_zb_buf_free(zb_buf_t *buf, u16 line)
#else
u8 zb_buf_free(zb_buf_t *buf)
#endif
{
	u8 r = irq_disable();

	if(!is_zb_buf((void *)buf)){
		//ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_BUFFER_EXCEPTION_FREE_OVERFLON);
	}

#if ZB_BUFFER_DEBUG
	u32 idx = buf->freeCnt % ZB_BUFF_DBG_NUM;
	T_zbBufFreeDbgLine = buf->allocInfo[idx].freeLine;
	T_zbBufFreeDbgIdx1 = buf->allocCnt;
	T_zbBufFreeDbgIdx = idx;
	buf->allocInfo[idx].freeLine = line;
	buf->allocInfo[idx].handler = buf->hdr.handle;
	buf->allocInfo[idx].id = buf->hdr.id;
	buf->allocInfo[idx].nlmeStatus = buf->hdr.resvHdr;
	T_zbBufDbg = (u32)buf;
#endif

	buf->freeCnt++;
	if(buf->hdr.macTxFifo == 1 || buf->freeCnt > buf->allocCnt || buf->hdr.used == 0){
		T_zbBufFreeDbg = (buf->hdr.macTxFifo << 24) | ((buf->freeCnt > buf->allocCnt) << 16) | buf->hdr.used ;
		T_zbBufDbg = (u32)buf;
		//ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_BUFFER_EXCEPTION_FREE_MULIT);
	}

	if((((u32)(g_mPool.head) < (u32)(&g_mPool.pool[0])) && (g_mPool.head != NULL)) ||
		((u32)(g_mPool.head) > (u32)(&g_mPool.pool[ZB_BUF_POOL_SIZE - 1]))) {
		//ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_BUFFER_EXCEPTION);
	}

	g_mPool.usedNum--;
	buf->next = g_mPool.head;
	g_mPool.head = buf;

	if((((u32)(g_mPool.head) < (u32)(&g_mPool.pool[0])) && (g_mPool.head != NULL)) ||
	    ((u32)(g_mPool.head) > (u32)(&g_mPool.pool[ZB_BUF_POOL_SIZE - 1]))) {
		//ZB_EXCEPTION_POST(SYS_EXCEPTTION_ZB_BUFFER_EXCEPTION);
	}
	buf->hdr.used = 0;
	buf->hdr.handle = 0xff;

	irq_restore(r);
	return SUCCESS;
}


void *tl_bufInitalloc(zb_buf_t *p, u8 size){
#ifdef	ZB_SECURITY
  size += 8;//Extra 4 bytes for APS MIC and 4 bytes for NWK MIC
#endif
  return (void *)((u8 *)p + ((ZB_BUF_SIZE - size) & (~0x03)));
}

/* clear the buffer, some info set as 0 */
void zb_buf_clear(zb_buf_t *p)
{
  u8 used = p->hdr.used;

  u8 r = irq_disable();
  memset(&p->hdr, 0, sizeof(p->hdr));
  p->hdr.used = used;
  irq_restore(r);
}

bool is_zb_buf(void *p){
	if(((u32)p >= (u32)(&g_mPool.pool[0])) && ((u32)p < (u32)((&g_mPool.pool[ZB_BUF_POOL_SIZE - 1]))+ZB_BUF_SIZE)){
		return 1;
	}
	return 0;
}

#if ZB_BUFFER_DEBUG
volatile u8 T_zbbud_debug_start = 0;
typedef struct{
	u8   id;
	u8   handle;
	u16  allocateLine;
}zb_bufDbg_t;
zb_bufDbg_t g_zbBufDBG[ZB_BUF_POOL_NUM];

void zb_buf_debug_start(void){
	if(T_zbbud_debug_start){
		u8 cnt = 0;
		T_zbbud_debug_start = 0;
		for(s32 i = 0; i < ZB_BUF_POOL_SIZE - 1;i++){
			if(g_mPool.pool[i].hdr.used){
				g_zbBufDBG[cnt].handle = g_mPool.pool[i].hdr.handle;
				g_zbBufDBG[cnt].id = g_mPool.pool[i].hdr.id;
				//g_zbBufDBG[cnt].allocateLine = g_mPool.pool[i].allocLine;
				cnt++;
			}
		}
	}
}
#endif


/*! @} */
