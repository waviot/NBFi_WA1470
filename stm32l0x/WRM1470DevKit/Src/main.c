#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "radio.h"
#include "log.h"
#include "defines.h"
#include "pca9454.h"


uint32_t volatile systimer = 0;

void HAL_SYSTICK_Callback(void)
{
  systimer++;
}

nbfi_ul_sent_status_t last_send_status;
void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{
   
    uint8_t string[] = "Hello, we are testing 25600bps receiving stability. This huge packet is sending for giving a numerous quantity of packets";     
    if(ul.id == last_send_status.id)
    {
      string[0] = (ul.id>>8);
      string[1] = (ul.id & 0xff);
      last_send_status = NBFi_Send5((uint8_t*)string, sizeof(string));
    }   
}

void nbfi_receive_complete(uint8_t * data, uint16_t length)
{
  NBFi_Send5(data, length);  //echo
}


void switch_to_short_range_and_send_hello()
{
  
  static uint32_t old_systimer = 0;
  static uint16_t sent_packet_id;
  if(systimer - old_systimer > 10000)
  {
    old_systimer = systimer;
    radio_switch_to_from_short_range(1);
    sent_packet_id =  NBFi_Send5("Hello", sizeof("Hello")).id;
  }
  else
  {
     if(NBFi_is_Switched_to_Custom_Settings() && NBFi_get_UL_status(sent_packet_id) >= DELIVERED)
     {
      radio_switch_to_from_short_range(0);
     }
  }
    
}

//uint8_t conf;

int main(void)
{
        
  HAL_Init();
 
  SystemClock_Config();
  
  MX_GPIO_Init();
  
  PCA9454_init();
    
  radio_init();

  log_init();
  
  //last_send_status = NBFi_Send5("Hello!", sizeof("Hello!"));   
  
  PCA9454_set_out_pin(EXT_OUTPIN_POWER_LED);
  PCA9454_set_out_pin(EXT_OUTPIN_NBACKLIGHT);
  while (1) 
  {     
     
      #ifdef PLOT_SPECTRUM
      #include "plot_spectrum.h"
      plot_spectrum();
      #endif
      NBFI_Main_Level_Loop();
      switch_to_short_range_and_send_hello();
      if (NBFi_can_sleep() && scheduler_can_sleep()) 
      {
        //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
      else HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
}
