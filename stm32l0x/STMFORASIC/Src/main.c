#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "radio.h"
#include "log.h"
#include "defines.h"
#include "rs485_uart.h"
#include "at_user.h"
#include "adf4350.h"


uint32_t volatile systimer = 0;

void setTxFreq(uint64_t freq);
void setRxFreq(uint64_t freq);

void HAL_SYSTICK_Callback(void)
{
  if((++systimer%10000) == 0) {
    //setTxFreq(868860000);
  }
  
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
    else NBFi_Send(data, length);

  }

}


  adf4350_state rx_st, tx_st;
  adf4350_platform_data pdata_rx, pdata_tx;

  
void wa1471_spi_write(uint16_t address, uint8_t *data, uint8_t length);  
  
void setRxFreq(uint64_t freq)
{
  static uint64_t last_freq = 0;
  if(freq != last_freq) 
  {
    adf4350_set_freq(&rx_st, 147500000 + freq);
    wa1471_spi_write(0x2000 + 32, (uint8_t*)(&rx_st.regs[0]), 4*6 + 1);
    wa1471_spi_write(0x2000 + 32 + 24, 0, 1);
    last_freq = freq;
  }
}

void setTxFreq(uint64_t freq)
{
  //static uint64_t last_freq = 0;
  //if(freq != last_freq) 
  {
      adf4350_set_freq(&tx_st, freq);
      wa1471_spi_write(0x2000 + 64, (uint8_t*)(&tx_st.regs[0]), 4*6 + 1);
      wa1471_spi_write(0x2000 + 64 + 24, 0, 1);
    //  last_freq = freq;
  }

  
}
  

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  
  init_config(&rx_st, &pdata_rx);
  init_config(&tx_st, &pdata_tx);
  
  radio_init();

  log_init();

  radio_load_id_and_key_of_sr_server(&sr_server_modem_id_and_key);

  if(sr_server_modem_id_and_key.id == 0) sr_server_modem_id_and_key.id = DEFAULT_REMOTE_ID;

#ifdef NBFI_AT_SERVER
  //nbfi_at_server_define_user_handler(user_defined_at_command_handler);
#endif

  
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  
  
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
