#include "nbfi_at_server.h"
#include "nbfi.h"
#include "radio.h"


uint8_t hex2bin(const char* hexstr, char * binstr)
{
    uint16_t hexstrLen = strlen(hexstr);
    int count = 0;
    const char* pos = hexstr;

    for(count = 0; count < hexstrLen / 2; count++) {
        sscanf(pos, "%2hhx", &binstr[count]);
        pos += 2;
    }
    return count;
    //binstr[count] = 0;
}


uint16_t nbfi_at_server_list_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{

  uint8_t buf[1024];
  uint8_t *ptr = buf;
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
          for(uint8_t i = 0; i != NBFI_AT_SERVER_TAGS_NUMBER; i++)
          {
            uint8_t len = strlen(nbfi_at_server_tags_mas[i]);
            memcpy(ptr, nbfi_at_server_tags_mas[i], len);
            ptr += len;
            *ptr++ = 0x0d;
            *ptr++ = 0x0a;
          }
          return nbfi_at_server_return_str(reply, (const char*)buf, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the list of AT commands supported");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_send_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{

  uint8_t buf[256];
  switch(action)
  {
    case AT_SET:
      {
        uint8_t len = hex2bin((const char*)value[0], (char*)buf);
        nbfi_ul_sent_status_t   status = NBFi_Send5(buf, len, 0);
        return nbfi_at_server_return_uint(reply, status.id, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "send hexademical data");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_send_status_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{

  switch(action)
  {
    case AT_GET:
    case AT_CMD:
    {
      if(sub_param == 0) break;
      nbfi_ul_sent_status_t status =  NBFi_get_UL_status(atoi((const char*)sub_param));
      if(status.id == 0) return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
      return nbfi_at_server_return_uint(reply, status.status, AT_OK);
    }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the status of the sent packet");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_receive_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{

  switch(action)
  {
    case AT_GET:
    case AT_CMD:
    {

      return nbfi_at_server_return_hex_str(reply, at_server_last_rx_pkt, at_server_last_rx_pkt_len, at_server_last_rx_pkt_len?AT_OK:AT_EMPTY_ERROR);
    }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the last received data packet");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


void NBFi_ReadConfig(nbfi_settings_t * settings);

uint16_t nbfi_at_server_id_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  nbfi_settings_t settings;
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      NBFi_ReadConfig(&settings);
      return  nbfi_at_server_return_uint(reply, *settings.modem_id, AT_OK);
    case AT_SET:
      return nbfi_at_server_return_str(reply, 0, AT_READONLY_ERROR);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the device ID");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_key_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_hex_str(reply, (uint8_t*)nbfi.master_key, 32,  AT_OK);
    case AT_SET:
      return nbfi_at_server_return_str(reply, 0, AT_READONLY_ERROR);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the device master key");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_mode_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.mode, AT_OK);
    case AT_SET:
      {
        uint8_t mode = (uint8_t)atoi((char const*)value[0]);
        if(mode > OFF) break;
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_MODE;
        buf[1] = mode;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the NBFi mode(0-NRX,1-DRX,2-CRX,3-OFF)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_tx_phy_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.tx_phy_channel, AT_OK);
    case AT_SET:
      {
        uint8_t phy = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_MODE;
        buf[3] = phy;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the tx phy default value");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_rx_phy_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.rx_phy_channel, AT_OK);
    case AT_SET:
      {
        uint8_t phy = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_MODE;
        buf[4] = phy;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the rx phy default value");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_handshake_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.handshake_mode, AT_OK);
    case AT_SET:
      {
        uint8_t handshake = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_HANDSHAKE;
        buf[1] = handshake;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the handshake mode(0-HANDSHAKE_NONE,1-HANDSHAKE_SIMPLE)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_mack_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.mack_mode, AT_OK);
    case AT_SET:
      {
        uint8_t mack = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_HANDSHAKE;
        buf[2] = mack;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the mack mode");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_retries_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.num_of_retries, AT_OK);
    case AT_SET:
      {
        uint8_t retries = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_MODE;
        buf[6] = retries;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the number of send retries");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_max_pld_len_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.max_payload_len, AT_OK);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the ul packet payload length(always 8 bytes)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_wait_ack_timeout_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.wait_ack_timeout, AT_OK);
    case AT_SET:
      {
        uint16_t timeout = (uint16_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_WAIT_ACK_TIMEOUT;
        buf[1] = (timeout>>8);
        buf[2] = timeout&0xff;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the send timeout(ms, 0 - auto)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


void bigendian_cpy(uint8_t* from, uint8_t* to, uint8_t len);

uint16_t nbfi_at_server_tx_freq_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.tx_freq, AT_OK);
    case AT_SET:
      {
        uint32_t freq = (uint32_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_TXFREQ;
        bigendian_cpy((uint8_t*)&freq, &buf[1],4);
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the tx frequency(Hz, 0 - auto)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_rx_freq_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.rx_freq, AT_OK);
    case AT_SET:
      {
        uint32_t freq = (uint32_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_RXFREQ;
        bigendian_cpy((uint8_t*)&freq, &buf[1],4);
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the rx frequency(Hz, 0 - auto)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_tx_ant_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.tx_antenna, AT_OK);
    case AT_SET:
      {
        uint8_t ant = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_ANT;
        buf[2] = ant;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the tx antenna type(0-PCB, 1-SMA)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_rx_ant_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.rx_antenna, AT_OK);
    case AT_SET:
      {
        uint8_t ant = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_ANT;
        buf[3] = ant;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the rx antenna type(0-PCB, 1-SMA)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_max_power_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.tx_pwr, AT_OK);
    case AT_SET:
      {
        int8_t pwr = (int8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_MODE;
        buf[5] = pwr;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the tx max power(dBm)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_hb_interval_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      if(nbfi.heartbeat_num != 0) return  nbfi_at_server_return_uint(reply, nbfi.heartbeat_interval, AT_OK);
      else return  nbfi_at_server_return_uint(reply, 0, AT_OK);
    case AT_SET:
      {
        uint16_t interval = (uint16_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_HEART_BEAT;
        if(interval == 0 ) buf[1] = 0;
        else  buf[1] = 255;
        buf[2] = (interval >> 8);
        buf[3] = (interval&0xff);
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the heartbeat interval(sec in CRX, min in NRX/DRX)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_hb_num_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.heartbeat_num, AT_OK);
    case AT_SET:
      {
        uint8_t num = (uint8_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_HEART_BEAT;
        buf[1] = num;
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the total number of heartbeats (255-unlimited)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_flags_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.additional_flags, AT_OK);
    case AT_SET:
      {
        uint16_t flags = (uint16_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_ADD_FLAGS;
        buf[2] = (flags >> 8);
        buf[1] = (flags&0xff);
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the NBFi flags bitmap");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_ul_base_freq_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.ul_freq_base, AT_OK);
    case AT_SET:
      {
        uint32_t freq = (uint32_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_UL_BASE_FREQ;
        bigendian_cpy((uint8_t*)&freq, &buf[1],4);
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the uplink base frequency(Hz)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_dl_base_freq_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.dl_freq_base, AT_OK);
    case AT_SET:
      {
        uint32_t freq = (uint32_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_DL_BASE_FREQ;
        bigendian_cpy((uint8_t*)&freq, &buf[1],4);
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the downlink base frequency(Hz)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_fplan_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, nbfi.nbfi_freq_plan.fp, AT_OK);
    case AT_SET:
      {
        uint16_t fplan = (uint16_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_FPLAN;
        buf[1] = (fplan >> 8);
        buf[2] = (fplan&0xff);
        NBFi_send_Packet_to_Config_Parser(buf);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the NBFi frequency plan");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_alt_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_hex_str(reply, (uint8_t*)&nbfi.try_alternative[atoi((const char*)sub_param)], 5, AT_OK);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the NBFi alternative #n(ATL.n)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_factory_settings_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_CMD:
      NBFi_clear_Saved_Configuration();
      return  nbfi_at_server_return_str(reply, 0, AT_OK);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "reset NBFi settings to factory defaults");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_reset_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_CMD:
      NBFi_CPU_Reset();
      break;
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "device reset");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_nbfi_settings_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_hex_str(reply, (uint8_t*)&nbfi, sizeof(nbfi), AT_OK);
    case AT_SET:
      {
        nbfi_settings_t _nbfi;
        hex2bin((const char*)value[0], (char*)&_nbfi);
        NBFi_set_Settings(&_nbfi, 0);
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set NBFi settings as hexademical array");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_nbfi_rtc_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, NBFi_get_RTC(), AT_OK);
    case AT_SET:
      {
        NBFi_set_RTC(atoi((const char*)value[0]));
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set NBFi RTC time(seconds from 1.1.1970 UTC)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_rssi_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        uint8_t buf[20];
        sprintf((char*)buf, "%3.1f", NBFi_get_rssi());
        return  nbfi_at_server_return_str(reply, (char const*)buf, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get receiver RSSI current level");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_noise_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        uint8_t buf[20];
        sprintf((char*)buf, "%3.1f", NBFi_RF_get_noise());
        return  nbfi_at_server_return_str(reply, (char const*)buf, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get receiver noise level");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_last_snr_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        nbfi_state_t _nbfi_state;
        NBFi_get_state(&_nbfi_state);
        return  nbfi_at_server_return_uint(reply, _nbfi_state.last_snr, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get last received packed SNR level");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_last_rssi_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        nbfi_state_t _nbfi_state;
        NBFi_get_state(&_nbfi_state);
        return  nbfi_at_server_return_uint(reply, _nbfi_state.last_rssi, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get last received packed RSSI level");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_aver_ul_snr_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        nbfi_state_t _nbfi_state;
        NBFi_get_state(&_nbfi_state);
        return  nbfi_at_server_return_uint(reply, _nbfi_state.aver_tx_snr, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get average uplink SNR level");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_vcc_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        uint8_t buf[20];
        sprintf((char*)buf, "%.2f", (float)nbfi_HAL_measure_valtage_or_temperature(1)/100);
        return  nbfi_at_server_return_str(reply, (char const*)buf, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get VCC value");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_temp_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        return nbfi_at_server_return_uint(reply, nbfi_HAL_measure_valtage_or_temperature(0), AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get device temperature");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_aver_dl_snr_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      {
        nbfi_state_t _nbfi_state;
        NBFi_get_state(&_nbfi_state);
        return  nbfi_at_server_return_uint(reply, _nbfi_state.aver_rx_snr, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get average downlink SNR level");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_sr_server_id_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  nbfi_device_id_and_key_st id_and_key;
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      radio_load_id_and_key_of_sr_server(&id_and_key);
      return  nbfi_at_server_return_uint(reply, id_and_key.id, AT_OK);
    case AT_SET:
      radio_load_id_and_key_of_sr_server(&id_and_key);
      id_and_key.id = (uint32_t)atoi((char const*)value[0]);
      radio_save_id_and_key_of_sr_server(&id_and_key);
      return nbfi_at_server_return_str(reply, 0, AT_OK);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the short-range server device ID");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}


uint16_t nbfi_at_server_sr_server_key_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  nbfi_device_id_and_key_st id_and_key;
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      radio_load_id_and_key_of_sr_server(&id_and_key);
      return  nbfi_at_server_return_hex_str(reply, id_and_key.key, 32,  AT_OK);
    case AT_SET:
      radio_load_id_and_key_of_sr_server(&id_and_key);
      hex2bin((const char*)value[0], (char*)id_and_key.key);
      radio_save_id_and_key_of_sr_server(&id_and_key);
      return nbfi_at_server_return_str(reply, 0, AT_OK);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the short-range server device master key");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_sr_mode_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      if(!NBFi_is_Switched_to_Custom_Settings()) return  nbfi_at_server_return_uint(reply, 0, AT_OK);
      nbfi_device_id_and_key_st id_and_key;
      radio_load_id_and_key_of_sr_server(&id_and_key);
      if(*nbfi.modem_id != id_and_key.id) return  nbfi_at_server_return_uint(reply, 1, AT_OK);
      else return  nbfi_at_server_return_uint(reply, 2, AT_OK);
    case AT_SET:
      {
        uint8_t mode = (uint8_t)atoi((char const*)value[0]);
        radio_switch_to_from_short_range((mode != 0), (mode == 2));
        return nbfi_at_server_return_str(reply, 0, AT_OK);
      }
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get/set the short-range mode(0-disabled, 1-server, 2 - client)");
    default:
      break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

uint16_t nbfi_at_server_user_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  if(nbfi_at_server_user_defined_handler) return nbfi_at_server_user_defined_handler(reply, action, sub_param, value);
  return nbfi_at_server_return_str(reply, 0, AT_ERROR);
}


uint16_t nbfi_at_server_common_handler(nbfi_at_server_tags_t tag, uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(tag)
  {
  case LIST:
    return nbfi_at_server_list_handler(reply, action, sub_param, value);
  case SEND:
    return nbfi_at_server_send_handler(reply, action, sub_param, value);
  case SEND_STATUS:
    return nbfi_at_server_send_status_handler(reply, action, sub_param, value);
  case RECEIVE:
    return nbfi_at_server_receive_handler(reply, action, sub_param, value);
  case ID:
    return nbfi_at_server_id_handler(reply, action, sub_param, value);
  case KEY:
    return nbfi_at_server_key_handler(reply, action, sub_param, value);
  case MODE:
    return nbfi_at_server_mode_handler(reply, action, sub_param, value);
  case TX_PHY:
    return nbfi_at_server_tx_phy_handler(reply, action, sub_param, value);
  case RX_PHY:
    return nbfi_at_server_rx_phy_handler(reply, action, sub_param, value);
  case HANDSHAKE:
    return nbfi_at_server_handshake_handler(reply, action, sub_param, value);
  case MACK:
    return nbfi_at_server_mack_handler(reply, action, sub_param, value);
  case RETRIES:
    return nbfi_at_server_retries_handler(reply, action, sub_param, value);
  case MAX_PLD_LEN:
    return nbfi_at_server_max_pld_len_handler(reply, action, sub_param, value);
  case WAIT_ACK_TIMEOUT:
    return nbfi_at_server_wait_ack_timeout_handler(reply, action, sub_param, value);
  case TX_FREQ:
    return nbfi_at_server_tx_freq_handler(reply, action, sub_param, value);
  case RX_FREQ:
    return nbfi_at_server_rx_freq_handler(reply, action, sub_param, value);
  case TX_ANT:
    return nbfi_at_server_tx_ant_handler(reply, action, sub_param, value);
  case RX_ANT:
    return nbfi_at_server_rx_ant_handler(reply, action, sub_param, value);
  case MAX_POWER:
    return nbfi_at_server_max_power_handler(reply, action, sub_param, value);
  case HB_INTERVAL:
    return nbfi_at_server_hb_interval_handler(reply, action, sub_param, value);
  case HB_NUM:
    return nbfi_at_server_hb_num_handler(reply, action, sub_param, value);
  case FLAGS:
    return nbfi_at_server_flags_handler(reply, action, sub_param, value);
  case UL_BASE_FREQ:
    return nbfi_at_server_ul_base_freq_handler(reply, action, sub_param, value);
  case DL_BASE_FREQ:
    return nbfi_at_server_dl_base_freq_handler(reply, action, sub_param, value);
  case FPLAN:
    return nbfi_at_server_fplan_handler(reply, action, sub_param, value);
  case ALT:
    return nbfi_at_server_alt_handler(reply, action, sub_param, value);
  case FACTORY_SETTINGS:
    return nbfi_at_server_factory_settings_handler(reply, action, sub_param, value);
  case CPU_RESET:
    return nbfi_at_server_reset_handler(reply, action, sub_param, value);
  case NBFI_SETTINGS:
    return nbfi_at_server_nbfi_settings_handler(reply, action, sub_param, value);
  case NBFI_RTC:
    return nbfi_at_server_nbfi_rtc_handler(reply, action, sub_param, value);
  case RSSI:
    return nbfi_at_server_rssi_handler(reply, action, sub_param, value);
  case NOISE:
    return nbfi_at_server_noise_handler(reply, action, sub_param, value);
  case LAST_SNR:
    return nbfi_at_server_last_snr_handler(reply, action, sub_param, value);
  case LAST_RSSI:
    return nbfi_at_server_last_rssi_handler(reply, action, sub_param, value);
  case AVER_UL_SNR:
    return nbfi_at_server_aver_ul_snr_handler(reply, action, sub_param, value);
  case AVER_DL_SNR:
    return nbfi_at_server_aver_ul_snr_handler(reply, action, sub_param, value);
  case VCC:
    return nbfi_at_server_vcc_handler(reply, action, sub_param, value);
  case TEMP:
    return nbfi_at_server_temp_handler(reply, action, sub_param, value);
  case SR_SERVER_ID:
    return nbfi_at_server_sr_server_id_handler(reply, action, sub_param, value);
  case SR_SERVER_KEY:
    return nbfi_at_server_sr_server_key_handler(reply, action, sub_param, value);
  case SR_MODE:
    return nbfi_at_server_sr_mode_handler(reply, action, sub_param, value);
  case USER:
    return nbfi_at_server_user_handler(reply, action, sub_param, value);
  default:
    break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

