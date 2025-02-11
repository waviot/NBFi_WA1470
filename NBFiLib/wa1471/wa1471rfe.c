#ifdef WA1471
#include "wa1471.h"

uint16_t rfe_rx_total_vga_gain;
_Bool rfe_pll_mode_fractional = 0;
_Bool rfe_zero_gain_mode = 0;

static uint16_t slowfreq = 0;

void wa1471rfe_init()
{
	wa1471_hal->__wa1471_chip_enable();

        wa1471_spi_wait_for(RFE_INIT_DONE, 1, 0x01);

	wa1471rfe_set_mode(RFE_MODE_IDLE);
	wa1471_spi_write8(RFE_RX_MX_CTRL, 87);
	wa1471_spi_write8(RFE_ADC_REF, 222);
        wa1471_spi_write8(RFE_BYPASS, 218);
	wa1471_spi_write8(RFE_ADC_COMPVALID, 23);
	wa1471_spi_write8(RFE_GP_ADC_SETTING, 115);
	wa1471_spi_write8(RFE_GP_ADC_DELAY, 120);
	wa1471_spi_write8(RFE_LPF_TUNE, 136);
	wa1471_spi_write8(RFE_CLKGEN_SETTING_0, 70);
	wa1471_spi_write8(RFE_LNA_TUNE, 102);
	wa1471_spi_write8(RFE_CLKGEN_SETTING_1, 24);
	wa1471_spi_write8(RFE_CLKGEN_SETTING_2, 100);
	wa1471_spi_write8(RFE_ADC_Q_SETTINGS, 1);
	wa1471_spi_write8(RFE_TX_DAC_CLK, 107);
	wa1471_spi_write8(RFE_LOW_POWER, 16);
        wa1471_spi_write8(RFE_PLL_LOCK, 251);
        wa1471_spi_write8(RFE_POWER_CONTROL, 0xE0);

	wa1471rfe_set_pll_mode(RFE_PLL_MODE_FRACTIONAL);

	if(send_by_dbpsk)
		wa1471rfe_set_tx_mode(RFE_TX_MODE_BPSK);
	else
		wa1471rfe_set_tx_mode(RFE_TX_MODE_I_Q);

	wa1471rfe_set_rx_mode(RFE_RX_MODE_LONF);
	wa1471rfe_set_rx_gain(RFE_DEFAULT_VGA_GAIN);

}

void wa1471rfe_deinit()
{
	wa1471_hal->__wa1471_chip_disable();
        wa1471_hal->__spi_cs_set(0);
}

void wa1471rfe_set_mode(rfe_mode_s mode)
{
        wa1471_spi_write8(RFE_MODE, (uint8_t)mode);
	wa1471_spi_wait_for(RFE_SET_MODE_BUSY, 0, 0x01);
}

static void wa1471rfe_set_lna_current(uint8_t curr)
{
	wa1471_spi_write8(RFE_RX_LNA, (wa1471_spi_read8(RFE_RX_LNA)&0x87)+((curr&0x0f)<<3));
}

static void wa1471rfe_set_rx_gain_custom(uint8_t LNA_GAIN, uint8_t MIXER_GAIN, uint8_t VGA1_GAIN, uint8_t VGA2_GAIN)
{
	uint8_t data_r[1];
	wa1471_spi_read(RFE_RX_LNA, data_r, 1);
	wa1471_spi_write8(RFE_RX_LNA, (data_r[0]&(256-8)) + (LNA_GAIN<<1) +	(data_r[0]&1) );
	wa1471_spi_write8(RFE_RX_MX_CTRL, (wa1471_spi_read8(RFE_RX_MX_CTRL)&0xc3) + (MIXER_GAIN << 2));
	wa1471_spi_write8(RFE_RX_VGA_CTRL, ((8 - VGA1_GAIN)<<4) + (8 - VGA2_GAIN));
	rfe_rx_total_vga_gain = (VGA1_GAIN + VGA2_GAIN)*3;
}

