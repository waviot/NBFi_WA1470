#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "radio.h"
#include "log.h"
#include "defines.h"
#include "pca9454.h"
#include "rs485_uart.h"

uint32_t volatile systimer = 0;

void HAL_SYSTICK_Callback(void)
{
  systimer++;
}

nbfi_ul_sent_status_t last_send_status;
void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{
   
   /* uint8_t string[] = "Hello, we are testing 25600bps receiving stability. This huge packet is sending for giving a numerous quantity of packets";     
    if(ul.id == last_send_status.id)
    {
      string[0] = (ul.id>>8);
      string[1] = (ul.id & 0xff);
      last_send_status = NBFi_Send5((uint8_t*)string, sizeof(string), 0);
    }   */
}

void nbfi_receive_complete(uint8_t * data, uint16_t length)
{
  //NBFi_Send5(data, length);  //echo
  if((length == 1) && (data[0] == 0xaa))
  {
    uint8_t string[] = "This license does not grant you any rights to use the Licensor's name, logo, or trademarks";
    NBFi_Send(string, sizeof(string));  
  }
  else if(length > 100) 
    NBFi_Send(data, length);
}

#define MODEM_ID  ((uint32_t *)0x0801ff80)

void send_beacon_packet()
{
  uint8_t pkt[8];
  *((uint32_t*)&pkt[0]) = *MODEM_ID;
  NBFi_Send5(pkt, 8, NBFI_UL_FLAG_NOACK|NBFI_UL_FLAG_UNENCRYPTED|NBFI_UL_FLAG_DEFAULT_PREAMBLE|NBFI_UL_FLAG_SEND_ON_CENTRAL_FREQ);
}


void switch_to_short_range_and_send_hello()
{
  
  static uint32_t old_systimer = 0;
  if(!NBFi_is_Switched_to_Custom_Settings() && ((systimer - old_systimer) > 10000))
  {
    if(!NBFi_is_Idle() && ((systimer - old_systimer) < 300000) ) return;
    radio_switch_to_from_short_range(1);
    send_beacon_packet();
    uint8_t string[] = "Hello";
    NBFi_Send(string, sizeof(string));
    old_systimer = systimer;
  }
  else
  {
     //if(NBFi_is_Switched_to_Custom_Settings() && (NBFi_get_UL_status(sent_packet_id) >= DELIVERED))
     if(NBFi_is_Switched_to_Custom_Settings() && NBFi_is_Idle())
     {
      radio_switch_to_from_short_range(0);
     }
  }
    
}

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

      #ifdef NBFI_AT_SERVER
      uint8_t *buf;
      if(!RS485_UART_is_empty()) 
      {
         uint8_t c = RS485_UART_get(); 
         if(nbfi_at_server_echo_mode) RS485_UART_send(c);
         uint16_t reply_len = nbfi_at_server_parse_char(c, &buf);
         for(uint16_t i = 0; i != reply_len; i++) RS485_UART_send(buf[i]); 
      }
      #endif

      if (NBFi_can_sleep() && scheduler_can_sleep()) 
      {
        //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
      else HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
}
