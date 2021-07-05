#include "nbfi.h"

nbfi_state_t nbfi_state = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};



nbfi_transport_packet_t idle_pkt = {PACKET_FREE, 0, 0, HANDSHAKE_NONE, 0, 0, 0, {0,0} };
nbfi_transport_packet_t* nbfi_active_pkt = &idle_pkt;
nbfi_packet_state_t nbfi_active_pkt_old_state;

struct scheduler_desc nbfi_processTask_desc;
struct scheduler_desc dl_receive_desc;
struct scheduler_desc dl_drx_desc;
struct scheduler_desc wait_for_extra_desc;
struct scheduler_desc nbfi_heartbeat_desc;


uint8_t not_acked = 0;

int16_t noise = -150;
//uint8_t nbfi_last_snr = 0;

_Bool wait_Receive = 0;
_Bool wait_Extra = 0;
_Bool wait_RxEnd = 0;
_Bool rx_complete = 0;
_Bool was_not_cleared_after_groupe = 0;
_Bool uplink_received_after_send = 0;

uint32_t info_timer;

uint32_t MinVoltage = 0;

uint32_t nbfi_rtc = 0;

uint32_t last_ack_send_ts = 0;

#define TX_MAX_TIME 1000
//_Bool process_rx_external = 0;
_Bool rtc_synchronised = 0;

static void    NBFi_Receive_Timeout_cb(struct scheduler_desc *desc);
static void    NBFi_RX_DL_EndHandler(struct scheduler_desc *desc);
static void    NBFi_Wait_Extra_Handler(struct scheduler_desc *desc);
static void    NBFi_SendHeartBeats(struct scheduler_desc *desc);
static nbfi_status_t NBFi_RX_Controller();

static uint32_t NBFI_PhyTo_Delay(nbfi_phy_channel_t chan);
static uint32_t NBFi_UL_Delivery_Time(nbfi_phy_channel_t chan);
static uint32_t NBFi_DL_Delivery_Time(nbfi_phy_channel_t chan);

void NBFI_Transport_Init()
{

    #ifdef NBFI_USE_MALLOC
    for(uint8_t i = 0; i < NBFI_TX_PKTBUF_SIZE; i++) nbfi_TX_pktBuf[i] = 0;
    for(uint8_t i = 0; i < NBFI_RX_PKTBUF_SIZE; i++) nbfi_RX_pktBuf[i] = 0;
    #else
    for(uint8_t i = 0; i < NBFI_TX_PKTBUF_SIZE; i++) nbfi_TX_pktBuf[i].state = PACKET_FREE;
    for(uint8_t i = 0; i < NBFI_RX_PKTBUF_SIZE; i++) nbfi_RX_pktBuf[i].state = PACKET_FREE;
    #endif

    for(uint8_t i = 0; i < NBFI_SENT_STATUSES_BUF_SIZE; i++) NBFi_sent_UL_stat_Buf[i].id = 0;


    for(uint8_t i = 0; i < NBFI_RECEIVED_BUF_SIZE; i++)
    {
      #ifdef NBFI_USE_MALLOC
      NBFi_received_DL_Buf[i].payload = 0;
      #endif
      NBFi_received_DL_Buf[i].ready = 0;
    }

    info_timer = dev_info.send_info_interval - 300 - rand()%600;

    if(nbfi.additional_flags&NBFI_OFF_MODE_ON_INIT)
    {
      NBFi_go_to_Sleep(1);
      nbfi_scheduler->__scheduler_add_task(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(60));
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
      nbfi_hal->__nbfi_measure_voltage_or_temperature(1);
      nbfi_scheduler->__scheduler_add_task(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(1));
    }

}



