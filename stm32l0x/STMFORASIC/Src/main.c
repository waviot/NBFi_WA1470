#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "radio.h"
#include "log.h"
#include "defines.h"
#include "rs485_uart.h"
#include "at_user.h"

uint32_t volatile systimer = 0;

void HAL_SYSTICK_Callback(void)
{
  systimer++;
 }

void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{

}

void nbfi_receive_complete(uint8_t * data, uint16_t length)
{

  #ifdef PHOBOS_HDLC_FORWARDER
  if(phobos_hdlc_mode)
  {
    if(data[0] == 0xD3)
    {
      for(uint16_t i = 1; i != length; i++) RS485_UART_send(data[i]);
    }
  }
  else
  #endif
  {
    nbfi_at_server_receive_complete(data, length);

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

  }

}


int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  radio_init();

  log_init();

  radio_load_id_and_key_of_sr_server(&sr_server_modem_id_and_key);

  if(sr_server_modem_id_and_key.id == 0) sr_server_modem_id_and_key.id = DEFAULT_REMOTE_ID;

#ifdef NBFI_AT_SERVER
  nbfi_at_server_define_user_handler(user_defined_at_command_handler);
#endif

  while (1)
  {

     #ifdef PLOT_SPECTRUM
      #include "plot_spectrum.h"
      plot_spectrum();
      #endif

      NBFI_Main_Level_Loop();

      #ifdef PHOBOS_HDLC_FORWARDER
      if(phobos_hdlc_mode)
      {
        if(((systimer - last_uart_rx_time) > 10) && !RS485_UART_is_empty())
        {
          uint8_t send_buf[240];
          uint8_t size = 1;
          send_buf[0] = 0xD3;
          while(!RS485_UART_is_empty())
          {
            if(size < 240) send_buf[size++] = RS485_UART_get();
            else RS485_UART_get();
          }
          NBFi_Send(send_buf, size);
        }
      }
      else
      #endif
      {
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
      };



      if (NBFi_can_sleep() && scheduler_can_sleep() && RS485_can_sleep())
      {
        RS485_go_to_sleep(1);
        HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        SystemClock_Config();
      }
      else
      {
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
  }
}
