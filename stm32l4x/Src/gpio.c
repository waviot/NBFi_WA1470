/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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
#include "gpio.h"
/* USER CODE BEGIN 0 */
#include "wa1470_hal.h"
#include "WVT_Button.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

#define WA_IRQ_EXTI_IRQn EXTI9_5_IRQn

enum spi_device_state SpiDeviceState = SPI_DEVICE_NONE;
/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through
        * the Code Generation settings)
     PA8   ------> LPTIM2_OUT
*/
void MX_GPIO_Init(void)
{

  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOH);

  /**/
  LL_GPIO_ResetOutputPin(GPIOB, LCD_LOAD_Pin|SPI1_NSS_Pin);

  /**/
  LL_GPIO_SetOutputPin(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_6|LL_GPIO_PIN_9
                          |LL_GPIO_PIN_10|LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LCD_LOAD_Pin|WA_CHIP_EN_Pin|SPI1_NSS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE5);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE7);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_5;
  EXTI_InitStruct.Line_32_63 = LL_EXTI_LINE_NONE;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_7;
  EXTI_InitStruct.Line_32_63 = LL_EXTI_LINE_NONE;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  LL_GPIO_SetPinPull(WA_IRQ_GPIO_Port, WA_IRQ_Pin, LL_GPIO_PULL_DOWN);

  /**/
  LL_GPIO_SetPinPull(Button_GPIO_Port, Button_Pin, LL_GPIO_PULL_UP);

  /**/
  LL_GPIO_SetPinMode(WA_IRQ_GPIO_Port, WA_IRQ_Pin, LL_GPIO_MODE_INPUT);

  /**/
  LL_GPIO_SetPinMode(Button_GPIO_Port, Button_Pin, LL_GPIO_MODE_INPUT);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),7, 0));
  NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */

#ifdef USE_WA1470
/*!
 * \brief enable interrupt from WA1470 IRQ pin
 *
 */
void GPIO_EnablePinIrq(void)
{
  //  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_5);
  //  LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_5);
  //  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE5);
}

/*!
 * \brief disable interrupt from WA1470 IRQ pin
 *
 */
void GPIO_DisablePinIrq(void)
{
  //  LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_5);
}

/*!
 * \brief Turn on WA1470 chip
 *
 */
void GPIO_ChipEnable(void)
{
  LL_GPIO_ResetOutputPin(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin);
}

/*!
 * \brief Turn off WA1470 chip
 *
 */
void GPIO_ChipDisable(void)
{
  LL_GPIO_SetOutputPin(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin);
}

/*!
 * \brief get WA1470 IRQ pin state
 *
 * \return uint8_t true if we have high voltage level
 */
uint8_t GPIO_GetIrqPinState(void)
{
  return LL_GPIO_IsOutputPinSet(WA_IRQ_GPIO_Port, WA_IRQ_Pin);
}

/*!
 * \brief turn off or turn off chip select pin of wa1470
 *
 * \param state true if we enable chip communication
 */
void GPIO_Wa1470_WriteCs(uint8_t state)
{
  if (state)
    LL_GPIO_SetOutputPin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin);
  else
    LL_GPIO_ResetOutputPin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin);
}
#endif //USE_WA1470

#if defined(MAX35101) || defined(MAX35103) || defined(MAX35104)
/*!
 * \brief turn off or turn off chip select pin of max35103
 *
 * \param state true if we enable chip communication
 */
void GPIO_MAX35103_WriteCs(uint8_t state)
{
  if (state)
  {
    LL_GPIO_SetOutputPin(MAX_NSEL_GPIO_Port, MAX_NSEL_Pin);
  }
  else
  {
    LL_GPIO_ResetOutputPin(MAX_NSEL_GPIO_Port, MAX_NSEL_Pin);
  }
}

/*!
 * \brief reset max35103 chip
 *
 */
void GPIO_MAX35103_Reset(void)
{
  LL_GPIO_ResetOutputPin(MAX_RST_GPIO_Port, MAX_RST_Pin);
  LL_mDelay(1);
  LL_GPIO_SetOutputPin(MAX_RST_GPIO_Port, MAX_RST_Pin);
  LL_mDelay(1);
}
#endif //(MAX35101) || (MAX35103) || (MAX35104)

/*!
 * \brief init gpio tic33 / made by by cubemx
 *
 */
void GPIO_Tic33Init(void)
{
}