nbfi_ul_sent_status_t NBFi_Send5(uint8_t* payload, uint8_t length, uint8_t flags)
{
    nbfi_transport_packet_t* packet;
    uint8_t groupe = 0;
    uint8_t len = length;

    nbfi_ul_sent_status_t err_status;
    err_status.id = 0;

    nbfi_ul_sent_status_t* ul_status;


    if(length/nbfi.max_payload_len > 30)
    {
      err_status.status = ERR_PACKET_IS_TOO_LONG;
      return err_status;
    }

    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    uint8_t free = NBFI_TX_PKTBUF_SIZE - NBFi_Packets_To_Send();

    if((length <= nbfi.max_payload_len) && (free < nbfi.mack_mode + 3 ) )
    {
      err_status.status = ERR_BUFFER_FULL;
      nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
      return err_status;
    }
    else if((length/nbfi.max_payload_len + 3) > free)
    {
      err_status.status = ERR_BUFFER_FULL;
      nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
      return err_status;
    }

    ul_status = NBFi_Queue_Next_UL(flags);

    if(length < nbfi.max_payload_len)
    {
        packet =  NBFi_AllocateTxPkt(length + 1);
        if(!packet)
        {
          err_status.status = ERR_BUFFER_FULL;
          nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
          return err_status;
        }
        packet->id = (ul_status->id&0xff);
        packet->phy_data.SYS = 1;
        packet->phy_data.payload[0] = 0x80 + (length & 0x7f);
        memcpy(&packet->phy_data.payload[1], (void const*)payload, length);
        packet->state = PACKET_QUEUED;
        if(!(flags&NBFI_UL_FLAG_NOACK)) packet->handshake = nbfi.handshake_mode;
	if(flags&NBFI_UL_FLAG_NO_RETRIES) packet->retry_num = 0xf0;
        packet->phy_data.ITER = nbfi_state.UL_iter++ & 0x1f;
        if((packet->handshake != HANDSHAKE_NONE) && (nbfi.mode >= DRX))
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
          err_status.status = ERR_BUFFER_FULL;
          nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
          return err_status;
        }
        packet->id = (ul_status->id&0xff);
        memcpy(packet->phy_data.payload + first, (void const*)&payload[groupe * nbfi.max_payload_len - 3*(groupe != 0)], l);
        packet->state = PACKET_QUEUED;
        if(!(flags&NBFI_UL_FLAG_NOACK)) packet->handshake = nbfi.handshake_mode;
	if(flags&NBFI_UL_FLAG_NO_RETRIES) packet->retry_num = 0xf0;
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
            if((packet->handshake != HANDSHAKE_NONE)&&(nbfi.mode >= DRX))
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
            if((packet->handshake != HANDSHAKE_NONE)&& (nbfi.mode >= DRX))
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

    uplink_received_after_send = 0;

    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
    return *ul_status;
}

nbfi_status_t  NBFi_Send(uint8_t* payload, uint8_t length)
{
  return (NBFi_Send5(payload, length, 0).status <= ERR_BUFFER_FULL)?OK:ERR_BUFFER_FULL_v4;
}

void NBFi_ProcessRxPackets()
{
    nbfi_transport_packet_t* pkt;
    uint8_t data[256];
    uint8_t groupe;
    uint8_t last_group_iter;
    uint16_t total_length;
    _Bool group_with_crc = 0;

    while(1)
    {

        nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
        pkt = NBFi_Get_QueuedRXPkt(&groupe, &total_length);

        if(!pkt)
        {
          nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
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
            group_with_crc = ((pkt->phy_data.SYS) && (NBFi_Get_RX_Packet_Ptr(iter&0x1f)->phy_data.payload[0] == SYSTEM_PACKET_GROUP_START));
            uint16_t memcpy_len = total_length;
            for(uint8_t i = 0; i != groupe; i++)
            {
                uint8_t len;
                uint8_t first = 0;
                last_group_iter = (iter + i)&0x1f;
                if((i == 0)&&(groupe > 1)) {len = nbfi.max_payload_len - 2; first = 2;}
                else len = (memcpy_len>=nbfi.max_payload_len)?nbfi.max_payload_len:memcpy_len%nbfi.max_payload_len;
                memcpy(data + i*nbfi.max_payload_len - 2*(i != 0), (void const*)(&NBFi_Get_RX_Packet_Ptr(last_group_iter)->phy_data.payload[first]), len);
                memcpy_len -= len;
                if(NBFi_Get_RX_Packet_Ptr(last_group_iter)->phy_data.ACK) NBFi_Get_RX_Packet_Ptr(last_group_iter)->state = PACKET_CLEARED;
                else NBFi_Get_RX_Packet_Ptr(last_group_iter)->state = PACKET_PROCESSED;

                if((nbfi.mack_mode < MACK_2) && (groupe == 1))
                {
                  //NBFi_RxPacket_Free(nbfi_RX_pktBuf[(iter + i)&0x1f]);
                  NBFi_Get_RX_Packet_Ptr(last_group_iter)->state = PACKET_PROCESSED;
                }
            }
        }


        uint8_t *data_ptr;
        if(group_with_crc)
        {
            total_length--;
            if(CRC8((unsigned char*)(&data[1]), (unsigned char)(total_length)) != data[0])
            {
                NBFi_Clear_RX_Buffer(nbfi_state.DL_iter&0x1f, 0);
                #ifdef NBFI_LOG
                sprintf(nbfi_log_string, "%05u: CRC mismatch ", (uint16_t)(nbfi_scheduler->__scheduler_curr_time()&0xffff));
                nbfi_hal->__nbfi_log_send_str(nbfi_log_string);
                #endif

                nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
                return;
            }
            data_ptr = &data[1];
        }
        else data_ptr = &data[0];

        if(groupe > 1) NBFi_Wait_Extra_Handler(0);

        nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);

        NBFi_Queue_Next_DL(data_ptr, total_length);
    }

}



