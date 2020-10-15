#include "stm32l0xx_hal.h"
static RTC_HandleTypeDef hrtc;

volatile uint32_t rtc_counter = 0;

void RTC_init(void) 
{
 hrtc.Instance = RTC;
 if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2)
 {
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    HAL_RTC_Init(&hrtc);

    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
 }
  __HAL_RCC_RTC_ENABLE();

  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 32768 / 2 - 1, RTC_WAKEUPCLOCK_RTCCLK_DIV2);
  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(RTC_IRQn);
}

void RTC_GPIO_RTC_IRQ(void)
{
  if(__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc, RTC_FLAG_WUTF) != RESET)
  {
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
  }
  __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
  
  rtc_counter++;
  
}