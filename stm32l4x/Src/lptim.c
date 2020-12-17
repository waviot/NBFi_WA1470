/**
  ******************************************************************************
  * @file    lptim.c
  * @brief   This file provides code for the configuration
  *          of the LPTIM instances.
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
#include "lptim.h"

/* USER CODE BEGIN 0 */
#include "scheduler.h"
/* USER CODE END 0 */

/* LPTIM1 init function */
void MX_LPTIM1_Init(void)
{

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPTIM1);

  /* LPTIM1 interrupt Init */
  NVIC_SetPriority(LPTIM1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(LPTIM1_IRQn);

  LL_LPTIM_SetClockSource(LPTIM1, LL_LPTIM_CLK_SOURCE_INTERNAL);
  LL_LPTIM_SetPrescaler(LPTIM1, LL_LPTIM_PRESCALER_DIV32);
  LL_LPTIM_SetPolarity(LPTIM1, LL_LPTIM_OUTPUT_POLARITY_REGULAR);
  LL_LPTIM_SetUpdateMode(LPTIM1, LL_LPTIM_UPDATE_MODE_IMMEDIATE);
  LL_LPTIM_SetCounterMode(LPTIM1, LL_LPTIM_COUNTER_MODE_INTERNAL);
  LL_LPTIM_TrigSw(LPTIM1);
  LL_LPTIM_SetInput1Src(LPTIM1, LL_LPTIM_INPUT1_SRC_GPIO);
  LL_LPTIM_SetInput2Src(LPTIM1, LL_LPTIM_INPUT2_SRC_GPIO);

}

/* USER CODE BEGIN 1 */
void HAL_LPTIM_Start(void)
{
    LL_LPTIM_SetAutoReload(LPTIM1, 0xffff);
    HAL_LPTIM_EnableIt();
    LL_LPTIM_Enable(LPTIM1);
    LL_LPTIM_StartCounter(LPTIM1,LL_LPTIM_OPERATING_MODE_CONTINUOUS);
}

void HAL_LPTIM_EnableIt(void)
{
   LL_LPTIM_EnableIT_CMPM(LPTIM1);
}

void HAL_LPTIM_DisableIt(void)
{
   LL_LPTIM_DisableIT_CMPM(LPTIM1);
}

void HAL_LPTIM_SetCompare(uint16_t data)
{
   LL_LPTIM_SetCompare(LPTIM1, data);
}

uint16_t HAL_LPTIM_GetCompare(void)
{
   return LL_LPTIM_GetCompare(LPTIM1);
}

uint16_t HAL_LPTIM_GetCounter(void)
{
    return LL_LPTIM_GetCounter(LPTIM1);
}

uint8_t HAL_LPTIM_CheckIrq(void)
{
    return LL_LPTIM_IsEnabledIT_CMPM(LPTIM1) && LL_LPTIM_IsActiveFlag_CMPM(LPTIM1);
}

void HAL_LPTIM_Callback(void)
{
    LL_LPTIM_ClearFLAG_CMPM(LPTIM1);
    scheduler_irq();
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
