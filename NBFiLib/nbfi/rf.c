#include "nbfi.h"
#include "nbfi_config.h"
#include "rf.h"
#include "nbfi_misc.h"
#include <libmfwtimer.h>

_Bool rf_busy = 0;
_Bool transmit = 0;


nbfi_rf_state_s rf_state = STATE_OFF;

nbfi_phy_channel_t nbfi_phy_channel;

uint8_t PSK_BAND;

void    NBFi_TX_Finished();
void    wa1470_set_constants(void);


//const uint16_t AX5043_power[26] = {0x00aa, 0x00bf, 0x00d1, 0x00ec, 0x010f, 0x0132, 0x0156, 0x017f, 0x01af, 0x1e0, 0x207, 0x244, 0x290, 0x2eb, 0x35e, 0x3d6, 0x406, 0x4a9, 0x57c, 0x600, 0x700, 0x800, 0x9d4, 0xc00, 0xf00, 0xfff};


extern void (* __nbfi_before_tx)();
extern void (* __nbfi_before_rx)();
extern void (* __nbfi_before_off)();
/*
void RF_SetModeAndPower(int8_t dBm, rf_direction_t mode, rf_antenna_t ant)
{
    switch(mode)
    {
    case TX:    
      //nbfi_wa1470_pins.txpwr = AX5043_power[dBm + 10];
      if(__nbfi_before_tx) __nbfi_before_tx();
      break;
    case RX: 
      if(__nbfi_before_rx) __nbfi_before_rx();
      wa1470rfe_set_mode(RFE_MODE_RX);
      break;
    case IDLE:
      wa1470rfe_set_mode(RFE_MODE_DEEP_SLEEP);
      if(__nbfi_before_off)   __nbfi_before_off();
      break;
    }

}*/

nbfi_status_t RF_Init(  nbfi_phy_channel_t  phy_channel,
                        rf_antenna_t        antenna,
                        int8_t              power,
                        uint32_t            freq)
{

    if(rf_busy) return ERR_RF_BUSY;

    rf_busy = 1;

   // RF_SetModeAndPower(power, RX, antenna);
    

    //wa1470_tcxo_set_reset(1);


    switch(phy_channel)
    {
      
    case UL_DBPSK_50_PROT_D:
    case UL_DBPSK_400_PROT_D:
    case UL_DBPSK_3200_PROT_D:
    case UL_DBPSK_25600_PROT_D: 
        //RF_SetModeAndPower(power, TX, antenna);
        if(__nbfi_before_tx) __nbfi_before_tx();
        
        //wa1470rfe_init();
        
        wa1470mod_set_bitrate((mod_bitrate_s)phy_channel);
        //wa1470mod_set_freq(freq);
        //wa1470_spi_write8( ADDR_1_8_V_FRACTIONAL_PLL_MODE, 72);
        /*wa1470rfe_set_mode(RFE_MODE_DEEP_SLEEP);
        wa1470_spi_write8( ADDR_1_8_V_FRACTIONAL_PLL_MODE, 72);
        wa1470rfe_set_pll_mode(RFE_PLL_MODE_FRACTIONAL);
        wa1470rfe_set_mode(RFE_MODE_IDLE);*/
        
        wa1470mod_set_freq(freq);
        wa1470rfe_set_tx_power(power);
        wa1470rfe_set_mode(RFE_MODE_TX);           
               
      
        rf_busy = 0;
        rf_state = STATE_TX;
        return OK;
       
    case DL_DBPSK_50_PROT_D:
    case DL_DBPSK_400_PROT_D:
    case DL_DBPSK_3200_PROT_D:
    case DL_DBPSK_25600_PROT_D:
        if(__nbfi_before_rx) __nbfi_before_rx();
//        RF_SetModeAndPower(power, RX, antenna);
        
                //wa1470rfe_init_int();
        
        wa1470dem_set_bitrate((dem_bitrate_s)phy_channel);
                  //wa1470_spi_write8( ADDR_1_8_V_FRACTIONAL_PLL_MODE, 0);
      /*  wa1470rfe_set_mode(RFE_MODE_DEEP_SLEEP);
       extern void (*__wa1470_nop_dalay_ms)(uint32_t);
        __wa1470_nop_dalay_ms(100);
        wa1470_spi_write8( ADDR_1_8_V_FRACTIONAL_PLL_MODE, 0);
        __wa1470_nop_dalay_ms(100);
        wa1470rfe_set_pll_mode(RFE_PLL_MODE_INTEGER);
        __wa1470_nop_dalay_ms(100);
        wa1470rfe_set_mode(RFE_MODE_IDLE);
        
        __wa1470_nop_dalay_ms(100);*/
        //wa1470dem_set_freq(freq);
        wa1470rfe_set_mode(RFE_MODE_RX);
        wa1470dem_set_freq(freq);
        //wa1470dem_set_freq(freq);
        //wa1470dem_set_freq(freq);
        rf_busy = 0;
        rf_state = STATE_RX;
        return OK;
    }
    //wa1470_tcxo_set_reset(0);
    rf_busy = 0;
    return ERR;
}

nbfi_status_t RF_Deinit()
{
    if(rf_busy) return ERR_RF_BUSY;
    //RF_SetModeAndPower(0, IDLE, PCB);
    if(__nbfi_before_off)   __nbfi_before_off();    
    rf_busy = 1;
    wa1470rfe_set_mode(RFE_MODE_DEEP_SLEEP);
    void wa1470rfe_deinit();
    rf_busy = 0;
    transmit = 0;
    rf_state = STATE_OFF;
    return OK;
}


nbfi_status_t RF_Transmit(uint8_t* pkt, uint8_t len, nbfi_phy_channel_t  phy_channel, rf_blocking_t blocking)
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






