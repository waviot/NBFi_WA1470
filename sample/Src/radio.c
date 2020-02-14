#include "main.h"
#include "radio.h"
#include "wtimer.h"
#include "string.h"
#include "nbfi.h"
#include "nbfi_mac.h"
#include "nbfi_rf.h"
#include "nbfi_config.h"
#include "stm32l0xx_hal_conf.h"
#include "defines.h"

#define MODEM_ID  *((const uint32_t*)0x0801ff80)  
#define KEY  ((const uint32_t*)0x0801ff84)            

#define MANUFACTURER_ID         0x8888 //Waviot
#define HARDWARE_TYPE_ID        0x3       //ASIC_PROTOTYPE
#define PROTOCOL_ID             0       //undefined
#define TX_MAX_POWER 15
#define TX_MIN_POWER -13
#define SEND_INFO_PERIOD	2592000         //one time per month
#define BAND         UL868800_DL869100          //UL868800_DL864000



#if BAND == UL868800_DL446000
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL868800_DL864000
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       864000000
#elif BAND == UL868800_DL446000_DL864000
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       864000000
#elif BAND == UL867950_DL446000
#define NBFI_UL_FREQ_BASE       (867950000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL868500_DL446000
#define NBFI_UL_FREQ_BASE       (868500000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL868100_DL446000
#define NBFI_UL_FREQ_BASE       (868100000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL864000_DL446000
#define NBFI_UL_FREQ_BASE       (864000000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL863175_DL446000
#define NBFI_UL_FREQ_BASE       (863175000 - 25000)
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL864000_DL875000
#define NBFI_UL_FREQ_BASE       (864000000 - 25000)
#define NBFI_DL_FREQ_BASE       875000000
#elif BAND == UL868800_DL868000
#define NBFI_UL_FREQ_BASE       (868800000 - 25000)
#define NBFI_DL_FREQ_BASE       868800000
#elif BAND == UL868800_DL869100
#define NBFI_UL_FREQ_BASE       (868800000 - 25000) //(866342400 - 25000)//(868800000 - 25000)    
#define NBFI_DL_FREQ_BASE       869100000
#endif 


#ifdef BANKA
const nbfi_settings_t nbfi_set_default =
{
    CRX,//mode;
    UL_DBPSK_50_PROT_D,//UL_DBPSK_50_PROT_D, // tx_phy_channel;
    DL_DBPSK_3200_PROT_D, // rx_phy_channel;
    HANDSHAKE_NONE,
    MACK_1,             //mack_mode
    2,                  //num_of_retries;
    8,                  //max_payload_len;
    {0},                //dl_ID[3];
    {0},                //temp_ID[3];
    {0xFF,0,0},         //broadcast_ID[3];
    {0},                //full_ID[6];
    868800000 + 20000,                  //tx_freq;
    0,//858090000,//868791000,//0,//868790000,//0,//868735500,//868710000,//868800000,                  //rx_freq;
    PCB,                //tx_antenna;
    PCB,                //rx_antenna;
    TX_MAX_POWER,       //tx_pwr;
    1,//3600*6,             //heartbeat_interval
    0,                //heartbeat_num
    NBFI_FLG_FIXED_BAUD_RATE,                  //additional_flags
    NBFI_UL_FREQ_BASE,
    NBFI_DL_FREQ_BASE,
    NBFI_FREQ_PLAN_DEFAULT//NBFI_FREQ_PLAN_SHIFTED_HIGHPHY
};
#else

