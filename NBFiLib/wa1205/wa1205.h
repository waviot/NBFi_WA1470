#ifndef _WA1205_H
#define _WA1205_H

#include "wa1205dem.h"
#include "wa1205mod.h"

enum wa1205_func_name_t
{
	WARADIO_ENABLE_GLOBAL_IRQ = 0,
	WARADIO_DISABLE_GLOBAL_IRQ,
	WARADIO_ENABLE_IRQ_PIN,
	WARADIO_DISABLE_IRQ_PIN,
	WARADIO_GET_IRQ_PIN,
	WARADIO_SPI_RX,
	WARADIO_SPI_TX,
	WARADIO_SPI_TX_RX,
	WARADIO_SPI_CS_WRITE,
        WARADIO_DATA_RECEIVED,
        WARADIO_TX_FINISHED
};

void wa1205_reg_func(uint8_t name, void * fn);
void wa1205_isr();
_Bool wa1205_cansleep();
void wa1205_tcxo_set_reset(uint8_t set);
void wa1205_set_freq(uint32_t freq);
#endif