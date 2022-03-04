#include "multiport.h"

#define MULTIPORT_registered_protocols_size 5

MULTIPORT_receive_st MULTIPORT_registered_protocols[MULTIPORT_registered_protocols_size + 1] = {0};

_Bool MULTIPORT_register_protocol(uint8_t protocol_id, MULTIPORT_receive_handler_t handler)
{
    uint8_t i = 0;
    while((i < MULTIPORT_registered_protocols_size) && MULTIPORT_registered_protocols[i++].protocol_id);
    if(i == MULTIPORT_registered_protocols_size) return 0;
    MULTIPORT_registered_protocols[i].protocol_id = protocol_id;
    MULTIPORT_registered_protocols[i].rx_handler = handler;
    MULTIPORT_registered_protocols[i+1].protocol_id = 0;
    return 1;
}

nbfi_ul_sent_status_t   MULTIPORT_send_to_port(uint8_t* payload, uint8_t length, uint8_t flags, uint8_t port)
{
    uint8_t buf[256];
    buf[0] = port;
    memcpy((void*)(&buf[1]), (const void*)(payload), length);
    return NBFi_Send5(buf, length + 1, flags);
}


void MULTIPORT_receive_complete(uint8_t * data, uint16_t length)
{
        uint8_t i = 0;

        while((i < MULTIPORT_registered_protocols_size) && MULTIPORT_registered_protocols[i++].protocol_id != data[0]);

        if((i == MULTIPORT_registered_protocols_size) || (MULTIPORT_registered_protocols[i].protocol_id == 0)) return;

        MULTIPORT_registered_protocols[i].rx_handler(&data[1], length -1);
}