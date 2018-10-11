/**
 ******************************************************************************
 * @file    tl_ble_hci.c
 * @author  MCD Application Team
 * @version V1.8.0
 * @date    21-April-2017
 * @brief   Function for managing HCI interface.
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

/* Includes ------------------------------------------------------------------*/
#include "common.h"

#include "hci_tl_io.h"
#include "tl_ble.h"
#include "tl_ble_hci.h"
#include "list.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/**
 * The default HCI layer timeout is set to 33s
 */
#define DEFAULT_TIMEOUT (33000)

/* Private macros ------------------------------------------------------------*/
#if BLE_CONFIG_DBG_ENABLE
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Public variables ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_BLUENRG_DRIVER_CONTEXT
 */
static volatile uint8_t hci_timer_id;
static tListNode HciAsynchEventQueue;
static volatile TL_BLE_HCI_CmdStatus_t HCICmdStatus;
static TL_CmdPacket_t *pCmdBuffer;

/**
 * END of Section BLE_BLUENRG_DRIVER_CONTEXT
 */

static tListNode HciCmdEventQueue;
static volatile uint8_t HciLayerTimeout;

/* Private function prototypes -----------------------------------------------*/
static void hci_timeout_callback(void);
static void Cmd_SetStatus(TL_BLE_HCI_CmdStatus_t hcicmdstatus);
static TL_BLE_HCI_CmdStatus_t CmdGetStatus( void );
static void Initialization(void);
static void SendCmd(uint16_t opcode, uint8_t plen, void *param);



/* Private functions ----------------------------------------------------------*/
static void Initialization(void)
{
  LST_init_head (&HciAsynchEventQueue);

  Cmd_SetStatus(TL_BLE_HCI_CmdAvailable);

  return;
}

static void Cmd_SetStatus(TL_BLE_HCI_CmdStatus_t hcicmdstatus)
{
  if(hcicmdstatus == TL_BLE_HCI_CmdBusy)
  {
    TL_BLE_HCI_StatusNot(TL_BLE_HCI_CmdBusy);
    HCICmdStatus = TL_BLE_HCI_CmdBusy;
  }
  else
  {
    HCICmdStatus = TL_BLE_HCI_CmdAvailable;
    TL_BLE_HCI_StatusNot(TL_BLE_HCI_CmdAvailable);
  }
  return;
}

void TL_BLE_EvtReceived(TL_EvtPacket_t *hcievt)
{
  uint8_t evt_code;

  evt_code = hcievt->evtserial.evt.evtcode;

  if ( (evt_code == TL_EVT_CS_OPCODE) || (evt_code == TL_EVT_CC_OPCODE ) )
  {
    LST_insert_tail(&HciCmdEventQueue, (tListNode *)hcievt);
    TL_BLE_HCI_RxCmdResp(); /**< Notify the application a full Cmd Event has been received */
  }
  else
  {
    LST_insert_tail(&HciAsynchEventQueue, (tListNode *)hcievt);
    TL_BLE_HCI_UserEvtProcReq((TL_PacketHeader_t*)&HciAsynchEventQueue); /**< Notify the application a full HCI event has been received */
  }

  return;
}

void hci_timeout_callback(void)
{
  HciLayerTimeout = TRUE;

  /**
   * Unlock hci_send_req() that is waiting for an event that will never come
   */
  TL_BLE_HCI_RxCmdResp();

  /**
   * Notify the application the HCI interface has timed out
   */
  TL_BLE_HCI_ToNot();

  return;
}

void TL_BLE_HCI_Init(TL_BLE_HCI_InitMode_t initmode, TL_CmdPacket_t * p_cmdbuffer, uint8_t *p_evtpool, uint32_t evtpoolsize)
{
  /**
   * Always initialize the command event queue
   */
  LST_init_head (&HciCmdEventQueue);

  pCmdBuffer = p_cmdbuffer;

  if(initmode == TL_BLE_HCI_InitFull)
  {
    Initialization();

    TL_BLE_Init(TL_BLE_Init_Full, p_cmdbuffer, p_evtpool, evtpoolsize);
  }
  else
  {

#if (CFG_BLE_HCI_STDBY == 0)
    Initialization();
#endif /**< if (CFG_BLE_HCI_STDBY == 0) */

    TL_BLE_Init(TL_BLE_Init_Limited, p_cmdbuffer, p_evtpool, evtpoolsize);
  }

  return;
}

