#include "nbfi.h"
#include <stdlib.h>
#include <wtimer.h>
#include <string.h>

#ifdef NBFI_LOG
#include "log.h"
#endif

nbfi_state_t nbfi_state = {0,0,0,0,0,0,0,0,0,0,0,0,0};

#define DRXLISTENAFTERSEND  20
#define WAITALITTLEBIT  3000

nbfi_transport_packet_t idle_pkt = {PACKET_FREE, HANDSHAKE_NONE, 0, 0, 0, {0,0} };
nbfi_transport_packet_t* nbfi_active_pkt = &idle_pkt;
nbfi_packet_state_t nbfi_active_pkt_old_state;

struct wtimer_desc nbfi_processTask_desc;
struct wtimer_desc dl_receive_desc;
struct wtimer_desc dl_drx_desc;
struct wtimer_desc wait_for_extra_desc;
struct wtimer_desc nbfi_heartbeat_desc;

rx_handler_t  rx_handler = 0;

uint8_t not_acked = 0;

int16_t noise = -150;
uint8_t nbfi_last_snr = 0;

_Bool wait_Receive = 0;
_Bool wait_Extra = 0;
_Bool wait_RxEnd = 0;

_Bool rx_complete = 0;

uint32_t info_timer;

uint32_t MinVoltage = 0;

uint32_t nbfi_rtc = 0;

_Bool process_rx_external = 0;


static void    NBFi_Receive_Timeout_cb(struct wtimer_desc *desc);
static void    NBFi_RX_DL_EndHandler(struct wtimer_desc *desc);
static void    NBFi_Wait_Extra_Handler(struct wtimer_desc *desc);
static void    NBFi_SendHeartBeats(struct wtimer_desc *desc);
static nbfi_status_t NBFi_RX_Controller();
static uint32_t NBFI_PhyToDL_ListenTime(nbfi_phy_channel_t chan);
static uint32_t NBFI_PhyToDL_Delay(nbfi_phy_channel_t chan);
static uint32_t NBFI_PhyToDL_AddRndListenTime(nbfi_phy_channel_t chan);

void NBFI_Transport_Init()
{
	
    for(uint8_t i = 0; i < NBFI_TX_PKTBUF_SIZE; i++) nbfi_TX_pktBuf[i] = 0;
    for(uint8_t i = 0; i < NBFI_RX_PKTBUF_SIZE; i++) nbfi_RX_pktBuf[i] = 0;

    info_timer = dev_info.send_info_interval - 300 - rand()%600;

    if(nbfi.additional_flags&NBFI_OFF_MODE_ON_INIT)
    {
      NBFi_Go_To_Sleep(1);
      ScheduleTask(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(60));
    }
    else
    {
      NBFi_RX_Controller();
      if(!(nbfi.additional_flags&NBFI_FLG_DO_NOT_SEND_PKTS_ON_START))
      { 
            NBFi_Config_Send_Sync(0);
            NBFi_Send_Clear_Cmd(0);
      }

      NBFi_Force_process();
      __nbfi_measure_voltage_or_temperature(1);
      ScheduleTask(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(1));
    }

}

