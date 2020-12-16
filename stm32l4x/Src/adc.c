/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include <math.h>

/* Init variable out of expected ADC conversion data range */
#define VAR_CONVERTED_DATA_INIT_VALUE (__LL_ADC_DIGITAL_SCALE(LL_ADC_RESOLUTION_12B) + 1)
/* Definitions of environment analog values */
/* Value of analog reference voltage (Vref+), connected to analog voltage   */
/* supply Vdda (unit: mV).                                                  */
#define VDDA_APPLI ((uint32_t)3600)

#define ADC_DELAY_CALIB_ENABLE_CPU_CYCLES (LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES * 32)

#define ADC_TIMEOUT 100

static uint32_t ADCBuffer[ADC_BUF_SIZE] = {0};
/* Variables for ADC conversion data */
__IO uint16_t uhADCxConvertedData = VAR_CONVERTED_DATA_INIT_VALUE;     /* ADC group regular conversion data */
__IO uint16_t uhADCxConvertedDataTemp = VAR_CONVERTED_DATA_INIT_VALUE; /* ADC group regular conversion data */
/* Variables for ADC conversion data computation to physical values */
__IO uint16_t hADCxConvertedData_Temperature_DegreeCelsius = 0; /* Value of temperature calculated from ADC conversion data (unit: degree Celcius) */
/* USER CODE END 0 */

/* ADC1 init function */
void MX_ADC1_Init(void)
{
  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  /* Peripheral clock enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC);

  /* ADC1 interrupt Init */
  NVIC_SetPriority(ADC1_2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(ADC1_2_IRQn);

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM15_TRGO;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT|LL_ADC_PATH_INTERNAL_TEMPSENSOR);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV1;
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);
  LL_ADC_REG_SetTriggerEdge(ADC1, LL_ADC_REG_TRIG_EXT_FALLING);

  /* Disable ADC deep power down (enabled by default after reset state) */
  LL_ADC_DisableDeepPowerDown(ADC1);
  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC1);
  /* Delay for ADC internal voltage regulator stabilization. */
  /* Compute number of CPU cycles to wait for, from delay in us. */
  /* Note: Variable divided by 2 to compensate partially */
  /* CPU processing cycles (depends on compilation optimization). */
  /* Note: If system core clock frequency is below 200kHz, wait time */
  /* is only a few CPU processing cycles. */
  uint32_t wait_loop_index;
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_TEMPSENSOR);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_TEMPSENSOR, LL_ADC_SAMPLINGTIME_2CYCLES_5);
  LL_ADC_SetChannelSingleDiff(ADC1, LL_ADC_CHANNEL_TEMPSENSOR, LL_ADC_SINGLE_ENDED);

}

/* USER CODE BEGIN 1 */

/**
  * @brief  Configure ADC (ADC instance: ADC1) and GPIO used by ADC channels.
  * @note   In case re-use of this function outside of this example:
  * @note   Peripheral configuration is minimal configuration from reset values.
  *         Thus, some useless LL unitary functions calls below are provided as
  *         commented examples - setting is default configuration from reset.
  * @param  None
  * @retval None
  */
void Configure_ADC(void)
{
  /*## Configuration of ADC hierarchical scope: ADC group regular ############*/

  /* Set ADC group regular trigger source */
  LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);

  /* Set ADC group regular trigger polarity */
  // LL_ADC_REG_SetTriggerEdge(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

  /* Set ADC group regular continuous mode */
  LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);

  /* Set ADC channels sampling time */
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_TEMPSENSOR, LL_ADC_SAMPLINGTIME_640CYCLES_5);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_VREFINT, LL_ADC_SAMPLINGTIME_640CYCLES_5);
}

/**
  * @brief  Perform ADC activation procedure to make it ready to convert
  *         (ADC instance: ADC1).
  * @note   Operations:
  *         - ADC instance
  *           - Disable deep power down
  *           - Enable internal voltage regulator
  *           - Run ADC self calibration
  *           - Enable ADC
  *         - ADC group regular
  * @param  None
  * @retval None
  */
