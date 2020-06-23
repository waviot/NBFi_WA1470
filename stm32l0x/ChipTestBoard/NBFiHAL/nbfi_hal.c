#include <stm32l0xx_hal.h>
#include "stm32l0xx_ll_adc.h"
#include "stm32l0xx_hal_adc.h"
#include "nbfi_hal.h"
#include "log.h"
#include "scheduler_hal.h"

void nbfi_HAL_before_tx(nbfi_settings_t* nbfi)
{

}

void nbfi_HAL_before_rx(nbfi_settings_t* nbfi)
{

}

void nbfi_HAL_before_off(nbfi_settings_t* nbfi)
{

  
}

void nbfi_HAL_lock_unlock_loop_irq(uint8_t lock)
{
  scheduler_HAL_lock_unlock(lock);
}

const nbfi_settings_t* _default_settings;

void nbfi_HAL_read_default_settings(nbfi_settings_t* settings)
{
  for(uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
  {
    ((uint8_t *)settings)[i] = ((uint8_t *)_default_settings)[i];
  }
}


#define EEPROM_INT_nbfi_data (DATA_EEPROM_BASE + 1024*5)

void nbfi_HAL_read_flash_settings(nbfi_settings_t* settings) 
{
	memcpy((void*)settings, ((const void*)EEPROM_INT_nbfi_data), sizeof(nbfi_settings_t));
}

void nbfi_HAL_write_flash_settings(nbfi_settings_t* settings)
{	
    if(HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) return;
    for(uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
    {
	if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_INT_nbfi_data + i, ((uint8_t *)settings)[i]) != HAL_OK) break;
    }
    HAL_FLASHEx_DATAEEPROM_Lock(); 
}


static ADC_HandleTypeDef 		AdcHandle;
static ADC_ChannelConfTypeDef 	        sConfig;

void nbfi_HAL_ADC_init(void){
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

void nbfi_HAL_ADC_deinit(void){
	HAL_ADC_DeInit(&AdcHandle);
	ADC->CCR = 0;
}

#define ADC_TIMEOUT		100

int nbfi_HAL_ADC_get(uint32_t * voltage, uint32_t * temp){
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


uint32_t nbfi_HAL_measure_valtage_or_temperature(uint8_t val)
{
	uint32_t voltage, temp;
	nbfi_HAL_ADC_get(&voltage, &temp);
	return val ? voltage / 10 : temp;
}

uint32_t nbfi_HAL_update_rtc()
{
  //you should use this callback when external RTC used
  //return rtc_counter;  
  return 0;
}

void nbfi_HAL_rtc_synchronized(uint32_t time)
{
  //you should use this callback for RTC counter correction when external RTC used
  //rtc_counter = time;
  
}

__weak void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{

  /* NOTE : This function Should not be modified, when the callback is needed,
            the nbfi_receive_complete could be implemented in the user file
   */
}

__weak void nbfi_receive_complete(uint8_t * data, uint16_t length)
{

  /* NOTE : This function Should not be modified, when the callback is needed,
            the nbfi_receive_complete could be implemented in the user file
   */
}

void nbfi_HAL_reset()
{
  NVIC_SystemReset();
}

void nbfi_HAL_get_iterator(nbfi_crypto_iterator_t * iter)
{
	//	Read iterator from retain storage
	iter->ul = iter->dl = 0;
        
        //iter->dl = 260;
}

void nbfi_HAL_set_iterator(nbfi_crypto_iterator_t * iter)
{
	//	Write iterator to retain storage
	//	Cause every send/receive packet
}

nbfi_HAL_st nbfi_hal_struct = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


void nbfi_HAL_init(const nbfi_settings_t* settings, nbfi_dev_info_t* info)
{
  
  _default_settings = settings;
  
  nbfi_HAL_ADC_init();
  
  nbfi_hal_struct.__nbfi_before_tx = &nbfi_HAL_before_tx;
  nbfi_hal_struct.__nbfi_before_rx = &nbfi_HAL_before_rx;
  nbfi_hal_struct.__nbfi_before_off = &nbfi_HAL_before_off;
  nbfi_hal_struct.__nbfi_lock_unlock_loop_irq = &nbfi_HAL_lock_unlock_loop_irq;
  nbfi_hal_struct.__nbfi_send_status_handler = &nbfi_send_complete;
  nbfi_hal_struct.__nbfi_rx_handler = &nbfi_receive_complete;
  nbfi_hal_struct.__nbfi_read_default_settings = &nbfi_HAL_read_default_settings;
  nbfi_hal_struct.__nbfi_read_flash_settings = &nbfi_HAL_read_flash_settings;
  nbfi_hal_struct.__nbfi_write_flash_settings = &nbfi_HAL_write_flash_settings;
  nbfi_hal_struct.__nbfi_measure_voltage_or_temperature = &nbfi_HAL_measure_valtage_or_temperature;
  nbfi_hal_struct.__nbfi_reset = &nbfi_HAL_reset;
  nbfi_hal_struct.__nbfi_get_iterator = &nbfi_HAL_get_iterator;
  nbfi_hal_struct.__nbfi_set_iterator = &nbfi_HAL_set_iterator; 
  nbfi_hal_struct.__nbfi_log_send_str = &log_send_str;
  
  //register this callbacks when external RTC used
  //nbfi_hal_struct.__nbfi_update_rtc = &nbfi_HAL_update_rtc;
  //nbfi_hal_struct.__nbfi_rtc_synchronized = &nbfi_HAL_rtc_synchronized;  
         
  NBFI_Init(&nbfi_hal_struct, _scheduler, info);
}
