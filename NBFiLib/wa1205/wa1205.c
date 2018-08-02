#include "wa1205.h"
#include "wa1205dem.h"
#include "wa1205mod.h"
#include "adf4350.h"

void (*__wa1205_enable_global_irq)(void) = 0;
void (*__wa1205_disable_global_irq)(void) = 0;
void (*__wa1205_enable_pin_irq)(void) = 0;
void (*__wa1205_disable_pin_irq)(void) = 0;
uint8_t (*__wa1205_get_irq_pin_state)(void);
void (*__spi_rx)(uint8_t *, uint16_t);
void (*__spi_tx)(uint8_t *, uint16_t);
void (*__spi_tx_rx)(uint8_t *, uint8_t *, uint16_t);
void (*__spi_cs_set)(uint8_t);
void (*__wa1205_data_received)(dem_packet_st *pkt, dem_packet_info_st * info) = 0;
void (*__wa1205_tx_finished)(void) = 0;


void wa1205_reg_func(uint8_t name, void*  fn)
{
	switch(name)
	{
	case WARADIO_ENABLE_GLOBAL_IRQ:
		__wa1205_enable_global_irq = (void(*)(void))fn;
		break;
	case WARADIO_DISABLE_GLOBAL_IRQ:
		__wa1205_disable_global_irq = (void(*)(void))fn;
		break;
	case WARADIO_ENABLE_IRQ_PIN:
		__wa1205_enable_pin_irq = (void(*)(void))fn;
		break;
	case WARADIO_DISABLE_IRQ_PIN:
		__wa1205_disable_pin_irq = (void(*)(void))fn;
		break;
	case WARADIO_GET_IRQ_PIN:
		__wa1205_get_irq_pin_state = (uint8_t(*)(void))fn;
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
		__wa1205_data_received = (void(*)(dem_packet_st*, dem_packet_info_st*))fn;
		break;
        case WARADIO_TX_FINISHED:
		__wa1205_tx_finished = (void(*)(void))fn;
		break;
	default:
		break;
	}
}

void wa1205_spi_write(uint16_t address, uint8_t *data, uint8_t length)
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

void wa1205_spi_read(uint16_t address, uint8_t *data, uint8_t length)
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


void wa1205_spi_write8(uint16_t address, uint8_t data)
{
  wa1205_spi_write(address, &data, 1);
}

void wa1205_init()
{
  wa1205dem_init();
  wa1205mod_init();
  
}

void wa1205_isr()
{
  wa1205dem_isr();
  wa1205mod_isr();
}

_Bool wa1205_cansleep()
{
  return 1;
}

//uint8_t mas[32];
/*
void wa1205_test()
{
  uint32_t hop_table = 0x33333333;
  wa1205_spi_write(0x32, ((uint8_t*)&hop_table), 4);
  wa1205_spi_read(0x20, mas, 32);
}*/

uint32_t tmp_freq;
//extern uint8_t mas[128];

void wa1205_set_freq(uint32_t freq)
{
    //return;
    //freq = 868800000;
    //static _Bool tmp = 0;
   
    tmp_freq = freq - 90000;
    uint32_t koeff[6];    
    
    adf4350_state st;
    adf4350_platform_data pdata;
    init_config(&st, &pdata);
    adf4350_set_freq(&st, freq/* + 137500000*/);
    for(int i = 0; i != 6; i++) koeff[i] = st.regs[i];


    for(int i = 0; i != 6; i++)
    {
      wa1205_spi_write(DEM_BS3_FREQ + i*4, ((uint8_t*)&koeff[i]), 4);
    }
        
    wa1205_spi_write8(DEM_BS3_FREQ_APPLY, 1);
       
}

void wa1205_tcxo_set_reset(uint8_t set)
{
	
}
