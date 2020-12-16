/**
  ******************************************************************************
  * File Name          : SPI.c
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

/* USER CODE BEGIN 0 */
#include <string.h>

#define SPI_RX_BUFFER_SIZE 32
uint8_t aRxBuffer[SPI_RX_BUFFER_SIZE] = {0};
__IO uint8_t ubReceiveIndex = 0;
/* USER CODE END 0 */

/* SPI1 init function */
void MX_SPI1_Init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA7   ------> SPI1_MOSI
  PA11   ------> SPI1_MISO
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5|LL_GPIO_PIN_7|LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV64;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_DisableNSSPulseMgt(SPI1);

}

/* USER CODE BEGIN 1 */

/**
  * @brief  This function Activate SPI1
  * @param  None
  * @retval None
  */
void SPI_Activate(void)
{
  LL_SPI_SetRxFIFOThreshold(SPI1, LL_SPI_RX_FIFO_TH_QUARTER); //8 bit
  /* Enable RXNE  Interrupt             */
  //  LL_SPI_EnableIT_RXNE(SPI1);
  /* Enable SPI1 */
  LL_SPI_Enable(SPI1);
}

/*!
 * \brief receive data from spi
 *
 * \param byte array of char
 * \param len number of bytes
 */
void SPI_Rx(uint8_t *byte, uint16_t len)
{
  volatile uint16_t timeout;

  //  for (uint16_t i = 0; i < len; i++)
  //  {
  //    LL_SPI_TransmitData8(SPI1, 0x00);
  //    timeout = SPI_TIMEOUT;
  //    while (!LL_SPI_IsActiveFlag_RXNE(SPI1) && timeout--)
  //      ;
  //    timeout = SPI_TIMEOUT;
  //    while (LL_SPI_IsActiveFlag_BSY(SPI1) && timeout--)
  //      ;
  //    byte[i] = LL_SPI_ReceiveData8(SPI1);
  //  }

  if (LL_SPI_IsActiveFlag_RXNE(SPI1))
  {
    /* Call function Slave Reception Callback */
    SPI1_Rx_Callback();
  }
  /* Enable RXNE  Interrupt             */
  //    LL_SPI_EnableIT_RXNE(SPI1);
  for (uint16_t i = 0; i < len; i++)
  {
    LL_SPI_TransmitData8(SPI1, 0x00);
    timeout = SPI_TIMEOUT;
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1) && timeout--)
      ;
    aRxBuffer[ubReceiveIndex++] = LL_SPI_ReceiveData8(SPI1);
  }
  //    while ((len > ubReceiveIndex)&& timeout--)
  //    {
  //    }
  /* Disable RXNE Interrupt */
  //    LL_SPI_DisableIT_RXNE(SPI1);

  memcpy_s(byte, len, aRxBuffer, len);
  ubReceiveIndex = 0;
}

/*!
 * \brief transmit data to spi
 *
 * \param byte array of char
 * \param len number of bytes
 */
void SPI_Tx(uint8_t *byte, uint16_t len)
{
  volatile uint16_t timeout;

  for (uint16_t i = 0; i < len; i++)
  {
    //SPI1->DR = byte[i];
    LL_SPI_TransmitData8(SPI1, byte[i]);
    timeout = SPI_TIMEOUT;
    while (!LL_SPI_IsActiveFlag_TXE(SPI1) && timeout--)
      ;
    timeout = SPI_TIMEOUT;
    while (LL_SPI_IsActiveFlag_BSY(SPI1) && timeout--)
      ;
    LL_SPI_ClearFlag_OVR(SPI1);
  }
}

/*!
 * \brief transmit and receive data to spi
 *
 * \param byteTx array of char to tx
 * \param byteRx array of char to rx
 * \param len number of bytes
 */
void SPI_RxTx(uint8_t *byteTx, uint8_t *byteRx, uint16_t len)
{
  volatile uint16_t timeout;

  for (uint16_t i = 0; i < len; i++)
  {
    SPI1->DR = byteTx[i];
    timeout = SPI_TIMEOUT;
    while (LL_SPI_IsActiveFlag_RXNE(SPI1) && timeout--)
      ;
    timeout = SPI_TIMEOUT;
    while (LL_SPI_IsActiveFlag_BSY(SPI1) == SET && timeout--)
      ;
    byteRx[i] = LL_SPI_ReceiveData8(SPI1);
  }
}

/**
  * @brief  Function called from SPI1 IRQ Handler when RXNE flag is set
  *         Function is in charge of retrieving received byte from SPI lines.
  * @param  None
  * @retval None
  */
void SPI1_Rx_Callback(void)
{
  /* Read character in Data register.
  RXNE flag is cleared by reading data in DR register */
  aRxBuffer[ubReceiveIndex++] = LL_SPI_ReceiveData8(SPI1); //LL_SPI_ReceiveData16(SPI1) >> 8;

  if (ubReceiveIndex >= SPI_RX_BUFFER_SIZE)
  {
    ubReceiveIndex = 0;
  }
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
