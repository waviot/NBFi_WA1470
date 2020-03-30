#ifndef RS485_UART_H_
#define RS485_UART_H_
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_uart.h"

void RS485_UART_init(void);
void RS485_UART_deinit(void);
void RS485_UART_IRQ(void);

void RS485_UART_send(uint8_t data);
uint8_t RS485_UART_is_empty(void);
uint8_t RS485_UART_TX_is_empty(void);
uint8_t RS485_UART_get(void);

void set_baud_opto_uart(uint8_t speed_byte);
void RS485_UART_set_baud_parity(uint32_t baud);
void RS485_UART_set_baud_no_parity(uint32_t baud);

#endif /* RS485_UART_H_ */