/*!
 * \brief set load pin /like chip select on rise or fall
 *
 */
void GPIO_Tic33SetLoad(void)
{
  LL_GPIO_SetOutputPin(LCD_LOAD_GPIO_Port, LCD_LOAD_Pin);
}
/*!

 * \brief clear load pin /like chip select on rise or fall
 *
 */
void GPIO_Tic33ClearLoad(void)
{
  LL_GPIO_ResetOutputPin(LCD_LOAD_GPIO_Port, LCD_LOAD_Pin);
}

/*!
 * \brief check if button pressed
 *
 * \return true if button pressed
 * \return false if not pressed
 */
bool GPIO_IsButtonPressed(void)
{
  return !LL_GPIO_IsInputPinSet(Button_GPIO_Port, Button_Pin);
}

bool GPIO_SetSpiState(enum spi_device_state NewSpiState, bool enable)
{
  /// \todo check this
  /// \todo check if enable that is choose only this
  GPIO_Wa1470_WriteCs(enable);

  //  if (NewSpiState == SPI_DEVICE_WA1470) //priority for nb-fi
  //  {
  //    SpiDeviceState = NewSpiState;
  //    GPIO_Wa1470_WriteCs(enable);
  //    if (!enable)
  //    {
  //      //SpiDeviceState = SPI_DEVICE_NONE;
  //
  //        /// \todo check this
  //    }
  //  }
  //  if (((SpiDeviceState == SPI_DEVICE_NONE) && (NewSpiState != SPI_DEVICE_NONE) ||
  //       (SpiDeviceState == NewSpiState)))
  //  {
  //    SpiDeviceState = NewSpiState;
  //    switch (NewSpiState)
  //    {
  //    case SPI_DEVICE_NONE:
  //    {
  //      GPIO_Tic33ClearLoad();
  //#if defined(MAX35101) || defined(MAX35103) || defined(MAX35104)
  //      GPIO_MAX35103_WriteCs(false);
  //#endif //(MAX35101) || (MAX35103) || (MAX35104)
  //      GPIO_Wa1470_WriteCs(false);
  //      break;
  //    }
  //    case SPI_DEVICE_TIC33:
  //    {
  //      if (enable)
  //      {
  //        GPIO_Tic33ClearLoad();
  //      }
  //      else
  //      {
  //        GPIO_Tic33SetLoad();
  //      }
  //    }
  //    break;
  //#if defined(MAX35101) || defined(MAX35103) || defined(MAX35104)
  //    case SPI_DEVICE_MAX:
  //        {
  //            GPIO_MAX35103_WriteCs(enable);
  //        }
  //        break;
  //#endif //(MAX35101) || (MAX35103) || (MAX35104)
  //    case SPI_DEVICE_WA1470:
  //    {
  //      GPIO_Wa1470_WriteCs(enable);
  //    }
  //    break;
  //    default:
  //    {
  //      SpiDeviceState = SPI_DEVICE_NONE;
  //      break;
  //    }
  //    }
  //
  //    if (!enable)
  //    {
  //      SpiDeviceState = SPI_DEVICE_NONE;
  //    }
  //    return true;
  //  }
  //  else
  //  {
  return false;
  //  }
}

enum spi_device_state GPIO_GetSpiState(void)
{
  return SpiDeviceState;
}

void GPIO_TurnOnGenPins(void)
{
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_15, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);

  LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_15, LL_GPIO_AF_1);
  LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_3, LL_GPIO_AF_1);
}

void GPIO_TurnOffGenPins(void)
{
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_15, LL_GPIO_MODE_ANALOG);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ANALOG);

  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_15, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_3, LL_GPIO_PULL_NO);
}
/*!
 * \brief IRQ from WA1470
 *
 */
void GPIO_EXTI5_Callback(void)
{
#ifdef USE_WA1470
  WA_EXT_IRQHandler();
#endif //USE_WA1470
}

/*!
 * \brief IRQ from button
 *
 */
void GPIO_EXTI7_Callback(void)
{
  ButtonHandler();
}

/*!
 * \brief IRQ from MAX35103 \ref stm32l4xx_it.c
 *
 */
void GPIO_EXTI10_Callback(void)
{
#if defined(MAX35101) || defined(MAX35103) || defined(MAX35104)
  WVT_MAX35103_Interrupt_Handler();
#endif //(MAX35101) || (MAX35103) || (MAX35104)
}

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
