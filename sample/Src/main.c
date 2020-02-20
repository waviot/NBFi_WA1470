#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "radio.h"
#include "log.h"
#include "defines.h"

struct wtimer_desc test_desc;

void send_data(struct wtimer_desc *desc) {
 
  const uint8_t string[] = "Hello, we are testing 3200bps receaiving stability. This huge packet is sending for giving a numerous quantity of packets";
  NBFi_Send((uint8_t*)string, sizeof(string));
  
#ifdef BANKA
    #include "nbfi.h"
    #include "nbfi_config.h"
    static uint32_t counter = 0;

    uint8_t packet[8] = {0,0,0,0,0,0,0,0};

    ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(1));
   
    if(NBFi_GetQueuedTXPkt()) return;

    packet[0] = (counter/5)>>8;
    packet[1] = (counter/5)&0xff;

    if(counter > 2000) return;
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
            if(--nbfi.tx_pwr < -9) nbfi.tx_pwr = 16;
            nbfi.tx_phy_channel = UL_DBPSK_50_PROT_E;
            break;
        case 1:
            nbfi.tx_phy_channel = UL_DBPSK_400_PROT_E;
            break;
        case 2:
            nbfi.tx_phy_channel = UL_DBPSK_3200_PROT_E;
            break;
        case 3:
            nbfi.tx_phy_channel = UL_DBPSK_25600_PROT_E;
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


void HAL_SYSTICK_Callback(void)
{
  NBFI_Interrupt_Level_Loop();
}

void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{
    char log_string[256];
    sprintf(log_string, "UL #%d %s", ul.id, (ul.status == DELIVERED)?"DELIVERED":"LOST");
    log_send_str(log_string);   
}

void nbfi_receive_complete(uint8_t * data, uint16_t length)
{
    char log_string[256];
    sprintf(log_string, "DL of %d bytes received", length);
    NBFi_Send(data, length); //loopback
}


int main(void)
{
        
  HAL_Init();

  SystemClock_Config();
  
  MX_GPIO_Init();
 
  ADC_init();
  
  radio_init();

  log_init();
  
  ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(1));
     
  
  while (1) 
  {     
      //NBFi_ProcessRxPackets(1);
      
      #ifdef PLOT_SPECTRUM
      plot_spectrum();
      #endif
      
     // NBFI_Main_Level_Loop();
      
      if (wa1470_cansleep()&& NBFi_can_sleep()) 
      {
          //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
         HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
      else HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
}
