#include "nbfi.h"
#include "preambula.h"

_Bool rf_busy = 0;
_Bool transmit = 0;


nbfi_rf_state_s rf_state = STATE_UNDEFINED;

nbfi_phy_channel_t nbfi_phy_channel;


static void _memcpy(uint8_t *dst, const uint8_t *src, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
		dst[len - i - 1] = src[i];
}

uint32_t NBFi_DL_ID()
{
  return *((uint32_t*)FULL_ID);
}

static nbfi_phy_channel_t last_phy = UNDEFINED;
static uint16_t last_additional_flags = 0;

static nbfi_rf_iface_t nbfi_rf_iface =
{
	.init = wa1470_init,
	.reinit = wa1470_reinit,
	.deinit = wa1470_deinit,
	.cansleep = wa1470_cansleep,

	.rfe_set_mode = wa1470rfe_set_mode,
	.rfe_set_tx_power = wa1470rfe_set_tx_power,

	.mod_set_freq = wa1470mod_set_freq,
	.mod_send = wa1470mod_send,
	.mod_is_tx_in_progress = wa1470mod_is_tx_in_progress,

	.dem_rx_enable = wa1470dem_rx_enable,
	.dem_set_bitrate = wa1470dem_set_bitrate,
	.dem_set_freq = wa1470dem_set_freq,
	.dem_get_rssi = wa1470dem_get_rssi,
	.dem_get_noise = wa1470dem_get_noise,
    .transmit_carrier = wa1470rfe_transmit_carrier,
};

void NBFI_RF_iface(nbfi_rf_iface_t iface)
{
	nbfi_rf_iface = iface;
}

nbfi_status_t NBFi_RF_Init(  nbfi_phy_channel_t  phy_channel,
                        nbfi_rf_antenna_t        antenna,
                        int8_t              power,
                        uint32_t            freq)
{

    static int8_t last_tx_prw;
    static uint32_t last_tx_freq;
    static uint32_t last_rx_freq;

    static uint32_t _preambule = 0;
    static uint32_t last_dl_add = 0;

	const uint32_t protD_preambula = 0x6f7a1597;//0x97157a6f;

	if(!_preambule || (last_dl_add != NBFi_DL_ID()))
    {
        last_dl_add = NBFi_DL_ID();
		uint32_t preambule_tmp = preambula(NBFi_DL_ID(), (uint32_t *)0, (uint32_t *)0);
		_memcpy((uint8_t *)&_preambule, (uint8_t *)&preambule_tmp, 4);
    }


    if(rf_busy) return ERR_RF_BUSY;

    rf_busy = 1;

    if((last_phy != phy_channel) || ((nbfi.additional_flags&NBFI_FLG_RX_DEFAULT_PREAMBLE)!=(last_additional_flags&NBFI_FLG_RX_DEFAULT_PREAMBLE)))
    {

      if (nbfi_rf_iface.reinit != NULL)
        nbfi_rf_iface.reinit((nbfi.additional_flags&NBFI_FLG_RX_DEFAULT_PREAMBLE)?protD_preambula:_preambule);
      last_tx_prw = 100;
      last_tx_freq = 0;
      last_rx_freq = 0;
      last_phy = UNDEFINED;
    }
    last_additional_flags = nbfi.additional_flags;
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
        if (nbfi_rf_iface.dem_rx_enable != NULL)
          nbfi_rf_iface.dem_rx_enable(0);
        nbfi_hal->__nbfi_before_tx(&nbfi);

        if(freq != last_tx_freq)
        {
          nbfi_state.last_tx_freq = last_tx_freq = freq;
          if (nbfi_rf_iface.mod_set_freq != NULL)
            nbfi_rf_iface.mod_set_freq(freq);

        }

        if(power != last_tx_prw)
        {
          last_tx_prw = power;
          if (nbfi_rf_iface.rfe_set_tx_power != NULL)
            nbfi_rf_iface.rfe_set_tx_power(power);
        }

        if (nbfi_rf_iface.rfe_set_mode != NULL)
          nbfi_rf_iface.rfe_set_mode(RFE_MODE_TX);

        rf_busy = 0;
        rf_state = STATE_TX;
        last_phy = phy_channel;
        return OK;
    case UL_CARRIER:
        if (nbfi_rf_iface.dem_rx_enable != NULL)
          nbfi_rf_iface.dem_rx_enable(0);
        nbfi_rf_iface.rfe_set_tx_power(power);
        nbfi_rf_iface.transmit_carrier(freq);
        rf_busy = 0;
        return OK;
    case DL_DBPSK_50_PROT_D:
    case DL_DBPSK_400_PROT_D:
    case DL_DBPSK_3200_PROT_D:
    case DL_DBPSK_25600_PROT_D:
    case DL_DBPSK_100H_PROT_D:
        nbfi_hal->__nbfi_before_rx(&nbfi);
        if (nbfi_rf_iface.dem_rx_enable != NULL)
          nbfi_rf_iface.dem_rx_enable(1);

        if(last_phy != phy_channel)
          if (nbfi_rf_iface.dem_set_bitrate != NULL)
            nbfi_rf_iface.dem_set_bitrate((dem_bitrate_s)phy_channel);

        if (nbfi_rf_iface.rfe_set_mode != NULL)
          nbfi_rf_iface.rfe_set_mode(RFE_MODE_RX);

        if(freq != last_rx_freq)
        {
          nbfi_state.last_rx_freq = last_rx_freq = freq;
          if (nbfi_rf_iface.dem_set_freq != NULL)
            nbfi_rf_iface.dem_set_freq(freq);
        }
        //nbfi_rf_iface.dem_set_freq(freq);
        rf_busy = 0;
        rf_state = STATE_RX;
        last_phy = phy_channel;
    }
    rf_busy = 0;
    return ERR;
}