nbfi_status_t NBFi_Send(uint8_t* payload, uint8_t length)
{
    nbfi_transport_packet_t* packet;
    uint8_t groupe = 0;
    uint8_t len = length;

    if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(1);

    uint8_t free = NBFI_TX_PKTBUF_SIZE - NBFi_Packets_To_Send();

    if((length <= nbfi.max_payload_len) && (free < nbfi.mack_mode + 3 ) ) 
    {
      if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);
      return ERR_BUFFER_FULL;
    }
    else if((length/nbfi.max_payload_len + 3) > free) 
    {
      if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);
      return ERR_BUFFER_FULL;
    }
    
    if(length < nbfi.max_payload_len)
    {
        packet =  NBFi_AllocateTxPkt(length + 1);
        if(!packet)
        {
            if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);
            return ERR_BUFFER_FULL;
        }
        packet->phy_data.SYS = 1;
        packet->phy_data.payload[0] = 0x80 + (length & 0x7f);
        memcpy(&packet->phy_data.payload[1], (void const*)payload, length);
        packet->state = PACKET_QUEUED;
        packet->handshake = nbfi.handshake_mode;
        packet->phy_data.ITER = nbfi_state.UL_iter++ & 0x1f;
        if((nbfi.handshake_mode != HANDSHAKE_NONE) && (nbfi.mode >= DRX))
        {
            if(((nbfi_state.UL_iter) % nbfi.mack_mode) == 0)
            {
                packet->phy_data.ACK = 1;
                packet->mack_num = not_acked + 1;
                not_acked = 0;
            }
            else not_acked++;
        }

    }
    else do
    {
        uint8_t l;
        uint8_t first = 0;
        if(length > nbfi.max_payload_len)
        {
            first = (groupe == 0)*3;
            l = nbfi.max_payload_len - first;
        }
        else l = length;
        packet =  NBFi_AllocateTxPkt(l + first);
        if(!packet)
        {
            if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);
            return ERR_BUFFER_FULL;
        }
        memcpy(packet->phy_data.payload + first, (void const*)&payload[groupe * nbfi.max_payload_len - 3*(groupe != 0)], l);
        packet->state = PACKET_QUEUED;
        packet->handshake = nbfi.handshake_mode;
        packet->phy_data.ITER = nbfi_state.UL_iter++ & 0x1f;
        if(l < length)
        {
            packet->phy_data.MULTI = 1;
            if(groupe == 0) //the start packet of the groupe must be system
            {
                packet->phy_data.SYS = 1;
                packet->phy_data.payload[0] = SYSTEM_PACKET_GROUP_START;
                packet->phy_data.payload[1] = len + 1;
                packet->phy_data.payload[2] = CRC8(payload, len);
            }
        }

        length -= l;
        groupe++;
        if((length == 0) && (groupe == 1))
        {
            if((nbfi.handshake_mode != HANDSHAKE_NONE)&&(nbfi.mode >= DRX))
            {
                if(((nbfi_state.UL_iter) % nbfi.mack_mode) == 0)
                {
                    packet->phy_data.ACK = 1;
                    packet->mack_num = not_acked + 1;
                    not_acked = 0;
                }
                else not_acked++;
            }
        }
        else   //the last packet of groupe must be acked
        {
            packet->phy_data.MULTI = 1;
            if((nbfi.handshake_mode != HANDSHAKE_NONE)&& (nbfi.mode >= DRX))
            {
                if(length == 0)
                {
                    packet->phy_data.ACK = 1;
                    packet->mack_num = groupe + not_acked;
                    not_acked = 0;
                }
            }
        }

    }while(length);

    NBFi_Force_process();
    if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);
    return OK;
}


void NBFi_ProcessRxPackets(_Bool external)
{
    nbfi_transport_packet_t* pkt;
    uint8_t data[256];
    uint8_t groupe;
    uint8_t last_group_iter;
    uint16_t total_length;
    _Bool group_with_crc = 0;
    process_rx_external = external;
    
    while(1)
    {

        if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(1);
        
        pkt = NBFi_Get_QueuedRXPkt(&groupe, &total_length);

        if(!pkt)    
        {
          if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);
          return;
        }

        
        if((pkt->phy_data.SYS) && (pkt->phy_data.payload[0] & 0x80))
        {
            total_length = pkt->phy_data.payload[0] & 0x7f;
            total_length = total_length%nbfi.max_payload_len;
            memcpy(data, (void const*)(&pkt->phy_data.payload[1]), total_length);
            if(nbfi.mack_mode < MACK_2) pkt->state = PACKET_PROCESSED;//NBFi_RxPacket_Free(pkt);
        }
        else
        {
            uint8_t iter = pkt->phy_data.ITER;
            group_with_crc = ((pkt->phy_data.SYS) && (nbfi_RX_pktBuf[(iter)&0x1f]->phy_data.payload[0] == SYSTEM_PACKET_GROUP_START));
            uint16_t memcpy_len = total_length;
            for(uint8_t i = 0; i != groupe; i++)
            {
                uint8_t len;
                uint8_t first = 0;
                last_group_iter = (iter + i)&0x1f;
                if((i == 0)&&(groupe > 1)) {len = nbfi.max_payload_len - 2; first = 2;}
                else len = (memcpy_len>=nbfi.max_payload_len)?nbfi.max_payload_len:memcpy_len%nbfi.max_payload_len;
                memcpy(data + i*nbfi.max_payload_len - 2*(i != 0), (void const*)(&nbfi_RX_pktBuf[last_group_iter]->phy_data.payload[first]), len);
                memcpy_len -= len;
                if(nbfi_RX_pktBuf[last_group_iter]->phy_data.ACK) nbfi_RX_pktBuf[last_group_iter]->state = PACKET_CLEARED;
                else nbfi_RX_pktBuf[last_group_iter]->state = PACKET_PROCESSED;

                if((nbfi.mack_mode < MACK_2) && (groupe == 1)) 
                {
                  //NBFi_RxPacket_Free(nbfi_RX_pktBuf[(iter + i)&0x1f]);
                  nbfi_RX_pktBuf[last_group_iter]->state = PACKET_PROCESSED;
                }
            }
        }
        
        
        uint8_t *data_ptr;
        if(group_with_crc)
        {
            total_length--;
            if(CRC8((unsigned char*)(&data[1]), (unsigned char)(total_length)) != data[0]) 
            {
                NBFi_Clear_RX_Buffer();
                if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);
                return;
            }
            data_ptr = &data[1];
        }
        else data_ptr = &data[0];
        
        if(groupe > 1) NBFi_Wait_Extra_Handler(0);

        if(__nbfi_lock_unlock_nbfi_irq) __nbfi_lock_unlock_nbfi_irq(0);

        if(rx_handler) rx_handler(data_ptr, total_length);

    }

}


