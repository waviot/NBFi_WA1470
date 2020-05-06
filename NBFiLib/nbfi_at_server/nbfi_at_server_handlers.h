#ifndef NBFI_AT_SERVER_HANDLERS_H
#define NBFI_AT_SERVER_HANDLERS_H


uint16_t nbfi_at_server_common_handler(nbfi_at_server_tags_t tag, uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[]);

#endif //NBFI_AT_SERVER_HANDLERS_H