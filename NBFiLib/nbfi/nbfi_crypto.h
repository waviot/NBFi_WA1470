#ifndef NBFI_CRYPTO_H
#define NBFI_CRYPTO_H

#define KEY_SCAN_DEPTH		100
#define CRYPTO_ITER_SIZE	20	//	bits

void NBFi_Crypto_Set_KEY(uint32_t *key, uint32_t *ul_iter, uint32_t *dl_iter);
void NBFi_Crypto_Encode(uint8_t *buf, uint32_t modem_id, uint32_t crypto_iter, uint8_t len);
void NBFi_Crypto_Decode(uint8_t *buf, uint32_t modem_id, uint32_t crypto_iter, uint8_t len);
uint8_t NBFI_Crypto_mic_check(uint8_t *buf, uint8_t len, uint8_t *mic, uint32_t *iter_int, uint8_t iter);
uint32_t NBFi_Crypto_UL_MIC(const uint8_t *buf, const uint8_t len);
uint32_t NBFi_Crypto_DL_MIC(const uint8_t *buf, const uint8_t len);
uint32_t NBFI_Crypto_inc_iter(uint32_t iter);
uint8_t NBFi_Crypto_Available();

#endif // NBFI_CRYPTO_H
