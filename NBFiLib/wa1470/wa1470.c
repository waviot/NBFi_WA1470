#include "wa1470.h"
#include "wa1470dem.h"
#include "wa1470mod.h"
#include "wa1470rfe.h"

#define SPI_WAIT_TIMEOUT    100000


void (*__wa1470_enable_global_irq)(void) = 0;
void (*__wa1470_disable_global_irq)(void) = 0;
void (*__wa1470_enable_pin_irq)(void) = 0;
void (*__wa1470_disable_pin_irq)(void) = 0;
void (*__wa1470_chip_enable)(void) = 0;
void (*__wa1470_chip_disable)(void) = 0;
uint8_t (*__wa1470_get_irq_pin_state)(void);
void (*__spi_rx)(uint8_t *, uint16_t);
void (*__spi_tx)(uint8_t *, uint16_t);
void (*__spi_tx_rx)(uint8_t *, uint8_t *, uint16_t);
void (*__spi_cs_set)(uint8_t);
void (*__wa1470_data_received)(dem_packet_st *pkt, dem_packet_info_st * info) = 0;
void (*__wa1470_tx_finished)(void) = 0;
void (*__wa1470_nop_dalay_ms)(uint32_t) = 0;


void wa1470_reg_func(uint8_t name, void*  fn)
{
	switch(name)
	{
	case WARADIO_ENABLE_GLOBAL_IRQ:
		__wa1470_enable_global_irq = (void(*)(void))fn;
		break;
	case WARADIO_DISABLE_GLOBAL_IRQ:
		__wa1470_disable_global_irq = (void(*)(void))fn;
		break;
	case WARADIO_ENABLE_IRQ_PIN:
		__wa1470_enable_pin_irq = (void(*)(void))fn;
		break;
	case WARADIO_DISABLE_IRQ_PIN:
		__wa1470_disable_pin_irq = (void(*)(void))fn;
		break;
        case WARADIO_CHIP_ENABLE:
		__wa1470_chip_enable = (void(*)(void))fn;
		break;
	case WARADIO_CHIP_DISABLE:
		__wa1470_chip_disable = (void(*)(void))fn;
		break;
	case WARADIO_GET_IRQ_PIN:
		__wa1470_get_irq_pin_state = (uint8_t(*)(void))fn;
		break;
	case WARADIO_SPI_RX:
		__spi_rx = (void(*)(uint8_t*,uint16_t))fn;
		break;
	case WARADIO_SPI_TX:
		__spi_tx = (void(*)(uint8_t*,uint16_t))fn;
		break;
	case WARADIO_SPI_TX_RX:
		__spi_tx_rx = (void(*)(uint8_t*,uint8_t*,uint16_t))fn;
		break;
	case WARADIO_SPI_CS_WRITE:
		__spi_cs_set = (void(*)(uint8_t))fn;
		break;
	case WARADIO_DATA_RECEIVED:
		__wa1470_data_received = (void(*)(dem_packet_st*, dem_packet_info_st*))fn;
		break;
        case WARADIO_TX_FINISHED:
		__wa1470_tx_finished = (void(*)(void))fn;
		break;
        case WARADIO_NOP_DELAY_MS:
              	__wa1470_nop_dalay_ms = (void(*)(uint32_t))fn;
		break;
	default:
		break;
	}
}

void wa1470_spi_write(uint16_t address, uint8_t *data, uint8_t length)
{
  if(__spi_tx && __spi_cs_set)
  {
    __spi_cs_set(0);
    address |= 0x8000;
    __spi_tx(((uint8_t*)(&address)) + 1, 1);
    __spi_tx(((uint8_t*)(&address)), 1);
    __spi_tx(data, length);
    __spi_cs_set(1);
  }
}

void wa1470_spi_read(uint16_t address, uint8_t *data, uint8_t length)
{
  if(__spi_tx && __spi_rx && __spi_cs_set)
  {
    __spi_cs_set(0);
    address &= 0x7fff;
    __spi_tx(((uint8_t*)(&address)) + 1, 1);
    __spi_tx(((uint8_t*)(&address)), 1);
    __spi_rx(data, length);
    __spi_cs_set(1);
  }
}


void wa1470_spi_write8(uint16_t address, uint8_t data)
{
  wa1470_spi_write(address, &data, 1);
}

uint8_t wa1470_spi_read8(uint16_t address)
{
  uint8_t data;
  wa1470_spi_read(address, &data, 1);
  return data;
}

_Bool wa1470_spi_wait_for(uint16_t address, uint8_t value, uint8_t mask)
{
  uint32_t timeout = 0;
  while((wa1470_spi_read8(address) & mask) != value)
  {
    if(++timeout >= SPI_WAIT_TIMEOUT) return 0;
  }
  return 1;
}


void wa1470_init()
{
  wa1470rfe_init();
  wa1470dem_init();
  wa1470mod_init(); 
}

void wa1470_isr()
{
  wa1470dem_isr();
  wa1470mod_isr();
}

_Bool wa1470_cansleep()
{
  return 1;
}

//uint8_t mas[32];
/*
void wa1470_test()
{
  uint32_t hop_table = 0x33333333;
  wa1470_spi_write(0x32, ((uint8_t*)&hop_table), 4);
  wa1470_spi_read(0x20, mas, 32);
}*/

//uint32_t tmp_freq;
//extern uint8_t mas[128];

/*
void wa1470_tcxo_set_reset(uint8_t set)
{
	
}*/
