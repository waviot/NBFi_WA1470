//#include "stm32l0xx_hal_conf.h"
#include "target.h"
#include "wa1470_hal.h"
#include "scheduler_hal.h"
//#include "log.h"



#define SPI_TIMEOUT	1000

#define WA_SPI			SPI1
//#define WA_SPI_RCC_ENABLE	__HAL_RCC_SPI1_CLK_ENABLE
//#define WA_SPI_RCC_DISABLE	__HAL_RCC_SPI1_CLK_DISABLE
#define WA_SPI_MOSI_Port	GPIOB
#define WA_SPI_MOSI_Pin		GPIO_Pin_12
//#define WA_SPI_MOSI_AF		GPIO_AF0_SPI1
#define WA_SPI_MISO_Port	GPIOB
#define WA_SPI_MISO_Pin		GPIO_Pin_11
//#define WA_SPI_MISO_AF		GPIO_AF0_SPI1
#define WA_SPI_SCK_Port		GPIOB
#define WA_SPI_SCK_Pin		GPIO_Pin_10
//#define WA_SPI_SCK_AF		GPIO_AF0_SPI1
#define WA_IRQ_GPIO_Port 	GPIOA
#define WA_IRQ_Pin 		    GPIO_Pin_11
//#define WA_IRQ_EXTI_IRQn 	PMU_IRQHandler
//#define WA_EXT_IRQHandler   EXTI4_15_IRQHandler
#define WA_CS_GPIO_Port 	GPIOB
#define WA_CS_Pin 		    GPIO_Pin_0

#define WA_CHIP_EN_GPIO_Port 	GPIOB
#define WA_CHIP_EN_Pin 		GPIO_Pin_6


//static SPI_HandleTypeDef hspi;



void wa1470_HAL_GPIO_Init()
{
  GPIO_InitType GPIO_InitStruct;

  /*GPIO_InitStruct.Pin = WA_CHIP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WA_CHIP_EN_GPIO_Port, &GPIO_InitStruct);*/

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUTPUT_CMOS;
  GPIO_InitStruct.GPIO_Pin = WA_CHIP_EN_Pin;
  GPIOBToF_Init(WA_CHIP_EN_GPIO_Port, &GPIO_InitStruct);


  /*GPIO_InitStruct.Pin = WA_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(WA_IRQ_GPIO_Port, &GPIO_InitStruct);*/

  PMU_WakeUpPinConfig(WA_IRQ_Pin, IOA_FALLING);
  PMU_ClearIOAINTStatus(WA_IRQ_Pin);
  //PMU_INTConfig(PMU_INT_IOAEN, ENABLE);
  CORTEX_SetPriority_ClearPending_EnableIRQ(PMU_IRQn, 1);

  //HAL_NVIC_SetPriority(WA_IRQ_EXTI_IRQn, 1, 0);

}



void wa1470_HAL_SPI_Init(void)
{
    /*GPIO_InitTypeDef GPIO_InitStruct;

    WA_SPI_RCC_ENABLE();

    hspi.Instance = WA_SPI;
    hspi.Init.Mode = SPI_MODE_MASTER;
    hspi.Init.Direction = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi.Init.NSS = SPI_NSS_SOFT;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial = 7;
    HAL_SPI_Init(&hspi);
    __HAL_SPI_ENABLE(&hspi);

  */

    GPIO_InitType GPIO_InitStruct;
    SPI_InitType SPI_InitStruct;

/*

    GPIO_InitStruct.Pin = WA_SPI_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = WA_SPI_MOSI_AF;
    HAL_GPIO_Init(WA_SPI_MOSI_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = WA_SPI_MISO_Pin;
    GPIO_InitStruct.Alternate = WA_SPI_MISO_AF;
    HAL_GPIO_Init(WA_SPI_MISO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = WA_SPI_SCK_Pin;
    GPIO_InitStruct.Alternate = WA_SPI_SCK_AF;
    HAL_GPIO_Init(WA_SPI_SCK_Port, &GPIO_InitStruct);

    */


    //GPIO_InitStruct.Pin = WA_CS_Pin;
    //GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    //HAL_GPIO_Init(WA_CS_GPIO_Port, &GPIO_InitStruct);


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
  if (PMU_GetIOAINTStatus(WA_IRQ_Pin))
  {
    PMU_ClearIOAINTStatus(WA_IRQ_Pin);
    wa1470_isr();
  }
}



void wa1470_HAL_enable_pin_irq(void)
{
  //HAL_NVIC_EnableIRQ(WA_IRQ_EXTI_IRQn);
    PMU_INTConfig(PMU_INT_IOAEN, ENABLE);

}

void wa1470_HAL_disable_pin_irq(void)
{
  //HAL_NVIC_DisableIRQ(WA_IRQ_EXTI_IRQn);
    PMU_INTConfig(PMU_INT_IOAEN, DISABLE);
}

void wa1470_HAL_chip_enable(void)
{
  //HAL_GPIO_WritePin(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin,  GPIO_PIN_RESET);
    GPIOBToF_ResetBits(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin);
}

void wa1470_HAL_chip_disable(void)
{
  //HAL_GPIO_WritePin(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin,  GPIO_PIN_SET);
  GPIOBToF_SetBits(WA_CHIP_EN_GPIO_Port, WA_CHIP_EN_Pin);
}

uint8_t wa1470_HAL_get_irq_pin_state(void)
{
  return GPIOA_ReadInputDataBit(WA_IRQ_GPIO_Port, WA_IRQ_Pin);
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
		/*hspi.Instance->DR = byteTx[i];
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_RXNE) == RESET && timeout--);
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_BSY) == SET && timeout--);
		byteRx[i] = hspi.Instance->DR;*/
        timeout = SPI_TIMEOUT;
        while (SPI_GetStatus(WA_SPI, SPI_STS_TXEMPTY) == 0U && timeout--);
        if (byteTx == NULL)
            SPI_SendData(WA_SPI, 0x00);
        else
            SPI_SendData(WA_SPI, byteTx[i]);
        timeout = SPI_TIMEOUT;
        while (SPI_GetStatus(WA_SPI, SPI_STS_RNE) == 0U && timeout--);
        if (byteRx == NULL)
            SPI_ReceiveData(WA_SPI);
        else
            byteRx[i] = SPI_ReceiveData(WA_SPI);
	}
    timeout = SPI_TIMEOUT;
    while (SPI_GetStatus(WA_SPI, SPI_STS_BSY) == 1U && timeout--);
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
  if (state)
	//HAL_GPIO_WritePin(WA_CS_GPIO_Port, WA_CS_Pin, GPIO_PIN_SET);
      GPIOBToF_SetBits(WA_CS_GPIO_Port, WA_CS_Pin);
  else
	//HAL_GPIO_WritePin(WA_CS_GPIO_Port, WA_CS_Pin, GPIO_PIN_RESET);
      GPIOBToF_ResetBits(WA_CS_GPIO_Port, WA_CS_Pin);
}


#define NOP_DELAY_MS_TICK		1600
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

