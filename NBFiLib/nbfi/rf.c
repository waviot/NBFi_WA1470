#include "nbfi.h"
#include "nbfi_config.h"
#include "rf.h"
#include "nbfi_misc.h"
#include <libmfwtimer.h>

_Bool rf_busy = 0;
_Bool transmit = 0;

//struct axradio_address rf_destination;

nbfi_rf_state_s rf_state = STATE_OFF;

nbfi_phy_channel_t nbfi_phy_channel;

uint8_t PSK_BAND;

NBFi_wa1205_pins_s  nbfi_wa1205_pins;

void    NBFi_TX_Finished();
void    NBFi_ParseReceivedPacket(struct axradio_status *st);
void    wa1205_set_constants(void);


const uint16_t AX5043_power[26] = {0x00aa, 0x00bf, 0x00d1, 0x00ec, 0x010f, 0x0132, 0x0156, 0x017f, 0x01af, 0x1e0, 0x207, 0x244, 0x290, 0x2eb, 0x35e, 0x3d6, 0x406, 0x4a9, 0x57c, 0x600, 0x700, 0x800, 0x9d4, 0xc00, 0xf00, 0xfff};

/*
struct axradio_address  fastdladdress = {
	{ 0x6f, 0x6f, 0x6f, 0x6f}
};
*/
extern void (* __nbfi_before_tx)(NBFi_wa1205_pins_s *);
extern void (* __nbfi_before_rx)(NBFi_wa1205_pins_s *);
extern void (* __nbfi_before_off)(NBFi_wa1205_pins_s *);

void RF_SetModeAndPower(int8_t dBm, rf_direction_t mode, rf_antenna_t ant)
{
    switch(mode)
    {
    case TX:    
      nbfi_wa1205_pins.txpwr = AX5043_power[dBm + 10];
      nbfi_wa1205_pins.cfga = PA_DIFFERENTIAL;            
      if(__nbfi_before_tx) __nbfi_before_tx(&nbfi_wa1205_pins);
      break;
    case RX: 
      nbfi_wa1205_pins.cfga = PA_DIFFERENTIAL;
      if(__nbfi_before_rx) __nbfi_before_rx(&nbfi_wa1205_pins);
      break;
    case IDLE:
      if(__nbfi_before_off)   __nbfi_before_off(&nbfi_wa1205_pins);
      break;
    }

}

nbfi_status_t RF_Init(  nbfi_phy_channel_t  phy_channel,
                        rf_antenna_t        antenna,
                        int8_t              power,
                        uint32_t            freq)
{

    if(rf_busy) return ERR_RF_BUSY;

    rf_busy = 1;

    RF_SetModeAndPower(power, RX, antenna);
    
    wa1205_set_freq(freq);   

    wa1205_tcxo_set_reset(1);


    switch(phy_channel)
    {
      
    case UL_DBPSK_50_PROT_D:
    case UL_DBPSK_400_PROT_D:
    case UL_DBPSK_3200_PROT_D:
    case UL_DBPSK_25600_PROT_D:
 
        RF_SetModeAndPower(power, TX, antenna);
        
        wa1205_set_freq(freq);
        
       // er = axradio_init();    // Init radio registers
   
        rf_busy = 0;
        rf_state = STATE_TX;
        return OK;
       
    case DL_DBPSK_50_PROT_D:
    case DL_DBPSK_400_PROT_D:
    case DL_DBPSK_3200_PROT_D:
    case DL_DBPSK_25600_PROT_D:
  
        RF_SetModeAndPower(power, RX, antenna);

       // RF_SetLocalAddress((uint8_t *)&fastdladdress);

        wa1205_set_freq(freq);

        //er = axradio_init();    // Init radio registers

        rf_busy = 0;
        rf_state = STATE_RX;
        return OK;

    }
    wa1205_tcxo_set_reset(0);
    rf_busy = 0;
    return ERR;
}

nbfi_status_t RF_Deinit()
{
    if(rf_busy) return ERR_RF_BUSY;
    RF_SetModeAndPower(0, IDLE, PCB);
    rf_busy = 1;

//    er = axradio_set_mode(WARADIO_MODE_OFF);
    rf_busy = 0;
    transmit = 0;

    wa1205_tcxo_set_reset(0);
    rf_state = STATE_OFF;

    return OK;
}


nbfi_status_t RF_Transmit(uint8_t* pkt, uint8_t len,  rf_padding_t padding, rf_blocking_t blocking)
{
    if(rf_busy) return ERR_RF_BUSY;

    rf_busy = 1;

   // axradio_transmit(&rf_destination, pkt, len, padding);
    
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






