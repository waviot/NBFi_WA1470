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

uint8_t NBFi_Crypto_Available()
{
	return inited;
}

uint8_t NBFI_Crypto_mic_check(uint8_t *buf, uint8_t len, uint8_t *mic, uint32_t *iter_int, uint8_t iter)
{
	magma_ctx_t tmp_ctx_mic, tmp_ctx_master;
	uint32_t mic_calced;

	if (!(*iter_int && ((*iter_int & 0xFF) >=  iter)))
	{
		mic_calced = NBFi_Crypto_DL_MIC(buf, len);

		if (((uint8_t *)&mic_calced)[2] == mic[0] &&
			((uint8_t *)&mic_calced)[1] == mic[1] &&
			((uint8_t *)&mic_calced)[0] == mic[2])
		{
			*iter_int &= 0xFFFFFF00;
			*iter_int += iter;
			
			return 1;
		}
	}

	Magma_Init(&tmp_ctx_master, key_dl_master_ctx.key_orig);

	for (uint32_t i = 0; i < KEY_SCAN_DEPTH; i++)
	{
		Magma_KEY_mesh(&tmp_ctx_master, &tmp_ctx_master, 0x0F);
		Magma_KEY_mesh(&tmp_ctx_master, &tmp_ctx_mic, 0x00);

		mic_calced = NBFi_Crypto_MIC(&tmp_ctx_mic, buf, len);
		if (((uint8_t *)&mic_calced)[2] == mic[0] &&
			((uint8_t *)&mic_calced)[1] == mic[1] &&
			((uint8_t *)&mic_calced)[0] == mic[2])
		{
			Magma_Init(&key_dl_master_ctx, tmp_ctx_master.key_orig);
			Magma_Init(&key_dl_mic_ctx, tmp_ctx_mic.key_orig);
			Magma_KEY_mesh(&key_dl_master_ctx, &key_dl_work_ctx, 0xFF);

			*iter_int &= 0xFFFFFF00;
			*iter_int += ((i + 1) << 8) + iter;
			
			return 1;
		}
	}

	return 0;
}

uint32_t NBFI_Crypto_inc_iter(uint32_t iter)
{
	iter++;
	if (!(iter & 0xFF))
	{
		Magma_KEY_mesh(&key_ul_master_ctx, &key_ul_master_ctx, 0x0F);
		Magma_KEY_mesh(&key_ul_master_ctx, &key_ul_mic_ctx, 0x00);
		Magma_KEY_mesh(&key_ul_master_ctx, &key_ul_work_ctx, 0xFF);		
	}
	
	return iter;
}

void NBFi_Crypto_Set_KEY(uint32_t *key, uint32_t ul_iter, uint32_t dl_iter)
{
	Magma_Init(&key_root_ctx, (uint8_t *)key);

	Magma_KEY_mesh(&key_root_ctx, &key_ul_master_ctx, 0x00);
	Magma_KEY_mesh(&key_root_ctx, &key_dl_master_ctx, 0xFF);
	
	for (uint32_t i = 0; i < (ul_iter >> 8); i++)
		Magma_KEY_mesh(&key_ul_master_ctx, &key_ul_master_ctx, 0x0F);
	for (uint32_t i = 0; i < (dl_iter >> 8); i++)
		Magma_KEY_mesh(&key_dl_master_ctx, &key_dl_master_ctx, 0x0F);
	
	Magma_KEY_mesh(&key_ul_master_ctx, &key_ul_mic_ctx, 0x00);
	Magma_KEY_mesh(&key_dl_master_ctx, &key_dl_mic_ctx, 0x00);
	Magma_KEY_mesh(&key_ul_master_ctx, &key_ul_work_ctx, 0xFF);
	Magma_KEY_mesh(&key_dl_master_ctx, &key_dl_work_ctx, 0xFF);

	inited = 1;
}
