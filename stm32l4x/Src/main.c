/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cb.h"
#include "libmfwtimer.h"
#include "meter.h"
#include "radio.h"
#include "scheduler_hal.h"
#include "water7.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_LINK_SEND_PERIOD (60)
#define TEST_LINK_TIMEOUT (10 * TEST_LINK_SEND_PERIOD)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/*!
 * \brief every second scheduler descriptor
 *
 */
struct scheduler_desc everysec_desc;
/*!
 * \brief for time difference
 *
 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void EnterRunMode_LowPower_DownTo2MHz(void);
void EnterRunMode_UpTo80MHz(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*!
 * \brief set water7 protocol and meter configuration
 *
 */
void waterAndMeterInit(void)
{
  water7_params_str *water7_params_p = Water7GetParams();
  meter_params_str *meter_params_p = 0; // meter_GetParams();
  /// \todo fix this
  get_saved_param(water7_params_p, meter_params_p);

  Water7PushFunc(WATER7_FUNC_SET_DATA, water7set_data);
  Water7PushFunc(WATER7_FUNC_GET_DATA, water7get_data);
  Water7PushFunc(WATER7_FUNC_SAVE_DATA, water7save_data);
  Water7PushFuncRfl(WATER7_FUNC_RFL, water7_rfl);

  Water7Init(water7_params_p);
  /// \todo fix this
  meter_init(meter_params_p, meter_inc_cb);
  Water7SendEvent(EVENT_RESET, 0);
}

/*!
 * \brief send link test info
 *
 * \param inc increment in seconds
 */
void test_link(uint8_t inc)
{
  static uint32_t test_link_prev, test_link_cnt = 0;
  //  if (inc)
  //  {
  if (test_link_cnt < TEST_LINK_TIMEOUT)
  {
    if (test_link_cnt >= test_link_prev + TEST_LINK_SEND_PERIOD)
    {
      Water7SendEvent(EVENT_TEST_LINK, test_link_cnt);
      test_link_prev = test_link_cnt;
    }
    test_link_cnt += inc;
  }
  //  }
  //  else
  //    test_link_cnt = test_link_prev = 0;
}

void SendErrors(uint8_t errors)
{
  if (errors & METER_ERROR_LEAK)
  {
    Water7SendEvent(EVENT_LEAK, 0);
  }
  if (errors & METER_ERROR_BREAK)
  {
    Water7SendEvent(EVENT_BREAK, 0);
  }
  if (errors & METER_ERROR_FROST)
  {
    Water7SendEvent(EVENT_FROST, 0);
  }
  if (errors & METER_ERROR_SENSOR)
  {
    Water7SendEvent(EVENT_SENSOR, 0);
  }
}
/*!
 * \brief every second tick
 *
 * \param desc
 */
void EverySec(struct scheduler_desc *desc)
{
  //  scheduler_add_task(desc, EverySec, RELATIVE, SECONDS(1));
  //IWDG_Refresh();

  NBFI_Main_Level_Loop();
//  time_t timeNow = RTC_GetSeconds();
//  Water7OneSec(RTC_GetTime());
//
//  uint8_t isOk = MeterEverySecHandler(timeNow);
//  test_link(timeNow - SecondsOld);
//  SecondsOld = timeNow;
}

/*!
 * \brief send complete callback from nb-fi
 *
 * \param ul
 */
void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{
}

/*!
 * \brief receive complete callback from nb-fi
 *
 * \param data
 * \param length
 */
void nbfi_receive_complete(uint8_t *data, uint16_t length)
{
  Water7RXcallback(data, length);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
#ifdef DEBUG
  LL_DBGMCU_EnableDBGStopMode();
  LL_DBGMCU_APB1_GRP1_FreezePeriph(LL_DBGMCU_APB1_GRP1_RTC_STOP);
#endif
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CRC_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_IWDG_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  IWDG_Init();
  /* Ensure that MSI is wake-up system clock */
  LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_MSI);

  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
  LL_LPM_EnableDeepSleep();

  SPI_Activate();
  radio_init();
  radio_load_id_and_key_of_sr_server(&sr_server_modem_id_and_key);
//  waterAndMeterInit();
  scheduler_add_task(&everysec_desc, EverySec, RUN_CONTINUOSLY_RELATIVE, SECONDS(1));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      IWDG_Refresh();
      //NBFI_Main_Level_Loop();
      //scheduler_run_callbacks();
      if (NBFi_can_sleep() && scheduler_can_sleep())
      {
          __WFI();//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
      else
      {
          //HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_LSI_Enable();

   /* Wait till LSI is ready */
  while(LL_RCC_LSI_IsReady() != 1)
  {

  }
  LL_RCC_MSI_Enable();

   /* Wait till MSI is ready */
  while(LL_RCC_MSI_IsReady() != 1)
  {

  }
  LL_RCC_MSI_EnableRangeSelection();
  LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_11);
  LL_RCC_MSI_SetCalibTrimming(0);
  LL_PWR_EnableBkUpAccess();
  LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
  LL_RCC_LSE_Enable();

   /* Wait till LSE is ready */
  while(LL_RCC_LSE_IsReady() != 1)
  {

  }
  LL_RCC_MSI_EnablePLLMode();
  if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
  {
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
  }
  LL_RCC_EnableRTC();
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(48000000);

  LL_SetSystemCoreClock(48000000);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
