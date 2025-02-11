#ifdef WA1471

#include "wa1471.h"

#define SPI_WAIT_TIMEOUT		1000 //100000

wa1471_HAL_st *wa1471_hal = 0;
ischeduler_st* wa1471_scheduler = 0;


_Bool send_by_dbpsk;

#ifdef WA1471_LOG
#warning WA1471_LOG
char wa1471_log_string[256];
#endif

void wa1471_spi_write(uint16_t address, uint8_t *data, uint8_t length)
{
        wa1471_hal->__wa1471_disable_pin_irq();
        wa1471_hal->__spi_cs_set(0);
	address |= 0x8000;
	wa1471_hal->__spi_tx(((uint8_t*)(&address)) + 1, 1);
	wa1471_hal->__spi_tx(((uint8_t*)(&address)), 1);
	wa1471_hal->__spi_tx(data, length);
	wa1471_hal->__spi_cs_set(1);
        wa1471_hal->__wa1471_enable_pin_irq();
}

void wa1471_spi_read(uint16_t address, uint8_t *data, uint8_t length)
{
        wa1471_hal->__wa1471_disable_pin_irq();
	wa1471_hal->__spi_cs_set(0);
	address &= 0x7fff;
	wa1471_hal->__spi_tx(((uint8_t*)(&address)) + 1, 1);
	wa1471_hal->__spi_tx(((uint8_t*)(&address)), 1);
	wa1471_hal->__spi_rx(data, length);
	wa1471_hal->__spi_cs_set(1);
	wa1471_hal->__wa1471_enable_pin_irq();
}


void wa1471_spi_write8(uint16_t address, uint8_t data)
{
	wa1471_spi_write(address, &data, 1);
}

uint8_t wa1471_spi_read8(uint16_t address)
{
	uint8_t data;
	wa1471_spi_read(address, &data, 1);
	return data;
}



_Bool wa1471_spi_wait_for(uint16_t address, uint8_t value, uint8_t mask)
{
	uint32_t timeout = 0;
        while((wa1471_spi_read8(address) & mask) != value)
	{
		if(++timeout >= SPI_WAIT_TIMEOUT) 
                {
                  return 0;
                }
	}
	return 1;
}


void wa1471_init(_Bool send_by_bpsk_pin, uint32_t preambule, wa1471_HAL_st* hal_ptr, ischeduler_st* scheduler)
{
        wa1471_hal = hal_ptr;
        wa1471_scheduler = scheduler;
        if((wa1471_hal == 0) || (wa1471_scheduler == 0)) while(1); //HAL and scheduler pointers must be provided
        send_by_dbpsk = send_by_bpsk_pin;
	//wa1471rfe_init();
        wa1471dem_init(preambule);
	wa1471mod_init(send_by_dbpsk);

}

void wa1471_reinit(uint32_t preambule)
{
        wa1471rfe_deinit();
        wa1471_hal->__wa1471_nop_dalay_ms(2);
        wa1471rfe_init();
        wa1471dem_init(preambule);
  	    wa1471mod_init(send_by_dbpsk);
}

void wa1471_deinit()
{
        wa1471_hal->__wa1471_disable_pin_irq();
        wa1471dem_rx_enable(0);
        wa1471rfe_deinit();

}

void wa1471_isr()
{
	wa1471dem_isr();
	wa1471mod_isr();
}

_Bool wa1471_cansleep()
{
	return 1;
}

#endif //#ifdef WA1471