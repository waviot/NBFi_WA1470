#ifndef NBFI_TRANSPORT_H
#define NBFI_TRANSPORT_H

//extern rx_handler_t  rx_handler;

extern nbfi_state_t nbfi_state;
extern nbfi_transport_packet_t* nbfi_active_pkt;
extern nbfi_transport_packet_t idle_pkt;
extern uint8_t nbfi_last_snr;
extern int16_t noise;
extern uint32_t nbfi_rtc;
extern _Bool rtc_synchronised;
extern uint32_t info_timer;

void            NBFI_Transport_Init();
void            NBFi_Force_process();
void            NBFi_ProcessRxPackets();
void            NBFi_update_RTC();
void            NBFi_set_RTC_irq(uint32_t time);
void            NBFi_TX_Finished();
#endif // NBFI_TRANSPORT_H
