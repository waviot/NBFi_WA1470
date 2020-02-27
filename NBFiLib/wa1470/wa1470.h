#ifndef _wa1470_H
#define _wa1470_H
#include <stm32l0xx_hal.h>
#include "wa1470rfe.h"
#include "wa1470dem.h"
#include "wa1470mod.h"
#include "nbfi_defines.h"

#define WA1470_SEND_BY_I_Q_MODULATOR	0
#define WA1470_SEND_BY_BPSK_PIN			1

enum wa1470_func_name_t
{
	WARADIO_ENABLE_IRQ_PIN,
	WARADIO_DISABLE_IRQ_PIN,
	WARADIO_CHIP_ENABLE,
	WARADIO_CHIP_DISABLE,
	WARADIO_GET_IRQ_PIN,
	WARADIO_SPI_RX,
	WARADIO_SPI_TX,
	WARADIO_SPI_TX_RX,
	WARADIO_SPI_CS_WRITE,
	WARADIO_DATA_RECEIVED,
	WARADIO_TX_FINISHED,
	WARADIO_NOP_DELAY_MS,
	WARADIO_SEND_TO_BPSK_PIN
};

void wa1470_reg_func(uint8_t name, void * fn);
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