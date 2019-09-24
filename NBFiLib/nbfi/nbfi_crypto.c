#include "nbfi_crypto.h"

#include "xtea.h"

#define NBFI_CRYPTO_KEY_LENGTH 8   // In 32-bit words

typedef  uint32_t*  nbfi_crypto_key_t;

uint32_t nbfi_crypto_local_key[8] = {0,0,0,0,0,0,0,0};

nbfi_crypto_key_t nbfi_crypto_key_ptr = (uint32_t *)nbfi_crypto_local_key;


void NBFi_Crypto_Encode(uint8_t * buf)
{
    XTEA_Encode((unsigned long *)buf, 2, nbfi_crypto_key_ptr);
    XTEA_Encode((unsigned long *)buf, 2, nbfi_crypto_key_ptr + 4);
}

void NBFi_Crypto_Decode(uint8_t * buf)
{
    XTEA_Decode((unsigned long *)buf, 2, nbfi_crypto_key_ptr + 4);
    XTEA_Decode((unsigned long *)buf, 2, nbfi_crypto_key_ptr);
}

void NBFi_Crypto_OFB(uint8_t* buf, uint8_t len, uint8_t* ID, uint8_t iter)
{
 uint8_t vector[8];
 for(uint8_t i = 0; i != 3; i++)
 {
    vector[i] = 0;
    vector[i+5] = ID[i];
 }
 vector[3] = 0;
 vector[4] = iter;

 uint8_t n = 0;// number of cyphered bytes

 while(n < len)
 {

  if((n % 8) == 0) NBFi_Crypto_Encode(vector); // next block

  buf[n] = vector[n%8] ^ buf[n];
  n++;
 }
}


_Bool NBFi_Crypto_Available()
{
    // Check if KEY area not empty
    for(int i = 0; i < NBFI_CRYPTO_KEY_LENGTH; i++)
    {
        if(((nbfi_crypto_key_t)nbfi_crypto_key_ptr)[i] != 0) return 1;
    }
    return 0;
}

void NBFi_Crypto_Set_KEY(uint32_t* key)
{
  for(int i = 0; i != NBFI_CRYPTO_KEY_LENGTH; i++) nbfi_crypto_local_key[i] = key[i];
  nbfi_crypto_key_ptr = (uint32_t *)nbfi_crypto_local_key;
}

void NBFi_Crypto_Set_KEY_Ptr(uint32_t* ptr)
{
  nbfi_crypto_key_ptr = ptr;
}

