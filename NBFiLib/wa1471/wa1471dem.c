#ifdef WA1471
#include "wa1471.h"

#define DEM_MAS_SIZE	8


dem_packet_st dem_mas[DEM_MAS_SIZE];

//dem_packet_info_st dem_info_mas[DEM_MAS_SIZE];

uint8_t	dem_mess_received;

struct scheduler_desc dem_processMessages_desc;

dem_packet_st tmp_dem_mas[DEM_MAS_SIZE];
//dem_packet_info_st tmp_dem_info_mas[DEM_MAS_SIZE];
uint8_t	tmp_dem_mess_received;

dem_bitrate_s current_rx_phy = DBPSK_100H_PROT_D;

_Bool dem_rx_enabled = 0;

//uint8_t current_hop_table[8] = {DEM_MINUS90000,DEM_MINUS40000,DEM_PLUS40000,DEM_PLUS15000,DEM_MINUS15000,DEM_MINUS40000,DEM_MINUS65000,DEM_MINUS90000};

#define DEM_FREQ_OFFSET         50000

//const int32_t DEM_FREQ_OFFSETS[8] = {90000,65000,40000,15000,-15000,-40000,-65000,-90000};

struct scheduler_desc dem_update_noise_desc;
float dem_noise = -150;
static void wa1471dem_update_noise(struct scheduler_desc *desc);

uint32_t last_rx_freq = 0;

#ifdef DEM_CALC_SPECTRUM
uint32_t dem_spectrum_mas[32];
#endif


void wa1471dem_init(uint32_t preambule)
{

	uint8_t NB_FI_RX_CRC_POLY[4] = {0xb7, 0x1d, 0xc1, 0x04};
	wa1471_spi_write8(DEM_CONTROL, DEM_CONTROL_RESET);
        wa1471dem_set_threshold(800); //1024
	wa1471dem_set_alpha(128, 5);
	
        wa1471dem_set_crc_poly(NB_FI_RX_CRC_POLY);
	
       // wa1471dem_set_hop_table(current_hop_table);
	
        //wa1471dem_set_hop_len(36);
	if(preambule) wa1471dem_set_preambule((uint8_t *)&preambule);
	wa1471_spi_write8(DEM_FFT_MSB, 0x80 + 23); //?
	wa1471_spi_write8(DEM_CONTROL, 0);
        wa1471dem_update_noise(0);
}

void wa1471dem_reset(void)
{
	wa1471_spi_write8(DEM_CONTROL, DEM_CONTROL_RESET);
	wa1471_spi_write8(DEM_CONTROL, 0);
}

static uint8_t wa1471dem_get_bitrate_gain(dem_bitrate_s bitrate)
{
	switch(bitrate)
	{
	case DBPSK_50_PROT_D:
		return 0;
	case DBPSK_400_PROT_D:
		return 18;
	case DBPSK_3200_PROT_D:
		return 56;
	case DBPSK_25600_PROT_D:
		return 112;
	case DBPSK_100H_PROT_D:
		return 6;
	default:
		return 1;
	}
}

#define DEM_LOGOFFSET  208
static int16_t wa1471dem_get_rssi_logoffset()
{
	return DEM_LOGOFFSET + rfe_rx_total_vga_gain - wa1471dem_get_bitrate_gain(current_rx_phy);
}

