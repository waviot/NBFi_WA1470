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
  NVIC_SetPriority(RTC_Alarm_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),4, 0));
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
  RTC_DateStruct.Month = LL_RTC_MONTH_AUGUST;
  RTC_DateStruct.Year = 20;
  LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);
  /** Enable the Alarm A
  */
  RTC_AlarmStruct.AlarmTime.Hours = 0x0;
  RTC_AlarmStruct.AlarmTime.Minutes = 0x0;
  RTC_AlarmStruct.AlarmTime.Seconds = 0x0;
  RTC_AlarmStruct.AlarmMask = LL_RTC_ALMA_MASK_DATEWEEKDAY;
  RTC_AlarmStruct.AlarmDateWeekDaySel = LL_RTC_ALMA_DATEWEEKDAYSEL_DATE;
  RTC_AlarmStruct.AlarmDateWeekDay = 1;
  LL_RTC_ALMA_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_AlarmStruct);
  LL_RTC_EnableIT_ALRA(RTC);
  /** Enable the Alarm B
  */
  RTC_AlarmStruct.AlarmMask = LL_RTC_ALMA_MASK_ALL;
  RTC_AlarmStruct.AlarmDateWeekDay = 0x1;

  LL_RTC_ALMB_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_AlarmStruct);
  LL_RTC_EnableIT_ALRB(RTC);
  /** Initialize RTC and set the Time and Date
  */
  /** Enable the WakeUp
  */
  LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_16);

}

/* USER CODE BEGIN 1 */
//  RTC_AlarmStruct.AlarmMask = LL_RTC_ALMA_MASK_DATEWEEKDAY;

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
 * \brief get realtime
 * \return struct tm in unixtimestamp
 */
struct tm RTC_GetTime(void)
{
    struct tm currTime1 = DEFAULT_TM;

    uint32_t date2 = LL_RTC_DATE_Get(RTC);
    uint32_t time2 = LL_RTC_TIME_Get(RTC);
    uint32_t date = LL_RTC_DATE_Get(RTC);
    uint32_t time = LL_RTC_TIME_Get(RTC);
    date2 = LL_RTC_DATE_Get(RTC);
    time2 = LL_RTC_TIME_Get(RTC);

    if ((date != date2) || (time != time2))
    {
        date = LL_RTC_DATE_Get(RTC);
        time = LL_RTC_TIME_Get(RTC);
    }

    currTime1.tm_year = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_YEAR(date)) + 100; // In fact: 2000 + 20(example) - 1900
    currTime1.tm_mday = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_DAY(date));
    currTime1.tm_mon = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_MONTH(date)) - 1;

    currTime1.tm_hour = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_HOUR(time));
    currTime1.tm_min = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_MINUTE(time));
    currTime1.tm_sec = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_SECOND(time));

    return currTime1;
}

/*!
 * \brief get realtime seconds
 * \return time_t in unixtimestamp
 */
time_t RTC_GetSeconds(void)
{
    struct tm currTime1 = RTC_GetTime();
    return mktime(&currTime1);
}

/**
  * Brief   This function sets the TIME in RTC.
  * Param   Hour   New Hour to set
  * Param   Minute New Minute to set
  * Param   Second New Second to set
  * Retval  None
  */
void Set_RTC_Time(uint32_t weekDay, uint32_t year, uint32_t month, uint32_t days, uint32_t Hour, uint32_t Minute, uint32_t Second)
{
    /* Disable RTC registers write protection */
    LL_PWR_EnableBkUpAccess();
    LL_RTC_DisableWriteProtection(RTC);

    /* Enter in initialization mode */
    LL_RTC_EnableInitMode(RTC);
    WaitForSynchro_RTC();

    LL_RTC_DATE_Config(RTC, __LL_RTC_CONVERT_BIN2BCD(weekDay),
                       __LL_RTC_CONVERT_BIN2BCD(days),
                       __LL_RTC_CONVERT_BIN2BCD(month),
                       __LL_RTC_CONVERT_BIN2BCD(year));
    /* New time in TR */
    LL_RTC_TIME_Config(RTC,
                       LL_RTC_TIME_FORMAT_AM_OR_24,
                       __LL_RTC_CONVERT_BIN2BCD(Hour),
                       __LL_RTC_CONVERT_BIN2BCD(Minute),
                       __LL_RTC_CONVERT_BIN2BCD(Second));

    /* Exit of initialization mode */
    LL_RTC_DisableInitMode(RTC);
    //WaitForSynchro_RTC();
    /* Enable RTC registers write protection */
    LL_RTC_EnableWriteProtection(RTC);

    RTC_CcSet(0,1000);
}

/**
  * @brief  Wait until the RTC Time and Date registers (RTC_TR and RTC_DR) are
  *         synchronized with RTC APB clock.
  * @param  None
  * @retval false if no error (RTC_ERROR_TIMEOUT will occur if RTC is
  *         not synchronized)
  */
