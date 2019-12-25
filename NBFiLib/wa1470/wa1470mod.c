#include "wa1470.h"
#include "wtimer.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

#define MODOSCFREQ			26000000

struct wtimer_desc mod_callTXfinished_desc;

_Bool mod_i_q_or_bpsk_pin;

mod_hop_channels_t mod_current_hop_table[8] = {MOD_MINUS90000, MOD_MINUS65000, MOD_MINUS40000, MOD_MINUS15000, MOD_PLUS15000, MOD_PLUS40000, MOD_PLUS65000, MOD_PLUS90000};
mod_bitrate_s current_tx_phy;

extern void (*__wa1470_tx_finished)(void);
extern void (*__wa1470_nop_dalay_ms)(uint32_t);
void wa1470_bpsk_pin_send(uint8_t* data, mod_bitrate_s bitrate);

void wa1470mod_init(_Bool send_by_bpsk_pin)
{
	mod_i_q_or_bpsk_pin = send_by_bpsk_pin;
	if(send_by_bpsk_pin == WA1470_SEND_BY_I_Q_MODULATOR)
		wa1470mod_set_hop_table((mod_hop_channels_t *)mod_current_hop_table);
}

static void	wa1470mod_call_TX_finished(struct wtimer_desc *desc)
{
	__wa1470_tx_finished();
}

void wa1470_tx_finished()
{
  #ifdef NBFI_LOG
        sprintf(log_string, "%05u: TX finished ", (uint16_t)(NBFi_get_RTC()&0xffff));
	log_send_str(log_string);
#endif
 	ScheduleTask(&mod_callTXfinished_desc,	wa1470mod_call_TX_finished, RELATIVE, MILLISECONDS(2));
}

void wa1470mod_isr(void)
{
	uint8_t status;

	if(mod_i_q_or_bpsk_pin != WA1470_SEND_BY_I_Q_MODULATOR)
		return;
	
	wa1470_spi_read(MOD_STATUS, &status, 1);

	if(!(status&MOD_STATUS_IRQ_ON_TX_FLAG))
		return;
	
	wa1470_spi_write8(MOD_CONFIG, MOD_CONF_CLEAR_IRQ);
	
	wa1470_tx_finished();
}

void wa1470mod_send(uint8_t* data, mod_bitrate_s bitrate)
{
	if(mod_i_q_or_bpsk_pin == WA1470_SEND_BY_BPSK_PIN)
	{
		wa1470_bpsk_pin_send(data, bitrate);
		return;
	}
	
	wa1470mod_set_bitrate(bitrate);
	
	switch(bitrate)
	{
	case MOD_DBPSK_50_PROT_D:
	case MOD_DBPSK_400_PROT_D:
	case MOD_DBPSK_3200_PROT_D:
	case MOD_DBPSK_25600_PROT_D:
        #ifndef NBFI_OLD_PROTE  
        case MOD_DBPSK_50_PROT_E:
	case MOD_DBPSK_400_PROT_E:
	case MOD_DBPSK_3200_PROT_E:
	case MOD_DBPSK_25600_PROT_E: 
        #endif
		for(int i = 0; i != 36; i++) 
			wa1470_spi_write8(MOD_DATA_START + i, data[i]);
		//wa1470_spi_write(MOD_DATA_START, data, 36);
		wa1470_spi_write8(MOD_CONFIG, MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
		break;
        #ifdef NBFI_OLD_PROTE        
	case MOD_DBPSK_50_PROT_E:
	case MOD_DBPSK_400_PROT_E:
	case MOD_DBPSK_3200_PROT_E:
	case MOD_DBPSK_25600_PROT_E:
		for(int i = 0; i != 40; i++) 
			wa1470_spi_write8(MOD_DATA_START + i, data[i]);
		//wa1470_spi_write(MOD_DATA_START, data, 40);
		wa1470_spi_write8(MOD_CONFIG, MOD_CONF_PROT_E_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
		break;
        #endif
	case MOD_DBPSK_100H_PROT_D:
		for(int i = 0; i != 36; i++) 
			wa1470_spi_write8(MOD_DATA_START + i, data[i]);
		//wa1470_spi_write(MOD_DATA_START, data, 36);
		wa1470_spi_write8(MOD_CONFIG, MOD_CONF_HOP_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
		break;
	case MOD_DBPSK_100H_PROT_E:
		for(int i = 0; i != 40; i++) 
			wa1470_spi_write8(MOD_DATA_START + i, data[i]);
		//wa1470_spi_write(MOD_DATA_START, data, 40);
		wa1470_spi_write8(MOD_CONFIG, MOD_CONF_HOP_EN|MOD_CONF_PROT_E_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
		break;
	}
}

void wa1470mod_set_hop_table(mod_hop_channels_t *hop_table)
{
	for(uint8_t i = 0; i != 8; i++) 
	{
		wa1470_spi_write8(MOD_HOP_TBL_START + i, (uint8_t)hop_table[i]);
		mod_current_hop_table[i] = hop_table[i]; 
	}
}

uint16_t wa1470mod_phy_to_bitrate(mod_bitrate_s bitrate)
{
	switch(bitrate)
	{
	case MOD_DBPSK_50_PROT_D:
	case MOD_DBPSK_50_PROT_E:
	default:
		 return 50;
	case MOD_DBPSK_400_PROT_D:
	case MOD_DBPSK_400_PROT_E:
		return 400;
	case MOD_DBPSK_3200_PROT_D:
	case MOD_DBPSK_3200_PROT_E:
		return 3200;
	case MOD_DBPSK_25600_PROT_D:
	case MOD_DBPSK_25600_PROT_E:
		return 25600;
	case MOD_DBPSK_100H_PROT_D:
	case MOD_DBPSK_100H_PROT_E:
		return 100;
	}
}

void wa1470mod_set_bitrate(mod_bitrate_s bitrate)
{
	uint64_t rate = wa1470mod_phy_to_bitrate(bitrate);
	rate = rate*16777216;
	rate = ((rate%100000)>=5)?(rate/1000000 + 1):(rate/1000000);
	wa1470_spi_write8(MOD_PER0, *(((uint8_t*)&rate)+0));
	wa1470_spi_write8(MOD_PER1, *(((uint8_t*)&rate)+1));
	wa1470_spi_write8(MOD_PER2, *(((uint8_t*)&rate)+2));
	current_tx_phy = bitrate;
}

void wa1470mod_set_freq(uint32_t freq)
{
#ifdef NBFI_LOG
        sprintf(log_string, "%05u: mod_set_freq to %ld", ((uint16_t)(NBFi_get_RTC()&0xffff)), freq); 
	log_send_str(log_string);
#endif
	if(mod_i_q_or_bpsk_pin == WA1470_SEND_BY_BPSK_PIN)
	{
		wa1470rfe_set_freq(freq);
	}
	else
	{
		switch(current_tx_phy)
		{
		case MOD_DBPSK_100H_PROT_D:
		case MOD_DBPSK_100H_PROT_E:
			wa1470rfe_set_freq(freq);
			break;
		default:
			wa1470rfe_set_freq(freq + 90000);
			break;
		}
	}
}
