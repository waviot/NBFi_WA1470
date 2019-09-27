#ifndef _STRIBOG_H
#define _STRIBOG_H

#include <stdint.h>

#define STRIBOG_BLOCK_SIZE		64
#define STRIBOG_BLOCK_BIT_SIZE 	(STRIBOG_BLOCK_SIZE * 8)

#define STRIBOG_OUTPUT_SIZE_512	64
#define STRIBOG_OUTPUT_SIZE_256	32

#define STRIBOG_HASH256			0
#define STRIBOG_HASH512			1

typedef struct
{
	uint8_t h[STRIBOG_BLOCK_SIZE];
	uint8_t N[STRIBOG_BLOCK_SIZE];
	uint8_t S[STRIBOG_BLOCK_SIZE];

	uint8_t size;
} stribog_ctx_t;

void stribog_calc(stribog_ctx_t *ctx, const uint8_t *message, uint64_t len);
void stribog_init(stribog_ctx_t *ctx, uint8_t size);

#endif
