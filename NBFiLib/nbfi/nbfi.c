#include "nbfi.h"
#include <string.h>
#include <stdlib.h>

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

_Bool switched_to_custom_settings = 0;

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

uint8_t NBFi_get_Received_Packet(uint8_t * payload)
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
        uint8_t length = NBFi_get_Received_Packet(payload);
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
    
   if(rtc_synchronised && __nbfi_rtc_synchronized) 
   {
     rtc_synchronised = 0;
     __nbfi_rtc_synchronized(nbfi_rtc);
   }
   
     if(nbfi_settings_need_to_save_to_flash && (__nbfi_write_flash_settings != 0)) 
     {
        __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
        __nbfi_write_flash_settings(&nbfi);
        nbfi_settings_need_to_save_to_flash = 0;
        __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
     }
   
}

/*
void   NBFI_Interrupt_Level_Loop()
{

  scheduler_run_callbacks();
 
}*/

void NBFi_go_to_Sleep(_Bool sleep)
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

void NBFi_get_state(nbfi_state_t * state)
{
  __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    memcpy(state, &nbfi_state , sizeof(nbfi_state_t));
  __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

uint32_t NBFi_get_RTC()
{
  __nbfi_lock_unlock_loop_irq(NBFI_LOCK);    
    NBFi_update_RTC();
    uint32_t rtc = nbfi_rtc;
  __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);    
    return rtc;
}

void NBFi_set_RTC(uint32_t time)
{
    __nbfi_lock_unlock_loop_irq(NBFI_LOCK);    
      NBFi_set_RTC_irq(time);
    __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);    
}

void NBFi_set_Device_Info(nbfi_dev_info_t *info)
{
    __nbfi_lock_unlock_loop_irq(NBFI_LOCK);  
    dev_info = *info;
    if(info->key != 0)
    {
      NBFi_MAC_Get_Iterator();
      NBFi_Crypto_Set_KEY(dev_info.key, nbfi_iter.ul, nbfi_iter.dl);
    }
    info_timer = dev_info.send_info_interval - 300 - rand()%600;
    __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK); 
}

void NBFi_get_Settings(nbfi_settings_t* settings)
{
    __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    memcpy(settings, &nbfi , sizeof(nbfi_settings_t));
  __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

void NBFi_set_Settings(nbfi_settings_t* settings)
{
    __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    memcpy(&nbfi, settings , sizeof(nbfi_settings_t));
  __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

_Bool NBFi_send_Packet_to_Config_Parser(uint8_t* buf)
{
    __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    _Bool has_request = NBFi_Config_Parser(buf);
    __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
    return has_request;
}

void NBFi_clear_Saved_Configuration()
{
	if(__nbfi_write_flash_settings == 0) 
		return;
	nbfi_settings_t empty;
	empty.tx_phy_channel = DL_PSK_200;
	__nbfi_write_flash_settings(&empty);
}
        
void    NBFi_switch_to_another_settings(nbfi_settings_t* settings, _Bool to_or_from)
{
    static nbfi_settings_t old_settings;
    static nbfi_state_t state; 
    __nbfi_lock_unlock_loop_irq(NBFI_LOCK);
   
    if(to_or_from)
    {
      memcpy(&old_settings, &nbfi, sizeof(nbfi_settings_t));
      memcpy(&state, &nbfi_state, sizeof(nbfi_state_t));
      
      NBFi_Clear_TX_Buffer();
           
      memcpy(&nbfi, settings, sizeof(nbfi_settings_t));

    }
    else
    {
        NBFi_Clear_TX_Buffer();
        memcpy(&nbfi_state, &state, sizeof(nbfi_state_t));
        memcpy(&nbfi, &old_settings, sizeof(nbfi_settings_t));
        NBFi_Config_Set_TX_Chan(old_settings.tx_phy_channel);
        NBFi_Config_Set_RX_Chan(old_settings.rx_phy_channel);
        rf_state = STATE_CHANGED;
    }

    if(rf_state == STATE_RX) NBFi_MAC_RX();
    
    switched_to_custom_settings = to_or_from;
    
    __nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

_Bool   NBFi_is_Switched_to_Custom_Settings()
{
  return switched_to_custom_settings;
}