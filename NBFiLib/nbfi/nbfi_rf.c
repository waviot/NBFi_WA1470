#include "nbfi.h"
#include "nbfi_config.h"
#include "nbfi_rf.h"
#include "nbfi_misc.h"
#include <libmfwtimer.h>

_Bool rf_busy = 0;
_Bool transmit = 0;


nbfi_rf_state_s rf_state = STATE_OFF;

nbfi_phy_channel_t nbfi_phy_channel;

//uint8_t PSK_BAND;

//void    NBFi_TX_Finished();
//void    wa1470_set_constants(void);


extern void (* __nbfi_before_tx)();
extern void (* __nbfi_before_rx)();
extern void (* __nbfi_before_off)();


nbfi_status_t NBFi_RF_Init(  nbfi_phy_channel_t  phy_channel,
                        nbfi_rf_antenna_t        antenna,
                        int8_t              power,
                        uint32_t            freq)
{

    static nbfi_phy_channel_t last_phy = UNDEFINED;
    static int8_t last_tx_prw;
    static uint32_t last_tx_freq;
    static uint32_t last_rx_freq;
    
    if(rf_busy) return ERR_RF_BUSY;

    rf_busy = 1;

    if(last_phy != phy_channel)
    {
      wa1470_reinit();
      last_tx_prw = 100;
      last_tx_freq = 0;
      last_rx_freq = 0;
    }
    
    switch(phy_channel)
    {
      
    case UL_DBPSK_50_PROT_D:
    case UL_DBPSK_400_PROT_D:
    case UL_DBPSK_3200_PROT_D:
    case UL_DBPSK_25600_PROT_D: 
    case UL_DBPSK_50_PROT_E:
    case UL_DBPSK_400_PROT_E:
    case UL_DBPSK_3200_PROT_E:
    case UL_DBPSK_25600_PROT_E:
        wa1470dem_rx_enable(0);
        if(__nbfi_before_tx) __nbfi_before_tx();
                 
        //wa1470mod_set_bitrate((mod_bitrate_s)phy_channel);
        
        if(freq != last_tx_freq)
        {
          last_tx_freq = freq;
          wa1470mod_set_freq(freq);
        }
                
        if(power != last_tx_prw)
        {
          last_tx_prw = power;
          wa1470rfe_set_tx_power(power);
        }
        
        wa1470rfe_set_mode(RFE_MODE_TX);           
                     
        rf_busy = 0;
        rf_state = STATE_TX;
        last_phy = phy_channel;
        return OK;
       
    case DL_DBPSK_50_PROT_D:
    case DL_DBPSK_400_PROT_D:
    case DL_DBPSK_3200_PROT_D:
    case DL_DBPSK_25600_PROT_D:
        if(__nbfi_before_rx) __nbfi_before_rx();
        wa1470dem_rx_enable(1);
        
       if(last_phy != phy_channel)
          wa1470dem_set_bitrate((dem_bitrate_s)phy_channel);
        
        wa1470rfe_set_mode(RFE_MODE_RX);
        
        if(freq != last_rx_freq)
        {
          last_rx_freq = freq;
          wa1470mod_set_freq(freq);
        }
        wa1470dem_set_freq(freq);
        rf_busy = 0;
        rf_state = STATE_RX;
        last_phy = phy_channel;
        return OK;
    }
    rf_busy = 0;
    return ERR;
}

nbfi_status_t NBFi_RF_Deinit()
{
    if(rf_busy) return ERR_RF_BUSY;
    if(__nbfi_before_off)   __nbfi_before_off();    
    rf_busy = 1;
    wa1470rfe_set_mode(RFE_MODE_DEEP_SLEEP);
    void wa1470rfe_deinit();
    rf_busy = 0;
    transmit = 0;
    rf_state = STATE_OFF;
    return OK;
}


nbfi_status_t NBFi_RF_Transmit(uint8_t* pkt, uint8_t len, nbfi_phy_channel_t  phy_channel, rf_blocking_t blocking)
{
    if(rf_busy) return ERR_RF_BUSY;

    rf_busy = 1;

    wa1470mod_send(pkt, (mod_bitrate_s) phy_channel);
    
    rf_busy = 0;
    
    transmit = 1;
      
    if(blocking == BLOCKING)
    {

        while(1) // Wait for TX complete
        {
            if(!transmit) break;
            wtimer_runcallbacks();
        }

    }

    return OK;
}


void NBFi_TX_Finished();

void NBFi_RF_TX_Finished()
{
  NBFi_TX_Finished();
}


float NBFi_RF_get_noise()
{
  if(wa1470dem_get_noise() < -150) return -150;
  else return wa1470dem_get_noise();
}


