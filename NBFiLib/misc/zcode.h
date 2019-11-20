#ifndef ZCODE_H
#define ZCODE_H


#define ZCODE_LEN 16

#define ZCODE_E_LEN 32

void ZCODE_Append(uint8_t * src_buf, uint8_t * dst_buf, _Bool parity);

void ZCODE_E_Append(uint8_t * src_buf, uint8_t * dst_buf, _Bool parity);


#endif // ZCODE_H