void NBFi_ParseReceivedPacket(nbfi_transport_frame_t *phy_pkt, nbfi_mac_info_packet_t* info)
{
    
    int16_t rtc_offset;

    rx_complete = 1;

    nbfi_state.DL_total++;
    

    nbfi_state.aver_rx_snr = (((uint16_t)nbfi_state.aver_rx_snr)*3 + info->snr)>>2;
    nbfi_last_snr = info->snr;  
    noise = info->rssi - info->snr;  
   
    nbfi_transport_packet_t* pkt = 0;


    wtimer0_remove(&wait_for_extra_desc);
    wait_Extra = 0;

    if(nbfi_active_pkt->state == PACKET_WAIT_FOR_EXTRA_PACKETS)
    {
        nbfi_active_pkt->state = nbfi_active_pkt_old_state;
    }

    uint32_t mask = 0;
    uint8_t i = 1;
    uint32_t rtc;
    if(phy_pkt->SYS)
    {
            /* System messages */
            if(phy_pkt->payload[0] & 0x80) goto place_to_stack;
            switch(phy_pkt->payload[0]) // Message type
            {
            case SYSTEM_PACKET_ACK:    //ACK received
            case SYSTEM_PACKET_ACK_ON_SYS:    //ACK on system packet received
                if(((nbfi_active_pkt->state == PACKET_WAIT_ACK) ) && (phy_pkt->ITER == nbfi_active_pkt->phy_data.ITER))
                {
                    wtimer0_remove(&dl_receive_desc);
                    wait_Receive = 0;

                    if(nbfi_active_pkt->mack_num == 0)
                    {
                        nbfi_state.success_total++;
                        nbfi_active_pkt->state =  PACKET_DELIVERED;
                    }
                    nbfi_state.aver_tx_snr = (((uint16_t)nbfi_state.aver_tx_snr)*3 + phy_pkt->payload[5])>>2;
                    nbfi_station_info.info.byte = phy_pkt->payload[7];
                    
                    if(nbfi_station_info.info.RTC_MSB&0x20) rtc_offset = 0xC0 | nbfi_station_info.info.RTC_MSB;
                    else rtc_offset = nbfi_station_info.info.RTC_MSB;
                    rtc_offset <<= 8;
                    rtc_offset |= phy_pkt->payload[6];
                    if(rtc_offset) NBFi_set_RTC(NBFi_get_RTC() + rtc_offset);                 
                    if(phy_pkt->payload[0] == 0x00)
                    {
                      do
                      {
                          mask = (mask << 8) + phy_pkt->payload[i];
                      }   while (++i < 5);

                      NBFi_Resend_Pkt(nbfi_active_pkt, mask);
                    }
                    else
                    {
                       nbfi_station_info.fp.fp = phy_pkt->payload[3];
                       nbfi_station_info.fp.fp = (nbfi_station_info.fp.fp << 8) + phy_pkt->payload[4];
                       if(nbfi_station_info.fp.fp != 0 ) nbfi_state.bs_id = (((uint16_t)phy_pkt->payload[1]) << 8) + phy_pkt->payload[2];
                       else nbfi_state.server_id = (((uint16_t)phy_pkt->payload[1]) << 8) + phy_pkt->payload[2];
                    }
                }
                break;
            case SYSTEM_PACKET_CLEAR:  //clear RX buffer message received
                NBFi_Clear_RX_Buffer();
                break;
            case SYSTEM_PACKET_GROUP_START:  //start packet of the groupe
            case SYSTEM_PACKET_GROUP_START_OLD:  
                goto place_to_stack;
            case SYSTEM_PACKET_CONFIG:  //nbfi configure

                if(NBFi_Config_Parser(&phy_pkt->payload[1]))
                {
                    nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);
                    if(!ack_pkt) break;
                    ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_CONFIG;
                    memcpy(&ack_pkt->phy_data.payload[1], &phy_pkt->payload[1], 7);
                    ack_pkt->phy_data.ITER = phy_pkt->ITER;
                    ack_pkt->phy_data.header |= SYS_FLAG;
                    ack_pkt->state = PACKET_NEED_TO_SEND_RIGHT_NOW;
                }
                break;
            case SYSTEM_PACKET_RESET: //software reset
                if(__nbfi_reset && (phy_pkt->payload[1] == 0xDE) && (phy_pkt->payload[2] == 0xAD)) __nbfi_reset();
                break;
            case SYSTEM_PACKET_TIME:  //time correction
              memcpy(&rtc, &phy_pkt->payload[1], 4);
              NBFi_set_RTC(rtc);
              break;
            }
            if(phy_pkt->ACK && !NBFi_Calc_Queued_Sys_Packets_With_Type(0))    //send ACK on system packet
            {
                    nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);
                    if(ack_pkt)
                    {

                        ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_ACK_ON_SYS; //ACK on SYS
                        ack_pkt->phy_data.payload[1] = phy_pkt->payload[0];//type of sys packet
                        ack_pkt->phy_data.payload[2] = 0;
                        ack_pkt->phy_data.payload[3] = 0;
                        ack_pkt->phy_data.payload[4] = 0;
                        ack_pkt->phy_data.payload[5] = info->snr;
                        ack_pkt->phy_data.payload[6] = (uint8_t)(noise + 150);
                        ack_pkt->phy_data.payload[7] = you_should_dl_power_step_down + you_should_dl_power_step_up + (nbfi.tx_pwr & 0x3f); 
                        ack_pkt->phy_data.ITER = phy_pkt->ITER;
                        ack_pkt->phy_data.header |= SYS_FLAG;
                        ack_pkt->handshake = HANDSHAKE_NONE;
                        ack_pkt->state = PACKET_NEED_TO_SEND_RIGHT_NOW;
                    }
            }

    }
    else
    {
        //Get application packet
place_to_stack:
        if(!NBFi_Check_RX_Packet_Duplicate(phy_pkt, 9))   //if there is no rx duplicate
        {
            pkt = NBFi_AllocateRxPkt(phy_pkt->header, 8);
            if(!pkt) return;
            memcpy(&pkt->phy_data.header, phy_pkt, 9);
            pkt->state = PACKET_RECEIVED;
        }

        if(process_rx_external == 0) NBFi_ProcessRxPackets(0);
        
        if(phy_pkt->ACK && !NBFi_Calc_Queued_Sys_Packets_With_Type(0))
        {
            // Send ACK
            nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);
            if(ack_pkt)
            {
                uint32_t mask = NBFi_Get_RX_ACK_Mask();
                ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_ACK;
                ack_pkt->phy_data.payload[1] = (mask >> 24)&0xff;
                ack_pkt->phy_data.payload[2] = (mask >> 16)&0xff;
                ack_pkt->phy_data.payload[3] = (mask >> 8)&0xff;
                ack_pkt->phy_data.payload[4] = (mask >> 0)&0xff;
                ack_pkt->phy_data.payload[5] = info->snr;
                ack_pkt->phy_data.payload[6] = (uint8_t)(noise + 150);
                ack_pkt->phy_data.payload[7] = you_should_dl_power_step_down + you_should_dl_power_step_up + (nbfi.tx_pwr & 0x3f); 
                ack_pkt->phy_data.ITER = nbfi_state.DL_iter&0x1f;
                ack_pkt->phy_data.header |= SYS_FLAG;
                ack_pkt->handshake = HANDSHAKE_NONE;
                ack_pkt->state = PACKET_NEED_TO_SEND_RIGHT_NOW;
                NBFi_Force_process();
            }
        }
    }


    if(phy_pkt->MULTI && !phy_pkt->ACK)
    {
        //wait for extra packets
        if(nbfi_active_pkt->state != PACKET_WAIT_FOR_EXTRA_PACKETS)
        {
          nbfi_active_pkt_old_state = nbfi_active_pkt->state;
          nbfi_active_pkt->state = PACKET_WAIT_FOR_EXTRA_PACKETS;
        }
        ScheduleTask(&wait_for_extra_desc, NBFi_Wait_Extra_Handler, RELATIVE, NBFI_PhyToDL_ListenTime(nbfi.rx_phy_channel));
        wait_Extra = 1;
    }
    else
    {
        if(nbfi_active_pkt->state == PACKET_WAIT_FOR_EXTRA_PACKETS) nbfi_active_pkt->state = nbfi_active_pkt_old_state;

    }
   // if(process_rx_external == 0) NBFi_ProcessRxPackets(0);
    if(!phy_pkt->ACK) NBFI_Config_Check_State();
    if(NBFi_GetQueuedTXPkt()) NBFi_Force_process();
    else
    {
        if(nbfi.mode == DRX)
        {
            wait_RxEnd = 1;
            ScheduleTask(&dl_drx_desc, NBFi_RX_DL_EndHandler, RELATIVE, MILLISECONDS(WAITALITTLEBIT));
        }
        
        NBFi_RX_Controller();
    }

}