static void  wa1471dem_process_messages(struct scheduler_desc *desc)
{
	wa1471_hal->__wa1471_disable_pin_irq();

	tmp_dem_mess_received = dem_mess_received;
	memcpy(tmp_dem_mas, dem_mas, sizeof(tmp_dem_mas));
	//memcpy(tmp_dem_info_mas, dem_info_mas, sizeof(tmp_dem_info_mas));
	dem_mess_received = 0;
	memset(dem_mas, 0 , sizeof(dem_mas));
	//memset(dem_info_mas, 0 , sizeof(dem_info_mas));
    dem_packet_info_st info;


	wa1471_hal->__wa1471_enable_pin_irq();

	wa1471dem_update_noise(0);

	if(wa1471_hal->__wa1471_data_received)
	{
		for(uint8_t i = 0; i != tmp_dem_mess_received; i++)
		{
#ifdef WA1471_LOG
			sprintf(wa1471_log_string, "%05u: PLD: ", (uint16_t)(wa1471_scheduler->__scheduler_curr_time()&0xffff));
			for(uint8_t k = 0; k != 8; k++)
				sprintf(wa1471_log_string + strlen(wa1471_log_string), "%02X", tmp_dem_mas[i].packet.payload[k]);
			sprintf(wa1471_log_string + strlen(wa1471_log_string), " IT crypto=%3d FREQ=%2d", tmp_dem_mas[i].packet.iter, tmp_dem_mas[i].freq&0x1f);
#endif
                        uint64_t rssi64 = tmp_dem_mas[i].rssi_39_32;
			rssi64 <<= 32;
			rssi64 += tmp_dem_mas[i].rssi;
			float rssi = log10f(rssi64)*20 - 48 - wa1471dem_get_rssi_logoffset();
			info.rssi = (int16_t)rssi;
			float snr = rssi - dem_noise;
			if(snr < 0) snr = 0;
			info.snr = (uint8_t)snr;
            info.freq = last_rx_freq;
            info.bitrate = current_rx_phy;
            switch(current_rx_phy)
			{
                case DBPSK_50_PROT_D:
                    info.freq += tmp_dem_mas[i].freq*50;
                    break;
                case DBPSK_400_PROT_D:
                    if(tmp_dem_mas[i].freq < 2)
                        info.freq += tmp_dem_mas[i].freq*400;
                    else info.freq -= tmp_dem_mas[i].freq*400;
                    break;
            }


#ifdef WA1471_LOG
            float dsnr = log10f(((float)rssi64)/tmp_dem_mas[i].noise/4)*20;
			sprintf(wa1471_log_string + strlen(wa1471_log_string), " RSSI=%ld", tmp_dem_mas[i].rssi);
			sprintf(wa1471_log_string + strlen(wa1471_log_string), " LRSSI=%f", rssi);
			sprintf(wa1471_log_string + strlen(wa1471_log_string), " SNR=%f", rssi - dem_noise);
			sprintf(wa1471_log_string + strlen(wa1471_log_string), " DSNR=%f", dsnr);

			switch(current_rx_phy)
			{
			case DBPSK_50_PROT_D:
				sprintf(wa1471_log_string + strlen(wa1471_log_string), " 50BPS");
				break;
			case DBPSK_400_PROT_D:
				sprintf(wa1471_log_string + strlen(wa1471_log_string), " 400BPS");
				break;
			case DBPSK_3200_PROT_D:
				sprintf(wa1471_log_string + strlen(wa1471_log_string), " 3200BPS");
				break;
			case DBPSK_25600_PROT_D:
				sprintf(wa1471_log_string + strlen(wa1471_log_string), " 25600BPS");
				break;
			case DBPSK_100H_PROT_D:
				sprintf(wa1471_log_string + strlen(wa1471_log_string), " 100HBPS");
				break;

			}
			wa1471_hal->__wa1471_log_send_str(wa1471_log_string);
#endif
			wa1471_hal->__wa1471_data_received((uint8_t*)&tmp_dem_mas[i].packet, (uint8_t*)&info);
		}
	}
}

void wa1471dem_isr(void)
{
	uint8_t status;

	wa1471_spi_read(DEM_CONTROL, &status, 1);

	if(!(status&DEM_CONTROL_IRQ_FLAG))
		return;
	dem_packet_st new_packet;

	do
	{
		wa1471_spi_read(DEM_RECEIVED_MES_BUF, (uint8_t*)(&new_packet), 32);

		wa1471_spi_write8(DEM_RECEIVED_MES_BUF, 0);


		dem_mas[dem_mess_received] = new_packet;


		if(dem_mess_received == (DEM_MAS_SIZE - 1))
			return;

		wa1471_scheduler->__scheduler_add_task(&dem_processMessages_desc,  wa1471dem_process_messages, RELATIVE, (current_rx_phy == DBPSK_25600_PROT_D)?MILLISECONDS(10):MILLISECONDS(20));
                wa1471_scheduler->__scheduler_add_task(&dem_update_noise_desc,  wa1471dem_update_noise, RELATIVE, MILLISECONDS(DEM_NOISE_TICK));
		uint8_t i;
		for(i = 0; i < dem_mess_received; i++)
		{
			if(dem_mas[i].packet.flags == dem_mas[dem_mess_received].packet.flags)
				break;
		}

		/*if(new_packet.crc_or_zigzag)
			dem_info_mas[i].num_of_zigzag++;
		else
			dem_info_mas[i].num_of_crc++;*/

		if(i == dem_mess_received)
		{
			if(++dem_mess_received > 1);
		}
		else
		{
			if((dem_mas[i].rssi_39_32 < new_packet.rssi_39_32))
			{
				dem_mas[i] = dem_mas[dem_mess_received];
			}
			else if((dem_mas[i].rssi_39_32 == new_packet.rssi_39_32) && (dem_mas[i].rssi < new_packet.rssi))
			{
				dem_mas[i] = dem_mas[dem_mess_received];
			}
		}
	}
	while (wa1471_hal->__wa1471_get_irq_pin_state());
}

