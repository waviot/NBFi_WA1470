#include "nbfi_crypto.h"
#include "magma.h"
#include "stribog.h"
#include <string.h>

static magma_ctx_t mic_ctx, app_ctx;
static uint8_t inited;

static const uint8_t mic_random[] = 
{
	0x9f, 0x91, 0xa7, 0x6e, 0x86, 0x6d, 0x25, 0xe2, 
	0x08, 0xbf, 0x57, 0x85, 0x85, 0x43, 0x63, 0x56
};

static const uint8_t app_random[] = 
{
 	0x51, 0xc6, 0xb8, 0x85, 0xf0, 0x21, 0x31, 0xc3, 
	0xdd, 0xf2, 0x93, 0x67, 0x1b, 0x43, 0x40, 0x67
};

void NBFi_Crypto_Encode(uint8_t * buf)
{
  	Magma_ECB_enc(&app_ctx, buf);
	memcpy(buf, app_ctx.out, MAGMA_DATA_SIZE);
}

void NBFi_Crypto_Decode(uint8_t * buf)
{
  	Magma_ECB_dec(&app_ctx, buf);
	memcpy(buf, app_ctx.out, MAGMA_DATA_SIZE);
}

_Bool NBFi_Crypto_Available()
{
	return inited ? 1 : 0;
}

void NBFi_Crypto_Set_KEY(uint32_t *key, uint32_t *id)
{
  	uint8_t hash_data[STRIBOG_BLOCK_SIZE];
	stribog_ctx_t hash_ctx;
	
	memset(hash_data, 0x00, STRIBOG_BLOCK_SIZE);
	memcpy(hash_data, key, MAGMA_KEY_SIZE);
	memcpy(&hash_data[MAGMA_KEY_SIZE], (uint8_t *)id, 4);
	  
	memcpy(&hash_data[MAGMA_KEY_SIZE + 4], mic_random, HASH_RANDOM_SIZE);
	stribog_init(&hash_ctx, STRIBOG_OUTPUT_SIZE_256);
	stribog_calc(&hash_ctx, hash_data, STRIBOG_BLOCK_SIZE);
	Magma_Init(&mic_ctx, hash_ctx.h);
	
	memcpy(&hash_data[MAGMA_KEY_SIZE + 4], app_random, HASH_RANDOM_SIZE);
	stribog_init(&hash_ctx, STRIBOG_OUTPUT_SIZE_256);
	stribog_calc(&hash_ctx, hash_data, STRIBOG_BLOCK_SIZE);
	Magma_Init(&app_ctx, hash_ctx.h);
	
	inited = 1;
}