const nbfi_settings_t nbfi_set_default =
{
    CRX,//mode;
    UL_DBPSK_25600_PROT_E,//UL_DBPSK_50_PROT_D, // tx_phy_channel;
    DL_DBPSK_400_PROT_D, // rx_phy_channel;
    HANDSHAKE_SIMPLE,
    MACK_1,             //mack_mode
    0x82,                  //num_of_retries;
    8,                  //max_payload_len;
    {0},                //dl_ID[3];
    {0},                //temp_ID[3];
    {0xFF,0,0},         //broadcast_ID[3];
    {0},                //full_ID[6];
    0,                  //tx_freq;
    0,//858090000,//868791000,//0,//868790000,//0,//868735500,//868710000,//868800000,                  //rx_freq;
    PCB,                //tx_antenna;
    PCB,                //rx_antenna;
    TX_MAX_POWER,       //tx_pwr;
    30,//3600*6,             //heartbeat_interval
    255,                //heartbeat_num
    0,//NBFI_FLG_FIXED_BAUD_RATE,                  //additional_flags
    NBFI_UL_FREQ_BASE,
    NBFI_DL_FREQ_BASE,
    NBFI_FREQ_PLAN_DEFAULT//NBFI_FREQ_PLAN_SHIFTED_HIGHPHY
};
#endif

static SPI_HandleTypeDef hspi;

static LPTIM_HandleTypeDef hlptim;

#define SPI_TIMEOUT	1000

#ifdef PHOBOS_EXT_MODULE
  #define AX_LPTIM			LPTIM1
  #define AX_LPTIM_IRQn			LPTIM1_IRQn
  #define AX_LPTIM_RCC_ENABLE 	        __HAL_RCC_LPTIM1_CLK_ENABLE
  #define AX_LPTIM_RCC_DISABLE 	        __HAL_RCC_LPTIM1_CLK_DISABLE
  #define AX_SPI			SPI1
  #define AX_SPI_RCC_ENABLE		__HAL_RCC_SPI1_CLK_ENABLE
  #define AX_SPI_RCC_DISABLE	 	__HAL_RCC_SPI1_CLK_DISABLE
  #define AX_SPI_MOSI_Port		GPIOA
  #define AX_SPI_MOSI_Pin		GPIO_PIN_7
  #define AX_SPI_MOSI_AF		GPIO_AF0_SPI1
  #define AX_SPI_MISO_Port		GPIOA
  #define AX_SPI_MISO_Pin		GPIO_PIN_6
  #define AX_SPI_MISO_AF		GPIO_AF0_SPI1
  #define AX_SPI_SCK_Port		GPIOA
  #define AX_SPI_SCK_Pin		GPIO_PIN_5
  #define AX_SPI_SCK_AF			GPIO_AF0_SPI1
  #define AX_IRQ_GPIO_Port 		GPIOB
  #define AX_IRQ_Pin 			GPIO_PIN_0
  #define AX_IRQ_EXTI_IRQn 		EXTI0_1_IRQn
  #define AX_CS_GPIO_Port 		GPIOB
  #define AX_CS_Pin 			GPIO_PIN_1
  #define AX_TCXO_GPIO_Port 		GPIOB
  #define AX_TCXO_Pin 			GPIO_PIN_2
  #define AX_CHIP_EN_GPIO_Port 		GPIOA
  #define AX_CHIP_EN_Pin 		GPIO_PIN_4
  #define AX_BPSK_PIN_GPIO_Port 	GPIOB
  #define AX_BPSK_PIN_Pin 		GPIO_PIN_12
  #define AX_DFT_EN_GPIO_Port 		GPIOB
  #define AX_DFT_EN_Pin 		GPIO_PIN_9
  #define AX_SWITCH_TX_ON_GPIO_Port     GPIOB
  #define AX_SWITCH_TX_ON_Pin 		GPIO_PIN_10
  #define AX_SWITCH_NTX_ON_GPIO_Port    GPIOB
  #define AX_SWITCH_NTX_ON_Pin 		GPIO_PIN_11

