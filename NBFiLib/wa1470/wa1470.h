#ifndef _wa1470_H
#define _wa1470_H
#include <stm32l0xx_hal.h>
#include "wa1470rfe.h"
#include "wa1470dem.h"
#include "wa1470mod.h"
#include "nbfi_defines.h"

#define WA1470_SEND_BY_I_Q_MODULATOR	        0
#define WA1470_SEND_BY_BPSK_PIN			1

typedef struct 
{
  void (*__wa1470_enable_pin_irq)(void);
  void (*__wa1470_disable_pin_irq)(void);
  void (*__wa1470_chip_enable)(void);
  void (*__wa1470_chip_disable)(void);
  uint8_t (*__wa1470_get_irq_pin_state)(void);
  void (*__spi_rx)(uint8_t *, uint16_t);
  void (*__spi_tx)(uint8_t *, uint16_t);
  void (*__spi_cs_set)(uint8_t);
  void (*__wa1470_data_received)(uint8_t *, uint8_t *);
  void (*__wa1470_tx_finished)(void);
  void (*__wa1470_nop_dalay_ms)(uint32_t);
  void (*__wa1470_send_to_bpsk_pin)(uint8_t *, uint16_t, uint16_t);
}wa1470_HAL_st;


extern wa1470_HAL_st *wa1470_hal;

void wa1470_set_HAL(wa1470_HAL_st *);
void wa1470_init(_Bool send_by_bpsk_pin, uint32_t modem_id);
void wa1470_reinit();
void wa1470_spi_write(uint16_t address, uint8_t *data, uint8_t length);
void wa1470_spi_read(uint16_t address, uint8_t *data, uint8_t length);
void wa1470_spi_write8(uint16_t address, uint8_t data);
uint8_t wa1470_spi_read8(uint16_t address);
_Bool wa1470_spi_wait_for(uint16_t address, uint8_t value, uint8_t mask);
void wa1470_isr();
void wa1470_bpsk_pin_tx_finished();
_Bool wa1470_cansleep();
void wa1470_test();
extern _Bool send_by_dbpsk;
#endif