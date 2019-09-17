#ifndef _wa1470RFE_H
#define _wa1470RFE_H


#define FREF    26000000
#define RFE_DEFAULT_VGA_GAIN    24

//----------------------------------------------------------
// RADIO FRONT-END REGs
//----------------------------------------------------------

#define RFE_VCO_CURRENT                 0x4000
#define RFE_RX_LNA 	                0x4002
#define RFE_RX_MX_CTRL 	                0x4003

#define RFE_ADC_REF			0x4004
#define RFE_BYPASS      	        0x4005

#define RFE_IQ_TX_MODE			0x4006


#define RFE_ADC_COMPVALID		0x4008
#define RFE_GP_ADC_SETTING		0x400A
#define RFE_GP_ADC_DELAY		0x400C
#define RFE_LPF_TUNE			0x400D
#define RFE_CLKGEN_SETTING_0	        0x400E
#define RFE_LNA_TUNE			0x400F
#define RFE_CLKGEN_SETTING_1	        0x4010
#define RFE_CLKGEN_SETTING_2	        0x4011

#define RFE_ADC_Q_SETTINGS		0x4013
#define RFE_TX_DAC_CLK			0x4016
#define RFE_LOW_POWER			0x4019
#define RFE_PLL_NINT			0x4023
#define RFE_PLL_NFRAQ0			0x4026
#define RFE_PLL_NFRAQ1			0x4027
#define RFE_PLL_NFRAQ2			0x4028
#define RFE_RX_VGA_CTRL	                0x4029
#define RFE_VCO_RUN	                0x4030
#define RFE_VCO_RESULT          	0x4034
#define RFE_BB_TUNER			0x402A
#define RFE_POWER_CONTROL               0x402D
#define RFE_MODE                        0x402F
#define RFE_CLOSE_PLL_LOOP		0x4030
#define ADDR_1_8_V_FRACTIONAL_PLL_MODE	0x4032
#define RFE_INIT_DONE      		0x4039
#define RFE_SET_MODE_BUSY      		0x403B


/*#define  MODE_DEEP_SLEEP  (uint8_t) 128
#define  MODE_SLEEP   (uint8_t)64
#define  MODE_IDLE  (uint8_t)32
#define  MODE_TX (uint8_t) 16
#define  MODE_RX  (uint8_t) 0*/

typedef enum
{
        RFE_MODE_RX             = 0,
        RFE_MODE_TX             = 16,
        RFE_MODE_IDLE           = 32,
        RFE_MODE_SLEEP          = 64,
        RFE_MODE_DEEP_SLEEP     = 128
}rfe_mode_s;


typedef enum
{
        RFE_BAND_450            = 1,
        RFE_BAND_900            = 2
}rfe_band_s;

typedef enum
{
        RFE_PLL_MODE_INTEGER        = 170,
        RFE_PLL_MODE_FRACTIONAL     = 174
}rfe_pll_mode_s;

typedef enum
{
        RFE_TX_MODE_I_Q         = 122,
        RFE_TX_MODE_BPSK        = 123
}rfe_tx_mode_s;

typedef enum
{
        RFE_RX_MODE_NORMAL          = 0,
        RFE_RX_MODE_LONF            = 1,
        RFE_BAND_LOCUR              = 2
}rfe_rx_mode_s;

void wa1470rfe_init(_Bool send_by_bpsk_pin);
void wa1470rfe_deinit();
//void wa1470rfe_reset();
//void wa1470rfe_enable();
//void wa1470rfe_disable();
void wa1470rfe_set_mode(rfe_mode_s mode);
void wa1470rfe_set_pll_mode(rfe_pll_mode_s mode);
void wa1470rfe_set_tx_mode(rfe_tx_mode_s mode);
void wa1470rfe_set_rx_mode(rfe_rx_mode_s mode);
void wa1470rfe_set_tx_power(uint8_t power);
void wa1470rfe_set_band(rfe_band_s band);
_Bool wa1470rfe_set_freq(uint32_t freq);

void  wa1470rfe_set_rx_gain(uint8_t gain);
extern uint16_t rfe_rx_total_vga_gain;
//extern uint32_t rfe_logoffset;

#endif