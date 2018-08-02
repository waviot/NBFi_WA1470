#include "wa1205mod.h"
#include "wa1205.h"
#include "wtimer.h"
#include "log.h"
#include  <string.h>
#include <stdio.h>



#define MODOSCFREQ      26000000

const mod_hop_channels_t hop_table[8] = {MINUS90000, MINUS65000, MINUS40000, MINUS15000, PLUS15000, PLUS40000, PLUS65000, PLUS90000};

struct wtimer_desc mod_callTXfinished_desc;

mod_bitrate_s current_tx_phy;

void wa1205mod_init()
{
  wa1205mod_set_hop_table((mod_hop_channels_t *)hop_table);
}

extern void (*__wa1205_tx_finished)(void);

static void  wa1205mod_call_TX_finished(struct wtimer_desc *desc)
{
  __wa1205_tx_finished();
}

void wa1205mod_isr(void)
{
    uint8_t status;
 
    wa1205_spi_read(MOD_STATUS, &status, 1);  
 
    if(!(status&MOD_STATUS_IRQ_ON_TX_FLAG)) return;
    
    wa1205_spi_write8(MOD_CONFIG, MOD_CONF_CLEAR_IRQ);  
    
    ScheduleTask(&mod_callTXfinished_desc,  wa1205mod_call_TX_finished, RELATIVE, MILLISECONDS(5));

}


void wa1205mod_send(uint8_t* data, mod_bitrate_s bitrate)
{ 
  wa1205mod_set_bitrate(bitrate);
  switch(bitrate)
  {
    case MOD_DBPSK_50_PROT_D:
    case MOD_DBPSK_400_PROT_D:
    case MOD_DBPSK_3200_PROT_D:
    case MOD_DBPSK_25600_PROT_D:
      for(uint8_t i = 0; i != 36; i++) wa1205_spi_write8(MOD_DATA_START + i, data[i]);
      //wa1205_spi_write(MOD_DATA_START, data, 36);
      wa1205_spi_write8(MOD_CONFIG, MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
      break;
    case MOD_DBPSK_50_PROT_E:
    case MOD_DBPSK_400_PROT_E:
    case MOD_DBPSK_3200_PROT_E:
    case MOD_DBPSK_25600_PROT_E:
      wa1205_spi_write(MOD_DATA_START, data, 40);
      wa1205_spi_write8(MOD_CONFIG, MOD_CONF_PROT_E_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
      break;
    case MOD_DBPSK_100H_PROT_D:
      wa1205_spi_write(MOD_DATA_START, data, 36);
      wa1205_spi_write8(MOD_CONFIG, MOD_CONF_HOP_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
      break;
    case MOD_DBPSK_100H_PROT_E:
      wa1205_spi_write(MOD_DATA_START, data, 40);
      wa1205_spi_write8(MOD_CONFIG, MOD_CONF_HOP_EN|MOD_CONF_PROT_E_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
      break;
  }
  
}


void wa1205mod_set_hop_table(mod_hop_channels_t *hop_table)
{
 // wa1205_spi_write(MOD_HOP_TBL_START, (uint8_t*)hop_table, 8);
 for(uint8_t i = 0; i != 8; i++) wa1205_spi_write8(MOD_HOP_TBL_START + i, (uint8_t)hop_table[i]);
 
}

void wa1205mod_set_bitrate(mod_bitrate_s bitrate)
{
  uint64_t rate;
  switch(bitrate)
  {
    case MOD_DBPSK_50_PROT_D:
    case MOD_DBPSK_50_PROT_E:
      rate = 50;
      break;
    case MOD_DBPSK_400_PROT_D:
    case MOD_DBPSK_400_PROT_E:
      rate = 400;
      break;
    case MOD_DBPSK_3200_PROT_D:
    case MOD_DBPSK_3200_PROT_E:
      rate = 3200;
      break;
    case MOD_DBPSK_25600_PROT_D:
    case MOD_DBPSK_25600_PROT_E:
      rate = 25600;
      break;
    case MOD_DBPSK_100H_PROT_D:
    case MOD_DBPSK_100H_PROT_E:
      rate = 100;
      break;
  }
  rate = rate*16777216;
  rate = ((rate%100000)>=5)?(rate/1000000 + 1):(rate/1000000);
  //wa1205_spi_write(MOD_PER0, ((uint8_t*)&rate), 3);
  wa1205_spi_write8(MOD_PER0, *(((uint8_t*)&rate)+0));
  wa1205_spi_write8(MOD_PER1, *(((uint8_t*)&rate)+1));
  wa1205_spi_write8(MOD_PER2, *(((uint8_t*)&rate)+2));
  current_tx_phy = bitrate;
}

void wa1205mod_set_freq(uint32_t freq)
{
    sprintf(log_string, "mod_set_freq to %ld", freq);
    log_send_str(log_string);    
    switch(current_tx_phy)
    {
      case MOD_DBPSK_100H_PROT_D:
      case MOD_DBPSK_100H_PROT_E:
        wa1205_set_freq(freq);
        wa1205_set_freq(freq);
        break;
    default:
      wa1205_set_freq(freq + 90000);
      wa1205_set_freq(freq + 90000);
      break;
    }
}