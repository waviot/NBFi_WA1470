#include "nbfi_at_server.h"
#include <string.h>
#include <ctype.h>


const char nbfi_at_server_tags_mas[NBFI_AT_SERVER_TAGS_NUMBER][NBFI_AT_SERVER_TAGS_MAX_LEN] = 
{
  "SEND",
  "SEND_STATUS",
  "RECEIVE", 
  "ID",
  "KEY",
  "MODE",
  "TX_PHY",
  "RX_PHY",
  "HANDSHAKE",
  "MACK",
  "RETRIES",
  "MAX_PLD_LEN",
  "WAIT_ACK_TIMEOUT",
  "TX_FREQ",
  "RX_FREQ",
  "TX_ANT",
  "RX_ANT",
  "MAX_POWER",
  "HB_INTERVAL",
  "HB_NUM",
  "FLAGS",
  "UL_BASE_FREQ",
  "DL_BASE_FREQ",
  "FPLAN",
  "ALT",
  "FACTORY_SETTINGS",
  "CPU_RESET",
  "NBFI_SETTINGS",
  "NBFI_RTC"
};


nbfi_at_server_tags_t nbfi_at_server_str2tag(const char *str)
{
  //toUpperCase(str);
  for(uint8_t i = 0; i != NBFI_AT_SERVER_TAGS_NUMBER; i++)
  {
    if(strcasecmp(str, nbfi_at_server_tags_mas[i]) == 0) return (nbfi_at_server_tags_t)i;
  }
  
  return TAG_UNDEFINED;
}

_Bool nbfi_at_server_tag2str(nbfi_at_server_tags_t tag, char *str)
{
  if((uint8_t)tag >= NBFI_AT_SERVER_TAGS_NUMBER) return 0;
  strcpy(str, nbfi_at_server_tags_mas[(uint8_t)tag]);
  return 1;
}

uint8_t* nbfi_at_server_get_sub_param(uint8_t *param)
{
  uint8_t i = 0;
  
  while(param[i])
  {
    if(param[i] == '.')
    {
      param[i] = 0;
      return &param[i + 1];
    }
    i++;
  }
  return 0;
}

