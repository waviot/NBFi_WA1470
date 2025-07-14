#include "nbfi.h"

#ifdef NBFI_USE_MALLOC
nbfi_transport_packet_t* nbfi_TX_pktBuf[NBFI_TX_PKTBUF_SIZE];
nbfi_transport_packet_t* nbfi_RX_pktBuf[NBFI_RX_PKTBUF_SIZE];
#else
nbfi_transport_packet_t  nbfi_TX_pktBuf[NBFI_TX_PKTBUF_SIZE];
nbfi_transport_packet_t  nbfi_RX_pktBuf[NBFI_RX_PKTBUF_SIZE];
#endif

uint8_t     nbfi_TXbuf_head = 0;

uint16_t sent_id = 0;
uint8_t     nbfi_sent_buf_head = 0;
nbfi_ul_sent_status_t NBFi_sent_UL_stat_Buf[NBFI_SENT_STATUSES_BUF_SIZE];

uint16_t receive_id = 0;
uint8_t  nbfi_receive_buf_head = 0;
nbfi_dl_received_t NBFi_received_DL_Buf[NBFI_RECEIVED_BUF_SIZE];

nbfi_transport_packet_t* NBFi_Get_TX_Packet_Ptr(uint8_t index)
{
    #ifdef NBFI_USE_MALLOC
    return nbfi_TX_pktBuf[index];
    #else
    if(nbfi_TX_pktBuf[index].state == PACKET_FREE) return 0;
    else return &nbfi_TX_pktBuf[index];
    #endif
}

nbfi_transport_packet_t* NBFi_Get_RX_Packet_Ptr(uint8_t index)
{
    #ifdef NBFI_USE_MALLOC
    return nbfi_RX_pktBuf[index];
    #else
    if(nbfi_RX_pktBuf[index].state == PACKET_FREE) return 0;
    else return &nbfi_RX_pktBuf[index];
    #endif
}

nbfi_transport_packet_t* NBFi_AllocateTxPkt(uint8_t payload_length)
{
    uint8_t ptr = nbfi_TXbuf_head%NBFI_TX_PKTBUF_SIZE;

    nbfi_transport_packet_t* pointer = NBFi_Get_TX_Packet_Ptr(ptr);

    if(pointer)
    {
        switch(pointer->state)
        {
        case PACKET_QUEUED:
        case PACKET_QUEUED_AGAIN:
        case PACKET_WAIT_ACK:
        case PACKET_NEED_TO_SEND_RIGHT_NOW:
        case PACKET_SENT_NOACKED:
            return 0;   // tx buffer is full
        default:  break;
        }

        #ifdef NBFI_USE_MALLOC
        free(pointer);
        nbfi_TX_pktBuf[ptr] = 0;
        #else
        pointer->state = PACKET_FREE;
        #endif
    }

    #ifdef NBFI_USE_MALLOC
    pointer = (nbfi_transport_packet_t *) malloc(sizeof(nbfi_transport_packet_t) + payload_length);

    if(!pointer)
    {
        return 0;
    }
    nbfi_TX_pktBuf[ptr] = pointer;
    #else
    if(payload_length > NBFI_PACKET_SIZE) return 0;
    pointer = &nbfi_TX_pktBuf[ptr];
    #endif


    pointer->state = PACKET_ALLOCATED;

    pointer->phy_data_length = payload_length;

    pointer->handshake = HANDSHAKE_NONE;

    pointer->retry_num = 0;

    pointer->mack_num = 0;

    pointer->phy_data.header = 0;

    pointer->id = 0;

    pointer->ts = nbfi_scheduler->__scheduler_curr_time();

    nbfi_TXbuf_head++;

    return pointer;

}



nbfi_transport_packet_t* NBFi_AllocateRxPkt(uint8_t header, uint8_t payload_length)
{
    uint8_t ptr = header&0x1f;

    switch(nbfi_active_pkt->state)
    {
        case PACKET_QUEUED:
        case PACKET_QUEUED_AGAIN:
        case PACKET_WAIT_ACK:
            nbfi_active_pkt = &idle_pkt;
            break;
        default: break;

    }

    nbfi_transport_packet_t* pointer = NBFi_Get_RX_Packet_Ptr(ptr);

    if(pointer)
    {
        #ifdef NBFI_USE_MALLOC
        free(pointer);
        nbfi_RX_pktBuf[ptr] = 0;
        #else
        pointer->state = PACKET_FREE;
        #endif
    }

    #ifdef NBFI_USE_MALLOC
    pointer = (nbfi_transport_packet_t *) malloc(sizeof(nbfi_transport_packet_t) + payload_length);
    if(!pointer)
    {
        return 0;
    }
    nbfi_RX_pktBuf[ptr] = pointer;
    #else
    if(payload_length > NBFI_PACKET_SIZE) return 0;
    pointer = &nbfi_RX_pktBuf[ptr];
    #endif


    nbfi_state.DL_iter = ptr;

    pointer->state = PACKET_ALLOCATED;

    pointer->phy_data_length = payload_length;

    pointer->phy_data.header = header;

    pointer->ts = nbfi_scheduler->__scheduler_curr_time();


    return pointer;

}


