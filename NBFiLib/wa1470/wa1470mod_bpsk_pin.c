#include "wa1470.h"

extern void (*__wa1470_send_to_bpsk_pin)(uint8_t *, uint16_t, uint16_t);
void wa1470_bpsk_pin_send(uint8_t* data, mod_bitrate_s bitrate)
{
	uint8_t len; 
	switch(bitrate)
	{
	case MOD_DBPSK_50_PROT_D:
	case MOD_DBPSK_400_PROT_D:
	case MOD_DBPSK_3200_PROT_D:
	case MOD_DBPSK_25600_PROT_D:
	case MOD_DBPSK_100H_PROT_D:
		len = 36;
		break;
	default:
		len = 40;
		break;
	}
	if(__wa1470_send_to_bpsk_pin)
		__wa1470_send_to_bpsk_pin(data, len, wa1470mod_phy_to_bitrate(bitrate));
}
