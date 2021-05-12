/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
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
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
uint32_t RTC_BackupRead(uint32_t registerIndex);
void RTC_BackupWrite(uint32_t data, uint32_t registerIndex);
void RTC_WakeUpPeriodic(void);
uint64_t RTC_GetAbsMilliseconds(void);
uint32_t RTC_GetSeconds(void);
void RTC_SetSeconds(uint32_t seconds);
void HAL_RTCEx_WakeUpTimerEventCallback(void);

void HAL_LPRTC_Start(void);
void HAL_LPRTC_EnableIt(void);
void HAL_LPRTC_DisableIt(void);
void HAL_LPRTC_SetCompare(uint16_t data);
uint16_t HAL_LPRTC_GetCompare(void);
uint16_t HAL_LPRTC_GetCounter(void);
uint8_t HAL_LPRTC_CheckIrq(void);
uint16_t HAL_LPRTC_GetPrescalerS(void);
void Alarm_Callback(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
