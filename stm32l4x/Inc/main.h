/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_adc.h"
#include "stm32l4xx_ll_crc.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_iwdg.h"
#include "stm32l4xx_ll_crs.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "WVT_EEPROM.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
  typedef struct
  {
    uint32_t work_mode;
    uint32_t display_mode;
    uint32_t voltage;
    uint32_t temperature;
  } main_par_t;

  enum mode_t
  {
    MODE_NORMAL,
    MODE_TEST,
  };

  enum event_t
  {
    EVENT_FROST = 0,
    EVENT_CMD,
    EVENT_LEAK,
    EVENT_BREAK,
    EVENT_TEST_LINK,
    EVENT_RESET,
    EVENT_SENSOR
  };
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define BAIKAL_B

#define HW_VERSION ((uint32_t)0)
#define HW_SUB_VERSION ((uint32_t)1)
#define FW_VERSION ((uint32_t)1)
#define FW_SUB_VERSION ((uint32_t)7)

//#define BAND				UL868100_DL869550		//	EU
#define BAND UL868800_DL869150 //	RU
  //#define BAND				UL864000_DL863500		//	KZ
  //#define BAND				UL458550_DL453750		//	UZ

//#define USE_TIC33
#define USE_OSW
  //#define USE_LCD_AQUA3

  //#define USE_EXTERNAL_RTC

#define USE_WA1470
  //#define PLOT_SPECTRUM
  //#define NBFI_AT_SERVER
  //#define WA1470_LOG
#define USE_EXTERNAL_RTC

  //#define USE_HALL_METER
  //#define USE_WVT_DSP_FLOAT

#define UPDATE_TEMPERATURE_PERIOD (10)

#define ADC_ULTRA_TIMEOUT 500000
#define SPI_TIMEOUT 50000

#define CORR_DIFF_CALC_SIZE 8

#define CORR_SIN_CALC_SIZE 128

#define ADC_BUF_SIZE 512
#define WAVE_1_START 80
#define WAVE_1_MID 170
#define WAVE_1_SIZE 150 //128 not working hardfault with float
#define WAVE_1_END (WAVE_1_SIZE + WAVE_1_START)
#define WAVE_FREQ_SIZE 20
#define WAVE_THRESHOLD 3000
#define GEN_THRESHOLD 90000

#define ADC_POINT_SECOND_DIF 280

#define CORR_SIN_CALC_POINTS WAVE_1_SIZE

/*!
 * \brief milliseconds in seconds
 *
 */
#define MS_IN_SECOND 1000

/*!
 * \brief sub second counter
 */
#define SUB_SECOND_MAX 1024
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MAX_TIMEOUT 10000
#define COUNTER_FOR_1MGHZ 80
#define PWM_FOR_1MHZ 40
#define ADC_GEN_DELAY 4600
#define LCD_LOAD_Pin LL_GPIO_PIN_1
#define LCD_LOAD_GPIO_Port GPIOB
#define WA_CHIP_EN_Pin LL_GPIO_PIN_4
#define WA_CHIP_EN_GPIO_Port GPIOB
#define WA_IRQ_Pin LL_GPIO_PIN_5
#define WA_IRQ_GPIO_Port GPIOB
#define WA_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define SPI1_NSS_Pin LL_GPIO_PIN_6
#define SPI1_NSS_GPIO_Port GPIOB
#define Button_Pin LL_GPIO_PIN_7
#define Button_GPIO_Port GPIOB
#define Button_EXTI_IRQn EXTI9_5_IRQn
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif
/* USER CODE BEGIN Private defines */
  void SendErrors(uint8_t errors);
  void SystemClock_Recover(void);
  void SetWeCanSleep(bool enable);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