nbfi_transport_packet_t* NBFi_GetQueuedTXPkt()
{
    nbfi_transport_packet_t* pointer;
    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        switch(pointer->state )
        {
        case PACKET_NEED_TO_SEND_RIGHT_NOW:
            return pointer;
        default: break;
        }
    }

    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        switch(pointer->state )
        {
        case PACKET_QUEUED_AGAIN:
            goto end;
        default: break;
        }
    }

    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        switch(pointer->state )
        {
        case PACKET_WAIT_ACK:
            if(pointer == nbfi_active_pkt) continue;
             pointer->state = PACKET_QUEUED_AGAIN;
        case PACKET_QUEUED:
            goto end;
        default: break;
        }
    }
    return 0;

end:
   if((nbfi.additional_flags&NBFI_FLG_SEND_IN_RESPONSE))
   {
    if(!uplink_received_after_send) return 0;
   }
   return pointer;
}


void NBFi_TxPacket_Free(nbfi_transport_packet_t* pkt)
{
    nbfi_transport_packet_t* pointer;
    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer != pkt) continue;
        #ifdef NBFI_USE_MALLOC
        free(pointer);
        nbfi_TX_pktBuf[ptr] = 0;
        #else
        pkt->state = PACKET_FREE;
        #endif
     }

}

void NBFi_RxPacket_Free(nbfi_transport_packet_t* pkt)
{
    nbfi_transport_packet_t* pointer;
    for(uint8_t i = 0; i != NBFI_RX_PKTBUF_SIZE; i++)
    {
        pointer = NBFi_Get_RX_Packet_Ptr(i);
        if(pointer != pkt) continue;
        #ifdef NBFI_USE_MALLOC
        free(pointer);
        nbfi_RX_pktBuf[i] = 0;
        #else
        pkt->state = PACKET_FREE;
        #endif
    }

}

uint8_t NBFi_Packets_To_Send()
{
    nbfi_transport_packet_t* pointer;

    uint8_t packets_free = 0;

    for(uint16_t i = nbfi_TXbuf_head; i != (nbfi_TXbuf_head + NBFI_TX_PKTBUF_SIZE); i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;

        pointer = NBFi_Get_TX_Packet_Ptr(ptr);

        if(pointer == 0)
        {
            packets_free++;
            continue;
        }
        switch(pointer->state )
        {
        case PACKET_WAIT_ACK:
            if(pointer == nbfi_active_pkt) break;
             pointer->state = PACKET_QUEUED_AGAIN;
        case PACKET_QUEUED:
        case PACKET_QUEUED_AGAIN:
        case PACKET_NEED_TO_SEND_RIGHT_NOW:
            break;
        case PACKET_SENT_NOACKED:
            if((nbfi.mack_mode < MACK_2) && (nbfi_active_pkt->state != PACKET_FREE)) break;
        default:
            packets_free++;
            continue;
        }
        break;
    }

    if((transmit == 1) && (packets_free == NBFI_TX_PKTBUF_SIZE))
    {
        packets_free--;
    }

    return NBFI_TX_PKTBUF_SIZE - packets_free;
}



void NBFi_Close_Active_Packet()
{

    nbfi_transport_packet_t* pointer;

    nbfi_active_pkt->state = PACKET_LOST;

    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        if(pointer->state == PACKET_SENT_NOACKED) pointer->state = PACKET_LOST;
    }
}

uint8_t NBFi_Calc_Packets_With_State(uint8_t state)
{
    nbfi_transport_packet_t* pointer;
    uint8_t num = 0;
    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        if(pointer->state == state) num++;
    }
    return num;
}