void NBFi_ProcessTasks(struct wtimer_desc *desc)
{
   nbfi_transport_packet_t* pkt;
   if(nbfi.mode == OFF)
   {
        NBFi_RX_Controller();
        NBFi_Clear_TX_Buffer();
        ScheduleTask(desc, 0, RELATIVE, SECONDS(30));
        return;
   }
   if((rf_busy == 0)&&(transmit == 0))
   {
        switch(nbfi_active_pkt->state)
        {
        case PACKET_WAIT_ACK:
            if(!wait_Receive)
            {
                ScheduleTask(&dl_receive_desc, NBFi_Receive_Timeout_cb, RELATIVE, NBFI_PhyToDL_Delay(nbfi.tx_phy_channel) + NBFI_PhyToDL_ListenTime(nbfi.rx_phy_channel) + rand()%NBFI_PhyToDL_AddRndListenTime(nbfi.rx_phy_channel));
                wait_Receive = 1;
            }
            break;
        case PACKET_WAIT_FOR_EXTRA_PACKETS:
            if(!wait_Extra)
            {
                ScheduleTask(&wait_for_extra_desc, NBFi_Wait_Extra_Handler, RELATIVE, NBFI_PhyToDL_ListenTime(nbfi.rx_phy_channel));
                wait_Extra = 1;
            }
            break;
        default:

            pkt = NBFi_GetQueuedTXPkt();
            if(pkt)
            {
                if((pkt->handshake != HANDSHAKE_NONE))
                {
                    if(pkt->phy_data.ACK)
                    {
                        switch(nbfi.mode)
                        {
                        case DRX:
                        case CRX:
                            pkt->state = PACKET_WAIT_ACK;
                            ScheduleTask(&dl_receive_desc, NBFi_Receive_Timeout_cb, RELATIVE, NBFI_PhyToDL_Delay(nbfi.tx_phy_channel) + NBFI_PhyToDL_ListenTime(nbfi.rx_phy_channel) + rand()%NBFI_PhyToDL_AddRndListenTime(nbfi.rx_phy_channel));                           
                            wait_Receive = 1;
                            break;
                        case NRX:
                            pkt->state = PACKET_SENT;
                            break;
                        }
                    }else pkt->state = PACKET_SENT_NOACKED;
                }
                else pkt->state = PACKET_SENT;
                nbfi_active_pkt = pkt;
                if(/*pkt->phy_data.SYS &&*/ !pkt->phy_data.ACK && NBFi_GetQueuedTXPkt()) pkt->phy_data.header |= MULTI_FLAG;

                if(pkt->phy_data.SYS)
                {
                  if(pkt->phy_data.payload[0] == SYSTEM_PACKET_CLEAR_EXT)   //update current timestamp
                  {
                    uint32_t rtc = NBFi_get_RTC();
                    pkt->phy_data.SYS = 1;
                    memcpy(&pkt->phy_data.payload[1], &rtc, 4);
                  }
                  else if((pkt->phy_data.payload[0] == SYSTEM_PACKET_SYNC))
                  {
                     pkt->phy_data.payload[6] = (nbfi_iter.dl >> 16); 
                     pkt->phy_data.payload[7] = (nbfi_iter.dl >> 8);
                  }
                }
                

                if(wait_RxEnd) {wait_RxEnd = 0; wtimer0_remove(&dl_drx_desc);}
                NBFi_MAC_TX(pkt);
#ifdef NBFI_LOG
                sprintf(log_string, "%05u: DL ", (uint16_t)(NBFi_get_RTC()&0xffff));
                sprintf(log_string + strlen(log_string), " %c%c%c - %d - PLD:", pkt->phy_data.SYS?'S':' ', pkt->phy_data.ACK?'A':' ',pkt->phy_data.MULTI?'M':' ', pkt->phy_data.ITER&0x1f);
                for(uint8_t k = 0; k != 8; k++) sprintf(log_string + strlen(log_string), "%02X", pkt->phy_data.payload[k]);
                sprintf(log_string + strlen(log_string), " -    - %dBPS", NBFi_Phy_To_Bitrate(nbfi.tx_phy_channel));
                log_send_str(log_string);
#endif

                if(pkt->state == PACKET_SENT)
                {
                    NBFi_TxPacket_Free(pkt);
                    nbfi_active_pkt = &idle_pkt;
                }

            }
            else
            {
                    NBFi_RX_Controller();
            }
        }
    }
    else
    {
          uint32_t t = __nbfi_measure_voltage_or_temperature(1);
          if(t < MinVoltage || !MinVoltage) MinVoltage = t;

    }

    if(rf_state == STATE_CHANGED)  NBFi_RX_Controller();
  
    if(nbfi.mode <= DRX && !NBFi_GetQueuedTXPkt() && (rf_busy == 0) && (transmit == 0) )
    {
        NBFi_RX_Controller();
        if(rf_state == STATE_OFF) ScheduleTask(desc, 0, RELATIVE, SECONDS(10));
        else ScheduleTask(desc, 0, RELATIVE, MILLISECONDS(50));
    }
    else ScheduleTask(desc, 0, RELATIVE, MILLISECONDS(50));

}

