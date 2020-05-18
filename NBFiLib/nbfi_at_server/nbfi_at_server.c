#include "nbfi.h"
#include "nbfi_at_server.h"

#define NBFI_AT_SERVER_BUF_SIZE 512

_Bool nbfi_at_server_echo_mode = 1;

const char common_help[] = "\
AT-command interface:\n\r\
AT+XXX? provides a short help of the given command, for example AT+MODE?\n\r\
AT+XXX is used to run a command, such as AT+RESET\n\r\
AT+XXX=? is used to get the value of a given command, for example AT+ID=?\n\r\
AT+XXX=<value> is used to provide a value to a command, for example AT+MODE=CRX\n\r\
";

static nbfi_at_server_tags_t tag_for_help;


uint8_t nbfi_at_server_fill_result(uint8_t *data, nbfi_at_server_result_t result)
{
   switch(result)
  {
    case AT_OK:
      strcpy((char*)data, "OK");
      return 2;
    case AT_ERROR:
      strcpy((char*)data, "AT_ERROR");
      return 8;
    case AT_PARAM_ERROR:
      strcpy((char*)data, "AT_PARAM_ERROR");
      return 14;
    case AT_EMPTY_ERROR:
      strcpy((char*)data, "AT_EMPTY_ERROR");
      return 14;
    case AT_READONLY_ERROR:
      strcpy((char*)data, "AT_READONLY_ERROR");
      return 17;
    default:
      return 0;
  }
}

uint16_t nbfi_at_server_return_str(uint8_t *data, const char* str, nbfi_at_server_result_t result)
{
  uint16_t i = 0;
  if(str)
  {
    while(*str) data[i++] = *str++;
    data[i++] = 0x0d;
    data[i++] = 0x0a;
  }
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  i += nbfi_at_server_fill_result(&data[i], result);
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  return i;
}

uint16_t nbfi_at_server_return_uint(uint8_t *data, uint32_t n, nbfi_at_server_result_t result)
{
  uint16_t i = 0;
  sprintf((char*)data, "%d", n);
  i = strlen((const char*)data);
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  i += nbfi_at_server_fill_result(&data[i], result);
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  return i;
}

uint16_t nbfi_at_server_return_hex_str(uint8_t *data, uint8_t *str, uint8_t size, nbfi_at_server_result_t result)
{
  uint16_t i = 0;
  data[0] = 0;
  while(i < size) sprintf((char*)data + strlen((const char*)data), "%02X", str[i++]);
  i *= 2;
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  i += nbfi_at_server_fill_result(&data[i], result);
  data[i++] = 0x0d;
  data[i++] = 0x0a;
  return i;
}

static uint16_t nbfi_at_server_get_param(uint8_t *param, uint8_t *reply)
{  
  
  uint8_t* sub_param = nbfi_at_server_get_sub_param(param);
  
  nbfi_at_server_tags_t tag = nbfi_at_server_str2tag((const char*)param);
  
  if(tag == TAG_UNDEFINED) return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR); 
 
  return nbfi_at_server_common_handler(tag, reply, AT_GET, sub_param, 0);
  
}

static uint16_t nbfi_at_server_set_param(uint8_t *param, uint8_t *value, uint8_t *reply)
{
  
  uint8_t* sub_param = nbfi_at_server_get_sub_param(param);
    
  nbfi_at_server_tags_t tag = nbfi_at_server_str2tag((const char*)param);
  
  if(tag == TAG_UNDEFINED) return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR); 

  return nbfi_at_server_common_handler(tag, reply, AT_SET, sub_param, &value);
  
}

static uint16_t nbfi_at_server_run_cmd(uint8_t *cmd, uint8_t *reply)
{
  
  uint8_t* sub_param = nbfi_at_server_get_sub_param(cmd);
    
  nbfi_at_server_tags_t tag = nbfi_at_server_str2tag((const char*)cmd);
  
  if(tag == TAG_UNDEFINED) return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
  
  return nbfi_at_server_common_handler(tag, reply, AT_CMD, sub_param, 0);
}

static uint16_t nbfi_at_server_get_help(uint8_t *cmd, uint8_t *reply)
{
    uint8_t* sub_param = nbfi_at_server_get_sub_param(cmd);
    
    tag_for_help = nbfi_at_server_str2tag((const char*)cmd);
  
    if(tag_for_help == TAG_UNDEFINED) return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
    
    return nbfi_at_server_common_handler(tag_for_help, reply, AT_HELP, sub_param, 0);
}

uint16_t nbfi_at_server_return_common_help(uint8_t *reply)
{  
    return nbfi_at_server_return_str(reply, common_help, AT_OK);
}

uint16_t nbfi_at_server_return_tag_help(uint8_t *reply, const char* str)
{  
    char buf[256];
    char tag_str[20];
    nbfi_at_server_tag2str(tag_for_help, tag_str);
    sprintf(buf, "AT+%s:%s", tag_str, str);
    return nbfi_at_server_return_str(reply, buf, AT_OK);
}


uint16_t nbfi_at_server_parse_line(uint8_t *data, uint16_t request_len)
{
  
  if(((data[0] != 'A')&&(data[0] != 'a')) || ((data[1] != 'T')&&(data[1] != 't')))  return 0;
  
  if(request_len == 2) return nbfi_at_server_return_str(data, 0, AT_OK);
  
  if((data[2] != '+') || (request_len == 3)) return nbfi_at_server_return_common_help(data);
    
  uint8_t i;
  for(i = 3; i != request_len; i++)
  {
    if(data[i] == '=') 
    {
      data[i] = 0;
      if((i < request_len) && data[i + 1] == '?')
      {
        return nbfi_at_server_get_param(&data[3], data);
      }
      data[request_len] = 0;
      
      return nbfi_at_server_set_param(&data[3], &data[i + 1], data);
    }
    if(data[i] == '?')
    {
      data[i] = 0;
      return nbfi_at_server_get_help(&data[3], data);
    }
  }

  data[i] = 0;
  return nbfi_at_server_run_cmd(&data[3], data);
}

uint16_t nbfi_at_server_parse_char(uint8_t input_char, uint8_t ** reply_data_ptr)
{
       static uint8_t buf[NBFI_AT_SERVER_BUF_SIZE];
       static uint16_t len = 0;
       
       if((input_char == 0x0d) || (input_char == 0x0a))
       {
        if(!len) return 0;
        if((input_char == 0x0d))
        {  
          uint16_t reply_len = nbfi_at_server_parse_line(buf, len);
          *reply_data_ptr = buf;
          len = 0;
          return reply_len;
        }
        else return 0;
        
       }
       buf[len++] = input_char;
       return 0;
}

       