uint8_t NBFi_Calc_Queued_Sys_Packets_With_Type(uint8_t type, _Bool clean)
{
    nbfi_transport_packet_t* pointer;
    uint8_t num = 0;
    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        if(!pointer->phy_data.SYSTEM) continue;
        if(pointer->phy_data.payload[0] != type) continue;

        switch(pointer->state )
        {
        case PACKET_WAIT_ACK:
        case PACKET_QUEUED:
        case PACKET_QUEUED_AGAIN:
        case PACKET_NEED_TO_SEND_RIGHT_NOW:
        case PACKET_SENT_NOACKED:
            num++;
            if(clean) NBFi_TxPacket_Free(pointer);
            break;
        default: break;
        }
    }
    return num;
}


nbfi_transport_packet_t* NBFi_GetSentTXPkt_By_Iter(uint8_t iter)
{
    nbfi_transport_packet_t* pointer;

    for(uint8_t i = (nbfi_TXbuf_head) - 1; i != (uint8_t)(nbfi_TXbuf_head  - NBFI_TX_PKTBUF_SIZE - 1); i--)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        if(pointer->phy_data.SYSTEM && (pointer->phy_data.payload[0] != SYSTEM_PACKET_GROUP_START_OLD)&&(pointer->phy_data.payload[0] != SYSTEM_PACKET_GROUP_START)&& !(pointer->phy_data.payload[0] & 0x80))
        {
            continue;
        }
        switch(pointer->state)
        {
        case PACKET_SENT:
        case PACKET_SENT_NOACKED:
        case PACKET_WAIT_ACK:
        case PACKET_LOST:
        case PACKET_DELIVERED:
            if(pointer->phy_data.ITER == iter)
            {
                return pointer;
            }
            break;
        default: break;
        }

    }
    return 0;
}


uint32_t NBFi_Get_RX_ACK_Mask()
{

    nbfi_transport_packet_t* pointer;
    uint32_t mask = 0;
    uint32_t one = 1;
    for(uint8_t i = ((nbfi_state.DL_iter - 1)&0x1f); (i&0x1f) != (nbfi_state.DL_iter&0x1f); i-- , one <<= 1 )
    {
        pointer = NBFi_Get_RX_Packet_Ptr(i&0x1f);
        if(!pointer) continue;
        switch(pointer->state)
        {
            case PACKET_RECEIVED:
            case PACKET_PROCESSED:
                mask |= one;
                break;
            default:
                break;
        }
    }
    return mask;
}

_Bool NBFi_Check_RX_Packet_Duplicate(nbfi_transport_frame_t * pkt, uint8_t len)
{
    nbfi_transport_packet_t *p = NBFi_Get_RX_Packet_Ptr(nbfi_state.DL_iter&0x1f);
    if(p == 0) return 0;
    nbfi_transport_frame_t *rec_pkt = &(p->phy_data);
    for(uint8_t i = 0; i != len; i++)
    {
        if(((uint8_t*)rec_pkt)[i] != ((uint8_t*)pkt)[i]) return 0;
    }
    return 1;
}


nbfi_transport_packet_t* NBFi_Get_QueuedRXPkt(uint8_t *groupe, uint16_t *total_length)
{
    nbfi_transport_packet_t* pkt;

    uint32_t i;
    for(i = nbfi_state.DL_iter + 1; i <= (nbfi_state.DL_iter + NBFI_RX_PKTBUF_SIZE*2); i++ )
    {
        *groupe = 0;
        *total_length = 0;
        uint8_t total_groupe_len = 0;

        while((pkt = NBFi_Get_RX_Packet_Ptr((i + *groupe)&0x1f)) && pkt->state == PACKET_RECEIVED)
        {
            if((*groupe) == 0)
            {
                *groupe = 1;

                if((pkt->phy_data.MULTI)&&(pkt->phy_data.SYSTEM)&&((pkt->phy_data.payload[0] == SYSTEM_PACKET_GROUP_START_OLD)||(pkt->phy_data.payload[0] == SYSTEM_PACKET_GROUP_START))) //the start packet of the groupe
                {
                    total_groupe_len = pkt->phy_data.payload[1];
                    *total_length = pkt->phy_data_length - 2;
                    continue;
                }
                else
                {
                    if((pkt->phy_data.MULTI == 1)&&!(pkt->phy_data.SYSTEM)) break;
                    //single packet
                    *total_length = pkt->phy_data_length;
                    pkt->state = PACKET_PROCESSING;
                    break;
                }
            }
            (*total_length) += pkt->phy_data_length;
            (*groupe)++;

            if((pkt->phy_data.MULTI) && ((*total_length) < total_groupe_len) && ((*groupe) < NBFI_RX_PKTBUF_SIZE - 1))
            {
                    continue;
            }
            else
            {       if(pkt->phy_data.MULTI && ((*total_length) >= total_groupe_len))
                    {
                        (*total_length) = total_groupe_len;
                        pkt->state = PACKET_PROCESSING;
                        break;
                    }

            }
            break;

        }

        if((*groupe) && (NBFi_Get_RX_Packet_Ptr((i + (*groupe) - 1)&0x1f)->state == PACKET_PROCESSING))
        {
            return NBFi_Get_RX_Packet_Ptr(i&0x1f);
        }

    }
    return 0;
}

