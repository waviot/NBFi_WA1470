#ifndef AT_USER_H
#define AT_USER_H

#include "nbfi_at_server.h"

uint16_t user_defined_at_command_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[]);

extern _Bool phobos_scan_mode;

#endif //AT_USER_H