void NBFi_TX_Finished()
{
    transmit = 0;
    if(!nbfi_active_pkt->phy_data.ACK && NBFi_GetQueuedTXPkt())
    {
        NBFi_Force_process();
    }
    else
    {
        if(!nbfi_active_pkt->phy_data.ACK && (nbfi.mode == DRX))
        {
            wait_RxEnd = 1;
            ScheduleTask(&dl_drx_desc, NBFi_RX_DL_EndHandler, RELATIVE, SECONDS(DRXLISTENAFTERSEND));
        }
        else NBFI_Config_Check_State();
        NBFi_RX_Controller();
    }
}


static nbfi_status_t NBFi_RX_Controller()
{
    switch(nbfi.mode)
    {
    case  DRX:
        if(wait_RxEnd )
        {
          if(rf_state != STATE_RX) return NBFi_MAC_RX();
          else break;
        }
        switch(nbfi_active_pkt->state)
        {
        case PACKET_WAIT_ACK:
        case PACKET_QUEUED_AGAIN:
        case PACKET_WAIT_FOR_EXTRA_PACKETS:
            if(rf_state != STATE_RX) return NBFi_MAC_RX();
            break;
        default:
            if(rf_state != STATE_OFF)  return NBFi_RF_Deinit();
        }
        break;
    case CRX:
        if(rf_state != STATE_RX) return NBFi_MAC_RX();
        break;
    case NRX:
    case OFF:
        if(rf_state != STATE_OFF)  return NBFi_RF_Deinit();
        break;
    }
    return OK;
}

