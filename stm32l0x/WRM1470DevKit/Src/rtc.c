#include "stm32l0xx_hal.h"
#include "buttons.h"
static RTC_HandleTypeDef hrtc;

void RTC_init(void) 
{
 // RTC_TimeTypeDef sTime;
  //RTC_DateTypeDef sDate;
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

    /*sTime.Hours = 0x0;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 0x1;
    sDate.Year = 0x0;
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);*/
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
  
  GUI_get_button();

}