#include "nbfi.h"

void (* __nbfi_before_tx)() = 0;
void (* __nbfi_before_rx)() = 0;
void (* __nbfi_before_off)() = 0;
void (* __nbfi_read_default_settings)(nbfi_settings_t*) = 0;
void (* __nbfi_read_flash_settings)(nbfi_settings_t*)  = 0;
void (* __nbfi_write_flash_settings)(nbfi_settings_t*) = 0;
uint32_t (* __nbfi_measure_voltage_or_temperature)(uint8_t) = 0;
uint32_t (* __nbfi_update_rtc)(void) = 0;
void (* __nbfi_rtc_synchronized)(uint32_t) = 0;
void (* __nbfi_lock_unlock_nbfi_irq)(uint8_t) = 0;
void (* __nbfi_reset)(void) = 0;
void (* __nbfi_get_iterator)(nbfi_crypto_iterator_t*) = 0;
void (* __nbfi_set_iterator)(nbfi_crypto_iterator_t*) = 0;


void NBFI_reg_func(uint8_t name, void* fn)
{
	switch(name)
	{
	case NBFI_BEFORE_TX:
		__nbfi_before_tx = (void(*)(void))fn;
		break;
	case NBFI_BEFORE_RX:
		__nbfi_before_rx = (void(*)(void))fn;
		break;
        case NBFI_BEFORE_OFF:
		__nbfi_before_off = (void(*)(void))fn;
		break;
	case NBFI_RECEIVE_COMLETE:
		rx_handler = (rx_handler_t)fn;
		break;
	case NBFI_READ_DEFAULT_SETTINGS:
		__nbfi_read_default_settings = (void(*)(nbfi_settings_t*))fn;
		break;
	case NBFI_READ_FLASH_SETTINGS:
		__nbfi_read_flash_settings = (void(*)(nbfi_settings_t*))fn;
		break;
	case NBFI_WRITE_FLASH_SETTINGS:
		__nbfi_write_flash_settings = (void(*)(nbfi_settings_t*))fn;
		break;
	case NBFI_MEASURE_VOLTAGE_OR_TEMPERATURE:
		__nbfi_measure_voltage_or_temperature = (uint32_t(*)(uint8_t))fn;
		break;
	case NBFI_UPDATE_RTC:
		__nbfi_update_rtc = (uint32_t(*)(void))fn;
		break;
	case NBFI_RTC_SYNCHRONIZED:
		__nbfi_rtc_synchronized = (void(*)(uint32_t))fn;
		break;
	case NBFI_LOCKUNLOCKNBFIIRQ:
		__nbfi_lock_unlock_nbfi_irq = (void(*)(uint8_t))fn;
		break; 
	case NBFI_RESET:
		__nbfi_reset = (void(*)(void))fn;
		break;
	case NBFI_GET_ITERATOR:
		__nbfi_get_iterator = (void(*)(nbfi_crypto_iterator_t*))fn;
		break;
	case NBFI_SET_ITERATOR:
		__nbfi_set_iterator = (void(*)(nbfi_crypto_iterator_t*))fn;
		break;
	default:
		break;
	}
}

void NBFI_Init()
{
    NBFi_Config_Set_Default();	
    NBFI_Transport_Init();
}

void NBFi_Go_To_Sleep(_Bool sleep)
{
    static _Bool old_state = 1;
    if(sleep)
    {
        nbfi.mode = OFF;
        NBFi_Clear_TX_Buffer();
        NBFi_RF_Deinit();
    }
    else
    {
        if(old_state != sleep)
        {
            nbfi_settings_t settings;
            NBFi_ReadConfig(&settings);
            nbfi.mode = settings.mode;
            NBFi_Config_Send_Sync(0);
            NBFi_Send_Clear_Cmd(0);
            NBFi_Force_process();
        }
    }
    old_state = sleep;
}

uint8_t NBFi_can_sleep()
{
  return (!rf_busy) && (rf_state == STATE_OFF) && (NBFi_Packets_To_Send() == 0);
}

nbfi_state_t* NBFi_get_state()
{
    return &nbfi_state;
}