static void NBFi_RX_DL_EndHandler(struct wtimer_desc *desc)
{
    wait_RxEnd = 0;
    NBFi_RX_Controller();
}


static void NBFi_Receive_Timeout_cb(struct wtimer_desc *desc)
{
    if(rf_busy)
    {
        ScheduleTask(desc, NBFi_Receive_Timeout_cb, RELATIVE, NBFI_PhyToDL_ListenTime(nbfi.rx_phy_channel));
        return;
    }
    wtimer0_remove(&dl_receive_desc);
    wait_Receive = 0;
    if(nbfi_active_pkt->state != PACKET_WAIT_ACK)
    {
        #ifdef NBFi_DEBUG
                    my_sprintf((char *)string, "nbfi_active_pkt->state != PACKET_WAIT_ACK");
                    SLIP_Send_debug((uint8_t *)string, 50);
        #endif
        return;
    }
    nbfi_state.fault_total++;
    NBFi_Config_Tx_Power_Change(UP);
    if(++nbfi_active_pkt->retry_num > NBFi_Get_Retry_Number())
    {
       nbfi_active_pkt->state = PACKET_LOST;

       if(nbfi_active_pkt->phy_data.SYS && (nbfi_active_pkt->phy_data.payload[0] == SYSTEM_PACKET_SYNC)/* && (nbfi_active_pkt->phy_data.payload[1] != RATE_CHANGE_PARAM_CMD)*/)
        {
            NBFi_Mark_Lost_All_Unacked();
            NBFi_Config_Return(); //return to previous work configuration
            //nbfi_state.aver_rx_snr = nbfi_state.aver_tx_snr = 15;
        }
        else
        {
            if(!(nbfi.additional_flags&NBFI_FLG_NO_RESET_TO_DEFAULTS))
            {
                if((current_tx_rate == 0)&&(current_rx_rate == 0))
                {

                    NBFi_Mark_Lost_All_Unacked();

                }
                else
                {
                    nbfi_active_pkt->retry_num = 0;
                    nbfi_active_pkt->state = PACKET_QUEUED;
                }
                NBFi_Config_Set_Default(); //set default configuration
                NBFi_Config_Send_Sync(0);
            }
        }
    }
    else
    {
        nbfi_active_pkt->state = PACKET_QUEUED_AGAIN;
    }
    NBFi_Force_process();
    return;
}

