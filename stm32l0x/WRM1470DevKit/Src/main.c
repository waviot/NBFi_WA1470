#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "adc.h"
#include "rtc.h"
#include "radio.h"
#include "log.h"
#include "defines.h"
#include "rs485_uart.h"
#include "gui.h"

#define POWER_LED_GPIO_Port 	GPIOA
#define POWER_LED_Pin 		GPIO_PIN_12

uint32_t volatile systimer = 0;

void HAL_SYSTICK_Callback(void)
{
  systimer++;
  GUI_systick();
 }


void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{
   
}

void nbfi_receive_complete(uint8_t * data, uint16_t length)
{
  GUI_receive_complete(data, length);
}



int main(void)
{
        
  HAL_Init();
 
  SystemClock_Config();
  
  MX_GPIO_Init();
  
  RTC_init();
  
  ADC_init();
  
  radio_init();

  log_init();
    
  
  //last_send_status = NBFi_Send5("Hello!", sizeof("Hello!"));   
  

  GPIO_InitTypeDef GPIO_InitStruct; 
  GPIO_InitStruct.Pin = POWER_LED_Pin;    
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(POWER_LED_GPIO_Port, &GPIO_InitStruct);  

  
  while (1) 
  {     
    
      
       if(!GUI_is_inited())  GUI_Init(); 
             
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

      GUI_Update();
      
      if (NBFi_can_sleep() && scheduler_can_sleep() && GUI_can_sleep()) 
      {
        GUI_Deinit();
        HAL_GPIO_WritePin(POWER_LED_GPIO_Port, POWER_LED_Pin,  GPIO_PIN_RESET);
        HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        SystemClock_Config();
      }
      else 
      {       
        HAL_GPIO_WritePin(POWER_LED_GPIO_Port, POWER_LED_Pin,  GPIO_PIN_SET);
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
  }
}
