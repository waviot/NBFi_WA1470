#ifndef NBFI_TRANSPORT_H
#define NBFI_TRANSPORT_H

extern rx_handler_t  rx_handler;

extern nbfi_state_t nbfi_state;
extern nbfi_transport_packet_t* nbfi_active_pkt;
extern nbfi_transport_packet_t idle_pkt;
extern uint8_t nbfi_last_snr;
extern int16_t noise;

void NBFI_Transport_Init();
void NBFi_Force_process();

#endif // NBFI_TRANSPORT_H