static void NBFi_Wait_Extra_Handler(struct wtimer_desc *desc)
{
    wtimer0_remove(&wait_for_extra_desc);
    wait_Extra = 0;
    if(nbfi_active_pkt->state == PACKET_WAIT_FOR_EXTRA_PACKETS)     {nbfi_active_pkt->state = nbfi_active_pkt_old_state;}
    if(NBFi_GetQueuedTXPkt()) NBFi_Force_process();
}



static void NBFi_update_RTC()
{
    static uint32_t old_time_cur = 0;
 
    if(__nbfi_update_rtc) 
    {
      nbfi_rtc = __nbfi_update_rtc();
      return;
    }
    
    uint32_t delta;
    
    uint32_t tmp = (wtimer_state[0].time.cur >> 10);

    if(old_time_cur <= tmp)
    {
        delta = tmp - old_time_cur;
    }
    else delta = old_time_cur - tmp;

    nbfi_rtc += delta;

    old_time_cur = tmp;
}

//extern uint32_t systick_timer;
uint32_t NBFi_get_RTC()
{
    NBFi_update_RTC();
    return nbfi_rtc;
    //return systick_timer;
}

void NBFi_set_RTC(uint32_t time)
{
   NBFi_update_RTC();
   nbfi_rtc = time;
   if(__nbfi_rtc_synchronized) __nbfi_rtc_synchronized(nbfi_rtc);
}


