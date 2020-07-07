#include "at_user.h"
#include "defines.h"
#include <stdlib.h>
#include "rs485_uart.h"

uint16_t user_defined_at_command_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{    
  switch(atoi((const char*)sub_param))
  {
  #ifdef PHOBOS_HDLC_FORWARDER
  case 0:
    switch(action)
    {
      case AT_GET:
      case AT_CMD:
        nbfi_at_server_return_uint(reply, phobos_hdlc_mode, AT_OK);
      case AT_SET:
        {
          phobos_hdlc_mode = (uint8_t)atoi((char const*)value[0]);
          return nbfi_at_server_return_str(reply, 0, AT_OK);
        }
      case AT_HELP:
        return nbfi_at_server_return_tag_help(reply, "get/set the phobos hdlc mode(0-disabled, 1-enabled)");
      default:
        break;
    }
    break;
  #endif
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}
