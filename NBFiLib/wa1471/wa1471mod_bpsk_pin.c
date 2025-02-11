#ifdef WA1471
#include "wa1471.h"

void wa1471_bpsk_pin_send(uint8_t* data, mod_bitrate_s bitrate)
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
	if(wa1471_hal->__wa1471_send_to_bpsk_pin)
		wa1471_hal->__wa1471_send_to_bpsk_pin(data, len, wa1471mod_phy_to_bitrate(bitrate));
}
#endif //#ifdef WA1471