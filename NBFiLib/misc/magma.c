#include "magma.h"

#include <string.h>
#include <stdint.h>

static const unsigned char Pi[8][16] =
{
	{0x01, 0x07, 0x0e, 0x0d, 0x00, 0x05, 0x08, 0x03, 0x04, 0x0f, 0x0a, 0x06, 0x09, 0x0c, 0x0b, 0x02},
	{0x08, 0x0e, 0x02, 0x05, 0x06, 0x09, 0x01, 0x0c, 0x0f, 0x04, 0x0b, 0x00, 0x0d, 0x0a, 0x03, 0x07},
	{0x05, 0x0d, 0x0f, 0x06, 0x09, 0x02, 0x0c, 0x0a, 0x0b, 0x07, 0x08, 0x01, 0x04, 0x03, 0x0e, 0x00},
	{0x07, 0x0f, 0x05, 0x0a, 0x08, 0x01, 0x06, 0x0d, 0x00, 0x09, 0x03, 0x0e, 0x0b, 0x04, 0x02, 0x0c},
	{0x0c, 0x08, 0x02, 0x01, 0x0d, 0x04, 0x0f, 0x06, 0x07, 0x00, 0x0a, 0x05, 0x03, 0x0e, 0x09, 0x0b},
	{0x0b, 0x03, 0x05, 0x08, 0x02, 0x0f, 0x0a, 0x0d, 0x0e, 0x01, 0x07, 0x04, 0x0c, 0x09, 0x06, 0x00},
	{0x06, 0x08, 0x02, 0x03, 0x09, 0x0a, 0x05, 0x0c, 0x01, 0x0e, 0x04, 0x07, 0x0b, 0x0d, 0x00, 0x0f},
	{0x0c, 0x04, 0x06, 0x02, 0x0a, 0x05, 0x0b, 0x09, 0x0e, 0x08, 0x0d, 0x07, 0x00, 0x03, 0x0f, 0x01}
};

static void Xor_32(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
	for (uint8_t i = 0; i < 4; i++)
		c[i] = a[i] ^ b[i];
}

static void Xor_64(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
	for (uint8_t i = 0; i < 8; i++)
		c[i] = a[i] ^ b[i];
}

static void Add_32(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
	uint16_t internal = 0;
	for (int8_t i = 3; i >= 0; i--)
	{
		internal = a[i] + b[i] + (internal >> 8);
		c[i] = internal & 0xff;
	}
}

static void T(const uint8_t *in_data, uint8_t *out_data)
{
	uint8_t first_part_byte, sec_part_byte;

	for (uint8_t i = 0; i < MAGMA_BLOCK_SIZE; i++)
	{
		first_part_byte = (in_data[i] & 0xf0) >> 4;
		sec_part_byte = (in_data[i] & 0x0f);
		first_part_byte = Pi[i * 2][first_part_byte];
		sec_part_byte = Pi[i * 2 + 1][sec_part_byte];
		out_data[i] = (first_part_byte << 4) | sec_part_byte;
	}
}

static void g(const uint8_t *k, const uint8_t *a, uint8_t *out_data)
{
	uint8_t internal[MAGMA_BLOCK_SIZE];
	uint32_t out_data_32;

	Add_32(a, k, internal);

	T(internal, internal);

	out_data_32 = internal[0];
	out_data_32 = (out_data_32 << 8) + internal[1];
	out_data_32 = (out_data_32 << 8) + internal[2];
	out_data_32 = (out_data_32 << 8) + internal[3];

	out_data_32 = (out_data_32 << 11)|(out_data_32 >> 21);

	out_data[3] = out_data_32;
	out_data[2] = out_data_32 >> 8;
	out_data[1] = out_data_32 >> 16;
	out_data[0] = out_data_32 >> 24;
}

static void G(const uint8_t *k, const uint8_t *a, uint8_t *out_data)
{
	uint8_t a_0[MAGMA_BLOCK_SIZE];
	uint8_t a_1[MAGMA_BLOCK_SIZE];
	uint8_t G[MAGMA_BLOCK_SIZE];

	memcpy(a_0, a + MAGMA_BLOCK_SIZE, MAGMA_BLOCK_SIZE);
	memcpy(a_1, a, MAGMA_BLOCK_SIZE);

	g(k, a_0, G);
	Xor_32(a_1, G, G);

	memcpy(out_data, a_0, MAGMA_BLOCK_SIZE);
	memcpy(out_data + MAGMA_BLOCK_SIZE, G, MAGMA_BLOCK_SIZE);
}