uint32_t ts_curr;
uint32_t ts_pack;
void NBFi_Clear_RX_Buffer(int8_t besides, uint32_t time_expired)
{
    for(uint8_t i = 0; i != NBFI_RX_PKTBUF_SIZE; i++ )
    {
      	if(NBFi_Get_RX_Packet_Ptr(i) == 0) continue;
        if ((besides != -1) && (besides == NBFi_Get_RX_Packet_Ptr(i)->phy_data.ITER)) NBFi_Get_RX_Packet_Ptr(i)->state = PACKET_RECEIVED;
        else
	{
	  if((time_expired == 0) || ((ts_curr = nbfi_scheduler->__scheduler_curr_time()) - NBFi_Get_RX_Packet_Ptr(i)->ts) > time_expired)
	  {
	    if(time_expired)
	    {
	      ts_pack = NBFi_Get_RX_Packet_Ptr(i)->ts;
	    }
	    NBFi_Get_RX_Packet_Ptr(i)->state = PACKET_CLEARED;  //clear all old packets
	  }
	  else
	  {
	    	//ts_pack = NBFi_Get_RX_Packet_Ptr(i)->ts;

	  }
	}
    }
}

void NBFi_Clear_TX_Buffer()
{
    nbfi_transport_packet_t* pointer;
    for(uint8_t i = 0; i != NBFI_TX_PKTBUF_SIZE; i++ )
    {
        pointer = NBFi_Get_TX_Packet_Ptr(i);
        if(pointer)
        {
            #ifdef NBFI_USE_MALLOC
            free(pointer);
            nbfi_TX_pktBuf[i] = 0;
            #else
            pointer->state = PACKET_FREE;
            #endif
        }
    }
    nbfi_active_pkt = &idle_pkt;
}

void NBFi_Send_Clear_Cmd(uint8_t iter)
{
    nbfi_transport_packet_t* pkt =  NBFi_AllocateTxPkt(8);
    if(!pkt) return;
    pkt->phy_data.payload[0] = SYSTEM_PACKET_CLEAR_EXT; //clear RX buffer
    pkt->phy_data.payload[5] = nbfi_state.last_snr;
    pkt->phy_data.payload[6] = (uint8_t)(noise + 150);
    pkt->phy_data.payload[7] = you_should_dl_power_step_down + you_should_dl_power_step_up + (nbfi.tx_pwr & 0x3f);
    pkt->phy_data.ITER = iter;
    pkt->phy_data.header |= SYS_FLAG;
    pkt->handshake = HANDSHAKE_NONE;
    pkt->state = PACKET_NEED_TO_SEND_RIGHT_NOW;

                    


    #ifdef NBFI_LOG
                sprintf(nbfi_log_string, "%05u: Send clear packet ", (uint16_t)(nbfi_scheduler->__scheduler_curr_time()&0xffff));
                nbfi_hal->__nbfi_log_send_str(nbfi_log_string);
#endif


}


_Bool NBFi_Config_Send_Mode(_Bool ack, uint8_t param)
{
    nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);

    if(!ack_pkt)
    {
        return 0;
    }
    ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_CONFIG;
    ack_pkt->phy_data.payload[1] = (READ_PARAM_CMD << 6) + param;
    NBFi_Config_Parser(&ack_pkt->phy_data.payload[1]);
    ack_pkt->phy_data.ITER = nbfi_state.UL_iter & 0x1f;;
    ack_pkt->phy_data.header |= SYS_FLAG;
    if(ack)
    {
        ack_pkt->handshake = nbfi.handshake_mode;
        ack_pkt->phy_data.header |= ACK_FLAG;
    }
    ack_pkt->state = PACKET_NEED_TO_SEND_RIGHT_NOW;
    NBFi_Force_process();
    return 1;
}


