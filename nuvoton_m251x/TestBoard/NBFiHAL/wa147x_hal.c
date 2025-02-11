#include "NuMicro.h"
#include "wa147x_hal.h"
#include "scheduler_hal.h"
//#include "log.h"


#define SPI_TIMEOUT	1000



#define WA_SPI			    SPI0
#define WA_SPI_MODULE	    SPI0_MODULE

#define WA_IRQ                  PA4
#define WA_IRQ_GPIO_Port 	    PA
#define WA_IRQ_Pin 		        BIT4
#define WA_IRQn 		        4

#define WA_IRQ_EXTI_IRQn 	    GPA_IRQn
#define WA_EXT_IRQHandler       GPA_IRQHandler

#define WA_CS                   PA3
#define WA_CS_GPIO_Port 	    PA
#define WA_CS_Pin 		        BIT3


#define WA_CHIP_EN              PA5
#define WA_CHIP_EN_GPIO_Port 	PA
#define WA_CHIP_EN_Pin 		    BIT5

#define WA_MISO_MOSI_SCK_INIT	\
	SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk);	\
	SYS->GPA_MFPL = SYS_GPA_MFPL_PA0MFP_SPI0_MOSI | SYS_GPA_MFPL_PA1MFP_SPI0_MISO | SYS_GPA_MFPL_PA2MFP_SPI0_CLK | SYS_GPA_MFPL_PA3MFP_SPI0_SS;	\


void wa147x_HAL_GPIO_Init()
{
   GPIO_SetMode(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin, GPIO_MODE_OUTPUT);
   GPIO_SetMode(WA_IRQ_GPIO_Port, WA_IRQ_Pin, GPIO_MODE_INPUT);
   GPIO_EnableInt(WA_IRQ_GPIO_Port, WA_IRQn, GPIO_INT_RISING);
   NVIC_SetPriority(WA_IRQ_EXTI_IRQn, 1);
}


void wa147x_HAL_spi_write_cs(uint8_t state);

void wa147x_HAL_SPI_Init(void)
{

    CLK_EnableModuleClock(WA_SPI_MODULE);

    WA_MISO_MOSI_SCK_INIT;

    SPI_Open(WA_SPI, SPI_MASTER, SPI_MODE_0, 8, 2000000);
    SPI_DisableAutoSS(WA_SPI);
    
    wa147x_HAL_spi_write_cs(1);

}


void WA_EXT_IRQHandler(void)
{
    if (GPIO_GET_INT_FLAG(WA_IRQ_GPIO_Port, WA_IRQ_Pin))
    {
        GPIO_CLR_INT_FLAG(WA_IRQ_GPIO_Port, WA_IRQ_Pin);
        #ifdef WA1471
          wa1471_isr();
        #else
          wa1470_isr();
        #endif
        
    }
}


void wa147x_HAL_enable_pin_irq(void)
{
     NVIC_EnableIRQ(WA_IRQ_EXTI_IRQn);

}

void wa147x_HAL_disable_pin_irq(void)
{
     NVIC_DisableIRQ(WA_IRQ_EXTI_IRQn);
}


#define NOP_DELAY_MS_TICK		8000
void NOP_Delay(uint32_t i)
{
	while (i--)
		asm volatile("nop");
}

void NOP_Delay_ms(uint32_t val)
{
	while (val--)
		NOP_Delay(NOP_DELAY_MS_TICK);
}


void wa147x_HAL_chip_enable(void)
{
    wa147x_HAL_spi_write_cs(1);
    WA_CHIP_EN = 0;
    NOP_Delay_ms(1);
}

void wa147x_HAL_chip_disable(void)
{
  WA_CHIP_EN = 1;
}

uint8_t wa147x_HAL_get_irq_pin_state(void)
{
    return WA_IRQ;
}

void SPI_RX_TX(uint8_t *byteTx, uint8_t *byteRx, uint16_t len) {

   	volatile uint16_t timeout;
        SPI_ClearRxFIFO(WA_SPI);
	for (uint16_t i = 0; i < len; i++)
	{
          timeout = SPI_TIMEOUT;
          
          while (SPI_IS_BUSY(WA_SPI) && timeout--);
          
          if(timeout == 0xffff)
          {
            SPI_Close(WA_SPI);
            wa147x_HAL_SPI_Init();
          }
          
          if (byteTx == NULL)   SPI_WRITE_TX(WA_SPI, 0x00);
          else  SPI_WRITE_TX(WA_SPI, byteTx[i]);
          
          timeout = SPI_TIMEOUT;
          
          while (SPI_IS_BUSY(WA_SPI) && timeout--);
          
          if (byteRx == NULL)   SPI_READ_RX(WA_SPI);
          else	byteRx[i] = SPI_READ_RX(WA_SPI);
	}
	timeout = SPI_TIMEOUT;
	while (SPI_IS_BUSY(WA_SPI) && timeout--);
}


