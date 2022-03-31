#include "NuMicro.h"
#include "wa1470_hal.h"
#include "scheduler_hal.h"
//#include "log.h"


#define SPI_TIMEOUT	1000



#define WA_SPI			    SPI0
#define WA_SPI_MODULE	    SPI0_MODULE

//#define WA_SPI_MOSI_Port	GPIOB
//#define WA_SPI_MOSI_Pin		GPIO_Pin_12
//#define WA_SPI_MOSI_AF		GPIO_AF0_SPI1
//#define WA_SPI_MISO_Port	GPIOB
//#define WA_SPI_MISO_Pin		GPIO_Pin_11
//#define WA_SPI_MISO_AF		GPIO_AF0_SPI1
//#define WA_SPI_SCK_Port		GPIOB
//#define WA_SPI_SCK_Pin		GPIO_Pin_10
//#define WA_SPI_SCK_AF		GPIO_AF0_SPI1

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


//static SPI_HandleTypeDef hspi;



void wa1470_HAL_GPIO_Init()
{
/*
    GPIO_InitType GPIO_InitStruct;

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUTPUT_CMOS;
  GPIO_InitStruct.GPIO_Pin = WA_CHIP_EN_Pin;
  GPIOBToF_Init(WA_CHIP_EN_GPIO_Port, &GPIO_InitStruct);



  PMU_WakeUpPinConfig(WA_IRQ_Pin, IOA_RISING);
  PMU_ClearIOAINTStatus(WA_IRQ_Pin);
  //PMU_INTConfig(PMU_INT_IOAEN, ENABLE);
  CORTEX_SetPriority_ClearPending_EnableIRQ(PMU_IRQn, 1);

  //HAL_NVIC_SetPriority(WA_IRQ_EXTI_IRQn, 1, 0);


*/

   GPIO_SetMode(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin, GPIO_MODE_OUTPUT);


   GPIO_SetMode(WA_IRQ_GPIO_Port, WA_IRQ_Pin, GPIO_MODE_INPUT);
   GPIO_EnableInt(WA_IRQ_GPIO_Port, WA_IRQn, GPIO_INT_RISING);
   NVIC_SetPriority(WA_IRQ_EXTI_IRQn, 1);

}



void wa1470_HAL_SPI_Init(void)
{
/*
    GPIO_InitType GPIO_InitStruct;
    SPI_InitType SPI_InitStruct;



    GPIOBToF_SetBits(WA_CS_GPIO_Port, WA_CS_Pin);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUTPUT_CMOS;
    GPIO_InitStruct.GPIO_Pin = WA_CS_Pin;
    GPIOBToF_Init(WA_CS_GPIO_Port, &GPIO_InitStruct);

    SPI_DeviceInit(WA_SPI);
    SPI_StructInit(&SPI_InitStruct);
    SPI_InitStruct.ClockDivision = SPI_CLKDIV_8;
    SPI_InitStruct.CSNSoft = SPI_CSNSOFT_ENABLE;
    SPI_Init(WA_SPI, &SPI_InitStruct);
    SPI_Cmd(WA_SPI, ENABLE);
*/

    CLK_EnableModuleClock(WA_SPI_MODULE);

    WA_CS = 1;
    GPIO_SetMode(WA_CS_GPIO_Port, WA_CS_Pin, GPIO_MODE_OUTPUT);

    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk);
    SYS->GPA_MFPL = SYS_GPA_MFPL_PA0MFP_SPI0_MOSI | SYS_GPA_MFPL_PA1MFP_SPI0_MISO | SYS_GPA_MFPL_PA2MFP_SPI0_CLK | SYS_GPA_MFPL_PA3MFP_GPIO ;

    /* Enable SPI0 clock pin (PA2) schmitt trigger */
    //PA->SMTEN |= GPIO_SMTEN_SMTEN2_Msk;

    SPI_Open(WA_SPI, SPI_MASTER, SPI_MODE_0, 8, 2000000);

}

/*
void wa1470_HAL_bpsk_pin_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = WA_BPSK_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(WA_BPSK_PIN_GPIO_Port, &GPIO_InitStruct);
}
*/

void WA_EXT_IRQHandler(void)
{
    if (GPIO_GET_INT_FLAG(WA_IRQ_GPIO_Port, WA_IRQ_Pin))
    {
        GPIO_CLR_INT_FLAG(WA_IRQ_GPIO_Port, WA_IRQ_Pin);
        wa1470_isr();
    }
}



void wa1470_HAL_enable_pin_irq(void)
{
     NVIC_EnableIRQ(WA_IRQ_EXTI_IRQn);

}

void wa1470_HAL_disable_pin_irq(void)
{
     NVIC_DisableIRQ(WA_IRQ_EXTI_IRQn);
}

void wa1470_HAL_chip_enable(void)
{
  WA_CHIP_EN = 0;
}

void wa1470_HAL_chip_disable(void)
{
  WA_CHIP_EN = 1;
}

