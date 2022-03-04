
#ifndef RADIO_H_
#define RADIO_H_

#include "wa1470_hal.h"
#include "scheduler_hal.h"
#include "nbfi_hal.h"


#define MANUFACTURER_ID         0x8888  //Waviot
#define HARDWARE_TYPE_ID        48      //WRM1470DevKit
#define PROTOCOL_ID             0//NBFI_MULTIPORT_PROTOCOL_ID //0       //undefined


typedef struct
{
  uint32_t id;
  uint8_t key[32];
} nbfi_device_id_and_key_st;

extern nbfi_device_id_and_key_st sr_server_modem_id_and_key;

void radio_init(void);

void radio_switch_to_from_short_range(_Bool en, _Bool client_or_server);

void radio_load_id_and_key_of_sr_server(nbfi_device_id_and_key_st *data);
void radio_save_id_and_key_of_sr_server(nbfi_device_id_and_key_st *data);

#endif /* RADIO_H_ */