_Bool NBFi_Config_Send_Sync(_Bool ack)
{

    NBFi_Calc_Queued_Sys_Packets_With_Type(SYSTEM_PACKET_SYNC, 1); //clean previously queued sync packets
    nbfi_transport_packet_t* ack_pkt =  NBFi_AllocateTxPkt(8);

    if(!ack_pkt)
    {
        return 0;
    }
    ack_pkt->phy_data.payload[0] = SYSTEM_PACKET_SYNC;
    ack_pkt->phy_data.payload[1] = nbfi.mode + (NBFI_REV << 3);
    ack_pkt->phy_data.payload[2] = nbfi.tx_phy_channel;
    ack_pkt->phy_data.payload[3] = nbfi.rx_phy_channel;
    ack_pkt->phy_data.payload[4] = (nbfi.nbfi_freq_plan.fp>>8);
    ack_pkt->phy_data.payload[5] = (nbfi.nbfi_freq_plan.fp&0xff);

    ack_pkt->phy_data.ITER = nbfi_state.UL_iter & 0x1f;;
    ack_pkt->phy_data.header |= SYS_FLAG;
    if(ack)
    {
        ack_pkt->handshake = nbfi.handshake_mode;
        ack_pkt->phy_data.header |= ACK_FLAG;
    }
    ack_pkt->state = PACKET_NEED_TO_SEND_RIGHT_NOW;
    return 1;
}


void NBFi_Resend_Pkt(nbfi_transport_packet_t* act_pkt, uint32_t mask)
{
    uint8_t iter = act_pkt->phy_data.ITER;
    uint32_t selection;
    mask = (~mask) << 1;
    selection = 0;
    for(uint8_t i = (act_pkt->mack_num&0x3f) - 1; i != 0; i-- )
    {
      selection |= (((uint32_t)1) << i);
    }
    mask &= selection;
    uint32_t one = 1;

    nbfi_transport_packet_t* pkt = 0, *last_resending_pkt = 0;

    for(uint8_t i = (act_pkt->mack_num&0x3f); i > 0; i--)
    {
        pkt = NBFi_GetSentTXPkt_By_Iter(iter&0x1f);

        if(!pkt)
        {
          break;
        }
        if(one&mask)
        {
          mask &= ~one;
          if(++pkt->retry_num > NBFi_Get_Retry_Number())
          {
            NBFi_Set_UL_Status(pkt->id, LOST);
            NBFi_Close_Active_Packet();
            //pkt->state = PACKET_LOST;
          }
          else
          {
            pkt->state = PACKET_QUEUED_AGAIN;
            if(last_resending_pkt == 0) last_resending_pkt = pkt;
            nbfi_state.fault_total++;
          }
        }
        else
        {
          pkt->state = PACKET_DELIVERED;
          nbfi_state.success_total++;
        }
        iter--;
        one <<= 1;
    }
    if(last_resending_pkt)
    {
        last_resending_pkt->phy_data.ACK = 1;
        last_resending_pkt->mack_num = act_pkt->mack_num - (act_pkt->phy_data.ITER - last_resending_pkt->phy_data.ITER);
        if(act_pkt->phy_data.ITER < last_resending_pkt->phy_data.ITER) last_resending_pkt->mack_num += 32;
        last_resending_pkt->mack_num |= 0x80;
    }
    else
    {
      if((mask == 0))  //all packets delivered, send message to clear receiver's input buffer
      {
           //if(!NBFi_GetQueuedTXPkt()) NBFi_SlowDown_Process(100);
           NBFi_Send_Clear_Cmd(nbfi_active_pkt->phy_data.ITER);
      }
      NBFi_Set_UL_Status(nbfi_active_pkt->id, DELIVERED);
   }

}

uint16_t NBFi_Phy_To_Bitrate(nbfi_phy_channel_t ch)
{
    switch(ch)
    {
    case DL_PSK_200:
    case UL_PSK_200:
        return 200;
    case DL_PSK_500:
    case UL_PSK_500:
        return 500;
    case DL_PSK_5000:
    case UL_PSK_5000:
        return 5000;
    case DL_PSK_FASTDL:
    case UL_PSK_FASTDL:
        return 57600;
    case UL_DBPSK_50_PROT_D:
    case UL_DBPSK_50_PROT_E:
    case DL_DBPSK_50_PROT_D:
        return 50;
    case UL_DBPSK_400_PROT_D:
    case UL_DBPSK_400_PROT_E:
    case DL_DBPSK_400_PROT_D:
        return 400;
    case UL_DBPSK_3200_PROT_D:
    case UL_DBPSK_3200_PROT_E:
    case DL_DBPSK_3200_PROT_D:
        return 3200;
    case UL_DBPSK_25600_PROT_D:
    case UL_DBPSK_25600_PROT_E:
    case DL_DBPSK_25600_PROT_D:
        return 25600;
    default: return 0; break;
    }

    return 0;
}

