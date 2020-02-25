#include "rs485_uart.h"
#include "radio.h"
#include "log.h"

struct scheduler_desc spectrum_desc;

void plot_spectrum_task(struct scheduler_desc *desc) {

  log_print_spectrum(32); 
  scheduler_add_task(desc, 0, RELATIVE, MILLISECONDS(500));
}

void plot_spectrum()
{
      
      char log_string[256];
      
      if(!RS485_UART_is_empty())
      {
        switch(RS485_UART_get())
        {
        case 0x2c:
          wa1470rfe_set_rx_gain(rfe_rx_total_vga_gain + 3);
          sprintf(log_string, "VGA gain increased to %d dB", rfe_rx_total_vga_gain);
          log_send_str(log_string);
          break;
        case 0x1b:
          wa1470rfe_set_rx_gain(rfe_rx_total_vga_gain - 3);
          sprintf(log_string, "VGA gain reduced to %d dB", rfe_rx_total_vga_gain);
          log_send_str(log_string);
          break;
        case 's':
          log_print_spectrum();
          break;
        case 'a':
          scheduler_add_task(&spectrum_desc, plot_spectrum_task, RELATIVE, MILLISECONDS(50));
          break;
        case 'z':
          scheduler_remove_task(&spectrum_desc);
          break;  
        case '5':
          wa1470dem_set_bitrate(DBPSK_50_PROT_D);
           break;
        case '4':
          wa1470dem_set_bitrate(DBPSK_400_PROT_D);
          break;
        case '3':
          wa1470dem_set_bitrate(DBPSK_3200_PROT_D);
          break;
        case '2':
          wa1470dem_set_bitrate(DBPSK_25600_PROT_D);
          break;
        case '1':
          wa1470dem_set_bitrate(DBPSK_100H_PROT_D);
          break;
        }
      }
}