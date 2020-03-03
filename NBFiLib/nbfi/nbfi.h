#ifndef NBFI_H
#define NBFI_H

#include "nbfi_defines.h"
#include "wa1470.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "nbfi_types.h"
#include "nbfi_transport.h"
#include "nbfi_transport_misc.h"
#include "nbfi_crc.h"
#include "nbfi_rf.h"
#include "nbfi_config.h"
#include "nbfi_mac.h"
#include "nbfi_crypto.h"
#include "ischeduler.h"

typedef struct 
{
  void (* __nbfi_before_tx)();
  void (* __nbfi_before_rx)();
  void (* __nbfi_before_off)();
  void (* __nbfi_lock_unlock_loop_irq)(uint8_t);
  void (*__nbfi_send_status_handler)(nbfi_ul_sent_status_t);
  void (*__nbfi_rx_handler)(uint8_t*, uint16_t);
  void (* __nbfi_read_default_settings)(nbfi_settings_t*);
  void (* __nbfi_read_flash_settings)(nbfi_settings_t*);
  void (* __nbfi_write_flash_settings)(nbfi_settings_t*);
  uint32_t (* __nbfi_measure_voltage_or_temperature)(uint8_t);
  uint32_t (* __nbfi_update_rtc)(void);
  void (* __nbfi_rtc_synchronized)(uint32_t);
  void (* __nbfi_reset)(void);
  void (* __nbfi_get_iterator)(nbfi_crypto_iterator_t*);
  void (* __nbfi_set_iterator)(nbfi_crypto_iterator_t*);
  void (*__nbfi_log_send_str)(const char *str);
}nbfi_HAL_st;

extern nbfi_HAL_st* nbfi_hal;
extern ischeduler_st* nbfi_scheduler;


#ifdef NBFI_LOG
extern char nbfi_log_string[];
#endif

#define NBFI_LOCK       1
#define NBFI_UNLOCK     0

void                    NBFI_Init(nbfi_HAL_st *, ischeduler_st*, nbfi_dev_info_t* info);
void                    NBFI_Main_Level_Loop();
nbfi_status_t           NBFi_go_to_Sleep(_Bool sleep);
nbfi_ul_sent_status_t   NBFi_Send5(uint8_t* payload, uint8_t length);
nbfi_status_t           NBFi_Send(uint8_t* payload, uint8_t length);
nbfi_ul_status_t        NBFi_get_UL_status(uint16_t id);
uint8_t                 NBFi_get_Received_Packet(uint8_t * payload);

uint8_t                 NBFi_Packets_To_Send();
void                    NBFi_get_state(nbfi_state_t * state);
uint8_t                 NBFi_can_sleep();
uint32_t                NBFi_get_RTC();
void                    NBFi_set_RTC(uint32_t time);

void                    NBFi_set_Device_Info(nbfi_dev_info_t *);
void                    NBFi_get_Settings(nbfi_settings_t*);
void                    NBFi_set_Settings(nbfi_settings_t*);

_Bool                   NBFi_send_Packet_to_Config_Parser(uint8_t* buf);

void                    NBFi_clear_Saved_Configuration();
void                    NBFi_switch_to_another_settings(nbfi_settings_t* settings, _Bool to_or_from);
_Bool                   NBFi_is_Switched_to_Custom_Settings();

#endif // NBFI_H
