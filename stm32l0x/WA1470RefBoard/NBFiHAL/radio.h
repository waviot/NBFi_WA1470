
#ifndef RADIO_H_
#define RADIO_H_

#include "wa1470_hal.h"
#include "scheduler_hal.h"
#include "nbfi_hal.h"

typedef struct 
{
  uint32_t id;
  uint8_t key[32];
} nbfi_device_id_and_key_st;


extern nbfi_device_id_and_key_st sr_server_modem_id_and_key;
//extern uint32_t sr_modem_id;
//extern uint32_t sr_key[8];

void radio_init(void);
void radio_switch_to_from_short_range(_Bool en, _Bool client_or_server);
void radio_load_id_and_key_of_sr_server(nbfi_device_id_and_key_st *data);
void radio_save_id_and_key_of_sr_server(nbfi_device_id_and_key_st *data);

#ifdef PHOBOS_HDLC_FORWARDER
void radio_switch_to_from_phobos_scan_mode(_Bool en);
#endif

#endif /* RADIO_H_ */
