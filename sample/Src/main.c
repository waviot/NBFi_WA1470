#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "wtimer.h"
#include "radio.h"
#include "log.h"
#include "defines.h"

struct wtimer_desc test_desc;
struct wtimer_desc spectrum_desc;

extern void (*__wa1470_enable_pin_irq)(void);
extern void (*__wa1470_disable_pin_irq)(void);

void send_data(struct wtimer_desc *desc) {

#ifdef BANKA
    #include "nbfi.h"
    #include "nbfi_config.h"
    static uint32_t counter = 0;

    uint8_t packet[8] = {0,0,0,0,0,0,0,0};

    ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(1));
   
    if(NBFi_GetQueuedTXPkt()) return;

    packet[0] = (counter/5)>>8;
    packet[1] = (counter/5)&0xff;

    if(counter > 500) return;
    if(counter == 0)
    {
        packet[2] = nbfi.tx_pwr;
        NBFi_Send(packet, 8);
        NBFi_Send(packet, 8);
        counter = 1;
    }

    switch(counter++%5)
    {
        case 0:
            if(--nbfi.tx_pwr < 0) nbfi.tx_pwr = 16;
            nbfi.tx_phy_channel = UL_DBPSK_50_PROT_D;
            break;
        case 1:
            nbfi.tx_phy_channel = UL_DBPSK_400_PROT_D;
            break;
        case 2:
            nbfi.tx_phy_channel = UL_DBPSK_3200_PROT_D;
            break;
        case 3:
            nbfi.tx_phy_channel = UL_DBPSK_25600_PROT_D;
            break;
        default:
            ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(3));
            return;
            break;
    }
    packet[2] = nbfi.tx_pwr;
    NBFi_Send(packet, 8);
#else
       
    ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(1));


#endif
}

#ifdef PLOT_SPECTRUM
void plot_spectrum(struct wtimer_desc *desc) {

  log_print_spectrum(32); 
  ScheduleTask(desc, 0, RELATIVE, MILLISECONDS(500));
}
#endif

extern uint8_t nbfi_lock;
uint32_t systick_timer = 0;
void HAL_SYSTICK_Callback(void)
{
  systick_timer++;
  if(!nbfi_lock) wtimer_runcallbacks();
}

extern uint16_t rfe_rx_total_vga_gain;
int main(void)
{
        
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
      
      #ifdef PLOT_SPECTRUM
      
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
          ScheduleTask(&spectrum_desc, plot_spectrum, RELATIVE, MILLISECONDS(50));
          break;
        case 'z':
          wtimer0_remove(&spectrum_desc);
          break;  
        case '5':
          wa1470dem_set_bitrate(DBPSK_50_PROT_D);
          //log_send_str_len("Switched to DBPSK_50_PROT_D\n\r", sizeof("Switched to DBPSK_50_PROT_D\n\r"));          
          break;
        case '4':
          wa1470dem_set_bitrate(DBPSK_400_PROT_D);
          //log_send_str_len("Switched to DBPSK_400_PROT_D\n\r", sizeof("Switched to DBPSK_400_PROT_D\n\r"));
          break;
        case '3':
          wa1470dem_set_bitrate(DBPSK_3200_PROT_D);
          //log_send_str_len("Switched to DBPSK_3200_PROT_D\n\r", sizeof("Switched to DBPSK_3200_PROT_D\n\r"));
          break;
        case '2':
          wa1470dem_set_bitrate(DBPSK_25600_PROT_D);
          //log_send_str_len("Switched to DBPSK_25600_PROT_D\n\r", sizeof("Switched to DBPSK_25600_PROT_D\n\r"));               
          break;
        case '1':
          wa1470dem_set_bitrate(DBPSK_100H_PROT_D);
         // log_send_str_len("Switched to DBPSK_100H_PROT_D\n\r", sizeof("Switched to DBPSK_100H_PROT_D\n\r"));
          break;
        }
      }
      #endif
      if (wa1470_cansleep()&& NBFi_can_sleep()) 
      {
          //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
  }
}
