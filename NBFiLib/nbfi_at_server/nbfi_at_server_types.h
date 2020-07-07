#ifndef NBFI_AT_SERVER_TYPES_H
#define NBFI_AT_SERVER_TYPES_H

typedef enum
{
  AT_OK,
  AT_ERROR,
  AT_PARAM_ERROR,
  AT_EMPTY_ERROR,
  AT_READONLY_ERROR,
}nbfi_at_server_result_t;

typedef enum
{
  AT_GET,
  AT_SET,
  AT_CMD,
  AT_HELP
}nbfi_at_server_action_t;


typedef enum
{
  LIST,
  SEND,
  SEND_STATUS,
  RECEIVE,  
  ID,
  KEY,
  MODE,
  TX_PHY,
  RX_PHY,
  HANDSHAKE,
  MACK,
  RETRIES,
  MAX_PLD_LEN,
  WAIT_ACK_TIMEOUT,
  TX_FREQ,
  RX_FREQ,
  TX_ANT,
  RX_ANT,
  MAX_POWER,
  HB_INTERVAL,
  HB_NUM,
  FLAGS,
  UL_BASE_FREQ,
  DL_BASE_FREQ,
  FPLAN,
  ALT,
  FACTORY_SETTINGS,
  CPU_RESET,
  NBFI_SETTINGS,
  NBFI_RTC,
  RSSI,
  NOISE,
  LAST_SNR,
  LAST_RSSI,
  AVER_UL_SNR,
  AVER_DL_SNR,
  VCC,
  TEMP,
  SR_SERVER_ID,
  SR_SERVER_KEY,
  SR_MODE,
  USER,
  TAG_UNDEFINED = 255
}nbfi_at_server_tags_t;


typedef uint16_t (*nbfi_at_server_handler_t)(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[]);



#endif //NBFI_AT_SERVER_TYPES_H