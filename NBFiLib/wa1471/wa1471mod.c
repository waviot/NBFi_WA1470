#ifdef WA1471
#include "wa1471.h"

#define MODOSCFREQ			26000000

struct scheduler_desc mod_callTXfinished_desc;

mod_hop_channels_t mod_current_hop_table[8] = {MOD_MINUS97000, MOD_MINUS65000, MOD_MINUS40000, MOD_MINUS15000, MOD_PLUS15000, MOD_PLUS40000, MOD_PLUS65000, MOD_PLUS90000};
const int32_t MOD_FREQ_OFFSETS[32] = {-97000,-89000,-83000,-90000,-79000,-73000,-59000,-65000,-53000,-47000,-37000,-40000,-29000,-19000,-11000,-15000,15000,11000,19000,29000,40000,37000,47000,53000,65000,59000,73000,79000,90000,83000,89000,97000};

mod_bitrate_s current_tx_phy;

void wa1471_bpsk_pin_send(uint8_t* data, mod_bitrate_s bitrate);
void wa1471mod_init()
{
	if(send_by_dbpsk == WA1471_SEND_BY_I_Q_MODULATOR)
          wa1471mod_set_hop_table((mod_hop_channels_t *)mod_current_hop_table);

}

static void	wa1471mod_call_TX_finished(struct scheduler_desc *desc)
{
	if(wa1471_hal->__wa1471_tx_finished) wa1471_hal->__wa1471_tx_finished();
}

void wa1471_tx_finished()
{
  #ifdef WA1471_LOG
        sprintf(wa1471_log_string, "%05u: TX finished ", (uint16_t)(wa1471_scheduler->__scheduler_curr_time()&0xffff));
	wa1471_hal->__wa1471_log_send_str(wa1471_log_string);
  #endif
 	wa1471_scheduler->__scheduler_add_task(&mod_callTXfinished_desc,	wa1471mod_call_TX_finished, RELATIVE, MILLISECONDS(1));
}

void wa1471mod_isr(void)
{
	uint8_t status;

	if(send_by_dbpsk != WA1471_SEND_BY_I_Q_MODULATOR)
		return;

	wa1471_spi_read(MOD_STATUS, &status, 1);

	if(!(status&MOD_STATUS_IRQ_ON_TX_FLAG))
		return;

	wa1471_spi_write8(MOD_CONFIG, MOD_CONF_CLEAR_IRQ);

	wa1471_tx_finished();
}

void wa1471mod_send(uint8_t* data, mod_bitrate_s bitrate)
{
	if(send_by_dbpsk == WA1471_SEND_BY_BPSK_PIN)
	{
		wa1471_bpsk_pin_send(data, bitrate);
		return;
	}

	wa1471mod_set_bitrate(bitrate);

	switch(bitrate)
	{
	case MOD_DBPSK_50_PROT_D:
	case MOD_DBPSK_400_PROT_D:
	case MOD_DBPSK_3200_PROT_D:
	case MOD_DBPSK_25600_PROT_D:
    case MOD_DBPSK_50_PROT_E:
	case MOD_DBPSK_400_PROT_E:
	case MOD_DBPSK_3200_PROT_E:
	case MOD_DBPSK_25600_PROT_E:
    default:
		for(int i = 0; i != 36; i++)
			wa1471_spi_write8(MOD_DATA_START + i, data[i]);
		wa1471_spi_write8(MOD_CONFIG, MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
		break;
	case MOD_DBPSK_100H_PROT_D:
		for(int i = 0; i != 36; i++)
			wa1471_spi_write8(MOD_DATA_START + i, data[i]);
		wa1471_spi_write8(MOD_CONFIG, MOD_CONF_HOP_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
		break;
	case MOD_DBPSK_100H_PROT_E:
		for(int i = 0; i != 40; i++)
			wa1471_spi_write8(MOD_DATA_START + i, data[i]);
		wa1471_spi_write8(MOD_CONFIG, MOD_CONF_HOP_EN|MOD_CONF_PROT_E_EN|MOD_CONF_IRQ_ON_TX_END_EN|MOD_CONF_CLEAR_IRQ|MOD_CONF_TX_START);
		break;
	}
}

void wa1471mod_set_hop_table(mod_hop_channels_t *hop_table)
{
	for(uint8_t i = 0; i != 8; i++)
	{
		wa1471_spi_write8(MOD_HOP_TBL_START + i, (uint8_t)hop_table[i]);
		mod_current_hop_table[i] = hop_table[i];
	}
}

uint16_t wa1471mod_phy_to_bitrate(mod_bitrate_s bitrate)
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

void wa1471mod_set_bitrate(mod_bitrate_s bitrate)
{
	uint64_t rate = wa1471mod_phy_to_bitrate(bitrate);
	rate = rate*16777216;
	rate = ((rate%100000)>=5)?(rate/1000000 + 1):(rate/1000000);
	wa1471_spi_write8(MOD_PER0, *(((uint8_t*)&rate)+0));
	wa1471_spi_write8(MOD_PER1, *(((uint8_t*)&rate)+1));
	wa1471_spi_write8(MOD_PER2, *(((uint8_t*)&rate)+2));
	current_tx_phy = bitrate;
}

void wa1471mod_set_freq(uint32_t freq)
{
#ifdef WA1471_LOG
        sprintf(wa1471_log_string, "%05u: mod_set_freq to %ld", ((uint16_t)(wa1471_scheduler->__scheduler_curr_time()&0xffff)), freq);
	wa1471_hal->__wa1471_log_send_str(wa1471_log_string);
#endif
	if(send_by_dbpsk == WA1471_SEND_BY_BPSK_PIN)
	{
		wa1471rfe_set_freq(freq);
	}
	else
	{
		switch(current_tx_phy)
		{
		case MOD_DBPSK_100H_PROT_D:
		case MOD_DBPSK_100H_PROT_E:
			wa1471rfe_set_freq(freq);
			break;
		default:
			wa1471rfe_set_freq(freq - MOD_FREQ_OFFSETS[mod_current_hop_table[0]]);
			break;
		}
	}
}

_Bool wa1471mod_is_tx_in_progress()
{
  return (wa1471_spi_read8(MOD_STATUS)&MOD_STATUS_TX_IN_PROGRESS);
}
#endif //#ifdef WA1471