#ifndef NBFI_TRANSPORT_H
#define NBFI_TRANSPORT_H


#define DRXLISTENAFTERSEND  20

#define WAITALITTLEBIT  (NBFi_RF_get_noise_calc_duration() + 2000) //3000

extern nbfi_state_t nbfi_state;
extern nbfi_transport_packet_t* nbfi_active_pkt;
extern nbfi_transport_packet_t idle_pkt;
extern uint8_t nbfi_last_snr;
extern int16_t noise;
extern uint32_t nbfi_rtc;
extern _Bool rtc_synchronised;
extern uint32_t info_timer;
extern _Bool uplink_received_after_send;
extern uint32_t last_ack_send_ts;

void            NBFI_Transport_Init();
void            NBFi_Force_process();
void            NBFi_SlowDown_Process(uint16_t msec);
void            NBFi_ProcessRxPackets();
void            NBFi_update_RTC();
void            NBFi_set_RTC_irq(uint32_t time);
void            NBFi_TX_Finished();
void            NBFi_run_Receive_Timeout_cb(uint32_t timeout);

#endif // NBFI_TRANSPORT_H
