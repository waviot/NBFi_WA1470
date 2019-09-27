#ifndef NBFI_CRYPTO_H
#define NBFI_CRYPTO_H

#define NBFI_CRYPTO_KEY_LENGTH 8   // In 32-bit words
#define HASH_RANDOM_SIZE 16
typedef uint32_t*  nbfi_crypto_key_t;

void NBFi_Crypto_Encode(uint8_t* buf);
void NBFi_Crypto_Decode(uint8_t* buf);
void NBFi_Crypto_Set_KEY(uint32_t *key, uint32_t *id);
_Bool NBFi_Crypto_Available();

#endif // NBFI_CRYPTO_H