bool WaitForSynchro_RTC(void)
{
    /* Clear RSF flag */
    //LL_RTC_ClearFlag_RS(RTC);

    volatile uint32_t Timeout = RTC_TIMEOUT_VALUE;

    /* Wait the registers to be synchronised */
    //while (LL_RTC_IsActiveFlag_RS(RTC) != 1)
    while (LL_RTC_IsActiveFlag_INIT(RTC) != 1)
    {
        if (LL_SYSTICK_IsActiveCounterFlag())
        {
            Timeout--;
        }
        if (Timeout == 0)
        {
            return true;
        }
    }
    return false;
}

/*!
 * \brief set realtime seconds in unix timestamp
 *  time_t in unixtimestamp
 */
void RTC_SetSeconds(const time_t *newTime)
{
//    time_t tempTime = 0;
//    tempTime = *newTime;
    struct tm timeNow = DEFAULT_TM;
#if defined(_WIN32) && !defined(EFIX64) && !defined(EFI32)
    gmtime_s( tm_buf, tt );
#elif defined(__IAR_SYSTEMS_ICC__)
    gmtime_s( newTime, &timeNow );
#elif !defined(PLATFORM_UTIL_USE_GMTIME)
    gmtime_r( tt, tm_buf );
#endif

    timeNow.tm_year -= 100; //for delete 1900 year
    timeNow.tm_mon += 1;
    Set_RTC_Time(timeNow.tm_wday, timeNow.tm_year, timeNow.tm_mon, timeNow.tm_mday, timeNow.tm_hour, timeNow.tm_min, timeNow.tm_sec);
}

/*!
 * \brief get milliseconds
 * \return time_t time in unixtimestamp in seconds
 */
time_t RTC_GetAbsMilliseconds(void)
{
    return RTC_CntGet(0);
}


/*!
 * \brief Set RTC for task scheduler
 *
 */
void RTC_Init(void)
{
    //main init in cubeMX
    /* Enables the PWR Clock and Enables access to the backup domain */
    LL_RTC_EnableShadowRegBypass(RTC); //for better reading time
    LL_PWR_EnableBkUpAccess();
    SET_BIT(TAMP->CR2, TAMP_CR2_TAMP1NOERASE | TAMP_CR2_TAMP2NOERASE);
    LL_RTC_DisableIT_TAMP1(TAMP);
    LL_RTC_DisableIT_TAMP2(TAMP);
    LL_RTC_TS_DisableOnTamper(RTC);
    LL_RTC_TAMPER_Disable(TAMP, LL_RTC_TAMPER_1 | LL_RTC_TAMPER_2);
    LL_RTC_TAMPER_EnableMask(TAMP, LL_RTC_TAMPER_MASK_TAMPER1 | LL_RTC_TAMPER_MASK_TAMPER2);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_18);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_18);
}

/*!
 * \brief loop tim  init for for scheduler delay
 * \todo refactor this to wakeup
 */
void RTC_LooptimInit(void)
{
    //main init in cubeMX
    LL_RTC_ALMB_Enable(RTC);
}

/*!
 * \brief enable irq for scheduler delay
 *
 * \param chan  not used
 * \todo fix this
 */
void RTC_LoopIrqEnable(uint8_t chan)
{
    LL_RTC_ALMB_Enable(RTC);
    LL_RTC_EnableIT_ALRB(RTC);
}

/*!
 * \brief disable irq for scheduler delay
 *
 * \param chan  not used
 * \todo fix this
 */
void RTC_LoopIrqDisable(uint8_t chan)
{
    LL_RTC_ALMB_Disable(RTC);
    LL_RTC_DisableIT_ALRB(RTC);
}

/*!
 * \brief enable irq from RTC
 *
 * \param chan
 */
void RTC_CcIrqEnable(uint8_t chan)
{
    LL_RTC_EnableIT_ALRA(RTC);
}

/*!
 * \brief disable irq from RTC
 *
 * \param chan
 */
void RTC_CcIrqDisable(uint8_t chan)
{
    LL_RTC_DisableIT_ALRA(RTC);
}

/*!
 * \brief set compare register
 *
 * \param chan not used
 * \param data value of compare register
 */
void RTC_CcSet(uint8_t chan, time_t data)
{
    LL_PWR_EnableBkUpAccess();
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_ALMA_Disable(RTC);

    uint32_t subsec = data % MS_IN_SECOND;

    uint32_t sec = (data / MS_IN_SECOND) % 60;
    subsec = (((MS_IN_SECOND - subsec ) % MS_IN_SECOND) * SUB_SECOND_MAX) / MS_IN_SECOND;//convert to rtc registers


    LL_RTC_ALMA_ConfigTime(RTC, LL_RTC_ALMA_TIME_FORMAT_AM, 0, 0, __LL_RTC_CONVERT_BIN2BCD(sec));
    LL_RTC_ALMA_SetSubSecond(RTC, subsec);

    LL_RTC_ALMA_Enable(RTC);
    LL_RTC_EnableIT_ALRA(RTC);
    LL_RTC_EnableWriteProtection(RTC);

}

