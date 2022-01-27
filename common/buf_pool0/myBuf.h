/********************************************************************************************************
 * @file	myBuf.h
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
 *  \file   my_buf.h
 *
 *  \brief  Buffer pool service.
 *
 *  Copyright (c) 2009-2018 Arm Ltd. All Rights Reserved.
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
#ifndef MY_BUF_H
#define MY_BUF_H

#ifdef __cplusplus
extern "C" {
#endif


#include "common/types.h"
#include "common/compiler.h"


/*! \addtogroup MY_BUF_API
 *  \{ */

/**************************************************************************************************
  Configuration
**************************************************************************************************/

/*! \brief Check if trying to free a buffer that is already free */
#ifndef MY_BUF_FREE_CHECK_ASSERT
#define MY_BUF_FREE_CHECK_ASSERT 				TRUE
#endif

/*! \brief Assert on best-fit buffer allocation failure */
#ifndef MY_BUF_ALLOC_BEST_FIT_FAIL_ASSERT
#define MY_BUF_ALLOC_BEST_FIT_FAIL_ASSERT 		TRUE//FALSE
#endif

/*! \brief Assert on buffer allocation failure */
#ifndef MY_BUF_ALLOC_FAIL_ASSERT
#define MY_BUF_ALLOC_FAIL_ASSERT 				TRUE
#endif

/*! \brief Buffer histogram stats */
#ifndef MY_BUF_STATS_HIST
#define MY_BUF_STATS_HIST 						FALSE//TRUE//
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief Length of the buffer statistics array */
#define MY_BUF_STATS_MAX_LEN       				128

/*! \brief Max number of pools can allocate */
#define MY_BUF_STATS_MAX_POOL      				32

/*! \brief Failure Codes */
#define MY_BUF_ALLOC_FAILED        				1

#ifndef MY_BUF_STATS
/*! \brief Enable buffer allocation statistics. */
#define MY_BUF_STATS 							FALSE//TRUE//
#endif

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief Buffer pool descriptor structure */
typedef struct
{
  u16   len;                  /*!< \brief Length of buffers in pool */
  u8    num;                  /*!< \brief Number of buffers in pool */
} myBufPoolDesc_t;

/*! \brief Pool statistics */
typedef struct
{
  u16   bufSize;              /*!< \brief Pool buffer size. */
  u8    numBuf;               /*!< \brief Total number of buffers. */
  u8    numAlloc;             /*!< \brief Number of outstanding allocations. */
  u8    maxAlloc;             /*!< \brief High allocation watermark. */
  u16   maxReqLen;            /*!< \brief Maximum requested buffer length. */
} myBufPoolStat_t;

/*! \brief MY buffer diagnostics - buffer allocation failure */
typedef struct
{
  u8    taskId;               /*!< \brief Task handler ID where failure occured */
  u16   len;                  /*!< \brief Length of buffer being allocated */
} myBufDiagAllocFail_t;

/*! \brief MY buffer diagnostics message */
typedef struct
{
  union
  {
    myBufDiagAllocFail_t alloc;   /*!< \brief Buffer allocation failure */
  } param;                         /*!< \brief Union of diagnostic data types. */

  u8 type;                    /*!< \brief Type of error */
} myBufDiag_t;

/**************************************************************************************************
  Callback Function Datatypes
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Callback providing MY buffer diagnostic messages.
 *
 *  \param  pInfo     Diagnostics message.
 */
/*************************************************************************************************/
typedef void (*myBufDiagCback_t)(myBufDiag_t *pInfo);

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Calculate size required by the buffer pool.
 *
 *  \param  numPools  Number of buffer pools.
 *  \param  pDesc     Array of buffer pool descriptors, one for each pool.
 *
 *  \return Amount of pBufMem used.
 */
/*************************************************************************************************/
u32 myBufCalcSize(u8 numPools, myBufPoolDesc_t *pDesc);

/*************************************************************************************************/
/*!
 *  \brief  Initialize the buffer pool service.  This function should only be called once
 *          upon system initialization.
 *
 *  \param  numPools  Number of buffer pools.
 *  \param  pDesc     Array of buffer pool descriptors, one for each pool.
 *
 *  \return Amount of pBufMem used or 0 for failures.
 */
/*************************************************************************************************/
u32 myBufInit(u8 numPools, myBufPoolDesc_t *pDesc);

/*************************************************************************************************/
/*!
 *  \brief  Allocate a buffer.
 *
 *  \param  len     Length of buffer to allocate.
 *
 *  \return Pointer to allocated buffer or NULL if allocation fails.
 */
/*************************************************************************************************/
void *myBufAlloc(u16 len);

/*************************************************************************************************/
/*!
 *  \brief  Free a buffer.
 *
 *  \param  pBuf    Buffer to free.
 */
/*************************************************************************************************/
void myBufFree(void *pBuf);

/*************************************************************************************************/
/*!
 *  \brief  Diagnostic function to get the buffer allocation statistics.
 *
 *  \return Buffer allocation statistics array.
 */
/*************************************************************************************************/
u8 *myBufGetAllocStats(void);

/*************************************************************************************************/
/*!
 *  \brief  Diagnostic function to get the number of overflow times for each pool.
 *
 *  \return Overflow times statistics array
 */
/*************************************************************************************************/
u8 *myBufGetPoolOverFlowStats(void);

/*************************************************************************************************/
/*!
 *  \brief  Get number of pools.
 *
 *  \return Number of pools.
 */
/*************************************************************************************************/
u8 myBufGetNumPool(void);

/*************************************************************************************************/
/*!
 *  \brief  Get statistics for each pool.
 *
 *  \param  pStat   Buffer to store the statistics.
 *  \param  numPool Number of pool elements.
 *
 *  \return Pool statistics.
 */
/*************************************************************************************************/
void myBufGetPoolStats(myBufPoolStat_t *pStat, u8 numPool);

/*************************************************************************************************/
/*!
 *  \brief  Called to register the buffer diagnostics callback function.
 *
 *  \param  callback   Pointer to the callback function.
 */
/*************************************************************************************************/
void myBufDiagRegister(myBufDiagCback_t callback);

/*! \} */    /* MY_BUF_API */

#ifdef __cplusplus
};
#endif

#endif /* MY_BUF_H */
