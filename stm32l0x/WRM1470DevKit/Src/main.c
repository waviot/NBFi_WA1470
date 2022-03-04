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
#include "at_user.h"
#include "multiport.h"
#include "settings.h"
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
   //NBFi_Send5("Hello!", sizeof("Hello!"),0);
}

void nbfi_receive_complete(uint8_t * data, uint16_t length)
{

  if(global_settings.uart_mode == UART_MODE_TRANSPARENT)
  {
    for(uint16_t i = 0; i != length; i++) RS485_UART_send(data[i]);
    return;
  }

  if(global_settings.uart_mode == UART_MODE_APPENDD3)
  {
    if(data[0] == 0xD3)
        for(uint16_t i = 1; i != length; i++) RS485_UART_send(data[i]);
    return;
  }

  GUI_receive_complete(data, length);

  nbfi_at_server_receive_complete(data, length);

  #if(PROTOCOL_ID == NBFI_MULTIPORT_PROTOCOL_ID)
    MULTIPORT_receive_complete(data, length);
  #else

  if((data[0] == 0x80)&&(data[1] == 0xEE)&&(length == 2+4+32))  //receive sr_server device id and master key
  {
      sr_server_modem_id_and_key.id = 0;
      for(uint8_t i = 0; i != 4; i++ )
      {
          sr_server_modem_id_and_key.id <<= 8;
          sr_server_modem_id_and_key.id += data[i + 2];

      }

      for(uint8_t i = 0; i != 32; i++ )
      {
          sr_server_modem_id_and_key.key[i] = data[i + 2 + 4];
      }

      radio_save_id_and_key_of_sr_server(&sr_server_modem_id_and_key);
  }
#endif
}


void electro5_receive_complete(uint8_t * data, uint16_t length)
{
      uint8_t packet[] = {0xEE,0x00,0x32,0x20,0x32,0x5B,0x00,0x78,0x86,0x46,0xC7,0x44,0x34,0xE0};
      MULTIPORT_send_to_port(packet, sizeof(packet), 0, 2);
}

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  RTC_init();

  ADC_init();

  radio_init();

  load_global_settings();

  log_init();


  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = POWER_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(POWER_LED_GPIO_Port, &GPIO_InitStruct);

  radio_load_id_and_key_of_sr_server(&sr_server_modem_id_and_key);

  nbfi_at_server_define_user_handler(user_defined_at_command_handler);

  MULTIPORT_register_protocol(2, &electro5_receive_complete);

  electro5_receive_complete(0,0);


  while (1)
  {

      if(!GUI_is_inited())  GUI_Init();

      #ifdef PLOT_SPECTRUM
      #include "plot_spectrum.h"
      plot_spectrum();
      #endif

      NBFI_Main_Level_Loop();

      switch(global_settings.uart_mode)
      {
        case UART_MODE_ATCOMMANDS:
            {
                uint8_t *buf;
                if(!RS485_UART_is_empty())
                {
                   uint8_t c = RS485_UART_get();
                   if(nbfi_at_server_echo_mode) RS485_UART_send(c);
                   uint16_t reply_len = nbfi_at_server_parse_char(c, &buf);
                   for(uint16_t i = 0; i != reply_len; i++) RS485_UART_send(buf[i]);
                }
                break;
            }
        case UART_MODE_TRANSPARENT:
        case UART_MODE_APPENDD3:
            if(((systimer - last_uart_rx_time) > 10) && !RS485_UART_is_empty())
            {
              uint8_t send_buf[240];
              uint8_t size = 0;
              if(global_settings.uart_mode == UART_MODE_APPENDD3)
              {
                send_buf[0] = 0xD3;
                size = 1;
              }
              while(!RS485_UART_is_empty())
              {
                if(size < 240) send_buf[size++] = RS485_UART_get();
                else RS485_UART_get();
              }
              NBFi_Send(send_buf, size);
            }
          break;
      }

      GUI_Update();

      if (NBFi_can_sleep() && scheduler_can_sleep() && GUI_can_sleep()&& RS485_can_sleep())
      {
        GUI_Deinit();
        HAL_GPIO_WritePin(POWER_LED_GPIO_Port, POWER_LED_Pin,  GPIO_PIN_RESET);
        RS485_go_to_sleep(1);
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