void NBFi_ParseReceivedPacket(nbfi_transport_frame_t *phy_pkt, nbfi_mac_info_packet_t* info)
{

    int16_t rtc_offset;

    rx_complete = 1;

    nbfi_state.DL_total++;

    //static uint32_t last_dl_scheduler_time = 0;

    //last_dl_scheduler_time = nbfi_scheduler->__scheduler_curr_time();

    int32_t time_from_last_dl = NBFi_get_RTC() - nbfi_state.DL_last_time;
    nbfi_state.DL_last_time = NBFi_get_RTC();

    nbfi_state.aver_rx_snr = (((uint16_t)nbfi_state.aver_rx_snr)*3 + info->snr)>>2;
    nbfi_state.last_snr = info->snr;
    noise = info->rssi - info->snr;
    nbfi_state.last_rssi = info->rssi;
    nbfi_transport_packet_t* pkt = 0;


    if(wait_Extra)
    {
      nbfi_scheduler->__scheduler_remove_task(&wait_for_extra_desc);
      wait_Extra = 0;
    }

    if(nbfi_active_pkt->state == PACKET_WAIT_FOR_EXTRA_PACKETS)
    {
        nbfi_active_pkt->state = nbfi_active_pkt_old_state;
    }

    uplink_received_after_send = 1;

    uint32_t mask = 0;
    uint8_t i = 1;
    uint32_t rtc;
    #ifdef NBFI_LOG
                sprintf(nbfi_log_string, "%05u: DL ", (uint16_t)(nbfi_scheduler->__scheduler_curr_time()&0xffff));
                sprintf(nbfi_log_string + strlen(nbfi_log_string), " %c%c%c - %d - PLD:", phy_pkt->SYS?'S':' ', phy_pkt->ACK?'A':' ',phy_pkt->MULTI?'M':' ', phy_pkt->ITER&0x1f);
                for(uint8_t k = 0; k != 8; k++) sprintf(nbfi_log_string + strlen(nbfi_log_string), "%02X", phy_pkt->payload[k]);
                sprintf(nbfi_log_string + strlen(nbfi_log_string), " -    - %dBPS", NBFi_Phy_To_Bitrate(nbfi.rx_phy_channel));
                nbfi_hal->__nbfi_log_send_str(nbfi_log_string);
    #endif

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
                    nbfi_scheduler->__scheduler_remove_task(&dl_receive_desc);
                    wait_Receive = 0;
                    try_counter = 0;
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
                    if(rtc_offset&&!NBFi_is_Switched_to_Custom_Settings()) NBFi_set_RTC_irq(NBFi_get_RTC() + rtc_offset);
                    if(phy_pkt->payload[0] == SYSTEM_PACKET_ACK)
                    {
                      do
                      {
                          mask = (mask << 8) + phy_pkt->payload[i];
                      }   while (++i < 5);

                      NBFi_Resend_Pkt(nbfi_active_pkt, mask);
                    }
                    else if(!NBFi_is_Switched_to_Custom_Settings())
                    {
                       nbfi_station_info.fp.fp = phy_pkt->payload[1];
                       nbfi_station_info.fp.fp = (nbfi_station_info.fp.fp << 8) + phy_pkt->payload[2];
                       if(nbfi_station_info.fp.fp == (NBFI_UL_FREQ_PLAN_NO_CHANGE + NBFI_DL_FREQ_PLAN_NO_CHANGE) ) nbfi_state.bs_id = (((uint16_t)phy_pkt->payload[3]) << 8) + phy_pkt->payload[4];
                       else nbfi_state.server_id = (((uint16_t)phy_pkt->payload[3]) << 8) + phy_pkt->payload[4];
                    }
                }
                break;
            case SYSTEM_PACKET_CLEAR:  //clear RX buffer message received
            case SYSTEM_PACKET_CLEAR_EXT:
                NBFi_Clear_RX_Buffer(-1, 0);
                was_not_cleared_after_groupe = 0;
                break;
            case SYSTEM_PACKET_GROUP_START:  //start packet of the groupe
            case SYSTEM_PACKET_GROUP_START_OLD:
                if(was_not_cleared_after_groupe)   NBFi_Clear_RX_Buffer(-1, 1000*60*10/*NBFI_PhyTo_Delay(nbfi.rx_phy_channel)*32*/);
                was_not_cleared_after_groupe = 1;

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
                if((phy_pkt->payload[1] == 0xDE) && (phy_pkt->payload[2] == 0xAD)) NBFi_CPU_Reset();
                break;
            case SYSTEM_PACKET_TIME:  //time correction
              memcpy(&rtc, &phy_pkt->payload[1], 4);
              NBFi_set_RTC_irq(rtc);
              break;
            }
            if(phy_pkt->ACK && !NBFi_Calc_Queued_Sys_Packets_With_Type(SYSTEM_PACKET_ACK_ON_SYS, 0))    //send ACK on system packet
            {
                    nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);
                    if(ack_pkt)
                    {

                        ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_ACK_ON_SYS; //ACK on SYS
                        ack_pkt->phy_data.payload[1] = (nbfi_state.bs_id >> 8);
                        ack_pkt->phy_data.payload[2] = (nbfi_state.bs_id & 0xff);
                        ack_pkt->phy_data.payload[3] = (nbfi_state.server_id >> 8);
                        ack_pkt->phy_data.payload[4] = (nbfi_state.server_id & 0xff);
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

        //if(process_rx_external == 0) NBFi_ProcessRxPackets(0);
        NBFi_ProcessRxPackets();

        if(phy_pkt->ACK && !NBFi_Calc_Queued_Sys_Packets_With_Type(SYSTEM_PACKET_ACK, 0))
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
        wait_Extra = 1;
        nbfi_scheduler->__scheduler_add_task(&wait_for_extra_desc, NBFi_Wait_Extra_Handler, RELATIVE, NBFI_PhyTo_Delay(nbfi.rx_phy_channel)*2/*NBFi_DL_Delivery_Time(nbfi.rx_phy_channel)*/);

    }
    else
    {
        if(nbfi_active_pkt->state == PACKET_WAIT_FOR_EXTRA_PACKETS) nbfi_active_pkt->state = nbfi_active_pkt_old_state;

    }
    if(!phy_pkt->ACK) NBFI_Config_Check_State();
    nbfi_transport_packet_t* queued;
    if(queued = NBFi_GetQueuedTXPkt())
    {
      if((queued->phy_data.header&SYS_FLAG) && (queued->phy_data.payload[0] == SYSTEM_PACKET_CLEAR_EXT)&&(NBFi_Packets_To_Send() == 1))
        NBFi_SlowDown_Process(100);
      else NBFi_Force_process();

    }
    else
    {
        if(nbfi.mode == DRX)
        {
            wait_RxEnd = 1;
            nbfi_scheduler->__scheduler_add_task(&dl_drx_desc, NBFi_RX_DL_EndHandler, RELATIVE, MILLISECONDS(WAITALITTLEBIT));
        }

        NBFi_RX_Controller();
    }

}