uint8_t wa1470_HAL_get_irq_pin_state(void)
{

    return WA_IRQ;//GPIOA_ReadInputDataBit(WA_IRQ_GPIO_Port, WA_IRQ_Pin);
      //HAL_GPIO_ReadPin(WA_IRQ_GPIO_Port, WA_IRQ_Pin);
}

/*
void SPI_RX(uint8_t* byte, uint16_t len) {
	volatile uint16_t timeout;

	for (uint16_t i = 0; i < len; i++)
	{
		hspi.Instance->DR = 0x00;
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_RXNE) == RESET && timeout--);
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_BSY) == SET && timeout--);
		byte[i] = hspi.Instance->DR;
	}
}

void SPI_TX(uint8_t *byte, uint16_t len) {
	volatile uint16_t timeout;

	for (uint16_t i = 0; i < len; i++)
	{
		hspi.Instance->DR = byte[i];
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_TXE) == RESET && timeout--);
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_BSY) == SET && timeout--);
		__HAL_SPI_CLEAR_OVRFLAG(&hspi);
	}
}
*/
void SPI_RX_TX(uint8_t *byteTx, uint8_t *byteRx, uint16_t len) {

    volatile uint16_t timeout;
	for (uint16_t i = 0; i < len; i++)
	{
        timeout = SPI_TIMEOUT;
        //while (SPI_GetStatus(WA_SPI, SPI_STS_TXEMPTY) == 0U && timeout--);

        while (SPI_IS_BUSY(WA_SPI) && timeout--);
        if (byteTx == NULL)
            SPI_WRITE_TX(WA_SPI, 0x00);
        else
            SPI_WRITE_TX(WA_SPI, byteTx[i]);
        timeout = SPI_TIMEOUT;
        //while (SPI_GetStatus(WA_SPI, SPI_STS_RNE) == 0U && timeout--);
        while (SPI_IS_BUSY(WA_SPI) && timeout--);
        if (byteRx == NULL)
            SPI_READ_RX(WA_SPI);
        else
            byteRx[i] = SPI_READ_RX(WA_SPI);
	}
    timeout = SPI_TIMEOUT;
    //while (SPI_GetStatus(WA_SPI, SPI_STS_BSY) == 1U && timeout--);
    while (SPI_IS_BUSY(WA_SPI) && timeout--);
}


void wa1470_HAL_spi_rx(uint8_t *pData, uint16_t Size)
{
    SPI_RX_TX(NULL, pData, Size);
    //SPI_RX(pData, Size);
}

void wa1470_HAL_spi_tx(uint8_t *pData, uint16_t Size)
{
    SPI_RX_TX(pData, NULL, Size);
  //SPI_TX(pData, Size);
}



void wa1470_HAL_spi_write_cs(uint8_t state)
{
  if (state) WA_CS = 1;
  else WA_CS = 0;
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


void wa1470_HAL_bpsk_pin_send(uint8_t* data, uint16_t len, uint16_t bitrate)
{
  wa1470_bpsk_pin_tx_finished();
}



wa1470_HAL_st wa1470_hal_struct = {0,0,0,0,0,0,0,0,0,0,0,0,0};

void wa1470_HAL_reg_data_received_callback(void* fn)
{
  wa1470_hal_struct.__wa1470_data_received = (void(*)(uint8_t*, uint8_t*))fn;
}

void wa1470_HAL_reg_tx_finished_callback(void* fn)
{
  wa1470_hal_struct.__wa1470_tx_finished = (void(*)(void))fn;
}


void wa1470_HAL_init()
{

  wa1470_HAL_GPIO_Init();

  wa1470_HAL_SPI_Init();

  wa1470_hal_struct.__wa1470_enable_pin_irq = &wa1470_HAL_enable_pin_irq;
  wa1470_hal_struct.__wa1470_disable_pin_irq = &wa1470_HAL_disable_pin_irq;
  wa1470_hal_struct.__wa1470_chip_enable = &wa1470_HAL_chip_enable;
  wa1470_hal_struct.__wa1470_chip_disable = &wa1470_HAL_chip_disable;
  wa1470_hal_struct.__wa1470_get_irq_pin_state = &wa1470_HAL_get_irq_pin_state;
  wa1470_hal_struct.__spi_rx = &wa1470_HAL_spi_rx;
  wa1470_hal_struct.__spi_tx = &wa1470_HAL_spi_tx;
  wa1470_hal_struct.__spi_cs_set = &wa1470_HAL_spi_write_cs;
  wa1470_hal_struct.__wa1470_nop_dalay_ms = &NOP_Delay_ms;
  wa1470_hal_struct.__wa1470_send_to_bpsk_pin = &wa1470_HAL_bpsk_pin_send;
  wa1470_hal_struct.__wa1470_log_send_str = 0;//&log_send_str;

  wa1470_init(WA1470_SEND_BY_I_Q_MODULATOR, 0, &wa1470_hal_struct, _scheduler);

}

