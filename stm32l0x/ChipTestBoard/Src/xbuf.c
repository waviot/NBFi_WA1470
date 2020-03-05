#include <xbuf.h>

void xbuf_clear(UART_X_BUF *xbuf) {
	xbuf->rd = xbuf->wr;
	xbuf->len = 0;
}

uint8_t xbuf_is_empty(UART_X_BUF *xbuf) {
	return (xbuf->wr == xbuf->rd ? 1 : 0);
}

uint8_t xbuf_get(UART_X_BUF *xbuf) {
	if (xbuf_is_empty(xbuf))
		return 0;

	uint8_t data = xbuf->buf[xbuf->rd];
	if (++xbuf->rd >= OPTO_BUFFER_SIZE) {
		xbuf->rd = 0;
	}

	return data;
}

void xbuf_send(UART_X_BUF *xbuf, uint8_t data) {
  	if (xbuf->wr != xbuf->rd - 1)
	{
		xbuf->buf[xbuf->wr] = data;
		if (++xbuf->wr >= OPTO_BUFFER_SIZE) {
			xbuf->wr = 0;
		}
		xbuf->len++;
	}
}
