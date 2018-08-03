
#include "log.h"

#include <stdio.h>

#include "rs485_uart.h"
#include <math.h>
char   log_string[1024];


void log_init(void)
{
        RS485_UART_init();
}

void log_send_str(const char *str)
{
	uint16_t ptr = 0;
	char c;
	while((c = str[ptr++])) RS485_UART_send(c);
	RS485_UART_send(0x0A);
        RS485_UART_send(0x0D); 
}

void log_send_str_len(const char *str, uint16_t len)
{
	uint16_t ptr = 0;
	while(ptr != len) RS485_UART_send(str[ptr++]);
}

/*
void log_send_to_sfmonitor(uint32_t *mas, uint16_t len)
{
  RS485_UART_send(0x12);
  for(uint16_t i = 0; i != len; i++)
  {
   // RS485_UART_send(0x12);
    uint16_t rssi = log10f(mas[i])*20;// - 48 - 192;
    RS485_UART_send((rssi>>8)&0xff);
    RS485_UART_send((rssi>>0)&0xff);
    //RS485_UART_send(0x13);
    RS485_UART_send(0x10);
  }
  RS485_UART_send(0x13);
}*/