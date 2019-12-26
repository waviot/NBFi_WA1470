#include "stm32_init.h"
#include "stm32l0xx_ll_adc.h"
#include "stm32l0xx_hal_adc.h"

static ADC_HandleTypeDef 		AdcHandle;
static ADC_ChannelConfTypeDef 	sConfig;

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
	AdcHandle.Init.ExternalTrigConv		 = ADC_SOFTWARE_START;
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
	AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_SEQ_CONV;
	AdcHandle.Init.DMAContinuousRequests = DISABLE;

	HAL_ADC_Init(&AdcHandle);
	HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED);

	sConfig.Channel = ADC_CHANNEL_VREFINT | ADC_CHANNEL_TEMPSENSOR;    
	HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
}

void ADC_deinit(void){
	HAL_ADC_DeInit(&AdcHandle);
	ADC->CCR = 0;
}

int ADC_get(uint32_t * voltage, uint32_t * temp){
	volatile uint16_t timeout = ADC_TIMEOUT;
	
	timeout = ADC_TIMEOUT;
	AdcHandle.Instance->CR |= ADC_CR_ADSTART;
	while(!__HAL_ADC_GET_FLAG(&AdcHandle, ADC_FLAG_EOC) && --timeout);
	if (!timeout)
	  	return -1;
	AdcHandle.Instance->ISR = 0xFFFF;
	*voltage = __LL_ADC_CALC_VREFANALOG_VOLTAGE(AdcHandle.Instance->DR, LL_ADC_RESOLUTION_12B);
	
	timeout = ADC_TIMEOUT;
	AdcHandle.Instance->CR |= ADC_CR_ADSTART;
	while(!__HAL_ADC_GET_FLAG(&AdcHandle, ADC_FLAG_EOC) && --timeout);
	if (!timeout)
	  	return -1;
	AdcHandle.Instance->ISR = 0xFFFF;	
	*temp = __LL_ADC_CALC_TEMPERATURE(*voltage, AdcHandle.Instance->DR, LL_ADC_RESOLUTION_12B);
	
	return 0;
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	HAL_PWR_EnableBkUpAccess();

	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;//16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;//RCC_PLL_ON;//RCC_PLL_NONE;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
	RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;//RCC_SYSCLKSOURCE_PLLCLK;//RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_LPTIM1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.LptimClockSelection = RCC_LPTIM1CLKSOURCE_LSE;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void MX_GPIO_Init(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE(); 
}

void _Error_Handler(char *file, int line)
{
	while (1) {
	}
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{ 

}

#endif