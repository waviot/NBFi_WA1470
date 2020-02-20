#ifndef NBFI_TRANSPORT_MISC_H
#define NBFI_TRANSPORT_MISC_H

#define NBFI_TX_PKTBUF_SIZE     64
#define NBFI_RX_PKTBUF_SIZE     32


#ifdef NBFI_USE_MALLOC
extern nbfi_transport_packet_t* nbfi_TX_pktBuf[NBFI_TX_PKTBUF_SIZE];
extern nbfi_transport_packet_t* nbfi_RX_pktBuf[NBFI_RX_PKTBUF_SIZE];
#else
extern nbfi_transport_packet_t  nbfi_TX_pktBuf[NBFI_TX_PKTBUF_SIZE];
extern nbfi_transport_packet_t  nbfi_RX_pktBuf[NBFI_RX_PKTBUF_SIZE];
#endif

nbfi_transport_packet_t*            NBFi_Get_TX_Packet_Ptr(uint8_t index);
nbfi_transport_packet_t*            NBFi_Get_RX_Packet_Ptr(uint8_t index);
//void                                NBFi_Free_Packet(nbfi_transport_packet_t* pkt);
nbfi_transport_packet_t*            NBFi_AllocateTxPkt(uint8_t payload_length);
void                                NBFi_TxPacket_Free(nbfi_transport_packet_t* pkt);
void                                NBFi_RxPacket_Free(nbfi_transport_packet_t* pkt);
nbfi_transport_packet_t*            NBFi_AllocateRxPkt(uint8_t header, uint8_t payload_length);
nbfi_transport_packet_t*            NBFi_GetQueuedTXPkt();
void                                NBFi_Close_Active_Packet();
_Bool                               NBFi_Check_RX_Packet_Duplicate(nbfi_transport_frame_t * pkt, uint8_t len);
nbfi_transport_packet_t*            NBFi_Get_QueuedRXPkt(uint8_t *groupe, uint16_t *total_length);
nbfi_transport_packet_t*            NBFi_GetSentTXPkt_By_Iter(uint8_t iter);
uint8_t                             NBFi_Calc_Queued_Sys_Packets_With_Type(uint8_t type);
uint8_t                             NBFi_Calc_Packets_With_State(uint8_t state);
uint32_t                            NBFi_Get_RX_ACK_Mask();
void                                NBFi_Resend_Pkt(nbfi_transport_packet_t* act_pkt, uint32_t mask);
void                                NBFi_Clear_RX_Buffer();
void                                NBFi_Clear_TX_Buffer();
void                                NBFi_Send_Clear_Cmd(uint8_t iter);
_Bool                               NBFi_Config_Send_Mode(_Bool, uint8_t param);
_Bool                               NBFi_Config_Send_Sync(_Bool);
uint16_t                            NBFi_Phy_To_Bitrate(nbfi_phy_channel_t ch);
uint8_t                             NBFi_Get_TX_Iter();
uint8_t                             NBFi_Get_Retry_Number();
#endif //NBFI_TRANSPORT_MISC_H