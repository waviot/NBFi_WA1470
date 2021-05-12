#ifndef _XBUF_H_
#define _XBUF_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_BUFFER_SIZE (1024)

typedef struct {
	uint8_t buf[UART_BUFFER_SIZE];
	uint16_t wr;
	uint16_t rd;
	uint16_t len;
} UART_X_BUF;

void xbuf_clear(UART_X_BUF *xbuf);
uint8_t xbuf_is_empty(UART_X_BUF *xbuf);
uint8_t xbuf_get(UART_X_BUF *xbuf);
void xbuf_send(UART_X_BUF *xbuf, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // _UART_BUF_H_