void Activate_ADC(void)
{
  __IO uint32_t wait_loop_index = 0;
  /* Disable ADC deep power down (enabled by default after reset state) */
  LL_ADC_DisableDeepPowerDown(ADC1);

  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC1);

  /* Delay for ADC internal voltage regulator stabilization.                */
  /* Compute number of CPU cycles to wait for, from delay in us.            */
  /* Note: Variable divided by 2 to compensate partially                    */
  /*       CPU processing cycles (depends on compilation optimization).     */
  /* Note: If system core clock frequency is below 200kHz, wait time        */
  /*       is only a few CPU processing cycles.                             */
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while (wait_loop_index != 0)
  {
    wait_loop_index--;
  }

  /* Run ADC self calibration */
  LL_ADC_StartCalibration(ADC1, LL_ADC_SINGLE_ENDED);

  /* Poll for ADC effectively calibrated */

  while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0)
  {
  }

  /* Delay between ADC end of calibration and ADC enable.                   */
  /* Note: Variable divided by 2 to compensate partially                    */
  /*       CPU processing cycles (depends on compilation optimization).     */
  wait_loop_index = (ADC_DELAY_CALIB_ENABLE_CPU_CYCLES >> 1);
  while (wait_loop_index != 0)
  {
    wait_loop_index--;
  }

  /* Enable ADC */
  LL_ADC_Enable(ADC1);

  /* Poll for ADC ready to convert */

  while (LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0)
  {
  }
}

/*!
 * \brief adc code for measurement voltage and temperature
 * \param voltage ptr
 * \param temp ptr
 * \return int8_t don't know
 */
int8_t ADC_GetVoltageAndTemp(uint32_t *voltage, int32_t *temp)
{
    /// \todo check this with ultrasound measurement every 10 seconds
  volatile uint32_t wait_loop_index, timeout;
  Configure_ADC();
  Activate_ADC();

  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_VREFINT);
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT | LL_ADC_PATH_INTERNAL_TEMPSENSOR);

  LL_ADC_REG_StartConversion(ADC1);
  timeout = ADC_TIMEOUT;
  while ((LL_ADC_IsActiveFlag_EOSMP(ADC1) == 0) && (LL_ADC_IsActiveFlag_EOC(ADC1) == 0) && --timeout)
  {
  }
  if (!timeout)
    return -1;

  LL_ADC_ClearFlag_EOC(ADC1);
  LL_ADC_ClearFlag_EOSMP(ADC1);
  LL_mDelay(1);
  /* Retrieve ADC conversion data */
  /* (data scale corresponds to ADC resolution: 12 bits) */
  uhADCxConvertedData = LL_ADC_REG_ReadConversionData12(ADC1);

  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_TEMPSENSOR);

  *voltage = __LL_ADC_CALC_VREFANALOG_VOLTAGE(uhADCxConvertedData, LL_ADC_RESOLUTION_12B);

  //    LL_mDelay(1);
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while (wait_loop_index != 0)
  {
    wait_loop_index--;
  }

  LL_ADC_REG_StartConversion(ADC1);

  timeout = ADC_TIMEOUT;
  while ((LL_ADC_IsActiveFlag_EOSMP(ADC1) == 0) && (LL_ADC_IsActiveFlag_EOC(ADC1) == 0) && --timeout)
  {
  }
  if (!timeout)
    return -1;

  LL_ADC_ClearFlag_EOC(ADC1);
  LL_ADC_ClearFlag_EOSMP(ADC1);
  /* Retrieve ADC conversion data */
  /* (data scale corresponds to ADC resolution: 12 bits) */
  uhADCxConvertedDataTemp = LL_ADC_REG_ReadConversionData12(ADC1);
  /* Computation of ADC conversions raw data to physical values             */
  /* using LL ADC driver helper macro.                                      */
  *temp = __LL_ADC_CALC_TEMPERATURE(3573, uhADCxConvertedDataTemp, LL_ADC_RESOLUTION_12B); //*voltage

  ADC_StopMeas();
  LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
  MX_ADC1_Init();
  return 0;
}

