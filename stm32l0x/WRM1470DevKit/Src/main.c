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

#include "ssd1306.h"
#include "ssd1306_tests.h"

I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;

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
    GUI_receive_complete(data, length);
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


static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00303D5B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
 /* if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }*/
  /** Configure Digital filter
  */
  /*if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }*/
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}


static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;//SPI_DIRECTION_1LINE;
  
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }


}



int main(void)
{

  HAL_Init();

  SystemClock_Config();

 
  MX_GPIO_Init();

    MX_SPI1_Init();

  //HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET);
  //MX_I2C1_Init();
  
 
  RTC_init();

  //ADC_init();

  //radio_init();

  //log_init();


  //NBFi_Send5("Hello!", sizeof("Hello!"),0);


  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = POWER_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(POWER_LED_GPIO_Port, &GPIO_InitStruct);

  radio_load_id_and_key_of_sr_server(&sr_server_modem_id_and_key);

#ifdef NBFI_AT_SERVER
  nbfi_at_server_define_user_handler(user_defined_at_command_handler);
#endif

  while (1)
  {

    ssd1306_TestAll();
    
//    HAL_GPIO_WritePin(OLED_SCK_GPIO_Port, OLED_SCK_Pin, GPIO_PIN_SET);
    /*  if(!GUI_is_inited())  GUI_Init();

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
*/
  }
}