uint8_t NBFi_Get_TX_Iter()
{
    return nbfi_state.UL_iter&0x1f;
}

uint8_t NBFi_Get_Retry_Number()
{
  if((NBFi_Phy_To_Bitrate(nbfi.rx_phy_channel) >= 3200) && (NBFi_Phy_To_Bitrate(nbfi.tx_phy_channel) >= 400))
    return (nbfi.num_of_retries&0x0f) + (nbfi.num_of_retries >> 4);
  else return nbfi.num_of_retries&0x0f;
}


nbfi_ul_sent_status_t* NBFi_Queue_Next_UL(uint8_t flags)
{
  nbfi_ul_sent_status_t* next = &NBFi_sent_UL_stat_Buf[nbfi_sent_buf_head++%NBFI_SENT_STATUSES_BUF_SIZE];

  next->id = (++sent_id)?sent_id:(++sent_id);
  next->status = QUEUED;
  next->reported = 0;
  next->flags = flags;
  return next;
}

void NBFi_Set_UL_Status(uint8_t id, nbfi_ul_status_t status)
{
  for(uint8_t i = nbfi_sent_buf_head - NBFI_SENT_STATUSES_BUF_SIZE; i != nbfi_sent_buf_head; i++)
  {
    nbfi_ul_sent_status_t* ul = &NBFi_sent_UL_stat_Buf[i%NBFI_SENT_STATUSES_BUF_SIZE];
    if(ul->id && ((ul->id&0xff) == id))
    {
      ul->reported = 0;
      ul->status = status;
      break;
    }
  }
}

nbfi_ul_sent_status_t* NBFi_Get_Next_Unreported_UL(nbfi_ul_status_t status)
{
  for(uint8_t i = nbfi_sent_buf_head - NBFI_SENT_STATUSES_BUF_SIZE; i != nbfi_sent_buf_head; i++)
  {
    nbfi_ul_sent_status_t* ul = &NBFi_sent_UL_stat_Buf[i%NBFI_SENT_STATUSES_BUF_SIZE];
    if(!ul->reported && (ul->status == status))
    {
      ul->reported = 1;
      return ul;
    }
  }
  return 0;
}

nbfi_ul_sent_status_t* NBFi_Get_UL_status(uint16_t id, _Bool eight_bits_id)
{
  if(id == 0) return 0;
  for(uint8_t i = nbfi_sent_buf_head - NBFI_SENT_STATUSES_BUF_SIZE; i != nbfi_sent_buf_head; i++)
  {
    nbfi_ul_sent_status_t* ul = &NBFi_sent_UL_stat_Buf[i%NBFI_SENT_STATUSES_BUF_SIZE];

	if((ul->id == id)||(eight_bits_id&&((ul->id&0xff) == id)))
    {
      return ul;
    }
  }
  return 0;
}


void NBFi_Queue_Next_DL(uint8_t* data, uint16_t length)
{

  nbfi_dl_received_t *next = &NBFi_received_DL_Buf[nbfi_receive_buf_head++%NBFI_RECEIVED_BUF_SIZE];
  next->id = receive_id++;
  next->length = length;
  next->ready = 1;
  #ifdef NBFI_USE_MALLOC
  if(next->payload) free(next->payload);
  next->payload = (uint8_t *) malloc(length);
  if(!next->payload)
  {
    next->ready = 0;
    return;
  }
  #endif
  for(uint8_t i = 0; i!= length; i++) next->payload[i] = data[i];
}

uint8_t NBFi_Next_Ready_DL(uint8_t* data)
{
 for(uint8_t i = nbfi_receive_buf_head - NBFI_RECEIVED_BUF_SIZE; i != nbfi_receive_buf_head; i++)
  {
    nbfi_dl_received_t* dl = &NBFi_received_DL_Buf[i%NBFI_RECEIVED_BUF_SIZE];
    if(dl->ready)
    {
      for(uint8_t i = 0; i!= dl->length; i++) data[i] = dl->payload[i];
      dl->ready = 0;
      #ifdef NBFI_USE_MALLOC
      if(dl->payload) {free(dl->payload); dl->payload = 0;}
      #endif
      return dl->length;
    }
  }
  return 0;
}