int16_t wa1471dem_get_bitrate_sensitivity(dem_bitrate_s bitrate)
{
	switch(bitrate)
	{
	case DBPSK_50_PROT_D:
	default:
		return -148;
	case DBPSK_400_PROT_D:
		return -139;
	case DBPSK_3200_PROT_D:
		return -130;
	case DBPSK_25600_PROT_D:
		return -118;
	case DBPSK_100H_PROT_D:
		return -145;
	}
}

static int8_t wa1471dem_get_sensitivity_diff(dem_bitrate_s bitrate_1, dem_bitrate_s bitrate_2)
{
	return wa1471dem_get_bitrate_sensitivity(bitrate_1) - wa1471dem_get_bitrate_sensitivity(bitrate_2);
}

void wa1471dem_set_bitrate(dem_bitrate_s bitrate)
{

	switch(bitrate)
	{
	case DBPSK_50_PROT_D:
		wa1471_spi_write8(DEM_RX_MODE, 0);
		wa1471rfe_set_rx_gain(RFE_DEFAULT_VGA_GAIN);
	break;
	case DBPSK_400_PROT_D:
		wa1471_spi_write8(DEM_RX_MODE, 1);
		wa1471rfe_set_rx_gain(RFE_DEFAULT_VGA_GAIN);
	break;
	case DBPSK_3200_PROT_D:
		wa1471_spi_write8(DEM_RX_MODE, 2);
		wa1471rfe_set_rx_gain(RFE_DEFAULT_VGA_GAIN + 6);
	break;
	case DBPSK_25600_PROT_D:
		wa1471_spi_write8(DEM_RX_MODE, 3);
		wa1471rfe_set_rx_gain(RFE_DEFAULT_VGA_GAIN + 24);
	break;
	case DBPSK_100H_PROT_D:
		wa1471_spi_write8(DEM_RX_MODE, 4);
		wa1471rfe_set_rx_gain(RFE_DEFAULT_VGA_GAIN);
	break;
	}
	dem_noise -= wa1471dem_get_sensitivity_diff(current_rx_phy, bitrate);
	if(current_rx_phy != bitrate) wa1471dem_update_noise(0); //reinit noise engine
	current_rx_phy = bitrate;
	wa1471dem_reset();

        #ifdef WA1471_LOG
	sprintf(wa1471_log_string, "%05u: dem_set_bitrate to %d", ((uint16_t)(wa1471_scheduler->__scheduler_curr_time()&0xffff)), bitrate);
	wa1471_hal->__wa1471_log_send_str(wa1471_log_string);
        #endif
}

void wa1471dem_set_alpha(uint8_t noise_start_bit, uint8_t shift)
{
	wa1471_spi_write8(DEM_NOSE_START_BIT, noise_start_bit);
	wa1471_spi_write8(DEM_ALPHA_SHIFT, shift);
}

void wa1471dem_set_threshold(uint16_t SOFT_DETECT_THR)
{
	uint8_t  SOFT_DETECT_THR_ARR[2] = {SOFT_DETECT_THR & 255, SOFT_DETECT_THR >> 8};
	wa1471_spi_write(DEM_DET_TRESHOLD, SOFT_DETECT_THR_ARR, 2);
}


void wa1471dem_set_crc_poly(uint8_t* crc)
{
	wa1471_spi_write(DEM_CRC_POLY, crc, 4);
}

/*
void wa1471dem_set_hop_len(uint8_t hop_len)
{
	wa1471_spi_write8(DEM_HOP_LENGTH, hop_len);
}*/

void wa1471dem_set_preambule(uint8_t* preambule)
{
	wa1471_spi_write(DEM_PREAMBLE_ID, preambule, 4);
}

/*
void wa1471dem_set_hop_table(uint8_t* hop)
{
	wa1471_spi_write8(DEM_HOP_TABLE ,  (hop[1] << 4) | hop[0]);
	wa1471_spi_write8(DEM_HOP_TABLE+1, (hop[3] << 4) | hop[2]);
	wa1471_spi_write8(DEM_HOP_TABLE+2, (hop[5] << 4) | hop[4]);
	wa1471_spi_write8(DEM_HOP_TABLE+3, (hop[7] << 4) | hop[6]);
	for(int i = 0; i<8; i++ )
		current_hop_table[i] = hop[i];
}*/

void wa1471dem_set_freq(uint32_t freq)
{
	switch(current_rx_phy)
	{
	case DBPSK_50_PROT_D:
	case DBPSK_400_PROT_D:
	case DBPSK_3200_PROT_D:
		wa1471rfe_set_freq(freq - DEM_FREQ_OFFSET/*DEM_FREQ_OFFSETS[current_hop_table[0]]*/);
		break;
	case DBPSK_25600_PROT_D:
		wa1471rfe_set_freq(freq - DEM_FREQ_OFFSET/*DEM_FREQ_OFFSETS[current_hop_table[1]]*/);
		break;
	case DBPSK_100H_PROT_D:
		wa1471rfe_set_freq(freq);
		break;
	}
    last_rx_freq = freq;

        #ifdef WA1471_LOG
	sprintf(wa1471_log_string, "%05u: dem_set_freq to %ld", ((uint16_t)(wa1471_scheduler->__scheduler_curr_time()&0xffff)), freq);
	wa1471_hal->__wa1471_log_send_str(wa1471_log_string);
        #endif
}

