#include "nbfi.h"
#include <string.h>
#include <stdlib.h>



nbfi_HAL_st* nbfi_hal = 0;
ischeduler_st* nbfi_scheduler = 0;

_Bool switched_to_custom_settings = 0;

#ifdef NBFI_LOG
char nbfi_log_string[256];
#endif


void NBFI_Init(nbfi_HAL_st* ptr, ischeduler_st* scheduler, nbfi_dev_info_t* info)
{
    nbfi_hal = ptr;
    nbfi_scheduler = scheduler;
    if((nbfi_hal == 0) || (nbfi_scheduler == 0)) while(1); //HAL and scheduler pointers must be provided 
    NBFi_set_Device_Info(info); 
    NBFi_Config_Set_Default();	
    NBFI_Transport_Init();
}

uint8_t NBFi_get_Received_Packet(uint8_t * payload)
{
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
  uint8_t length = NBFi_Next_Ready_DL(payload);
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
  return length;
}

//call this function in main loop 
void  NBFI_Main_Level_Loop()
{

    if(nbfi_hal->__nbfi_rx_handler)
    {
      uint8_t payload[256];
      while(1)
      {
        uint8_t length = NBFi_get_Received_Packet(payload);
        if(length) nbfi_hal->__nbfi_rx_handler(payload, length);
        else break;
      }
      
    }
    
    if(nbfi_hal->__nbfi_send_status_handler)
    {
      nbfi_ul_sent_status_t* ul;
      while(1)
      {
        nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
        ul = NBFi_Get_Next_Unreported_UL(DELIVERED);
        if(ul == 0) ul = NBFi_Get_Next_Unreported_UL(LOST);
        nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
        if(ul) nbfi_hal->__nbfi_send_status_handler(*ul);
        else return;
      }
    }
    
   if(rtc_synchronised && nbfi_hal->__nbfi_rtc_synchronized) 
   {
     rtc_synchronised = 0;
     nbfi_hal->__nbfi_rtc_synchronized(nbfi_rtc);
   }
   
     if(nbfi_settings_need_to_save_to_flash && (nbfi_hal->__nbfi_write_flash_settings != 0)) 
     {
        nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
        nbfi_hal->__nbfi_write_flash_settings(&nbfi);
        nbfi_settings_need_to_save_to_flash = 0;
        nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
     }
   
}


nbfi_status_t NBFi_go_to_Sleep(_Bool sleep)
{
    static _Bool old_state = 1;
    nbfi_status_t res = OK;
    if(sleep)
    {
        nbfi.mode = OFF;
        NBFi_Clear_TX_Buffer();
        res = NBFi_RF_Deinit();
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
    return res;
}

uint8_t NBFi_can_sleep()
{
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
  uint8_t can = (!rf_busy) && (rf_state == STATE_OFF) && (NBFi_Packets_To_Send() == 0) && NBFi_RF_can_Sleep();
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
  return can;
}

void NBFi_get_state(nbfi_state_t * state)
{
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    memcpy(state, &nbfi_state , sizeof(nbfi_state_t));
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

uint32_t NBFi_get_RTC()
{
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);    
    NBFi_update_RTC();
    uint32_t rtc = nbfi_rtc;
  nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);    
    return rtc;
}

void NBFi_set_RTC(uint32_t time)
{
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);    
    NBFi_set_RTC_irq(time);
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);    
}

void NBFi_set_Device_Info(nbfi_dev_info_t *info)
{
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);  
    dev_info = *info;
    if(info->key != 0)
    {
      NBFi_MAC_Get_Iterator();
      NBFi_Crypto_Set_KEY(dev_info.key, nbfi_iter.ul, nbfi_iter.dl);
    }
    info_timer = dev_info.send_info_interval - 300 - rand()%600;
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK); 
}

void NBFi_get_Settings(nbfi_settings_t* settings)
{
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    memcpy(settings, &nbfi , sizeof(nbfi_settings_t));
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

void NBFi_set_Settings(nbfi_settings_t* settings)
{
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    memcpy(&nbfi, settings , sizeof(nbfi_settings_t));
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

_Bool NBFi_send_Packet_to_Config_Parser(uint8_t* buf)
{
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
    _Bool has_request = NBFi_Config_Parser(buf);
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
    return has_request;
}

void NBFi_clear_Saved_Configuration()
{
	if(nbfi_hal->__nbfi_write_flash_settings == 0) 
		return;
	nbfi_settings_t empty;
	empty.tx_phy_channel = DL_PSK_200;
	nbfi_hal->__nbfi_write_flash_settings(&empty);
}
        
void    NBFi_switch_to_another_settings(nbfi_settings_t* settings, _Bool to_or_from)
{
    static nbfi_settings_t old_settings;
    static nbfi_state_t state; 
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_LOCK);
   
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
    
    nbfi_hal->__nbfi_lock_unlock_loop_irq(NBFI_UNLOCK);
}

_Bool   NBFi_is_Switched_to_Custom_Settings()
{
  return switched_to_custom_settings;
}