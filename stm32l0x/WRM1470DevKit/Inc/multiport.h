#ifndef __MULTIPORT_H__
#define __MULTIPORT_H__

#include "nbfi.h"

typedef void (*MULTIPORT_receive_handler_t)(uint8_t*, uint16_t);

typedef struct
{
  uint8_t protocol_id;
  MULTIPORT_receive_handler_t rx_handler;
} MULTIPORT_receive_st;

_Bool                   MULTIPORT_register_protocol(uint8_t protocol_id, MULTIPORT_receive_handler_t handler);
nbfi_ul_sent_status_t   MULTIPORT_send_to_port(uint8_t* payload, uint8_t length, uint8_t flags, uint8_t port);
void                    MULTIPORT_receive_complete(uint8_t * data, uint16_t length);

#endif //MULTIPORT_H_