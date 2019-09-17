
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
  if(__wa1470_send_to_bpsk_pin) __wa1470_send_to_bpsk_pin(data, len, wa1470mod_phy_to_bitrate(bitrate));
  
/*
	//sws1470_set_mode(MODE_TX);
	make_copy_msg(data,  len, bitrate);

	sprintf(log_string, " SEND (BY BPSK PIN) ID : =");
	for(uint8_t i = 0; i < len; i++) sprintf(log_string + strlen(log_string), "%02X", data[i]);


	log_send_str(log_string);


	{
	//for (int i=0; i<3;i++){
	PWM_CLOCK_CONFIG();
	 while(GET_STATUS_TX()) // Wait for TX complete
	 {
		  wtimer_runcallbacks();
		  cmd_type_log st = check_cmd();
		  if (st != SWITCH_PIN_BPSK && st != SWITCH_OPENUNB && st != NO_CMD)
			  ABORT_TX();

	 }
	}
*/
}


 