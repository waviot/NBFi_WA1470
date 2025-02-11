#ifdef WA1471
#ifndef _wa1471_H
#define _wa1471_H
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "wa1471rfe.h"
#include "wa1471dem.h"
#include "wa1471mod.h"
#include "wa1471_defines.h"
#include "ischeduler.h"

#define WA1471_SEND_BY_I_Q_MODULATOR	        0
#define WA1471_SEND_BY_BPSK_PIN			        1


#ifdef WA1471_LOG
extern char wa1471_log_string[];
#endif

typedef struct
{
  void (*__wa1471_enable_pin_irq)(void);
  void (*__wa1471_disable_pin_irq)(void);
  void (*__wa1471_chip_enable)(void);
  void (*__wa1471_chip_disable)(void);
  uint8_t (*__wa1471_get_irq_pin_state)(void);
  void (*__spi_rx)(uint8_t *, uint16_t);
  void (*__spi_tx)(uint8_t *, uint16_t);
  void (*__spi_cs_set)(uint8_t);
  void (*__wa1471_data_received)(uint8_t *, uint8_t *);
  void (*__wa1471_tx_finished)(void);
  void (*__wa1471_nop_dalay_ms)(uint32_t);
  void (*__wa1471_send_to_bpsk_pin)(uint8_t *, uint16_t, uint16_t);
  void (*__wa1471_log_send_str)(const char *str);
}wa1471_HAL_st;


extern wa1471_HAL_st *wa1471_hal;
extern ischeduler_st* wa1471_scheduler;


void wa1471_init(_Bool send_by_bpsk_pin, uint32_t modem_id, wa1471_HAL_st*, ischeduler_st*);
void wa1471_reinit(uint32_t preambule);
void wa1471_deinit();
void wa1471_spi_write(uint16_t address, uint8_t *data, uint8_t length);
void wa1471_spi_read(uint16_t address, uint8_t *data, uint8_t length);
void wa1471_spi_write8(uint16_t address, uint8_t data);
uint8_t wa1471_spi_read8(uint16_t address);
_Bool wa1471_spi_wait_for(uint16_t address, uint8_t value, uint8_t mask);
void wa1471_isr();
void wa1471_bpsk_pin_tx_finished();
_Bool wa1471_cansleep();
void wa1471_test();
extern _Bool send_by_dbpsk;
#endif
#endif //#ifdef WA1471