#include "nbfi_crypto.h"
#include <string.h>
#include "magma.h"

static magma_ctx_t key_root_ctx;
static magma_ctx_t key_ul_master_ctx, key_ul_mic_ctx, key_ul_work_ctx;
static magma_ctx_t key_dl_master_ctx, key_dl_mic_ctx, key_dl_work_ctx;
static uint8_t inited;

static uint32_t NBFi_Crypto_MIC(magma_ctx_t *ctx, const uint8_t *buf, uint8_t len)
{
	uint32_t mic;

	Magma_MIC(ctx, buf, len);
	memcpy((uint8_t *)&mic, ctx->out, 4);

	return mic;
}

void NBFi_Crypto_Encode(uint8_t *buf, uint32_t modem_id, uint32_t crypto_iter, uint8_t len)
{
	uint8_t iv[MAGMA_DATA_SIZE];
	memcpy(&iv[0], &modem_id, 4);
	memcpy(&iv[4], &crypto_iter, 4);
	Magma_CTR(&key_ul_work_ctx, buf, iv, buf, len);
}

void NBFi_Crypto_Decode(uint8_t *buf, uint32_t modem_id, uint32_t crypto_iter, uint8_t len)
{
	uint8_t iv[MAGMA_DATA_SIZE];
	memcpy(&iv[0], &modem_id, 4);
	memcpy(&iv[4], &crypto_iter, 4);
	Magma_CTR(&key_dl_work_ctx, buf, iv, buf, len);
}

uint32_t NBFi_Crypto_UL_MIC(const uint8_t *buf, const uint8_t len)
{
	return NBFi_Crypto_MIC(&key_ul_mic_ctx, buf, len);
}

uint32_t NBFi_Crypto_DL_MIC(const uint8_t *buf, const uint8_t len)
{
	return NBFi_Crypto_MIC(&key_dl_mic_ctx, buf, len);
}

_Bool NBFi_Crypto_Available()
{
	return inited ? 1 : 0;
}

void NBFi_Crypto_Set_KEY(uint32_t *key, uint32_t *id)
{
	uint8_t blk[MAGMA_KEY_SIZE], out[MAGMA_KEY_SIZE], iv[MAGMA_DATA_SIZE];
	Magma_Init(&key_root_ctx, (uint8_t *)key);
	memset(blk, 0, MAGMA_KEY_SIZE);

	memset(iv, 0x00, MAGMA_DATA_SIZE);
	Magma_CTR(&key_root_ctx, blk, iv, out, MAGMA_KEY_SIZE);
	Magma_Init(&key_ul_master_ctx, out);

	memset(iv, 0xFF, MAGMA_DATA_SIZE / 2);
	Magma_CTR(&key_root_ctx, blk, iv, out, MAGMA_KEY_SIZE);
	Magma_Init(&key_dl_master_ctx, out);

	memset(iv, 0x00, MAGMA_DATA_SIZE);
	Magma_CTR(&key_ul_master_ctx, blk, iv, out, MAGMA_KEY_SIZE);
	Magma_Init(&key_ul_mic_ctx, out);
	Magma_CTR(&key_dl_master_ctx, blk, iv, out, MAGMA_KEY_SIZE);
	Magma_Init(&key_dl_mic_ctx, out);

	memset(iv, 0xFF, MAGMA_DATA_SIZE / 2);
	Magma_CTR(&key_ul_master_ctx, blk, iv, out, MAGMA_KEY_SIZE);
	Magma_Init(&key_ul_work_ctx, out);
	Magma_CTR(&key_dl_master_ctx, blk, iv, out, MAGMA_KEY_SIZE);
	Magma_Init(&key_dl_work_ctx, out);	
	inited = 1;
}