/**
 * @brief begin measurement of ultrasound wave
 *
 */
void ADC_InitMeas(void)
{
  /* Run ADC self calibration */
  LL_ADC_StartCalibration(ADC1, LL_ADC_SINGLE_ENDED);
  /* Poll for ADC effectively calibrated */
  while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0)
  {
  }
  /* Run ADC self calibration */
  LL_ADC_StartCalibration(ADC2, LL_ADC_SINGLE_ENDED);
  /* Poll for ADC effectively calibrated */
  while (LL_ADC_IsCalibrationOnGoing(ADC2) != 0)
  {
  }
  /* Set DMA transfer addresses of source and destination */
  LL_DMA_ConfigAddresses(DMA1,
                         LL_DMA_CHANNEL_1,
                         LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA_MULTI), // LL_ADC_DMA_REG_REGULAR_DATA_MULTI), LL_ADC_DMA_REG_REGULAR_DATA
                         (uint32_t)&ADCBuffer,
                         LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  /* Enable DMA transfer interruption: transfer complete */
  LL_DMA_EnableIT_TC(DMA1,
                     LL_DMA_CHANNEL_1);

  /* Disable ADC deep power down (enabled by default after reset state) */
  LL_ADC_DisableDeepPowerDown(ADC1);
  LL_ADC_DisableDeepPowerDown(ADC2);
  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC1);
  LL_ADC_EnableInternalRegulator(ADC2);

  NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  LL_ADC_EnableIT_OVR(ADC1);
  NVIC_SetPriority(ADC1_2_IRQn, 0);
  NVIC_EnableIRQ(ADC1_2_IRQn);

    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_EXT_TIM15_TRGO);
}

/**
 * @brief begin measurement of ultrasound wave
 *
 */
void ADC_BeginMeas(void)
{

  LL_ADC_Enable(ADC1);
  LL_ADC_Enable(ADC2);

  LL_ADC_EnableInternalRegulator(ADC1);
  LL_ADC_EnableInternalRegulator(ADC2);

  /* Set DMA transfer size */
  LL_DMA_SetDataLength(DMA1,
                       LL_DMA_CHANNEL_1,
                       ADC_BUF_SIZE);

  /*## Activation of DMA #####################################################*/
  /* Enable the DMA transfer */
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

  LL_ADC_EnableIT_OVR(ADC1);

  LL_ADC_REG_StartConversion(ADC1);
  //LL_ADC_REG_StartConversion(ADC2);
}

void ADC_StopMeas(void)
{
  //AdcDmaTransferComplete_Callback();
  LL_ADC_REG_StopConversion(ADC1);
  LL_ADC_REG_StopConversion(ADC2);
//
//  LL_ADC_DisableInternalRegulator(ADC1);
//  LL_ADC_DisableInternalRegulator(ADC2);
//
//  LL_ADC_Disable(ADC1);
//  LL_ADC_Disable(ADC2);

  LL_ADC_ClearFlag_OVR(ADC1);
  LL_ADC_ClearFlag_EOSMP(ADC1);
  LL_ADC_ClearFlag_EOS(ADC1);
  LL_ADC_ClearFlag_EOC(ADC1);

  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
}

uint32_t *ADC_GetPtrBuffer(void)
{
  return (uint32_t *)ADCBuffer;
}

/**
  * @brief  Conversion complete callback in non-blocking mode. \ref stm32l4xx_it.c
  * @param hadc ADC handle
  * @retval None
  */
void AdcDmaTransferComplete_Callback()
{
  ADC_StopMeas();
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