static void NBFi_SendHeartBeats(struct wtimer_desc *desc)
{

    static uint16_t hb_timer = 0;

    NBFi_update_RTC();

    if(hb_timer == 0) hb_timer = rand()%nbfi.heartbeat_interval;

    if(nbfi.mode == OFF) 
    {
      ScheduleTask(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(60));
      return;
    }

    if(nbfi.mode <= DRX)
    {
        ScheduleTask(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(60));
    }
    else ScheduleTask(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(1));


    if(++hb_timer >= nbfi.heartbeat_interval + 1)
    {
        hb_timer = 1;
        if(nbfi.heartbeat_num == 0) return;
        if(nbfi.heartbeat_num != 0xff) nbfi.heartbeat_num--;
        if(NBFi_Calc_Queued_Sys_Packets_With_Type(1)) return;
        nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);
        if(!ack_pkt)   return;
        ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_HERTBEAT;
        ack_pkt->phy_data.payload[1] = 0;                      //heart beat type
        if(MinVoltage == 0) MinVoltage = __nbfi_measure_voltage_or_temperature(1);
        ack_pkt->phy_data.payload[2] = (MinVoltage >= 300 ? 0x80 : 0) + MinVoltage % 100;         //min supply voltage since last heartbeat
        MinVoltage = 0; //reset min voltage detection
        ack_pkt->phy_data.payload[3] = __nbfi_measure_voltage_or_temperature(0);    //temperature
        ack_pkt->phy_data.payload[4] = nbfi_state.aver_rx_snr; // DL average snr
        ack_pkt->phy_data.payload[5] = nbfi_state.aver_tx_snr; // UL average snr
        ack_pkt->phy_data.payload[6] = (uint8_t)(NBFi_RF_get_noise() + 150); // rx noice
        ack_pkt->phy_data.payload[7] = nbfi.tx_pwr;            // output power
        ack_pkt->phy_data.ITER = nbfi_state.UL_iter++ & 0x1f;
        ack_pkt->phy_data.header |= SYS_FLAG;
        if(nbfi.mode >= DRX)
        {
          if(nbfi.handshake_mode != HANDSHAKE_NONE)  
          {
            ack_pkt->handshake = HANDSHAKE_SIMPLE;
            ack_pkt->phy_data.header |= ACK_FLAG;
          }
        }
        ack_pkt->state = PACKET_QUEUED;
    }

    if(!(nbfi.additional_flags&NBFI_FLG_NO_SENDINFO))
    {
        if(nbfi.mode <= DRX) info_timer += 60;
        else info_timer++;
        if(info_timer >= dev_info.send_info_interval)
        {
                info_timer = 0;
                NBFi_Config_Send_Mode(nbfi.handshake_mode, NBFI_PARAM_VERSION);
                NBFi_Config_Send_Mode(nbfi.handshake_mode, NBFI_PARAM_TX_BRATES);
                NBFi_Config_Send_Mode(nbfi.handshake_mode, NBFI_PARAM_RX_BRATES);
                NBFi_Config_Send_Mode(nbfi.handshake_mode, NBFI_PARAM_APP_IDS);
                NBFi_Config_Send_Mode(nbfi.handshake_mode, NBFI_UL_BASE_FREQ);
                NBFi_Config_Send_Mode(nbfi.handshake_mode, NBFI_DL_BASE_FREQ);
        }
    }
}

void NBFi_Force_process()
{
    ScheduleTask(&nbfi_processTask_desc, NBFi_ProcessTasks, RELATIVE, MILLISECONDS(1));
}

static uint32_t NBFI_PhyToDL_Delay(nbfi_phy_channel_t chan)
{
	const uint32_t NBFI_DL_DELAY_C_D[10] = {30000, 30000, 30000, 5000, 5000, 5000, 1000, 1000, 500, 500};
	const uint32_t NBFI_DL_DELAY_E[10] = {6000, 1000, 500, 500};

	if (chan > UL_DBPSK_25600_PROT_E)
		return NBFI_DL_DELAY_E[0];
	else if (chan >= UL_DBPSK_50_PROT_E)
		return NBFI_DL_DELAY_E[chan - UL_DBPSK_50_PROT_E];
	else if (chan >= UL_DBPSK_50_PROT_C)
		return NBFI_DL_DELAY_C_D[chan - UL_DBPSK_50_PROT_C];
	return NBFI_DL_DELAY_E[0];
}

static uint32_t NBFI_PhyToDL_ListenTime(nbfi_phy_channel_t chan)
{
	const uint32_t NBFI_DL_LISTEN_TIME[4] = {60000, 55000, 4000, 4000};

	if (chan > DL_DBPSK_25600_PROT_E)
		return NBFI_DL_LISTEN_TIME[0];
	else if (chan >= DL_DBPSK_50_PROT_E)
		return NBFI_DL_LISTEN_TIME[chan - DL_DBPSK_50_PROT_E];
	else if (chan >= DL_DBPSK_50_PROT_D)
		return NBFI_DL_LISTEN_TIME[chan - DL_DBPSK_50_PROT_D];
	return NBFI_DL_LISTEN_TIME[0];		
}

static uint32_t NBFI_PhyToDL_AddRndListenTime(nbfi_phy_channel_t chan)
{
	const uint32_t NBFI_DL_ADD_RND_LISTEN_TIME[4] = {5000, 5000, 2000, 2000};
	
	if (chan > DL_DBPSK_25600_PROT_E)
		return NBFI_DL_ADD_RND_LISTEN_TIME[0];
	else if (chan >= DL_DBPSK_50_PROT_E)
		return NBFI_DL_ADD_RND_LISTEN_TIME[chan - DL_DBPSK_50_PROT_E];
	else if (chan >= DL_DBPSK_50_PROT_D)
		return NBFI_DL_ADD_RND_LISTEN_TIME[chan - DL_DBPSK_50_PROT_D];
	return NBFI_DL_ADD_RND_LISTEN_TIME[0];		
}