void wa147x_HAL_spi_rx(uint8_t *pData, uint16_t Size)
{
    SPI_RX_TX(NULL, pData, Size);
}

void wa147x_HAL_spi_tx(uint8_t *pData, uint16_t Size)
{
    SPI_RX_TX(pData, NULL, Size);
}



void wa147x_HAL_spi_write_cs(uint8_t state)
{
  if (state) SPI_SET_SS_HIGH(WA_SPI);
  else SPI_SET_SS_LOW(WA_SPI);
}


void wa147x_HAL_bpsk_pin_send(uint8_t* data, uint16_t len, uint16_t bitrate)
{
#ifdef WA1471
  wa1471_bpsk_pin_tx_finished();
#else
  wa1470_bpsk_pin_tx_finished();
#endif
}

#ifdef WA1471
  wa1471_HAL_st wa1471_hal_struct = {0,0,0,0,0,0,0,0,0,0,0,0,0};
#else
  wa1470_HAL_st wa1470_hal_struct = {0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif



void wa147x_HAL_reg_data_received_callback(void* fn)
{
#ifdef WA1471
   wa1471_hal_struct.__wa1471_data_received = (void(*)(uint8_t*, uint8_t*))fn;
#else
   wa1470_hal_struct.__wa1470_data_received = (void(*)(uint8_t*, uint8_t*))fn;
#endif
 
}

void wa147x_HAL_reg_tx_finished_callback(void* fn)
{
#ifdef WA1471
   wa1471_hal_struct.__wa1471_tx_finished = (void(*)(void))fn;
#else
   wa1470_hal_struct.__wa1470_tx_finished = (void(*)(void))fn;
#endif
  
}

void wa147x_HAL_init()
{

  wa147x_HAL_GPIO_Init();

  wa147x_HAL_SPI_Init();

#ifdef WA1471
  wa1471_hal_struct.__wa1471_enable_pin_irq = &wa147x_HAL_enable_pin_irq;
  wa1471_hal_struct.__wa1471_disable_pin_irq = &wa147x_HAL_disable_pin_irq;
  wa1471_hal_struct.__wa1471_chip_enable = &wa147x_HAL_chip_enable;
  wa1471_hal_struct.__wa1471_chip_disable = &wa147x_HAL_chip_disable;
  wa1471_hal_struct.__wa1471_get_irq_pin_state = &wa147x_HAL_get_irq_pin_state;
  wa1471_hal_struct.__spi_rx = &wa147x_HAL_spi_rx;
  wa1471_hal_struct.__spi_tx = &wa147x_HAL_spi_tx;
  wa1471_hal_struct.__spi_cs_set = &wa147x_HAL_spi_write_cs;
  wa1471_hal_struct.__wa1471_nop_dalay_ms = &NOP_Delay_ms;
  wa1471_hal_struct.__wa1471_send_to_bpsk_pin = &wa147x_HAL_bpsk_pin_send;
  wa1471_hal_struct.__wa1471_log_send_str = 0;//&log_send_str;
  wa1471_init(WA1471_SEND_BY_I_Q_MODULATOR, 0, &wa1471_hal_struct, _scheduler);
#else
  wa1470_hal_struct.__wa1470_enable_pin_irq = &wa147x_HAL_enable_pin_irq;
  wa1470_hal_struct.__wa1470_disable_pin_irq = &wa147x_HAL_disable_pin_irq;
  wa1470_hal_struct.__wa1470_chip_enable = &wa147x_HAL_chip_enable;
  wa1470_hal_struct.__wa1470_chip_disable = &wa147x_HAL_chip_disable;
  wa1470_hal_struct.__wa1470_get_irq_pin_state = &wa147x_HAL_get_irq_pin_state;
  wa1470_hal_struct.__spi_rx = &wa147x_HAL_spi_rx;
  wa1470_hal_struct.__spi_tx = &wa147x_HAL_spi_tx;
  wa1470_hal_struct.__spi_cs_set = &wa147x_HAL_spi_write_cs;
  wa1470_hal_struct.__wa1470_nop_dalay_ms = &NOP_Delay_ms;
  wa1470_hal_struct.__wa1470_send_to_bpsk_pin = &wa147x_HAL_bpsk_pin_send;
  wa1470_hal_struct.__wa1470_log_send_str = 0;//&log_send_str;
  wa1470_init(WA1470_SEND_BY_I_Q_MODULATOR, 0, &wa1470_hal_struct, _scheduler);
#endif
  
}

