/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "scheduler.h"
#include "scheduler_hal.h"
/* USER CODE END 0 */

/* TIM6 init function */
void MX_TIM6_Init(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);

  /* TIM6 interrupt Init */
  NVIC_SetPriority(TIM6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM6_IRQn);

  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 1;
  LL_TIM_Init(TIM6, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM6);
  LL_TIM_SetTriggerOutput(TIM6, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM6);

}

/* USER CODE BEGIN 1 */
void HAL_TIM6_Start(void)
{
    LL_TIM_SetPrescaler(TIM6, __LL_TIM_CALC_PSC(SystemCoreClock, WA_LOOPTIM_TIM_FREQ));
    HAL_TIM6_EnableIt();
    LL_TIM_EnableCounter(TIM6);
}

void HAL_TIM6_EnableIt(void)
{
    LL_TIM_EnableIT_UPDATE(TIM6);
}

void HAL_TIM6_DisableIt(void)
{
    LL_TIM_DisableIT_UPDATE(TIM6);
}

void HAL_TIM6_Callback(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM6);
    scheduler_run_callbacks();
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
