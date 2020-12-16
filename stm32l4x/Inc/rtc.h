/**
  ******************************************************************************
  * File Name          : RTC.h
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __rtc_H
#define __rtc_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define DEFAULT_TM {0, 0, 0, 1, 0, 70, 0, 0, 0}
#define RTC_TIMEOUT_VALUE          ((uint32_t)1000)  /* 1 s */
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
uint32_t RTC_BackupRead(uint32_t registerIndex);
void RTC_BackupWrite(uint32_t data, uint32_t registerIndex);
struct tm RTC_GetTime(void);
time_t RTC_GetSeconds(void);
void RTC_SetSeconds(const time_t *newTime);
time_t RTC_GetAbsMilliseconds(void);
void RTC_Init(void);
void RTC_LooptimInit(void);
void RTC_LoopIrqEnable(uint8_t chan);
void RTC_LoopIrqDisable(uint8_t chan);
void RTC_CcIrqEnable(uint8_t chan);
void RTC_CcIrqDisable(uint8_t chan);
void RTC_CcSet(uint8_t chan, time_t data);
time_t RTC_CcGet(uint8_t chan);
time_t RTC_CntGet(uint8_t chan);
uint8_t RTC_CheckCcIrq(uint8_t chan);
void RTC_SartPeriodicMeas(void);
void HAL_RTCEx_WakeUpTimerEventCallback(void);
void Alarm_Callback(void);
void TimerCaptureCompare_Callback(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ rtc_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
