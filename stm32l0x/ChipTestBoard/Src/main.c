#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "radio.h"
#include "log.h"
#include "defines.h"

void HAL_SYSTICK_Callback(void)
{

}


nbfi_ul_sent_status_t last_send_status;


void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{
 /*  
    uint8_t string[] = "Hello";//"Hello, we are testing 25600bps receiving stability. This huge packet is sending for giving a numerous quantity of packets";     
    if(ul.id == last_send_status.id)
    {
      string[0] = (ul.id>>8);
      string[1] = (ul.id & 0xff);
      last_send_status = NBFi_Send5(string, sizeof(string));
    }  */
}

void nbfi_receive_complete(uint8_t * data, uint16_t length)
{
  //NBFi_Send5(data, length);  //echo
 /* if(length == sizeof("Hello"))
  {
    uint8_t payload[] = {0xaa};
    NBFi_Send5(payload, sizeof(payload));
  }*/
  
  
  if((data[0] == 0x80)&&(length == 1+4+32))
  {
    aux_modem_id_and_key.id = 0;
    for(uint8_t i = 0; i != 4; i++ )
    {
        aux_modem_id_and_key.id <<= 8;
        aux_modem_id_and_key.id += data[i + 1];
        
    }
    
    for(uint8_t i = 0; i != 32; i++ )
    {
        aux_modem_id_and_key.key[i] = data[i + 1 + 4];      
    } 
    
    radio_save_id_and_key_of_aux_device(&aux_modem_id_and_key);
  }
  
  
  
}


int main(void)
{
        
  HAL_Init();
 
  SystemClock_Config();
  
  MX_GPIO_Init();
 
  radio_init();

  log_init();
  
  //radio_switch_to_from_short_range(1);
  
  //last_send_status = NBFi_Send5({0xaa}, 1);   
  //uint8_t payload[] = {0xaa};
  //last_send_status = NBFi_Send5(payload, sizeof(payload));
  
  radio_load_id_and_key_of_aux_device(&aux_modem_id_and_key);
  
  if(aux_modem_id_and_key.id != 0)  radio_switch_to_from_short_range(1);
  
  while (1) 
  {     
     
      #ifdef PLOT_SPECTRUM
      #include "plot_spectrum.h"
      plot_spectrum();
      #endif
      
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

      
      NBFI_Main_Level_Loop();
      
      if(NBFi_is_Switched_to_Custom_Settings())
      {
        if( NBFi_is_Idle() )
        {
          uint8_t payload[] = {0xaa};
          last_send_status = NBFi_Send5(payload, sizeof(payload),0);
        }
      }  
             
      //rssi = wa1470dem_get_rssi();
            
      if (NBFi_can_sleep()) 
      {
          //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
      else HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
}