#else
  #define AX_LPTIM			LPTIM1
  #define AX_LPTIM_IRQn			LPTIM1_IRQn
  #define AX_LPTIM_RCC_ENABLE 	        __HAL_RCC_LPTIM1_CLK_ENABLE
  #define AX_LPTIM_RCC_DISABLE 	        __HAL_RCC_LPTIM1_CLK_DISABLE
  #define AX_SPI			SPI1
  #define AX_SPI_RCC_ENABLE		__HAL_RCC_SPI1_CLK_ENABLE
  #define AX_SPI_RCC_DISABLE	 	__HAL_RCC_SPI1_CLK_DISABLE
  #define AX_SPI_MOSI_Port		GPIOA
  #define AX_SPI_MOSI_Pin		GPIO_PIN_7
  #define AX_SPI_MOSI_AF		GPIO_AF0_SPI1
  #define AX_SPI_MISO_Port		GPIOA
  #define AX_SPI_MISO_Pin		GPIO_PIN_6
  #define AX_SPI_MISO_AF		GPIO_AF0_SPI1
  #define AX_SPI_SCK_Port		GPIOA
  #define AX_SPI_SCK_Pin		GPIO_PIN_5
  #define AX_SPI_SCK_AF			GPIO_AF0_SPI1
  #define AX_IRQ_GPIO_Port 		GPIOB
  #define AX_IRQ_Pin 			GPIO_PIN_0
  #define AX_IRQ_EXTI_IRQn 		EXTI0_1_IRQn
  #define AX_CS_GPIO_Port 		GPIOB
  #define AX_CS_Pin 			GPIO_PIN_1
  #define AX_TCXO_GPIO_Port 		GPIOA
  #define AX_TCXO_Pin 			GPIO_PIN_8
  #define AX_CHIP_EN_GPIO_Port 		GPIOB
  #define AX_CHIP_EN_Pin 		GPIO_PIN_13
  #define AX_DFT_EN_GPIO_Port 		GPIOB
  #define AX_DFT_EN_Pin 		GPIO_PIN_11
  #define AX_BPSK_PIN_GPIO_Port 	GPIOB
  #define AX_BPSK_PIN_Pin 		GPIO_PIN_12
#endif


void RADIO_GPIO_Init()
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Pin = AX_TCXO_Pin;    
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AX_TCXO_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = AX_CHIP_EN_Pin;    
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AX_CHIP_EN_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = AX_DFT_EN_Pin;    
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AX_DFT_EN_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = AX_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(AX_IRQ_GPIO_Port, &GPIO_InitStruct);

  
  #ifdef PHOBOS_EXT_MODULE
  GPIO_InitStruct.Pin = AX_SWITCH_TX_ON_Pin;    
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AX_SWITCH_TX_ON_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = AX_SWITCH_NTX_ON_Pin;   
  HAL_GPIO_Init(AX_SWITCH_TX_ON_GPIO_Port, &GPIO_InitStruct);
  #endif

  
  HAL_NVIC_SetPriority(AX_IRQ_EXTI_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(AX_IRQ_EXTI_IRQn);
  

}

void RADIO_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    AX_SPI_RCC_ENABLE();

    hspi.Instance = AX_SPI;
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
    
    
    GPIO_InitStruct.Pin = AX_SPI_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = AX_SPI_MOSI_AF;
    HAL_GPIO_Init(AX_SPI_MOSI_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = AX_SPI_MISO_Pin;
    GPIO_InitStruct.Alternate = AX_SPI_MISO_AF;
    HAL_GPIO_Init(AX_SPI_MISO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = AX_SPI_SCK_Pin;
    GPIO_InitStruct.Alternate = AX_SPI_SCK_AF;
    HAL_GPIO_Init(AX_SPI_SCK_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = AX_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(AX_CS_GPIO_Port, &GPIO_InitStruct);
  
    HAL_GPIO_WritePin(AX_CS_GPIO_Port, AX_CS_Pin, GPIO_PIN_SET);

    

}

void RADIO_LPTIM_Init(void)
{

    AX_LPTIM_RCC_ENABLE();

    hlptim.Instance = LPTIM1;
    hlptim.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
    hlptim.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
    hlptim.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    HAL_LPTIM_Init(&hlptim);
    
    HAL_NVIC_SetPriority(AX_LPTIM_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(AX_LPTIM_IRQn);

}


void LPTIM1_IRQHandler(void)
{
  if (__HAL_LPTIM_GET_FLAG(&hlptim, LPTIM_FLAG_CMPM) != RESET) {
		__HAL_LPTIM_CLEAR_FLAG(&hlptim, LPTIM_FLAG_CMPM);
		wtimer_cc0_irq();
	}
}



void RADIO_BPSK_PIN_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = AX_BPSK_PIN_Pin;   
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(AX_BPSK_PIN_GPIO_Port, &GPIO_InitStruct);  
}      



    
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

void SPI_RX_TX(uint8_t *byteTx, uint8_t *byteRx, uint16_t len) {
	volatile uint16_t timeout;

	for (uint16_t i = 0; i < len; i++)
	{
		hspi.Instance->DR = byteTx[i];
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_RXNE) == RESET && timeout--);
		timeout = SPI_TIMEOUT;
		while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_BSY) == SET && timeout--);
		byteRx[i] = hspi.Instance->DR;
	}
}