/*!
 * \brief get compare register from RTC
 *
 * \param chan not used
 * \return uint16_t compare
 */
time_t RTC_CcGet(uint8_t chan)
{
    struct tm currTime = DEFAULT_TM;
    //    while (!LL_RTC_IsActiveFlag_TS(RTC));

    uint32_t alarm = LL_RTC_ALMA_GetTime(RTC);
    currTime.tm_hour = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_HOUR(alarm));
    currTime.tm_min = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_MINUTE(alarm));
    currTime.tm_sec = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_SECOND(alarm));
    time_t seconds = mktime(&currTime);
    uint32_t subsec = (LL_RTC_ALMA_GetSubSecond(RTC) * MS_IN_SECOND) / SUB_SECOND_MAX;
    return (time_t)(seconds * MS_IN_SECOND + subsec);
}

/*!
 * \brief get RTC counter
 *
 * \param chan not used
 * \return uint16_t counter from lptim
 */
time_t RTC_CntGet(uint8_t chan)
{
    struct tm currTime1 = DEFAULT_TM;
    time_t subSecondMs1 = 0;
    time_t subSecondMs2 = 0;

    uint32_t date = LL_RTC_DATE_Get(RTC);
    uint32_t time = LL_RTC_TIME_Get(RTC);
    subSecondMs1 =  LL_RTC_TIME_GetSubSecond(RTC);//((SUB_SECOND_MAX - x) * MS_IN_SECOND) / SUB_SECOND_MAX;
    uint32_t date2 = LL_RTC_DATE_Get(RTC);
    uint32_t time2 = LL_RTC_TIME_Get(RTC);
    subSecondMs2 = LL_RTC_TIME_GetSubSecond(RTC);//((SUB_SECOND_MAX - LL_RTC_TIME_GetSubSecond(RTC)) * MS_IN_SECOND) / SUB_SECOND_MAX;

    if ((date != date2) || (time != time2) || (subSecondMs1 != subSecondMs2))
    {
        date = LL_RTC_DATE_Get(RTC);
        time = LL_RTC_TIME_Get(RTC);
        subSecondMs1 = LL_RTC_TIME_GetSubSecond(RTC);//((SUB_SECOND_MAX - LL_RTC_TIME_GetSubSecond(RTC)) * MS_IN_SECOND) / SUB_SECOND_MAX;
    }

    currTime1.tm_year = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_YEAR(date)) + 100; // In fact: 2000 + 20(example) - 1900
    currTime1.tm_mday = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_DAY(date));
    currTime1.tm_mon = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_MONTH(date)) - 1;

    currTime1.tm_hour = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_HOUR(time));
    currTime1.tm_min = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_MINUTE(time));
    currTime1.tm_sec = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_SECOND(time));

    time_t seconds = mktime(&currTime1); //currTime1.tm_hour * 3600 + currTime1.tm_min * 60 + currTime1.tm_sec; //

    subSecondMs1 = ((SUB_SECOND_MAX - subSecondMs1) * MS_IN_SECOND) / SUB_SECOND_MAX;

    return seconds * MS_IN_SECOND + subSecondMs1;
}

/*!
 * \brief check flag irq of CMPM RTC
 *
 * \param chan not used
 * \return uint8_t  true if flag is active
 */
uint8_t RTC_CheckCcIrq(uint8_t chan)
{
    return LL_RTC_IsActiveFlag_ALRAM(RTC);
}

/*!
 * \brief setup periodic wakeup for ultrasound measurement
 *
 */
void RTC_SartPeriodicMeas(void)
{

    /* Disable RTC registers write protection */
    LL_RTC_DisableWriteProtection(RTC);

    LL_RTC_CAL_LowPower_Enable(RTC);

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
    LL_RTC_WAKEUP_SetAutoReload(RTC, 1023);
    //LL_RTC_WAKEUP_SetAutoClr(RTC, 1025);
    //LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_16);
    LL_RTC_WAKEUP_Enable(RTC);
    LL_RTC_EnableIT_WUT(RTC);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_20);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_20);

    LL_RTC_EnableWriteProtection(RTC);
}

/**
  * @brief  Alarm callback \ref stm32l4xx_it.c
  * @param  hrtc : RTC handle
  * @retval None
  */
void HAL_RTCEx_WakeUpTimerEventCallback(void)
{
    /* Alarm callback */
    SystemClock_Recover();
}

/**
  * @brief  Alarm callback
  * @param  None
  * @retval None
  */
void Alarm_Callback(void)
{
    scheduler_irq();
}

/**
  * @brief  Period elapsed callback in non-blocking mode \ref stm32l4xx_it.c
  * @retval None
  */
void TimerCaptureCompare_Callback(void)
{
    scheduler_run_callbacks();
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
