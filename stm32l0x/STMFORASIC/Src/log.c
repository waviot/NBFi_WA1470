#include "log.h"
#include <stdio.h>
#include <string.h>
#include "rs485_uart.h"

#include "nbfi.h"


#include <math.h>

char log_string[1024];


void log_init(void)
{
	RS485_UART_init();
}

void log_send_str(const char *str)
{
	uint16_t ptr = 0;
	char c;
	while(c = str[ptr++])
	RS485_UART_send(c);
	RS485_UART_send(0x0A);
	RS485_UART_send(0x0D); 
}



void log_send_str_len(const char *str, uint16_t len)
{
	uint16_t ptr = 0;
	while(ptr != len) RS485_UART_send(str[ptr++]);
}

extern uint16_t rfe_rx_total_vga_gain;

extern dem_bitrate_s current_rx_phy;

#ifdef WA1471
void log_print_spectrum()
{
  
  static float max_aver = -200;
    
  char log_string[256];
  RS485_UART_send(0x1B);
  RS485_UART_send(0x5B);
  //RS485_UART_send(0x32);
  //RS485_UART_send(0x4A);
  
  RS485_UART_send('6');
  RS485_UART_send('5');
  RS485_UART_send('F');
  RS485_UART_send(0x1B);
  RS485_UART_send(0x5B);
  RS485_UART_send('J');
  
  uint16_t offset = 200;//175;
  float max = -200;

  
  uint32_t freq;
  uint8_t len;
  float spectrum[64];
  switch(current_rx_phy)
  {
    case DBPSK_50_PROT_D:
    case DBPSK_50_PROT_E:
      len = 64;
      break;
    case DBPSK_400_PROT_D:
    case DBPSK_3200_PROT_D:
    case DBPSK_400_PROT_E:
    case DBPSK_3200_PROT_E:  
      len = 8;
      break;    
    case DBPSK_25600_PROT_D:
    case DBPSK_25600_PROT_E:  
      len = 4;
      break;  
  }
  
    wa1471dem_get_spectrum(len, spectrum);


  extern UART_HandleTypeDef huart;
  void scheduler_HAL_lock_unlock(uint8_t lock);
  scheduler_HAL_lock_unlock(1);
  
  for(int i=0; i != len; i++)
  {
     //while(!RS485_UART_TX_is_empty());
     //summ += aver_rssi_mas[i];
     
    
     float val = spectrum[i];
     if(val>max) {max=val; freq=i;}
     
     /*sprintf(log_string, "%4f ", val);
     
     
     log_send_str_len(log_string, strlen(log_string));*/
     
     int l;
     while(!RS485_UART_TX_is_empty());
     for(l = 0; l <= (int16_t)val + offset; l++)
     {
       huart.Instance->TDR = 0xdb;
      while( !__HAL_UART_GET_FLAG(&huart, UART_FLAG_TXE));
       //RS485_UART_send(0xdb); 
     }
     huart.Instance->TDR = 0x0A;
     while( !__HAL_UART_GET_FLAG(&huart, UART_FLAG_TXE));
     huart.Instance->TDR = 0x0D;
     while( !__HAL_UART_GET_FLAG(&huart, UART_FLAG_TXE)); 
  }

   
     float snr = max - wa1471dem_get_noise();
  
    max_aver = 0.8*max_aver + 0.2*max;
    sprintf(log_string, "noise=%4f, rssi=%4f, snr=%f, max=%4f, max_aver=%4f, freq=%2d, gain=%d", wa1471dem_get_noise(), wa1471dem_get_rssi(), snr, max, max_aver, freq, rfe_rx_total_vga_gain);
   
    log_send_str(log_string); 
    scheduler_HAL_lock_unlock(0);
}
#else

void log_print_spectrum()
{
  char log_string[256];
  RS485_UART_send(0x1B);
  RS485_UART_send(0x5B);
  RS485_UART_send('3');
  RS485_UART_send('3');
  RS485_UART_send('F');
  RS485_UART_send(0x1B);
  RS485_UART_send(0x5B);
  RS485_UART_send('J');
  uint16_t offset = 170;
  float max = -200;
  uint32_t freq;
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

     log_send_str(log_string);   
  }
  
     float snr = max - wa1470dem_get_noise();
     sprintf(log_string, "noise=%4f, rssi=%4f, snr=%f, max=%4f, freq=%2d, gain=%d", wa1470dem_get_noise(), wa1470dem_get_rssi(), snr, max, freq, rfe_rx_total_vga_gain);

    log_send_str(log_string); 
}
#endif