void NBFi_ProcessTasks(struct scheduler_desc *desc)
{
   nbfi_transport_packet_t* pkt;
   if(nbfi.mode == OFF)
   {
        NBFi_RX_Controller();
        NBFi_Clear_TX_Buffer();
        //nbfi_scheduler->__scheduler_add_task(desc, 0, RELATIVE, SECONDS(30));
        return;
   }
   static uint32_t tx_timer = 0;
   if((rf_busy == 0)&&(transmit == 0))
   //if((rf_busy == 0)&&!NBFi_RF_is_TX_in_Progress())
   {
     	tx_timer = 0;
        switch(nbfi_active_pkt->state)
        {
        case PACKET_WAIT_ACK:
            if(!wait_Receive)
            {
                nbfi_scheduler->__scheduler_add_task(&dl_receive_desc, NBFi_Receive_Timeout_cb, RELATIVE, NBFi_UL_Delivery_Time(nbfi.tx_phy_channel) + NBFi_DL_Delivery_Time(nbfi.rx_phy_channel));
                wait_Receive = 1;
            }
            break;
        case PACKET_WAIT_FOR_EXTRA_PACKETS:
            if(!wait_Extra)
            {
                nbfi_scheduler->__scheduler_add_task(&wait_for_extra_desc, NBFi_Wait_Extra_Handler, RELATIVE, NBFi_DL_Delivery_Time(nbfi.rx_phy_channel));
                wait_Extra = 1;
            }
            break;
        default:


            pkt = NBFi_GetQueuedTXPkt();

            if(pkt)
            {

                if(pkt->state != PACKET_NEED_TO_SEND_RIGHT_NOW)     uplink_received_after_send = 0;

                if((pkt->handshake != HANDSHAKE_NONE))
                {
                    if(pkt->phy_data.ACK)
                    {
                        switch(nbfi.mode)
                        {
                        case DRX:
                        case CRX:
                            pkt->state = PACKET_WAIT_ACK;
                            nbfi_scheduler->__scheduler_add_task(&dl_receive_desc, NBFi_Receive_Timeout_cb, RELATIVE, NBFi_UL_Delivery_Time(nbfi.tx_phy_channel)+NBFi_DL_Delivery_Time(nbfi.rx_phy_channel));
                            wait_Receive = 1;
                            break;
                        case NRX:
                            pkt->state = PACKET_SENT;
                            break;
                        }
                    }
                    else
                    {
                       switch(nbfi.mode)
                        {
                        case DRX:
                        case CRX:
                          pkt->state = PACKET_SENT_NOACKED;
                          break;
                        case NRX:
                          pkt->state = PACKET_SENT;
                          break;
                      }
                    }
                }
                else pkt->state = PACKET_SENT;
                nbfi_active_pkt = pkt;

                if(pkt->phy_data.SYS)
                {
                  switch(pkt->phy_data.payload[0])
                  {
                    case SYSTEM_PACKET_ACK:
                    case SYSTEM_PACKET_ACK_ON_SYS:
                      last_ack_send_ts = nbfi_scheduler->__scheduler_curr_time();
                      NBFI_Config_Check_State();
                      break;
                    case SYSTEM_PACKET_CLEAR_EXT: //update current timestamp
                      {
                        uint32_t rtc = NBFi_get_RTC();
                        pkt->phy_data.SYS = 1;
                        memcpy(&pkt->phy_data.payload[1], &rtc, 4);
                      }
                      break;
                    case SYSTEM_PACKET_SYNC:
		   	pkt->phy_data.payload[2] = nbfi.tx_phy_channel;
    			pkt->phy_data.payload[3] = nbfi.rx_phy_channel;
    			pkt->phy_data.payload[4] = (nbfi.nbfi_freq_plan.fp>>8);
    			pkt->phy_data.payload[5] = (nbfi.nbfi_freq_plan.fp&0xff);
                      	pkt->phy_data.payload[6] = (nbfi_iter.dl >> 16);
                      	pkt->phy_data.payload[7] = (nbfi_iter.dl >> 8);
                    	break;
                  }
                }

                if(!pkt->phy_data.ACK && pkt->phy_data.SYS && NBFi_GetQueuedTXPkt()) pkt->phy_data.header |= MULTI_FLAG;


                if(wait_RxEnd) {wait_RxEnd = 0; nbfi_scheduler->__scheduler_remove_task(&dl_drx_desc);}
                NBFi_Set_UL_Status(pkt->id, INPROCESS);

#ifdef NBFI_LOG
                sprintf(nbfi_log_string, "%05u: UL ", (uint16_t)(nbfi_scheduler->__scheduler_curr_time()&0xffff));
                sprintf(nbfi_log_string + strlen(nbfi_log_string), " %c%c%c - %d - PLD:", pkt->phy_data.SYS?'S':' ', pkt->phy_data.ACK?'A':' ',pkt->phy_data.MULTI?'M':' ', pkt->phy_data.ITER&0x1f);
                for(uint8_t k = 0; k != 8; k++) sprintf(nbfi_log_string + strlen(nbfi_log_string), "%02X", pkt->phy_data.payload[k]);
                sprintf(nbfi_log_string + strlen(nbfi_log_string), " -    - %dBPS", NBFi_Phy_To_Bitrate(nbfi.tx_phy_channel));
                nbfi_hal->__nbfi_log_send_str(nbfi_log_string);

        if(wait_Extra)
        {
        #ifdef NBFI_LOG
                sprintf(nbfi_log_string, "%05u: Send on  waitExtra ", (uint16_t)(nbfi_scheduler->__scheduler_curr_time()&0xffff));
                nbfi_hal->__nbfi_log_send_str(nbfi_log_string);
        #endif
        }

#endif
                NBFi_MAC_TX(pkt);

                if(pkt->state == PACKET_SENT)
                {
                    NBFi_Set_UL_Status(pkt->id, DELIVERED);
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
          uint32_t t = nbfi_hal->__nbfi_measure_voltage_or_temperature(1);
          if(t < MinVoltage || !MinVoltage) MinVoltage = t;
          if((rf_busy == 0)&&!NBFi_RF_is_TX_in_Progress()) NBFi_TX_Finished();

	  if(++tx_timer > TX_MAX_TIME)
	  {
	    NBFi_TX_Finished();
	  }
    }

    if(rf_state == STATE_CHANGED)  NBFi_RX_Controller();


    if(nbfi.mode <= DRX && !NBFi_GetQueuedTXPkt() && (rf_busy == 0) && !NBFi_RF_is_TX_in_Progress())
    {
        NBFi_RX_Controller();
        if(rf_state == STATE_OFF) ;//nbfi_scheduler->__scheduler_add_task(desc, 0, RELATIVE, SECONDS(10));
        else nbfi_scheduler->__scheduler_add_task(desc, 0, RELATIVE, MILLISECONDS(50));
    }
    else nbfi_scheduler->__scheduler_add_task(desc, 0, RELATIVE, MILLISECONDS(50));

}

void NBFi_TX_Finished()
{
    if(transmit == 0) return;
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
            nbfi_scheduler->__scheduler_add_task(&dl_drx_desc, NBFi_RX_DL_EndHandler, RELATIVE, SECONDS(DRXLISTENAFTERSEND));
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
    //nbfi_scheduler->__scheduler_add_task(&nbfi_processTask_desc, NBFi_ProcessTasks, RELATIVE, SECONDS(30));
    return OK;
}

static void NBFi_RX_DL_EndHandler(struct scheduler_desc *desc)
{
    wait_RxEnd = 0;
    NBFi_RX_Controller();
}


static void NBFi_Receive_Timeout_cb(struct scheduler_desc *desc)
{


    #ifdef NBFI_LOG
                sprintf(nbfi_log_string, "%05u: Receive timeout ", (uint16_t)(nbfi_scheduler->__scheduler_curr_time()&0xffff));
                 nbfi_hal->__nbfi_log_send_str(nbfi_log_string);
    #endif
    if(rf_busy)
    {
        nbfi_scheduler->__scheduler_add_task(desc, NBFi_Receive_Timeout_cb, RELATIVE, NBFi_DL_Delivery_Time(nbfi.rx_phy_channel));
        return;
    }
    nbfi_scheduler->__scheduler_remove_task(&dl_receive_desc);
    wait_Receive = 0;
    if(nbfi_active_pkt->state != PACKET_WAIT_ACK)
    {
        return;
    }
    nbfi_state.fault_total++;
    NBFi_Config_Tx_Power_Change(UP);
    if(++nbfi_active_pkt->retry_num > NBFi_Get_Retry_Number())
    {
       NBFi_Set_UL_Status(nbfi_active_pkt->id, LOST);
       NBFi_Close_Active_Packet();
       if(nbfi_active_pkt->phy_data.SYS && (nbfi_active_pkt->phy_data.payload[0] == SYSTEM_PACKET_SYNC))
       {
             NBFi_Config_Return(); //return to previous work configuration
       }
       else
       {
            if(!(nbfi.additional_flags&NBFI_FLG_NO_RESET_TO_DEFAULTS))
            {

                if(NBFi_Config_is_settings_default()||try_counter)
                {
                    NBFi_Config_Set_Default();
                    if(NBFi_Config_Try_Alternative())
					{
						nbfi_active_pkt->retry_num = 0;
                    	nbfi_active_pkt->state = PACKET_QUEUED;
					}
                }
                else
                {
                    nbfi_active_pkt->retry_num = 0;
                    nbfi_active_pkt->state = PACKET_QUEUED;
                    NBFi_Config_Set_Default();
                }
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

static void NBFi_Wait_Extra_Handler(struct scheduler_desc *desc)
{
    nbfi_scheduler->__scheduler_remove_task(&wait_for_extra_desc);
    wait_Extra = 0;
    if(nbfi_active_pkt->state == PACKET_WAIT_FOR_EXTRA_PACKETS)     {nbfi_active_pkt->state = nbfi_active_pkt_old_state;}
    if(NBFi_GetQueuedTXPkt()) NBFi_Force_process();
}



void NBFi_update_RTC()
{
    static uint32_t old_time_cur = 0;

    if(nbfi_hal->__nbfi_update_rtc)
    {
      nbfi_rtc = nbfi_hal->__nbfi_update_rtc();
      return;
    }

    uint32_t delta;

    uint32_t tmp = (nbfi_scheduler->__scheduler_curr_time() >> 10);

    if(old_time_cur <= tmp)
    {
        delta = tmp - old_time_cur;
    }
    else delta = old_time_cur - tmp;

    nbfi_rtc += delta;

    old_time_cur = tmp;
}


void NBFi_set_RTC_irq(uint32_t time)
{
   NBFi_update_RTC();
   nbfi_rtc = time;
   rtc_synchronised = 1;
   if(nbfi_hal->__nbfi_rtc_synchronized) nbfi_hal->__nbfi_rtc_synchronized(nbfi_rtc);
}


static void NBFi_SendHeartBeats(struct scheduler_desc *desc)
{

    static uint16_t hb_timer = 0;

    NBFi_update_RTC();

    if(hb_timer == 0) hb_timer = rand()%nbfi.heartbeat_interval;

    if(nbfi.mode == OFF)
    {
      nbfi_scheduler->__scheduler_add_task(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(60));
      return;
    }

    if(nbfi.mode <= DRX)
    {
        nbfi_scheduler->__scheduler_add_task(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(60));
    }
    else nbfi_scheduler->__scheduler_add_task(&nbfi_heartbeat_desc, NBFi_SendHeartBeats, RELATIVE, SECONDS(1));


    if(++hb_timer >= nbfi.heartbeat_interval + 1)
    {
        hb_timer = 1;
        if(nbfi.heartbeat_num == 0) return;
        if(nbfi.heartbeat_num != 0xff) nbfi.heartbeat_num--;
        if(NBFi_Calc_Queued_Sys_Packets_With_Type(SYSTEM_PACKET_HERTBEAT, 0))
        {
          return;
        }
        nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);
        if(!ack_pkt)   return;
        ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_HERTBEAT;
        ack_pkt->phy_data.payload[1] = 0;                      //heart beat type
        if(MinVoltage == 0) MinVoltage = nbfi_hal->__nbfi_measure_voltage_or_temperature(1);
        ack_pkt->phy_data.payload[2] = (MinVoltage >= 300 ? 0x80 : 0) + MinVoltage % 100;         //min supply voltage since last heartbeat
        MinVoltage = 0; //reset min voltage detection
        ack_pkt->phy_data.payload[3] = nbfi_hal->__nbfi_measure_voltage_or_temperature(0);    //temperature
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
        NBFi_Force_process();
    }

    if(!(nbfi.additional_flags&NBFI_FLG_NO_SENDINFO))
    {
        if(nbfi.mode <= DRX) info_timer += 60;
        else info_timer++;
        if(info_timer >= dev_info.send_info_interval)
        {
                info_timer = 0;
                NBFi_Config_Send_Mode(HANDSHAKE_NONE, NBFI_PARAM_VERSION);
                NBFi_Config_Send_Mode(HANDSHAKE_NONE, NBFI_PARAM_TX_BRATES);
                NBFi_Config_Send_Mode(HANDSHAKE_NONE, NBFI_PARAM_RX_BRATES);
                NBFi_Config_Send_Mode(HANDSHAKE_NONE, NBFI_PARAM_APP_IDS);
                NBFi_Config_Send_Mode(HANDSHAKE_NONE, NBFI_PARAM_UL_BASE_FREQ);
                NBFi_Config_Send_Mode(HANDSHAKE_NONE, NBFI_PARAM_DL_BASE_FREQ);
                NBFi_Config_Send_Mode(HANDSHAKE_NONE, NBFI_PARAM_HANDSHAKE);
        }
    }
}

void NBFi_Force_process()
{
  nbfi_scheduler->__scheduler_add_task(&nbfi_processTask_desc, NBFi_ProcessTasks, RELATIVE, MILLISECONDS(1));
}

void NBFi_SlowDown_Process(uint16_t msec)
{
  nbfi_scheduler->__scheduler_add_task(&nbfi_processTask_desc, NBFi_ProcessTasks, RELATIVE, MILLISECONDS(msec));
}

static uint32_t NBFI_PhyTo_Delay(nbfi_phy_channel_t chan)
{
	/*const uint32_t NBFI_DL_DELAY_C_D[10] = {30000, 30000, 30000, 5000, 5000, 5000, 1000, 1000, 500, 500};
	const uint32_t NBFI_DL_DELAY_E[10] = {6000, 1000, 500, 500};

	if (chan > UL_DBPSK_25600_PROT_E)
		return NBFI_DL_DELAY_E[0];
	else if (chan >= UL_DBPSK_50_PROT_E)
		return NBFI_DL_DELAY_E[chan - UL_DBPSK_50_PROT_E];
	else if (chan >= UL_DBPSK_50_PROT_C)
		return NBFI_DL_DELAY_C_D[chan - UL_DBPSK_50_PROT_C];
	return NBFI_DL_DELAY_E[0];*/

	switch(chan)
	{
        case DL_PSK_200:
        case UL_PSK_200:
                return 5700;
        case DL_PSK_500:
        case UL_PSK_500:
                return 2280;
        case DL_PSK_5000:
        case UL_PSK_5000:
                return 230;
        case DL_PSK_FASTDL:
        case UL_PSK_FASTDL:
                return 30;
        case UL_DBPSK_50_PROT_C:
        case UL_DBPSK_50_PROT_D:
        case DL_DBPSK_50_PROT_D:
                return 5900;
	case UL_DBPSK_50_PROT_E:
        case DL_DBPSK_50_PROT_E:
		return 5900;
        case UL_DBPSK_400_PROT_C:
        case UL_DBPSK_400_PROT_D:
        case DL_DBPSK_400_PROT_D:
                return 740;
	case UL_DBPSK_400_PROT_E:
	case DL_DBPSK_400_PROT_E:
		return 740;
        case UL_DBPSK_3200_PROT_D:
        case DL_DBPSK_3200_PROT_D:
                return 95;
	case UL_DBPSK_3200_PROT_E:
	case DL_DBPSK_3200_PROT_E:
		return 95;
        case UL_DBPSK_25600_PROT_D:
        case DL_DBPSK_25600_PROT_D:
	case UL_DBPSK_25600_PROT_E:
	case DL_DBPSK_25600_PROT_E:
                return 15;
        default:
                return 5900;
	}
}

static uint32_t NBFI_PhyToDL_ListenTime(nbfi_phy_channel_t chan)
{
	const uint32_t NBFI_DL_LISTEN_TIME[4] = {60000, 30000, 6000, 6000};

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
	const uint32_t NBFI_DL_ADD_RND_LISTEN_TIME[4] = {5000, 1000, 100, 100};

	if (chan > DL_DBPSK_25600_PROT_E)
		return NBFI_DL_ADD_RND_LISTEN_TIME[0];
	else if (chan >= DL_DBPSK_50_PROT_E)
		return NBFI_DL_ADD_RND_LISTEN_TIME[chan - DL_DBPSK_50_PROT_E];
	else if (chan >= DL_DBPSK_50_PROT_D)
		return NBFI_DL_ADD_RND_LISTEN_TIME[chan - DL_DBPSK_50_PROT_D];
	return NBFI_DL_ADD_RND_LISTEN_TIME[0];
}


static uint32_t NBFi_UL_Delivery_Time(nbfi_phy_channel_t chan)
{

  if(nbfi.wait_ack_timeout) return nbfi.wait_ack_timeout/2;
  else return NBFI_PhyTo_Delay(chan);

}

static uint32_t NBFi_DL_Delivery_Time(nbfi_phy_channel_t chan)
{
  if(nbfi.wait_ack_timeout) return nbfi.wait_ack_timeout/2;
  else return NBFI_PhyToDL_ListenTime(chan) + rand()%NBFI_PhyToDL_AddRndListenTime(chan);
}

