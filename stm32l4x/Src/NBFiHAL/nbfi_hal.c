#include "adc.h"
#include "nbfi_hal.h"
#include "rtc.h"
#include "scheduler_hal.h"
#include "WVT_EEPROM.h"

#ifdef WA1470_LOG
//#include "log.h"
#endif //WA1470_LOG

uint32_t Voltage = 0;
int32_t Temp = 0;

static inline void nbfi_HAL_GPIO_Init()
{
}

static inline void nbfi_HAL_before_tx(nbfi_settings_t *nbfi)
{
  //  if (nbfi->tx_antenna == SMA)
  //    HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin, GPIO_PIN_SET);
  //  else
  //    HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin, GPIO_PIN_RESET);
}

static inline void nbfi_HAL_before_rx(nbfi_settings_t *nbfi)
{
  //  if (nbfi->rx_antenna == SMA)
  //    HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin, GPIO_PIN_SET);
  //  else
  //    HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin, GPIO_PIN_RESET);
}

static inline void nbfi_HAL_before_off(nbfi_settings_t *nbfi)
{
  //  HAL_GPIO_WritePin(WA_EXTANT_GPIO_Port, WA_EXTANT_Pin, GPIO_PIN_RESET);
}

static inline void nbfi_HAL_lock_unlock_loop_irq(uint8_t lock)
{
  scheduler_HAL_lock_unlock(lock);
}

static const nbfi_settings_t *_default_settings;

static inline void nbfi_HAL_read_default_settings(nbfi_settings_t *settings)
{
  for (uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
  {
    ((uint8_t *)settings)[i] = ((uint8_t *)_default_settings)[i];
  }
}

static inline void nbfi_HAL_read_flash_settings(nbfi_settings_t *settings)
{
  /// \todo check this
  WVT_EEPROM_LoadAll((void *)settings, EEPROM_NBFI_SETTING);
}

static inline void nbfi_HAL_write_flash_settings(nbfi_settings_t *settings)
{
  WVT_EEPROM_SaveAll((void *)settings, EEPROM_NBFI_SETTING);
}

static inline uint32_t nbfi_HAL_measure_valtage_or_temperature(uint8_t val)
{
    /// \todo fix this
    ADC_GetVoltageAndTemp(&Voltage, &Temp);
    // voltage = 3300;
    // temp = 27;
    if (val == 1)
    {
        return Voltage / 10;
    }
    else
    {
        return Temp;
    }
}

#ifdef USE_EXTERNAL_RTC
static inline uint32_t nbfi_HAL_update_rtc()
{
  //you should use this callback when external RTC used
  //return rtc_counter;
  return RTC_GetSeconds();
}

static inline void nbfi_HAL_rtc_synchronized(uint32_t time)
{
  //you should use this callback for RTC counter correction when external RTC used
  //rtc_counter = time;
  RTC_SetSeconds(time);
}
#endif //USE_EXTERNAL_RTC
__weak void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{

  /* NOTE : This function Should not be modified, when the callback is needed,
            the nbfi_receive_complete could be implemented in the user file
   */
}

__weak void nbfi_receive_complete(uint8_t *data, uint16_t length)
{

  /* NOTE : This function Should not be modified, when the callback is needed,
            the nbfi_receive_complete could be implemented in the user file
   */
}

static inline void nbfi_HAL_reset()
{
  NVIC_SystemReset();
}

static inline void nbfi_HAL_get_iterator(nbfi_crypto_iterator_t *iter)
{
  //	Read iterator from retain storage
  iter->ul = iter->dl = 0;

  //iter->dl = 260;
}

static inline void nbfi_HAL_set_iterator(nbfi_crypto_iterator_t *iter)
{
  //	Write iterator to retain storage
  //	Cause every send/receive packet
}

static nbfi_HAL_st nbfi_hal_struct = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void log_send_str(const char *str)
{
  printf_s("%s", str);
  printf_s("\n");
}

void nbfi_HAL_init(const nbfi_settings_t *settings, nbfi_dev_info_t *info)
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
#ifdef NBFI_LOG
  nbfi_hal_struct.__nbfi_log_send_str = &log_send_str;
#endif //WA1470_LOG

#ifdef USE_EXTERNAL_RTC
  //register this callbacks when external RTC used
  nbfi_hal_struct.__nbfi_update_rtc = &nbfi_HAL_update_rtc;
  nbfi_hal_struct.__nbfi_rtc_synchronized = &nbfi_HAL_rtc_synchronized;
#endif //USE_EXTERNAL_RTC
  NBFI_Init(&nbfi_hal_struct, _scheduler, info);
}
