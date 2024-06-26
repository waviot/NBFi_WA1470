#include "NuMicro.h"
#include "nbfi_hal.h"
//#include "log.h"
#include "scheduler_hal.h"
//#include "adc.h"

//#define WA_EXTANT_GPIO_Port 	GPIOA
//#define WA_EXTANT_Pin 		GPIO_PIN_11

static void nbfi_HAL_GPIO_Init()
{
  /*GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = WA_EXTANT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WA_EXTANT_GPIO_Port, &GPIO_InitStruct);*/

}


void nbfi_HAL_before_tx(nbfi_settings_t* nbfi)
{

  //if(nbfi->tx_antenna == SMA) HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin,  GPIO_PIN_SET);
  //else  HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin,  GPIO_PIN_RESET);

}

void nbfi_HAL_before_rx(nbfi_settings_t* nbfi)
{
  //if(nbfi->rx_antenna == SMA) HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin,  GPIO_PIN_SET);
  //else  HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin,  GPIO_PIN_RESET);
}

void nbfi_HAL_before_off(nbfi_settings_t* nbfi)
{
  //HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin,  GPIO_PIN_RESET);
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
	//memcpy((void*)settings, ((const void*)EEPROM_INT_nbfi_data), sizeof(nbfi_settings_t));
}

void nbfi_HAL_write_flash_settings(nbfi_settings_t* settings)
{
   /* if(HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) return;
    for(uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
    {
	if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_INT_nbfi_data + i, ((uint8_t *)settings)[i]) != HAL_OK) break;
    }
    HAL_FLASHEx_DATAEEPROM_Lock(); */
}



uint32_t nbfi_HAL_measure_valtage_or_temperature(uint8_t val)
{
	uint32_t voltage, temp, ch8;
	//ADC_get(&voltage, &temp, &ch8);
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

  nbfi_HAL_GPIO_Init();

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
  nbfi_hal_struct.__nbfi_log_send_str = 0;//&log_send_str;


  //register this callbacks when external RTC used
  //nbfi_hal_struct.__nbfi_update_rtc = &nbfi_HAL_update_rtc;
  //nbfi_hal_struct.__nbfi_rtc_synchronized = &nbfi_HAL_rtc_synchronized;

  NBFI_Init(&nbfi_hal_struct, _scheduler, info);
}
