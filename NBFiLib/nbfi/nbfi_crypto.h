#ifndef NBFI_CRYPTO_H
#define NBFI_CRYPTO_H

void NBFi_Crypto_Encode(uint8_t* buf);

void NBFi_Crypto_Decode(uint8_t* buf);

void NBFi_Crypto_OFB(uint8_t* buf, uint8_t len, uint8_t* ID, uint8_t iter);

_Bool NBFi_Crypto_Available();

void NBFi_Crypto_Set_KEY(uint32_t* key);

void NBFi_Crypto_Set_KEY_Ptr(uint32_t* ptr);

#endif // NBFI_CRYPTO_H