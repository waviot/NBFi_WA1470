/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "scheduler.h"

volatile uint32_t rtc_counter = 0;

uint16_t PrescalerS = 0;
/*!
 * \brief backup register array
 *
 */
uint32_t aBKPDataReg[RTC_BKP_NUMBER] =
    {
        LL_RTC_BKP_DR0, LL_RTC_BKP_DR1, LL_RTC_BKP_DR2,
        LL_RTC_BKP_DR3, LL_RTC_BKP_DR4, LL_RTC_BKP_DR5,
        LL_RTC_BKP_DR6, LL_RTC_BKP_DR7, LL_RTC_BKP_DR8,
        LL_RTC_BKP_DR9, LL_RTC_BKP_DR10, LL_RTC_BKP_DR11,
        LL_RTC_BKP_DR12, LL_RTC_BKP_DR13, LL_RTC_BKP_DR14,
        LL_RTC_BKP_DR15, LL_RTC_BKP_DR16, LL_RTC_BKP_DR17,
        LL_RTC_BKP_DR18, LL_RTC_BKP_DR19, LL_RTC_BKP_DR20,
        LL_RTC_BKP_DR21, LL_RTC_BKP_DR22, LL_RTC_BKP_DR23,
        LL_RTC_BKP_DR24, LL_RTC_BKP_DR25, LL_RTC_BKP_DR26,
        LL_RTC_BKP_DR27, LL_RTC_BKP_DR28, LL_RTC_BKP_DR29,
        LL_RTC_BKP_DR30, LL_RTC_BKP_DR31};

bool WaitForSynchro_RTC(void);
/* USER CODE END 0 */

/* RTC init function */
void MX_RTC_Init(void)
{
  LL_RTC_InitTypeDef RTC_InitStruct = {0};
  LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
  LL_RTC_DateTypeDef RTC_DateStruct = {0};
  LL_RTC_AlarmTypeDef RTC_AlarmStruct = {0};

  /* Peripheral clock enable */
  LL_RCC_EnableRTC();

  /* RTC interrupt Init */
  NVIC_SetPriority(RTC_WKUP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),7, 0));
  NVIC_EnableIRQ(RTC_WKUP_IRQn);
  NVIC_SetPriority(RTC_Alarm_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(RTC_Alarm_IRQn);

  /** Initialize RTC and set the Time and Date
  */
  RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
  RTC_InitStruct.AsynchPrescaler = 31;
  RTC_InitStruct.SynchPrescaler = 1023;
  LL_RTC_Init(RTC, &RTC_InitStruct);
  RTC_TimeStruct.Hours = 0;
  RTC_TimeStruct.Minutes = 0;
  RTC_TimeStruct.Seconds = 0;
  LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct);
  RTC_DateStruct.WeekDay = LL_RTC_WEEKDAY_MONDAY;
  RTC_DateStruct.Month = LL_RTC_MONTH_JANUARY;
  RTC_DateStruct.Year = 0;
  LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);
  /** Enable the Alarm A
  */
  RTC_AlarmStruct.AlarmTime.Hours = 0x0;
  RTC_AlarmStruct.AlarmTime.Minutes = 0x0;
  RTC_AlarmStruct.AlarmTime.Seconds = 0x0;
  RTC_AlarmStruct.AlarmMask = LL_RTC_ALMA_MASK_ALL;
  RTC_AlarmStruct.AlarmDateWeekDaySel = LL_RTC_ALMA_DATEWEEKDAYSEL_DATE;
  RTC_AlarmStruct.AlarmDateWeekDay = 1;
  LL_RTC_ALMA_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_AlarmStruct);
  LL_RTC_EnableIT_ALRA(RTC);
  /** Initialize RTC and set the Time and Date
  */
  /** Enable the WakeUp
  */
  LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_16);

}

