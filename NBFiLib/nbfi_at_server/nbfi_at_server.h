#ifndef NBFI_AT_SERVER_H
#define NBFI_AT_SERVER_H

#include "nbfi_at_server_types.h"
#include "nbfi_at_server_tags.h"
#include "nbfi_at_server_handlers.h"



extern _Bool nbfi_at_server_echo_mode;

extern uint8_t at_server_last_rx_pkt[240];
extern uint8_t at_server_last_rx_pkt_len ;

extern nbfi_at_server_handler_t nbfi_at_server_user_defined_handler;

uint16_t nbfi_at_server_parse_line(uint8_t *, uint16_t);
uint16_t nbfi_at_server_parse_char(uint8_t input_char, uint8_t ** reply_data_ptr);
uint16_t nbfi_at_server_return_str(uint8_t *data, const char* str, nbfi_at_server_result_t result);
uint16_t nbfi_at_server_return_uint(uint8_t *data, uint32_t n, nbfi_at_server_result_t result);
uint16_t nbfi_at_server_return_hex_str(uint8_t *data, uint8_t *str, uint8_t size, nbfi_at_server_result_t result);
uint16_t nbfi_at_server_return_common_help(uint8_t *reply);
uint16_t nbfi_at_server_return_tag_help(uint8_t *reply, const char* str);
void nbfi_at_server_receive_complete(uint8_t * data, uint16_t length);

void nbfi_at_server_define_user_handler(nbfi_at_server_handler_t func);



#endif // NBFI_AT_SEVER_H
