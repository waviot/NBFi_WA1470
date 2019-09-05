
#include "log.h"

#include <stdio.h>

#include "rs485_uart.h"
#include "wa1470dem.h"
#include <math.h>
char   log_string[1024];


void log_init(void)
{
        RS485_UART_init();
}

void log_send_str(const char *str)
{
	uint16_t ptr = 0;
	char c;
	while((c = str[ptr++])) RS485_UART_send(c);
	RS485_UART_send(0x0A);
        RS485_UART_send(0x0D); 
}

void log_send_str_len(const char *str, uint16_t len)
{
	uint16_t ptr = 0;
	while(ptr != len) RS485_UART_send(str[ptr++]);
}

//extern uint32_t aver_rssi_mas[32];
extern uint16_t rfe_rx_total_vga_gain;
extern dem_bitrate_s current_rx_phy;
void log_print_spectrum()
{
  char   log_string[256];
  RS485_UART_send(0x1B);
  RS485_UART_send(0x5B);
  RS485_UART_send('3');
  RS485_UART_send('3');
  RS485_UART_send('F');
  RS485_UART_send(0x1B);
  RS485_UART_send(0x5B);
  RS485_UART_send('J');
  uint16_t offset = 170;
  uint32_t summ = 0;
  float max = -200;
  uint32_t freq;
  static float aver_snr = 0;
  static int aver_snr_count = 0;
  uint8_t len;
  float spectrum[32];
  switch(current_rx_phy)
  {
    case DBPSK_50_PROT_D:
      len = 32;
      break;
    case DBPSK_400_PROT_D:
      len = 4;
      break;
    case DBPSK_3200_PROT_D:
    case DBPSK_25600_PROT_D:
      len = 1;
      break;  
  }
  
  wa1470dem_get_spectrum(len, spectrum);
  
  for(int i=0; i != len; i++)
  {
     while(!RS485_UART_TX_is_empty());
     //summ += aver_rssi_mas[i];
     float val = spectrum[i];
     if(val>max) {max=val; freq=i;}
     sprintf(log_string, "%4f ", val);
     int l;
     for(l = 0; l <= (int16_t)val + offset; l++)
     {
        sprintf(log_string + strlen(log_string), "%c", 0xdb); 
     }
     /* while(l < 120)
     {
        sprintf(log_string + strlen(log_string), " "); 
        l++;  
     }*/
     log_send_str(log_string);   
  }
  //summ-=aver_rssi_mas[freq];
  float snr = max - wa1470dem_get_noise();
  //aver_snr += snr;
  //aver_snr_count++;
  sprintf(log_string, "noise=%4f, rssi=%4f, snr=%f, max=%4f, freq=%2d, gain=%d", wa1470dem_get_noise(), wa1470dem_get_rssi(), snr, max, freq, rfe_rx_total_vga_gain);
  log_send_str(log_string); 
}

  
/*
void log_send_to_sfmonitor(uint32_t *mas, uint16_t len)
{
  RS485_UART_send(0x12);
  for(uint16_t i = 0; i != len; i++)
  {
   // RS485_UART_send(0x12);
    uint16_t rssi = log10f(mas[i])*20;// - 48 - 192;
    RS485_UART_send((rssi>>8)&0xff);
    RS485_UART_send((rssi>>0)&0xff);
    //RS485_UART_send(0x13);
    RS485_UART_send(0x10);
  }
  RS485_UART_send(0x13);
}*/