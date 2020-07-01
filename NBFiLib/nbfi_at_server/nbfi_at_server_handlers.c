#include "nbfi_at_server.h"
#include "nbfi.h"



void hex2bin(const char* hexstr, char * binstr)
{
    uint16_t hexstrLen = strlen(hexstr);
    int count = 0;
    const char* pos = hexstr;

    for(count = 0; count < hexstrLen / 2; count++) {
        sscanf(pos, "%2hhx", &binstr[count]);
        pos += 2;
    }
    binstr[count] = 0;
}

uint16_t nbfi_at_server_send_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{ 
  
  uint8_t buf[256]; 
  switch(action)
  {
    case AT_SET:
      {
        hex2bin((const char*)value[0], (char*)buf);
        nbfi_ul_sent_status_t   status = NBFi_Send5(buf, sizeof((const char*)buf), 0);
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


uint16_t nbfi_at_server_id_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{  
  switch(action)
  {
    case AT_GET:
    case AT_CMD:
      return  nbfi_at_server_return_uint(reply, *nbfi.modem_id, AT_OK);
    case AT_SET:
      return nbfi_at_server_return_str(reply, 0, AT_READONLY_ERROR);
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "get the device address");
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
      return nbfi_at_server_return_tag_help(reply, "get/set the tx phy default mode");
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
      return nbfi_at_server_return_tag_help(reply, "get/set the rx phy default mode");
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
      return  nbfi_at_server_return_uint(reply, nbfi.heartbeat_interval, AT_OK);
    case AT_SET:
      {
        uint16_t interval = (uint16_t)atoi((char const*)value[0]);
        uint8_t buf[7];
        memset((void*)buf, 0xff, sizeof(buf));
        buf[0] = (WRITE_PARAM_AND_SAVE_CMD << 6) + NBFI_PARAM_HEART_BEAT;
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
      return nbfi_at_server_return_tag_help(reply, "get/set the NBFi flags bitmask");
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

uint16_t nbfi_at_server_cpu_reset_handler(uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{  
  switch(action)
  {
    case AT_CMD:
      NBFi_CPU_Reset();
      break;
    case AT_HELP:
      return nbfi_at_server_return_tag_help(reply, "CPU reset");
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

uint16_t nbfi_at_server_common_handler(nbfi_at_server_tags_t tag, uint8_t *reply, nbfi_at_server_action_t action, uint8_t* sub_param, uint8_t* value[])
{
  switch(tag)
  {
  case SEND:
    return nbfi_at_server_send_handler(reply, action, sub_param, value);
  case SEND_STATUS:
    return nbfi_at_server_send_status_handler(reply, action, sub_param, value);
  case RECEIVE:
    break;
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
    return nbfi_at_server_cpu_reset_handler(reply, action, sub_param, value);  
  case NBFI_SETTINGS:
    return nbfi_at_server_nbfi_settings_handler(reply, action, sub_param, value); 
  case NBFI_RTC:
    return nbfi_at_server_nbfi_rtc_handler(reply, action, sub_param, value);   
  default:
    break;
  }
  return nbfi_at_server_return_str(reply, 0, AT_PARAM_ERROR);
}