void wa1471rfe_set_rx_gain(uint8_t gain)
{
	uint8_t vga1, vga2;
	if(gain > 48) gain = 48;
	gain = gain/3;
	if(gain <= 8)
	{
		vga1 = gain;
		vga2 = 0;
	}
	else
	{
		vga1 = 8;
		vga2 = gain - 8;
	}
	if(rfe_zero_gain_mode) wa1471rfe_set_rx_gain_custom(0,0,0,0);
    else wa1471rfe_set_rx_gain_custom(3,5,vga1,vga2);
}

void wa1471rfe_set_pll_mode(rfe_pll_mode_s mode)
{
	wa1471_spi_write8(RFE_VCO_CURRENT, (uint8_t)mode);
	rfe_pll_mode_fractional = (mode == RFE_PLL_MODE_FRACTIONAL);
}

void wa1471rfe_set_tx_mode(rfe_tx_mode_s mode)
{
	wa1471_spi_write8( RFE_IQ_TX_MODE, (uint8_t)mode);
}

void wa1471rfe_set_rx_mode(rfe_rx_mode_s mode)
{
	switch(mode)
	{
	case RFE_RX_MODE_LONF:
		wa1471rfe_set_lna_current(15);
		break;
	case RFE_BAND_LOCUR:
		wa1471rfe_set_lna_current(2);
		break;
	default:
		wa1471rfe_set_lna_current(7);
		break;
	}
}

void wa1471rfe_set_tx_power(int8_t power)
{
        power = ((int16_t)(power + 13))*21/28;
	wa1471_spi_write8( RFE_POWER_CONTROL, (wa1471_spi_read8(RFE_POWER_CONTROL)&0xe0) + (power&0x1f));
}

void wa1471rfe_set_band(rfe_band_s band)
{
	if(band == RFE_BAND_450)
	{
		wa1471_spi_write8(RFE_RX_LNA, wa1471_spi_read8(RFE_RX_LNA)&0x7f);
	}
	else
	{
		wa1471_spi_write8(RFE_RX_LNA, wa1471_spi_read8(RFE_RX_LNA)|0x80);
	}
}

_Bool wa1471rfe_set_freq(uint32_t freq)
{
	uint32_t FREQ;

	if(freq < 600000000)
	{
		FREQ	= freq*2;
		wa1471rfe_set_band(RFE_BAND_450);
	}
	else
	{
		FREQ	= freq;
		wa1471rfe_set_band(RFE_BAND_900);
	}

        uint8_t PLL_INT = FREQ/FREF;
	uint32_t PLL_FRAQ = (((uint64_t)(FREQ%FREF)) << 22)/FREF;
        if(rfe_pll_mode_fractional) wa1471_spi_write8(RFE_PLL_NINT, ((uint8_t)PLL_INT << 2) + 2);
        else wa1471_spi_write8(RFE_PLL_NINT, ((uint8_t)PLL_INT << 2));
        wa1471_spi_write8(RFE_PLL_NFRAQ0, PLL_FRAQ & 0xff);
	wa1471_spi_write8(RFE_PLL_NFRAQ1, (PLL_FRAQ >> 8) & 0xff);
	wa1471_spi_write8(RFE_PLL_NFRAQ2, ((PLL_FRAQ >> 15) & 0x7e) + 1);

	uint8_t tmp = wa1471_spi_read8(RFE_VCO_RUN);
	wa1471_spi_write8(RFE_VCO_RUN, tmp&0xdf);
	wa1471_spi_write8(RFE_VCO_RUN, tmp|0x20);
        if (wa1471_spi_wait_for(RFE_VCO_RESULT, 0x04, 0x04))  return 1;
        else {
          slowfreq++;
          return 0;
        }
}



void wa1471rfe_set_zero_gain_mode(_Bool mode)
{
    rfe_zero_gain_mode = mode;
}

uint16_t wa1471rfe_get_slowfreq()
{
  return slowfreq;
}


void wa1471rfe_transmit_carrier(uint32_t freq)
{
    wa1471rfe_set_tx_mode(RFE_TX_MODE_BPSK);
    wa1471rfe_set_freq(freq);
    wa1471rfe_set_mode(RFE_MODE_TX);
}
#endif //#ifdef WA1471
