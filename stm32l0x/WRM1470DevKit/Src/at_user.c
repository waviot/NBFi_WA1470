#include "at_user.h"
#include "defines.h"
#include <stdlib.h>
#include "rs485_uart.h"
#include "radio.h"
#include "settings.h"



uint16_t user_defined_at_command_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(atoi((const char*)sub_param))
  {
  case 0:
    switch(action)
    {
        case AT_GET:
        case AT_CMD:
          return  nbfi_at_server_return_uint(reply, global_settings.uart_mode, AT_OK);
        case AT_SET:
          {
            uint8_t mode = (uint8_t)atoi((char const*)value[0]);
            if(mode == 0 || mode > UART_MODE_NBFI_UNUSED) break;
            global_settings.uart_mode = (uart_mode_t)mode;
            save_global_settings();
            return nbfi_at_server_return_str(reply, 0, AT_OK);
          }
        case AT_HELP:
          return nbfi_at_server_return_tag_help(reply, "get/set the UART mode(1-ATCMD,2-TRANSP,3-UNUSED)");
        default:
          break;
    }
   case 1:
    switch(action)
    {
        case AT_GET:
        case AT_CMD:
          return  nbfi_at_server_return_uint(reply, global_settings.uart_bitrate, AT_OK);
        case AT_SET:
          {
            uint32_t bitrate = (uint32_t)atoi((char const*)value[0]);
            if(!set_uart_bitrate(bitrate)) break;
            save_global_settings();
            return nbfi_at_server_return_str(reply, 0, AT_OK);
          }
        case AT_HELP:
          return nbfi_at_server_return_tag_help(reply, "get/set the UART bitrate");
        default:
          break;
    }
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}
