#include "nbfi.h"
#include <stdlib.h>
#include <string.h>

#ifdef NBFI_USE_MALLOC
nbfi_transport_packet_t* nbfi_TX_pktBuf[NBFI_TX_PKTBUF_SIZE];
nbfi_transport_packet_t* nbfi_RX_pktBuf[NBFI_RX_PKTBUF_SIZE];
#else
nbfi_transport_packet_t  nbfi_TX_pktBuf[NBFI_TX_PKTBUF_SIZE];
nbfi_transport_packet_t  nbfi_RX_pktBuf[NBFI_RX_PKTBUF_SIZE];
#endif

uint8_t     nbfi_TXbuf_head = 0;

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

/*
void NBFi_Free_Packet(nbfi_transport_packet_t* pkt)
{
    #ifdef NBFI_USE_MALLOC
    free(pkt);
    //pkt = 0;
    #else
    if(pkt->state = PACKET_FREE);
    #endif
}*/

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

    }

    nbfi_transport_packet_t* pointer = NBFi_Get_RX_Packet_Ptr(ptr);
    
    if(pointer)
    {
        //NBFi_Free_Packet(pointer);
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
            return pointer;
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

            return pointer;
        }
    }
    return 0;
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
            if(nbfi.mack_mode < MACK_2) break;
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

uint8_t NBFi_Calc_Queued_Sys_Packets_With_Type(uint8_t type)
{
    nbfi_transport_packet_t* pointer;
    uint8_t num = 0;
    for(uint8_t i = nbfi_TXbuf_head - NBFI_TX_PKTBUF_SIZE; i != nbfi_TXbuf_head; i++)
    {
        uint8_t ptr = i%NBFI_TX_PKTBUF_SIZE;
        pointer = NBFi_Get_TX_Packet_Ptr(ptr);
        if(pointer == 0) continue;
        if(!pointer->phy_data.SYS) continue;
        if(pointer->phy_data.payload[0] != type) continue;

        switch(pointer->state )
        {
        case PACKET_WAIT_ACK:
        case PACKET_QUEUED:
        case PACKET_QUEUED_AGAIN:
        case PACKET_NEED_TO_SEND_RIGHT_NOW:
        case PACKET_SENT_NOACKED:
            num++;
            break;
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
        if(pointer->phy_data.SYS && (pointer->phy_data.payload[0] != SYSTEM_PACKET_GROUP_START_OLD)&&(pointer->phy_data.payload[0] != SYSTEM_PACKET_GROUP_START)&& !(pointer->phy_data.payload[0] & 0x80))
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
    nbfi_transport_frame_t *rec_pkt = &(NBFi_Get_RX_Packet_Ptr(nbfi_state.DL_iter&0x1f)->phy_data);

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

                if((pkt->phy_data.MULTI)&&(pkt->phy_data.SYS)&&((pkt->phy_data.payload[0] == SYSTEM_PACKET_GROUP_START_OLD)||(pkt->phy_data.payload[0] == SYSTEM_PACKET_GROUP_START))) //the start packet of the groupe
                {
                    total_groupe_len = pkt->phy_data.payload[1];
                    *total_length = pkt->phy_data_length - 2;
                    continue;
                }
                else
                {
                    if((pkt->phy_data.MULTI == 1)&&!(pkt->phy_data.SYS)) break;
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

void NBFi_Clear_RX_Buffer()
{
    for(uint8_t i = 0; i != NBFI_RX_PKTBUF_SIZE; i++ )
    {
        if(NBFi_Get_RX_Packet_Ptr(i)->state != PACKET_RECEIVED) NBFi_Get_RX_Packet_Ptr(i)->state = PACKET_CLEARED;
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
    pkt->phy_data.payload[5] = nbfi_last_snr;
    pkt->phy_data.payload[6] = (uint8_t)(noise + 150);
    pkt->phy_data.payload[7] = you_should_dl_power_step_down + you_should_dl_power_step_up + (nbfi.tx_pwr & 0x3f);
    pkt->phy_data.ITER = iter;
    pkt->phy_data.header |= SYS_FLAG;
    pkt->handshake = HANDSHAKE_NONE;
    pkt->state = PACKET_NEED_TO_SEND_RIGHT_NOW;
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
    return 1;
}

_Bool NBFi_Config_Send_Sync(_Bool ack)
{
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
    else if(/*(act_pkt->mack_num > 1) && */(mask == 0))  //all packets delivered, send message to clear receiver's input buffer
    {
         NBFi_Send_Clear_Cmd(nbfi_active_pkt->phy_data.ITER);
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
    }
    return 0;
}

uint8_t NBFi_Get_TX_Iter()
{
    return nbfi_state.UL_iter&0x1f;
}

uint8_t NBFi_Get_Retry_Number()
{
  if((NBFi_Phy_To_Bitrate(nbfi.rx_phy_channel) >= 3200) && (NBFi_Phy_To_Bitrate(nbfi.tx_phy_channel) >= 3200))
    return (nbfi.num_of_retries&0x0f) + (nbfi.num_of_retries >> 4);
  else return nbfi.num_of_retries&0x0f;
}