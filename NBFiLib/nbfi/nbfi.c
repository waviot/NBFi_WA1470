#include "nbfi.h"

void (* __nbfi_before_tx)() = 0;
void (* __nbfi_before_rx)() = 0;
void (* __nbfi_before_off)() = 0;
void (* __nbfi_lock_unlock_loop_irq)(uint8_t);
void (*__nbfi_send_status_handler)(nbfi_ul_sent_status_t) = 0;
void (*__nbfi_rx_handler)(uint8_t*, uint16_t) = 0;
void (* __nbfi_read_default_settings)(nbfi_settings_t*) = 0;
void (* __nbfi_read_flash_settings)(nbfi_settings_t*)  = 0;
void (* __nbfi_write_flash_settings)(nbfi_settings_t*) = 0;
uint32_t (* __nbfi_measure_voltage_or_temperature)(uint8_t) = 0;
uint32_t (* __nbfi_update_rtc)(void) = 0;
void (* __nbfi_rtc_synchronized)(uint32_t) = 0;
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
        case NBFI_LOCK_UNLOCK_LOOP_IRQ:
                __nbfi_lock_unlock_loop_irq = (void(*)(uint8_t))fn;
                break;
        case NBFI_SEND_COMPLETE:
		__nbfi_send_status_handler = (void(*)(nbfi_ul_sent_status_t))fn;
		break;             
	case NBFI_RECEIVE_COMLETE:
		__nbfi_rx_handler = (void(*)(uint8_t*, uint16_t))fn;
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

uint8_t NBFi_Get_Received_Packet(uint8_t * payload)
{
  __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
  uint8_t length = NBFi_Next_Ready_DL(payload);
  __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
  return length;
}

//call this function in main loop 
void  NBFI_Main_Level_Loop()
{

    if(__nbfi_rx_handler)
    {
      uint8_t payload[256];
      while(1)
      {
        uint8_t length = NBFi_Get_Received_Packet(payload);
        if(length) __nbfi_rx_handler(payload, length);
        else break;
      }
      
    }
    
    if(__nbfi_send_status_handler)
    {
      nbfi_ul_sent_status_t* ul;
      while(1)
      {
        __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
        ul = NBFi_Get_Next_Unreported_UL(DELIVERED);
        if(ul == 0) ul = NBFi_Get_Next_Unreported_UL(LOST);
        __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
        if(ul) __nbfi_send_status_handler(*ul);
        else return;
      }
    }
}


//call this function at least 1 time per 1ms

//#define AX_BPSK_PIN_GPIO_Port 	        GPIOB
//#define AX_BPSK_PIN_Pin 		GPIO_PIN_12

void   NBFI_Interrupt_Level_Loop()
{
  //HAL_GPIO_WritePin(AX_BPSK_PIN_GPIO_Port, AX_BPSK_PIN_Pin, GPIO_PIN_SET);
  scheduler_run_callbacks();
  //HAL_GPIO_WritePin(AX_BPSK_PIN_GPIO_Port, AX_BPSK_PIN_Pin, GPIO_PIN_RESET);    
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
  __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
  uint8_t can = (!rf_busy) && (rf_state == STATE_OFF) && (NBFi_Packets_To_Send() == 0);
  __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
  return can;
}

nbfi_state_t* NBFi_get_state()
{
    return &nbfi_state;
}

