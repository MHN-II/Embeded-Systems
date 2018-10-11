/**
 ******************************************************************************
 * @file    dbg_trace.h
 * @author  MCD Application Team
 * @version V1.8.0
 * @date    21-April-2017
 * @brief   Header for dbg_trace.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other 
 *    contributors to this software may be used to endorse or promote products 
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this 
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under 
 *    this license is void and will automatically terminate your rights under 
 *    this license. 
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INC_DBG_TRACE_H
#define __INC_DBG_TRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#if (CFG_DEBUG_TRACE == 0)
#define   NDEBUG
#endif
 
/* Trace level meaning that no log messages are output. */
#define DBG_TRACE_LVL_NONE      0
/* Trace level meaning that messages generated by DBG_TRACE_ASSERT are output. */
#define DBG_TRACE_LVL_FATAL     1
/* Trace level meaning that messages generated by DBG_TRACE_ASSERT, DBG_TRACE_ERR, DBG_TRACE_PRIO are output. */
#define DBG_TRACE_LVL_ERR       2
/* Trace level meaning that messages generated by DBG_TRACE_ASSERT, DBG_TRACE_ERR, DBG_TRACE_PRIO, DBG_TRACE_INFO are output. */
#define DBG_TRACE_LVL_INFO      3
/* Trace level meaning that all messages are output (DBG_TRACE_ASSERT, DBG_TRACE_ERR, DBG_TRACE_PRIO, DBG_TRACE_INFO and CFG_DEBUG_TRACE). */
#define DBG_TRACE_LVL_DBG       4

#define  DBG_TRACE_OUTPUT_NONE   0
#define  DBG_TRACE_OUTPUT_STDOUT 1
#define  DBG_TRACE_OUTPUT_UART   2
                               
// Use a default setting if nobody defined a log level
#ifndef DBG_TRACE_LEVEL
#define DBG_TRACE_LEVEL DBG_TRACE_LVL_DBG
#endif
                               
#ifndef DBG_TRACE_OUTPUT
#define DBG_TRACE_OUTPUT DBG_TRACE_OUTPUT_UART
#endif                               

#define DBG_TRACE_UART_USE_CIRCULAR_QUEUE

  
#undef DBG_TRACE_ASSERT
#undef DBG_TRACE_ERR
#undef DBG_TRACE_PRIO
#undef DBG_TRACE_INFO
#undef DBG_TRACE

/* Errors definition */
#define DBG_TRACE_NO_ERR 0
#define DBG_TRACE_USART_ERR 1

#if ((DBG_TRACE_OUTPUT == DBG_TRACE_OUTPUT_UART) && defined(DBG_TRACE_USE_MY_PRINF))
#define DBG_TRACE_PRINTF MyPrintf
#else
#define DBG_TRACE_PRINTF printf
#endif

#define DBG_TRACE_UART                          USART1

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* If debug allowed define the DBG_TRACE Macro */
#if !defined(NDEBUG) && (DBG_TRACE_OUTPUT != DBG_TRACE_OUTPUT_NONE)
#define DBG_TRACE(DbgTraceLvl,...)   do{DBG_TRACE_PRINTF("\r\n**%s** [%s][%s][%d] ",DbgTraceLvl, DbgTraceGetFileName(__FILE__),__FUNCTION__,__LINE__);DBG_TRACE_PRINTF(__VA_ARGS__);}while(0);  
#else
#define DBG_TRACE(DbgTraceLvl,...) 
#endif
  
#if (DBG_TRACE_LEVEL >= DBG_TRACE_LVL_FATAL) && !defined(NDEBUG) && (DBG_TRACE_OUTPUT != DBG_TRACE_OUTPUT_NONE)
/* Checks the condition, and if it is not fulfilled, a corresponding fatal message is output. */
#define DBG_TRACE_ASSERT(condition) \
  ((condition) ? (void)0 : \
  DbgTraceAssert(DbgTraceGetFileName(__FILE__),__FUNCTION__, __LINE__, #condition))
#else
#define DBG_TRACE_ASSERT(condition)       /* Nothing */
#endif


#if (DBG_TRACE_LEVEL >= DBG_TRACE_LVL_ERR) && !defined(NDEBUG) && (DBG_TRACE_OUTPUT != DBG_TRACE_OUTPUT_NONE)
/* Outputs an error message. */
#define DBG_TRACE_ERR(...) \
  {DBG_TRACE("ERR",__VA_ARGS__);}
/* Outputs a high priority information message (log level DBG_TRACE_LVL_ERR). */
#define DBG_TRACE_PRIO(...) \
  {DBG_TRACE("ERR_PRIO",__VA_ARGS__);}
#else
/* Outputs an error message. */
#define DBG_TRACE_ERR(...)      /* Nothing */
/* Outputs a high priority information message (log level DBG_TRACE_LVL_ERR). */
#define DBG_TRACE_PRIO(...)      /* Nothing */
#endif

#if (DBG_TRACE_LEVEL >= DBG_TRACE_LVL_INFO) && !defined(NDEBUG) && (DBG_TRACE_OUTPUT != DBG_TRACE_OUTPUT_NONE)
/* Outputs an information message. */
#define DBG_TRACE_INFO(...) \
  {DBG_TRACE("INFO",__VA_ARGS__);}
#else
/*  Outputs an information message.*/
#define DBG_TRACE_INFO(...)       /* Nothing */
#endif

#if (DBG_TRACE_LEVEL >= DBG_TRACE_LVL_DBG) && !defined(NDEBUG) && (DBG_TRACE_OUTPUT != DBG_TRACE_OUTPUT_NONE)
/* Outputs a debug message. */
#define DBG_TRACE_DBG(...) \
  {DBG_TRACE("DBG",__VA_ARGS__);}
#else
/* Outputs a debug message. */
#define DBG_TRACE_DBG(...)        /* Nothing */
#endif

/* Exported functions ------------------------------------------------------- */

uint8_t DbgTraceInit(void);

/**********************************************************************************************************************/
/** This function outputs the provided format string and arguments into the log.
 ***********************************************************************************************************************
 *
 * @param strFormat The format string in printf() style.
 * @param ... Arguments of the format string.
 *
 **********************************************************************************************************************/
void DbgTrace(const char *strFormat, ...);

/**********************************************************************************************************************/
/** This function outputs into the log the buffer (in hex) and the provided format string and arguments.
 ***********************************************************************************************************************
 *
 * @param pBuffer Buffer to be output into the logs.
 * @param u32Length Length of the buffer, in bytes.
 * @param strFormat The format string in printf() style.
 * @param ... Arguments of the format string.
 *
 **********************************************************************************************************************/
void DbgTraceBuffer(const void *pBuffer, uint32_t u32Length, const char *strFormat, ...);

/**********************************************************************************************************************/
/** This function is called if the condition in DBG_TRACE_ASSERT is not met, it outputs a corresponding DBG_TRACE_LVL_FATAL message.
 ***********************************************************************************************************************
 *
 * @param file Source file name.
 * @param fubction function name
 * @param line Line number.
 * @param condition Checked condition.
 *
 **********************************************************************************************************************/
void DbgTraceAssert(const char *file, const char *function, uint32_t line, const char *condition);

#ifdef USE_MY_PRINTF
void MyPrintf(char *fmt, ...);
#endif

const char *DbgTraceGetFileName(const char *fullpath);

#ifdef __cplusplus
}
#endif

#endif /*__INC_DBG_TRACE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
