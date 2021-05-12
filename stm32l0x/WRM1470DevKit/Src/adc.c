#include "adc.h"
#include <stm32l0xx_hal.h>
#include "stm32l0xx_ll_adc.h"
#include "stm32l0xx_hal_adc.h"

#define ADC_TIMEOUT     100


static ADC_HandleTypeDef 		AdcHandle;
static ADC_ChannelConfTypeDef 	        sConfig;

void ADC_init(void){
	
        __HAL_RCC_ADC1_CLK_ENABLE();
  
	AdcHandle.Instance = ADC1;

	AdcHandle.Init.OversamplingMode      = DISABLE;

	AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV1;
	AdcHandle.Init.LowPowerAutoPowerOff  = ENABLE;
	AdcHandle.Init.LowPowerFrequencyMode = DISABLE;
	AdcHandle.Init.LowPowerAutoWait      = DISABLE;

	AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
	AdcHandle.Init.SamplingTime          = ADC_SAMPLETIME_160CYCLES_5;
	
	AdcHandle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;
	AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.ContinuousConvMode    = DISABLE;
	AdcHandle.Init.DiscontinuousConvMode = ENABLE;
	AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
	AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_SEQ_CONV;
	AdcHandle.Init.DMAContinuousRequests = DISABLE;

	HAL_ADC_Init(&AdcHandle);
	HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED);

	sConfig.Channel = ADC_CHANNEL_VREFINT | ADC_CHANNEL_TEMPSENSOR | ADC_CHANNEL_8;    
	HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
}

int ADC_get(uint32_t * voltage, uint32_t * temp, uint32_t *ch8){
	
        __disable_irq();

        volatile uint16_t timeout = ADC_TIMEOUT;
	        
        timeout = ADC_TIMEOUT;
	AdcHandle.Instance->CR |= ADC_CR_ADSTART;
	while(!__HAL_ADC_GET_FLAG(&AdcHandle, ADC_FLAG_EOC) && --timeout);
	if (!timeout) {__enable_irq();	return -1;}
	AdcHandle.Instance->ISR = 0xFFFF;	
	*ch8 = AdcHandle.Instance->DR;
       
	timeout = ADC_TIMEOUT;
	AdcHandle.Instance->CR |= ADC_CR_ADSTART;
	while(!__HAL_ADC_GET_FLAG(&AdcHandle, ADC_FLAG_EOC) && --timeout);
	if (!timeout) {__enable_irq();	return -1;}
	AdcHandle.Instance->ISR = 0xFFFF;
	*voltage = __LL_ADC_CALC_VREFANALOG_VOLTAGE(AdcHandle.Instance->DR, LL_ADC_RESOLUTION_12B);
		
        
        timeout = ADC_TIMEOUT;
	AdcHandle.Instance->CR |= ADC_CR_ADSTART;
	while(!__HAL_ADC_GET_FLAG(&AdcHandle, ADC_FLAG_EOC) && --timeout);
        if (!timeout) {__enable_irq();	return -1;}
	AdcHandle.Instance->ISR = 0xFFFF;	
	*temp = __LL_ADC_CALC_TEMPERATURE(*voltage, AdcHandle.Instance->DR, LL_ADC_RESOLUTION_12B);
        __enable_irq();
        
	return 0;
}


/*
extern ADC_HandleTypeDef hadc;

int8_t ADC_Get(void)
{
  uint16_t timeout;
  
  timeout = ADC_TIMEOUT;
  HAL_ADC_Start(&hadc);
  while(!__HAL_ADC_GET_FLAG(&hadc, ADC_FLAG_EOC) && --timeout);
  if (!timeout)
    return -1;
  //if(HAL_ADC_PollForConversion(&hadc, 0) == HAL_OK)
  adc_vdda = __LL_ADC_CALC_VREFANALOG_VOLTAGE(HAL_ADC_GetValue(&hadc), LL_ADC_RESOLUTION_12B);
  
  timeout = ADC_TIMEOUT;
  HAL_ADC_Start(&hadc);
  while(!__HAL_ADC_GET_FLAG(&hadc, ADC_FLAG_EOC) && --timeout);
  if (!timeout)
    return -1;
  //if(HAL_ADC_PollForConversion(&hadc, 0) == HAL_OK)
  adc_temp = __LL_ADC_CALC_TEMPERATURE(adc_vdda, HAL_ADC_GetValue(&hadc), LL_ADC_RESOLUTION_12B);
  
  return 0;
}
*/