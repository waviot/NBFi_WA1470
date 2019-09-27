#ifndef _MAGMA_H_
#define _MAGMA_H_

#include <stdint.h>

#define MAGMA_KEY_SIZE		32
#define MAGMA_BLOCK_COUNT	32
#define MAGMA_BLOCK_SIZE	4
#define MAGMA_ADD_KEY_SIZE	8
#define MAGMA_DATA_SIZE		8

typedef struct
{
	uint8_t out[MAGMA_DATA_SIZE];
	uint8_t key_add1[MAGMA_ADD_KEY_SIZE];
	uint8_t key_add2[MAGMA_ADD_KEY_SIZE];
	uint8_t key_orig[MAGMA_KEY_SIZE];
} magma_ctx_t;

void Magma_Init(magma_ctx_t *ctx, const uint8_t *key);
void Magma_ECB_enc(magma_ctx_t *ctx, const uint8_t *blk);
void Magma_ECB_dec(magma_ctx_t *ctx, const uint8_t *blk);
void Magma_MIC(magma_ctx_t *ctx, const uint8_t *blk[], uint8_t blk_len, uint8_t padded);

#endif