static void G_Fin(const uint8_t *k, const uint8_t *a, uint8_t *out_data)
{
	uint8_t a_0[MAGMA_BLOCK_SIZE];
	uint8_t a_1[MAGMA_BLOCK_SIZE];
	uint8_t G[MAGMA_BLOCK_SIZE];

	memcpy(a_0, a + MAGMA_BLOCK_SIZE, MAGMA_BLOCK_SIZE);
	memcpy(a_1, a, MAGMA_BLOCK_SIZE);

	g(k, a_0, G);
	Xor_32(a_1, G, G);

	memcpy(out_data, G, MAGMA_BLOCK_SIZE);
	memcpy(out_data + MAGMA_BLOCK_SIZE, a_0, MAGMA_BLOCK_SIZE);
}

static void shl_64(uint8_t *data, uint8_t i)
{
	uint64_t tmp = data[0];

	for (uint8_t i = 1; i < 8; i++)
		tmp = (tmp << 8) + data[i];
	
	tmp <<= i;
	
	for (uint8_t i = 0; i < 8; i++)
		data[i] = tmp >> (56 - i * 8);
}

void Magma_Init(magma_ctx_t *ctx, const uint8_t *key)
{
	uint8_t data[MAGMA_DATA_SIZE];

	memcpy(ctx->key_orig, key, MAGMA_KEY_SIZE);

	memset(data, 0x00, MAGMA_DATA_SIZE);
	Magma_ECB_enc(ctx, data);

	if (ctx->out[0] & 0x80)
	{
		shl_64(ctx->out, 1);
		ctx->out[7] ^= 0x33;
	}
	else
		shl_64(ctx->out, 1);
	memcpy(ctx->key_add1, ctx->out, MAGMA_ADD_KEY_SIZE);

	if (ctx->out[0] & 0x80)
	{
		shl_64(ctx->out, 1);
		ctx->out[7] ^= 0x33;
	}
	else
		shl_64(ctx->out, 1);
	memcpy(ctx->key_add2, ctx->out, MAGMA_ADD_KEY_SIZE);
}

static uint8_t *get_iter_key(uint8_t *key, uint8_t iter)
{
	if (iter < 24)
		return key + ((iter & 0x7) << 2);
	else
		return key + ((7 - (iter & 0x7)) << 2);
}

void Magma_ECB_enc(magma_ctx_t *ctx, const uint8_t *blk)
{
	if (blk != ctx->out)
		memcpy(ctx->out, blk, MAGMA_DATA_SIZE);

	for(uint8_t i = 0; i < 31; i++)
		G(get_iter_key(ctx->key_orig, i), ctx->out, ctx->out);
	G_Fin(get_iter_key(ctx->key_orig, 31), ctx->out, ctx->out);
}

void Magma_ECB_dec(magma_ctx_t *ctx, const uint8_t *blk)
{
	if (blk != ctx->out)
		memcpy(ctx->out, blk, MAGMA_DATA_SIZE);

	for(uint8_t i = 31; i > 0; i--)
		G(get_iter_key(ctx->key_orig, i), ctx->out, ctx->out);
	G_Fin(get_iter_key(ctx->key_orig, 0), ctx->out, ctx->out);
}

void Magma_MIC(magma_ctx_t *ctx, const uint8_t *blk[], uint8_t blk_len, uint8_t padded)
{
	memset(ctx->out, 0x00, MAGMA_DATA_SIZE);
	for(uint8_t i = 0; i < blk_len; i++)
	{
		Xor_64(ctx->out, blk[i], ctx->out);
		
		if (i == blk_len - 1)
		{
			if (padded) 
				Xor_64(ctx->out, ctx->key_add2, ctx->out);
			else
				Xor_64(ctx->out, ctx->key_add1, ctx->out);
		}
		Magma_ECB_enc(ctx, ctx->out);
	}
}
