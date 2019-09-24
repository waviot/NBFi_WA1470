#ifndef XTEA_H
#define XTEA_H

typedef  uint32_t*  xtea_key_t;

void XTEA_Encode(unsigned long * data, unsigned char dataLength, xtea_key_t key);

void XTEA_Decode(unsigned long * data, unsigned char dataLength, xtea_key_t key);

#endif
