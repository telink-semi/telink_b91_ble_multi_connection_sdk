/********************************************************************************************************
 * @file	myHeap.h
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
/*************************************************************************************************/
/*!
 *  \file   my_heap.h
 *
 *  \brief  Buffer heap service.
 *
 *  Copyright (c) 2018 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019-2020 Packetcraft, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*************************************************************************************************/

#pragma  once




#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup MY_HEAP_API
 *  \{ */


#include "common/types.h"
#include "common/compiler.h"


/*************************************************************************************************/
/*!
 *  \brief      Get heap available.
 *
 *  \return     Number of bytes of heap memory available.
 */
/*************************************************************************************************/
u32 myHeapCountAvailable(void);

/*************************************************************************************************/
/*!
 *  \brief      Get heap used.
 *
 *  \return     Number of bytes of heap memory used.
 */
/*************************************************************************************************/
u32 myHeapCountUsed(void);

/*************************************************************************************************/
/*!
 *  \brief      Reserve heap memory.
 *
 *  \param      size    Number of bytes of heap memory used.
 */
/*************************************************************************************************/
void myHeapAlloc(u32 size);

/*************************************************************************************************/
/*!
 *  \brief      Get next available heap memory.
 *
 *  \return     Address of the start of heap memory.
 */
/*************************************************************************************************/
void *myHeapGetFreeStartAddress(void);

/*! \} */    /* MY_HEAP_API */

#ifdef __cplusplus
};
#endif