nbfi_status_t NBFi_RF_Deinit()
{
    if(rf_busy) return ERR_RF_BUSY;
    if (nbfi_rf_iface.dem_rx_enable != NULL)
      nbfi_rf_iface.dem_rx_enable(0);
    nbfi_hal->__nbfi_before_off(&nbfi);
    rf_busy = 1;
    //nbfi_rf_iface.rfe_set_mode(RFE_MODE_DEEP_SLEEP);
    if (nbfi_rf_iface.deinit != NULL)
      nbfi_rf_iface.deinit();
    rf_busy = 0;
    transmit = 0;
    rf_state = STATE_OFF;
    last_phy = UNDEFINED;
    return OK;
}


nbfi_status_t NBFi_RF_Transmit(uint8_t* pkt, uint8_t len, nbfi_phy_channel_t  phy_channel, rf_blocking_t blocking)
{
    if(rf_busy) return ERR_RF_BUSY;

    rf_busy = 1;

    if (nbfi_rf_iface.mod_send != NULL)
      nbfi_rf_iface.mod_send(pkt, (mod_bitrate_s) phy_channel);

    rf_busy = 0;

    transmit = 1;

    if(blocking == BLOCKING)
    {

        while(1) // Wait for TX complete
        {
            if(!transmit) break;
            nbfi_scheduler->__scheduler_run_callbacks();
        }

    }

    return OK;
}

void NBFi_RF_TX_Finished()
{
  NBFi_TX_Finished();
}

_Bool NBFi_RF_is_TX_in_Progress()
{
  if((rf_state == STATE_TX)||(rf_state == STATE_RX))
  {
      if (nbfi_rf_iface.mod_is_tx_in_progress != NULL)
        return nbfi_rf_iface.mod_is_tx_in_progress();
      else
        return 0;
  }
  else return 0;
}

float NBFi_RF_get_noise()
{
  if (nbfi_rf_iface.dem_get_noise != NULL)
  {
    if(nbfi_rf_iface.dem_get_noise() < -150) return -150;
    else return nbfi_rf_iface.dem_get_noise();
  }
  else
    return -150;
}

float NBFi_RF_get_rssi()
{
  if (nbfi_rf_iface.dem_get_rssi != NULL)
  {
    if(rf_state == STATE_RX) return nbfi_rf_iface.dem_get_rssi();
    else return 0; //unavailable
  }
  else
    return 0;

}


_Bool NBFi_RF_can_Sleep()
{
  if (nbfi_rf_iface.cansleep != NULL)
    return nbfi_rf_iface.cansleep();
  else
    return 1;
}