static uint32_t wa1471dem_get_rssi_int(_Bool aver_or_max)
{
	uint32_t data[32];
	uint8_t size;
	uint32_t rssi = 0;
	uint32_t max = 0;
	switch(current_rx_phy)
	{
	case DBPSK_50_PROT_D:
		size = 32;
		break;
	case DBPSK_400_PROT_D:
		size = 4;
		break;
	case DBPSK_3200_PROT_D:
		size = 1;
		break;
	case DBPSK_25600_PROT_D:
		size = 1;
		break;
	case DBPSK_100H_PROT_D:
		size = 16;
		break;
	}
	wa1471_spi_read(DEM_FFT_READ_BUF, (uint8_t*)(&data[0]), 4*size);
	wa1471_spi_write8(DEM_FFT_READ_BUF + 100, 0);

	for(int i = 0; i != size; i++)
	{
		rssi += data[i];
		if(data[i] > max)
			max = data[i];
#ifdef DEM_CALC_SPECTRUM
		dem_spectrum_mas[i] = ((dem_spectrum_mas[i]*15 + data[i] + 1)>>4);
#endif
	}
	return (aver_or_max?rssi/size:max);
}

float wa1471dem_get_rssi()
{
	return 20*log10f(wa1471dem_get_rssi_int(0)) - wa1471dem_get_rssi_logoffset();
}

float wa1471dem_get_noise()
{
	return dem_noise;
}

#ifdef DEM_CALC_SPECTRUM
void wa1471dem_get_spectrum(uint8_t size, float* data)
{
	for(int i = 0; i != size; i++)
		data[i] = 20*log10f(dem_spectrum_mas[i]) - wa1471dem_get_rssi_logoffset();
}
#endif

uint8_t wa1471dem_get_noise_calc_duration()
{
    const uint8_t NBFI_NOISE_DINAMIC_D[4] = {20, 8, 5, 5};
    const uint8_t NBFI_NOISE_DINAMIC_H[1] = {10};
    if(current_rx_phy >= DBPSK_100H_PROT_D) return NBFI_NOISE_DINAMIC_H[current_rx_phy - DBPSK_100H_PROT_D];
    else return NBFI_NOISE_DINAMIC_D[current_rx_phy - DBPSK_50_PROT_D];
}

static void wa1471dem_update_noise(struct scheduler_desc *desc)
{

	static uint32_t dem_noise_sum;
	static uint32_t dem_noise_min;
	static uint8_t dem_noise_cntr;
	static uint8_t dem_noise_min_cntr ;

	if(desc == 0) //init noise engine
	{
		dem_noise_min = 0xffffffff;
		dem_noise_cntr = 0;
		dem_noise_min_cntr = wa1471dem_get_noise_calc_duration();
		return;
	}

	if(dem_noise_cntr == DEM_NOISE_AVER)
	{
		dem_noise_cntr = 0;
		uint32_t aver = dem_noise_sum/DEM_NOISE_AVER;
		dem_noise_sum = 0;
		if(aver < dem_noise_min)
			dem_noise_min = aver;
		if(--dem_noise_min_cntr == 0)
		{
			dem_noise_min_cntr = wa1471dem_get_noise_calc_duration();
			dem_noise = dem_noise_min;
			dem_noise = log10f(dem_noise)*20 - wa1471dem_get_rssi_logoffset() + 2; //2 dB is approximate difference between minimum and average noise level
			dem_noise_min = 0xffffffff;

		}

	}
	else
	{
		dem_noise_sum += wa1471dem_get_rssi_int(1);
		dem_noise_cntr++;
	}

	if(dem_rx_enabled) wa1471_scheduler->__scheduler_add_task(desc, 0, RELATIVE, MILLISECONDS(DEM_NOISE_TICK));
}

void wa1471dem_rx_enable(_Bool en)
{
	dem_rx_enabled = en;
	if(en)
	{
		wa1471_scheduler->__scheduler_add_task(&dem_update_noise_desc,  wa1471dem_update_noise, RELATIVE, MILLISECONDS(DEM_NOISE_TICK));
	}
	else
		wa1471_scheduler->__scheduler_remove_task(&dem_update_noise_desc);
}
#endif //#ifdef WA1471