TL_BLE_HCI_CmdStatus_t CmdGetStatus(void)
{
  return HCICmdStatus;
}

void TL_BLE_HCI_UserEvtProc(void)
{
  TL_EvtPacket_t *phcievtbuffer;

  while(LST_is_empty(&HciAsynchEventQueue) == FALSE)
  {
    LST_remove_head (&HciAsynchEventQueue, (tListNode **)&phcievtbuffer);
    TL_BLE_HCI_UserEvtRx((void*)&(phcievtbuffer->evtserial));
    TL_BLE_EvtDone( phcievtbuffer);
  }

  return;
}

int32_t TL_BLE_HCI_SendCmd( TL_BLE_HCI_Cmd_t *p_cmd )
{
  uint16_t opcode;
  TL_CcEvt_t  *pcommand_complete_event;
  TL_CsEvt_t    *pcommand_status_event;
  TL_EvtPacket_t *pevtpacket;
  uint8_t hci_cmd_complete_return_parameters_length;
  int return_value;

  Cmd_SetStatus(TL_BLE_HCI_CmdBusy);
  opcode = ((p_cmd->ocf) & 0x03ff) | ((p_cmd->ogf) << 10);
  SendCmd(opcode, p_cmd->clen, p_cmd->cparam);

  HciLayerTimeout = FALSE;
  TL_BLE_HCI_TimStart(DEFAULT_TIMEOUT, hci_timeout_callback, (uint8_t*)&hci_timer_id);

  while((CmdGetStatus() == TL_BLE_HCI_CmdBusy) && (HciLayerTimeout == FALSE))
  {
    TL_BLE_HCI_WaitCmdResp();

    /**
     * Process Cmd Event
     */
    while(LST_is_empty(&HciCmdEventQueue) == FALSE)
    {
      LST_remove_head (&HciCmdEventQueue, (tListNode **)&pevtpacket);

      if(pevtpacket->evtserial.evt.evtcode == TL_EVT_CS_OPCODE)
      {
        pcommand_status_event = (TL_CsEvt_t*)pevtpacket->evtserial.evt.payload;
        if(pcommand_status_event->cmdcode == opcode)
        {
          *(uint8_t *)(p_cmd->rparam) = pcommand_status_event->status;
        }

        if(pcommand_status_event->numcmd != 0)
        {
          Cmd_SetStatus(TL_BLE_HCI_CmdAvailable);
        }
      }
      else
      {
        pcommand_complete_event = (TL_CcEvt_t*)pevtpacket->evtserial.evt.payload;

        if(pcommand_complete_event->cmdcode == opcode)
        {
          hci_cmd_complete_return_parameters_length = pevtpacket->evtserial.evt.plen - TL_EVT_CC_HDR_SIZE;
          p_cmd->rlen = MIN(hci_cmd_complete_return_parameters_length, p_cmd->rlen);
          memcpy(p_cmd->rparam, pcommand_complete_event->payload, p_cmd->rlen);
        }

        if(pcommand_complete_event->numcmd != 0)
        {
          Cmd_SetStatus(TL_BLE_HCI_CmdAvailable);
        }
      }

      TL_BLE_EvtDone( pevtpacket);
    }
  }

  TL_BLE_HCI_TimStop(hci_timer_id);

  if(HciLayerTimeout == TRUE)
  {
    return_value = -1;
  }
  else
  {
    return_value = 0;
  }

  return return_value;
}

void SendCmd(uint16_t opcode, uint8_t plen, void *param)
{
  pCmdBuffer->cmdserial.cmd.cmdcode = opcode;
  pCmdBuffer->cmdserial.cmd.plen = plen;
  memcpy( pCmdBuffer->cmdserial.cmd.payload, param, plen );

  TL_BLE_SendCmd();

  return;
}



/* This API is imposed by the MW */
int32_t hci_send_req(struct hci_request *r, uint8_t async)
{
  return ( TL_BLE_HCI_SendCmd( (TL_BLE_HCI_Cmd_t *)r ) );
}


	

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