/* USER CODE BEGIN 1 */
/*!
 * \brief read backup register
 * \param registerIndex index of register must be below RTC_BKP_NUMBER
 * \return uint32_t data from register
 */
uint32_t RTC_BackupRead(uint32_t registerIndex)
{
    return LL_RTC_BKP_GetRegister(TAMP, aBKPDataReg[registerIndex]);
}

/*!
 * \brief write data to backup register
 *
 * \param data for writing to register
 * \param registerIndex index of register  must be below RTC_BKP_NUMBER
 */
void RTC_BackupWrite(uint32_t data, uint32_t registerIndex)
{
    LL_RTC_BKP_SetRegister(TAMP, aBKPDataReg[registerIndex], data);
}

/*!
 * \brief setup periodic wakeup for ultrasound measurement
 *
 */
void RTC_WakeUpPeriodic(void)
{

    /* Disable RTC registers write protection */
    LL_RTC_DisableWriteProtection(RTC);

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
    LL_RTC_WAKEUP_Enable(RTC);
    LL_RTC_EnableIT_WUT(RTC);
    LL_RTC_ClearFlag_WUT(RTC);
    /* Disable wake up timer to modify it */
    LL_RTC_WAKEUP_Disable(RTC);
    /* Wait until it is allow to modify wake up reload value */

    while (LL_RTC_IsActiveFlag_WUTW(RTC) != 1)
    {
    }

    /* Setting the Wakeup time to RTC_WUT_TIME s
    If LL_RTC_WAKEUPCLOCK_CKSPRE is selected, the frequency is 2048Hz,
    this allows to get a wakeup time equal to RTC_WUT_TIME
    if the counter is RTC_WUT_TIME */
    LL_RTC_WAKEUP_SetAutoReload(RTC, 2047);
    LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_16);
    LL_RTC_WAKEUP_Enable(RTC);
    LL_RTC_EnableIT_WUT(RTC);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_20);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_20);

    LL_RTC_EnableWriteProtection(RTC);
}

uint64_t RTC_GetAbsMilliseconds(void)
{
    return rtc_counter +(uint32_t)HAL_LPRTC_GetCounter() * SECONDS(1) / PrescalerS;
}

uint32_t RTC_GetSeconds(void)
{
    return rtc_counter;
}

void RTC_SetSeconds(uint32_t seconds)
{
    rtc_counter = seconds;
}

/**
  * @brief  Alarm callback \ref stm32l4xx_it.c
  * @param  hrtc : RTC handle
  * @retval None
  */
void HAL_RTCEx_WakeUpTimerEventCallback(void)
{
    /* Alarm callback */
    rtc_counter++;
}

void HAL_LPRTC_Start(void)
{
    HAL_LPRTC_EnableIt();
    LL_RTC_ALMA_Enable(RTC);

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_18);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_18);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_18);
    PrescalerS = LL_RTC_GetSynchPrescaler(RTC) + 1;

}

void HAL_LPRTC_EnableIt(void)
{
   LL_RTC_EnableIT_ALRA(RTC);
}

void HAL_LPRTC_DisableIt(void)
{
   LL_RTC_DisableIT_ALRA(RTC);
}

void HAL_LPRTC_SetCompare(uint16_t data)
{
   LL_RTC_ALMA_SetSubSecond(RTC, data);
}

uint16_t HAL_LPRTC_GetCompare(void)
{
   return LL_RTC_ALMA_GetSubSecond(RTC);
}

uint16_t HAL_LPRTC_GetCounter(void)
{
    return LL_RTC_TIME_GetSubSecond(RTC);
}

uint8_t HAL_LPRTC_CheckIrq(void)
{
    return LL_RTC_IsEnabledIT_ALRA(RTC) && LL_RTC_IsActiveFlag_ALRA(RTC);
}


uint16_t HAL_LPRTC_GetPrescalerS(void)
{
    return PrescalerS;
}

void Alarm_Callback(void)
{
    scheduler_irq();
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
