#include "rs485_uart.h"
#include "xbuf.h"

//			STM defines
#define RS485_USART	 		USART2
#define RS485_USART_IRQ 		USART2_IRQn
#define RS485_RCC_ENABLE		__HAL_RCC_USART2_CLK_ENABLE
#define RS485_RCC_DISABLE		__HAL_RCC_USART2_CLK_DISABLE
#define RS485_TX_GPIO_Port		GPIOA
#define RS485_TX_Pin 			GPIO_PIN_2
#define RS485_TX_AF			GPIO_AF4_USART2
#define RS485_RX_GPIO_Port 		GPIOA
#define RS485_RX_Pin 			GPIO_PIN_3
#define RS485_RX_AF			GPIO_AF4_USART2


static UART_HandleTypeDef huart;

static UART_X_BUF RS485_UART_rx;
static UART_X_BUF RS485_UART_tx;

void RS485_UART_send(uint8_t data){
  	xbuf_send(&RS485_UART_tx, data);
	__HAL_UART_ENABLE_IT(&huart, UART_IT_TXE);
}

uint8_t RS485_UART_is_empty(void){
	return xbuf_is_empty(&RS485_UART_rx);
}

uint8_t RS485_UART_TX_is_empty(void){
	return xbuf_is_empty(&RS485_UART_tx);
}

uint8_t RS485_UART_get(void){
	return xbuf_get(&RS485_UART_rx);
}


void RS485_UART_IRQ(void) {
	if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_RXNE)){
		xbuf_send(&RS485_UART_rx, huart.Instance->RDR);
	}
	else if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_TXE) && (huart.Instance->CR1 & 1 << (UART_IT_TXE & 0x1F))) {
		if (!xbuf_is_empty(&RS485_UART_tx))
			huart.Instance->TDR = xbuf_get(&RS485_UART_tx);
		else
			__HAL_UART_DISABLE_IT(&huart, UART_IT_TXE);		  
	}
	else
		huart.Instance->ICR = 0xFFFFFFFF;
}

void RS485_UART_init(void) {
  	
  GPIO_InitTypeDef GPIO_InitStruct;  

  RS485_RCC_ENABLE();

  huart.Instance = RS485_USART;
  huart.Init.BaudRate = 115200;
  huart.Init.WordLength = UART_WORDLENGTH_8B;
  huart.Init.StopBits = UART_STOPBITS_1;
  huart.Init.Parity = UART_PARITY_NONE;
  huart.Init.Mode = UART_MODE_TX_RX;
  huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart.Init.OverSampling = UART_OVERSAMPLING_16;
  huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart);	
  
  GPIO_InitStruct.Pin = RS485_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = RS485_TX_AF;
  HAL_GPIO_Init(RS485_TX_GPIO_Port, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pin = RS485_RX_Pin;
  GPIO_InitStruct.Alternate = RS485_RX_AF;
  HAL_GPIO_Init(RS485_RX_GPIO_Port, &GPIO_InitStruct);
	
  
  for(uint16_t i = 0; i != 100; i++) huart.Instance->RDR; //wait and clear first received char
    
  __HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&huart, UART_IT_TXE);
	
  HAL_NVIC_SetPriority(RS485_USART_IRQ, 0, 0);
  HAL_NVIC_EnableIRQ(RS485_USART_IRQ);
}

void RS485_UART_deinit(void) {
  	GPIO_InitTypeDef GPIO_InitStruct;  

    GPIO_InitStruct.Pin = RS485_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(RS485_TX_GPIO_Port, &GPIO_InitStruct);
	
    GPIO_InitStruct.Pin = RS485_RX_Pin;
    HAL_GPIO_Init(RS485_RX_GPIO_Port, &GPIO_InitStruct);
	
	RS485_TX_GPIO_Port->BRR |= RS485_TX_Pin;
	RS485_RX_GPIO_Port->BRR |= RS485_RX_Pin;

	HAL_UART_DeInit(&huart);	
  	RS485_RCC_DISABLE();
	
    HAL_NVIC_DisableIRQ(RS485_USART_IRQ);
}

void RS485_UART_set_baud_parity(uint32_t baud){
	while(!__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC));
	__HAL_UART_CLEAR_FLAG(&huart, UART_FLAG_TC);

	huart.Init.BaudRate = baud;
	huart.Init.Parity = UART_PARITY_EVEN;
	HAL_UART_Init(&huart);
	__HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart, UART_IT_TXE);
}

void RS485_UART_set_baud_no_parity(uint32_t baud){
	while(!__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC));
	__HAL_UART_CLEAR_FLAG(&huart, UART_FLAG_TC);

	huart.Init.BaudRate = baud;
	huart.Init.Parity = UART_PARITY_NONE;
	HAL_UART_Init(&huart);
	__HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart, UART_IT_TXE);
}

