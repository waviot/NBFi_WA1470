#include "at_user.h"
#include "defines.h"
#include <stdlib.h>
#include "rs485_uart.h"
#include "radio.h"

_Bool phobos_scan_mode = 0;

/*
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
        return nbfi_at_server_return_uint(reply, phobos_hdlc_mode, AT_OK);
      case AT_SET:
        {
          phobos_hdlc_mode = (uint8_t)atoi((char const*)value[0]);
          if(phobos_hdlc_mode) radio_switch_to_from_short_range(1,1);
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
}*/

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
        return nbfi_at_server_return_uint(reply, phobos_hdlc_mode, AT_OK);
      case AT_SET:
        {
          phobos_hdlc_mode = (uint8_t)atoi((char const*)value[0]);
	  radio_switch_to_from_short_range(0,1);
		  if(phobos_hdlc_mode) 
		  {
			radio_switch_to_from_short_range(1,1);
	  		nbfi.additional_flags &= ~NBFI_FLG_SEND_IN_RESPONSE;
		  }
		  else
		  {
		  	if ((sr_server_modem_id_and_key.id != DEFAULT_REMOTE_ID))
			radio_switch_to_from_short_range(1,1);
		  }
          return nbfi_at_server_return_str(reply, 0, AT_OK);
        }
      case AT_HELP:
        return nbfi_at_server_return_tag_help(reply, "get/set the phobos hdlc mode(0-disabled, 1-enabled)");
      default:
        break;
    }
    break;
	case 1:
    switch(action)
    {
      case AT_GET:
      case AT_CMD:
        return nbfi_at_server_return_uint(reply, phobos_scan_mode, AT_OK);
      case AT_SET:
        {
          phobos_scan_mode = (uint8_t)atoi((char const*)value[0]);
	  	  radio_switch_to_from_phobos_scan_mode(0);
		  if(phobos_scan_mode) 
		  {
			radio_switch_to_from_phobos_scan_mode(1);
		  }
		  else
		  {
		  	if ((sr_server_modem_id_and_key.id != DEFAULT_REMOTE_ID))
			radio_switch_to_from_short_range(1,1);
		  }
          return nbfi_at_server_return_str(reply, 0, AT_OK);
        }
      case AT_HELP:
        return nbfi_at_server_return_tag_help(reply, "get/set the phobos scan mode(0-disabled, 1-enabled)");
      default:
        break;
    }
    break;
  #endif
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}
