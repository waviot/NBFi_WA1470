#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "wtimer.h"
#include "radio.h"
#include "log.h"



struct wtimer_desc test_desc;
struct wtimer_desc spectrum_desc;
//uint32_t fft[32];
//uint32_t noise_tbl[32];


extern void (*__wa1470_enable_pin_irq)(void);
extern void (*__wa1470_disable_pin_irq)(void);

void send_data(struct wtimer_desc *desc) {

  static uint64_t mas[2]={0,0x0123456789ABCDEF};
  
/* if(!NBFi_Packets_To_Send())
 {
     uint8_t _data[]={1,2,3,4,5};
    NBFi_Send(_data, 5); 
   //NBFi_Send((uint8_t*)mas, 16);
    _data[4]++;
    //mas[0]++;
 }*/
  ScheduleTask(desc, 0, RELATIVE, SECONDS(10));
  
}

void plot_spectrum(struct wtimer_desc *desc) {

  log_print_spectrum(32); 
  ScheduleTask(desc, 0, RELATIVE, MILLISECONDS(500));
}

extern uint8_t nbfi_lock;
void HAL_SYSTICK_Callback(void)
{
  if(!nbfi_lock) wtimer_runcallbacks();
}
extern uint16_t rfe_rx_total_vga_gain;
int main(void)
{
  char   log_string[256];
        
  HAL_Init();

  SystemClock_Config();
  
  MX_GPIO_Init();
  
  radio_init();

  log_init();
  
  ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(1));
  
  
  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  
  while (1) 
  {     
      NBFi_ProcessRxPackets(1);
      
      //wa1470_test();
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
          ScheduleTask(&spectrum_desc, plot_spectrum, RELATIVE, MILLISECONDS(50));
          break;
        case 'z':
          wtimer0_remove(&spectrum_desc);
          break;  
        case '5':
          wa1470dem_set_bitrate(DBPSK_50_PROT_D);
          log_send_str_len("Switched to DBPSK_50_PROT_D\n\r", sizeof("Switched to DBPSK_50_PROT_D\n\r"));          
          break;
        case '4':
          wa1470dem_set_bitrate(DBPSK_400_PROT_D);
          log_send_str_len("Switched to DBPSK_400_PROT_D\n\r", sizeof("Switched to DBPSK_400_PROT_D\n\r"));
          break;
        case '3':
          wa1470dem_set_bitrate(DBPSK_3200_PROT_D);
          log_send_str_len("Switched to DBPSK_3200_PROT_D\n\r", sizeof("Switched to DBPSK_3200_PROT_D\n\r"));
          break;
        case '2':
          wa1470dem_set_bitrate(DBPSK_25600_PROT_D);
          log_send_str_len("Switched to DBPSK_25600_PROT_D\n\r", sizeof("Switched to DBPSK_25600_PROT_D\n\r"));               
          break;
        case '1':
          wa1470dem_set_bitrate(DBPSK_100H_PROT_D);
          log_send_str_len("Switched to DBPSK_100H_PROT_D\n\r", sizeof("Switched to DBPSK_100H_PROT_D\n\r"));
          break;
        }
      }
      
      if (wa1470_cansleep()&& NBFi_can_sleep()) 
      {
          //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
  }
}