void radio_enable_global_irq(void)
{
  __enable_irq();
}

void radio_disable_global_irq(void)
{
  __disable_irq();
}

void radio_enable_pin_irq(void)
{
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void radio_disable_pin_irq(void)
{
  HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
}

void radio_chip_enable(void)
{
  HAL_GPIO_WritePin(AX_DFT_EN_GPIO_Port, AX_DFT_EN_Pin,  GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AX_CHIP_EN_GPIO_Port, AX_CHIP_EN_Pin,  GPIO_PIN_SET);
  HAL_GPIO_WritePin(AX_TCXO_GPIO_Port, AX_TCXO_Pin,  GPIO_PIN_SET);
}

void radio_chip_disable(void)
{
  HAL_GPIO_WritePin(AX_CHIP_EN_GPIO_Port, AX_CHIP_EN_Pin,  GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AX_DFT_EN_GPIO_Port, AX_DFT_EN_Pin,  GPIO_PIN_RESET);
  HAL_GPIO_WritePin(AX_TCXO_GPIO_Port, AX_TCXO_Pin,  GPIO_PIN_RESET);	
}

uint8_t radio_get_irq_pin_state(void)
{
  return HAL_GPIO_ReadPin(AX_IRQ_GPIO_Port, AX_IRQ_Pin);

}

void radio_spi_rx(uint8_t *pData, uint16_t Size)
{
  SPI_RX(pData, Size);
  
}

void radio_spi_tx(uint8_t *pData, uint16_t Size)
{
  SPI_TX(pData, Size);
}

void radio_spi_tx_rx(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
  SPI_RX_TX(pTxData, pRxData, Size);
}


void radio_spi_write_cs(uint8_t state)
{
  if (state)
	HAL_GPIO_WritePin(AX_CS_GPIO_Port, AX_CS_Pin, GPIO_PIN_SET);
  else
	HAL_GPIO_WritePin(AX_CS_GPIO_Port, AX_CS_Pin, GPIO_PIN_RESET);
}


void radio_bpsk_pin_send(uint8_t* data, uint16_t len, uint16_t bitrate)
{
  wa1470_bpsk_pin_tx_finished();
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

void wtimer_cc_irq_enable(uint8_t chan)
{
	__HAL_LPTIM_ENABLE_IT(&hlptim, LPTIM_IT_CMPM);
}

void wtimer_cc_irq_disable(uint8_t chan)
{
	__HAL_LPTIM_DISABLE_IT(&hlptim, LPTIM_IT_CMPM);
}

void wtimer_cc_set(uint8_t chan, uint16_t data)
{
	hlptim.Instance->CMP = data;
}

uint16_t wtimer_cc_get(uint8_t chan)
{
	return (uint16_t) hlptim.Instance->CMP;
}


uint8_t wtimer_check_cc_irq(uint8_t chan)
{
	return __HAL_LPTIM_GET_FLAG(&hlptim, LPTIM_IT_CMPM);
}


uint16_t wtimer_cnt_get(uint8_t chan)
{
  static uint16_t prev = 0; 
  uint16_t timer = (uint16_t) hlptim.Instance->CNT;
  if((timer < prev) && ((prev - timer) < 10000))
  {
    return prev;
  }
  prev = timer;
  return timer;
}




void nbfi_before_tx()
{
  #ifdef PHOBOS_EXT_MODULE
    HAL_GPIO_WritePin(AX_SWITCH_TX_ON_GPIO_Port, AX_SWITCH_TX_ON_Pin,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(AX_SWITCH_NTX_ON_GPIO_Port, AX_SWITCH_NTX_ON_Pin,  GPIO_PIN_RESET);
  #endif
}

void nbfi_before_rx()
{
  #ifdef PHOBOS_EXT_MODULE
    HAL_GPIO_WritePin(AX_SWITCH_TX_ON_GPIO_Port, AX_SWITCH_TX_ON_Pin,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AX_SWITCH_NTX_ON_GPIO_Port, AX_SWITCH_NTX_ON_Pin,  GPIO_PIN_SET);
  #endif
}

void nbfi_before_off()
{
  #ifdef PHOBOS_EXT_MODULE
    HAL_GPIO_WritePin(AX_SWITCH_TX_ON_GPIO_Port, AX_SWITCH_TX_ON_Pin,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AX_SWITCH_NTX_ON_GPIO_Port, AX_SWITCH_NTX_ON_Pin,  GPIO_PIN_RESET);
  #endif
}


void nbfi_read_default_settings(nbfi_settings_t* settings)
{
  for(uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
  {
    ((uint8_t *)settings)[i] = ((uint8_t *)&nbfi_set_default)[i];
  }
}


#define EEPROM_INT_nbfi_data (DATA_EEPROM_BASE + 1024*5)

void nbfi_read_flash_settings(nbfi_settings_t* settings) 
{
	memcpy((void*)settings, ((const void*)EEPROM_INT_nbfi_data), sizeof(nbfi_settings_t));
}

void nbfi_write_flash_settings(nbfi_settings_t* settings)
{	
    if(HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) return;
    for(uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
    {
		if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_INT_nbfi_data + i, ((uint8_t *)settings)[i]) != HAL_OK) break;
    }
    HAL_FLASHEx_DATAEEPROM_Lock(); 
}

int ADC_get(uint32_t * voltage, uint32_t * temp);
uint32_t nbfi_measure_valtage_or_temperature(uint8_t val)
{
	uint32_t voltage, temp;
	ADC_get(&voltage, &temp);
	return val ? voltage / 10 : temp;
}

uint32_t nbfi_update_rtc()
{
  //you should use this callback when external RTC used
  //return rtc_counter;  
  return 0;
}

void nbfi_rtc_synchronized(uint32_t time)
{
  //you should use this callback for RTC counter correction when external RTC used
  //rtc_counter = time;
  
}


void nbfi_receive_complete(uint8_t * data, uint16_t length)
{

  NBFi_Send(data, length); //loopback
  //uint8_t _data[]={1,2,3,4,5};
  //NBFi_Send(_data, 5);
}

uint8_t nbfi_lock = 1;

void nbfi_lock_unlock_nbfi_irq(uint8_t lock)
{
    nbfi_lock = lock;
}

void nbfi_get_iterator(nbfi_crypto_iterator_t * iter)
{
	//	Read iterator from retain storage
	iter->ul = iter->dl = 0;
}

void nbfi_set_iterator(nbfi_crypto_iterator_t * iter)
{
	//	Write iterator to retain storage
	//	Cause every send/receive packet
}

void radio_init(void)
{
	RADIO_GPIO_Init();

	RADIO_LPTIM_Init();       

	HAL_LPTIM_Counter_Start(&hlptim, 0xffff);

	RADIO_SPI_Init();

	RADIO_BPSK_PIN_Init();

	wa1470_reg_func(WARADIO_ENABLE_GLOBAL_IRQ, (void*)radio_enable_global_irq);
	wa1470_reg_func(WARADIO_DISABLE_GLOBAL_IRQ, (void*)radio_disable_global_irq);
	wa1470_reg_func(WARADIO_ENABLE_IRQ_PIN, (void*)radio_enable_pin_irq);
	wa1470_reg_func(WARADIO_DISABLE_IRQ_PIN, (void*)radio_disable_pin_irq);
	wa1470_reg_func(WARADIO_CHIP_ENABLE, (void*)radio_chip_enable);
	wa1470_reg_func(WARADIO_CHIP_DISABLE, (void*)radio_chip_disable);
	wa1470_reg_func(WARADIO_GET_IRQ_PIN, (void*)radio_get_irq_pin_state);
	wa1470_reg_func(WARADIO_SPI_RX, (void*)radio_spi_rx);
	wa1470_reg_func(WARADIO_SPI_TX, (void*)radio_spi_tx);
	wa1470_reg_func(WARADIO_SPI_TX_RX, (void*)radio_spi_tx_rx);
	wa1470_reg_func(WARADIO_SPI_CS_WRITE, (void*)radio_spi_write_cs);
	wa1470_reg_func(WARADIO_NOP_DELAY_MS, (void*)NOP_Delay_ms);
	wa1470_reg_func(WARADIO_SEND_TO_BPSK_PIN, (void*)radio_bpsk_pin_send);

	wtimer_reg_func(WTIMER_GLOBAL_IRQ_ENABLE, (void*)radio_enable_global_irq);
	wtimer_reg_func(WTIMER_GLOBAL_IRQ_DISABLE, (void*)radio_disable_global_irq);
	wtimer_reg_func(WTIMER_CC_IRQ_ENABLE, (void*)wtimer_cc_irq_enable);
	wtimer_reg_func(WTIMER_CC_IRQ_DISABLE, (void*)wtimer_cc_irq_disable);
	wtimer_reg_func(WTIMER_SET_CC, (void*)wtimer_cc_set);
	wtimer_reg_func(WTIMER_GET_CC, (void*)wtimer_cc_get);
	wtimer_reg_func(WTIMER_GET_CNT, (void*)wtimer_cnt_get);
	wtimer_reg_func(WTIMER_CHECK_CC_IRQ, (void*)wtimer_check_cc_irq);

	wtimer_init();
	nbfi_lock = 0;

	NBFI_reg_func(NBFI_BEFORE_TX, (void*)nbfi_before_tx);
	NBFI_reg_func(NBFI_BEFORE_RX, (void*)nbfi_before_rx);
	NBFI_reg_func(NBFI_BEFORE_OFF, (void*)nbfi_before_off);
	NBFI_reg_func(NBFI_RECEIVE_COMLETE, (void*)nbfi_receive_complete);
	NBFI_reg_func(NBFI_READ_FLASH_SETTINGS, (void*)nbfi_read_flash_settings);
	NBFI_reg_func(NBFI_WRITE_FLASH_SETTINGS, (void*)nbfi_write_flash_settings);
	NBFI_reg_func(NBFI_READ_DEFAULT_SETTINGS, (void*)nbfi_read_default_settings);
	NBFI_reg_func(NBFI_MEASURE_VOLTAGE_OR_TEMPERATURE, (void*)nbfi_measure_valtage_or_temperature);
	NBFI_reg_func(NBFI_GET_ITERATOR, (void*)nbfi_get_iterator);
	NBFI_reg_func(NBFI_SET_ITERATOR, (void*)nbfi_set_iterator);

	//register callbacks when external RTC used
	//NBFI_reg_func(NBFI_UPDATE_RTC, (void*)nbfi_update_rtc);
	//NBFI_reg_func(NBFI_RTC_SYNCHRONIZED, (void*)nbfi_rtc_synchronized);

	nbfi_dev_info_t info = {MODEM_ID, (uint32_t*)KEY, TX_MIN_POWER, TX_MAX_POWER, MANUFACTURER_ID, HARDWARE_TYPE_ID, PROTOCOL_ID, BAND, SEND_INFO_PERIOD};

	NBFi_Config_Set_Device_Info(&info);

	NBFi_Clear_Saved_Configuration(); //if you need to clear previously saved nbfi configuration in EEPROM
	//wa1470_set_freq(868800000);

	wa1470_reg_func(WARADIO_DATA_RECEIVED, (void*)NBFi_MAC_RX_ProtocolD);
	wa1470_reg_func(WARADIO_TX_FINISHED, (void*)NBFi_RF_TX_Finished);
	//wa1470_init(WA1470_SEND_BY_BPSK_PIN/*WA1470_SEND_BY_I_Q_MODULATOR*/, MODEM_ID);
        wa1470_init(WA1470_SEND_BY_I_Q_MODULATOR, MODEM_ID);
	NBFI_Init();  
}
