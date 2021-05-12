#include <string.h>

#include "crc.h"

#include "update.h"
#include "main.h"

static uint32_t cache[FLASH_PAGE_SIZE / 4]; //2048 byte
/*!
 * \brief erase flash function from habr.com
 *
 * \param pageNumber - no matter in sector by every 2kB
 */
__ramfunc void UpdateInternalFlashErase(unsigned int pageNumber)
{
	__disable_irq();
	if (READ_BIT(FLASH->CR, FLASH_CR_LOCK))
	{
		WRITE_REG(FLASH->KEYR, 0x45670123);
		WRITE_REG(FLASH->KEYR, 0xCDEF89AB);
	}
	CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCEN);
	while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
		;
	if (READ_BIT(FLASH->SR, FLASH_SR_EOP))
	{
		SET_BIT(FLASH->SR, FLASH_SR_EOP);
	}
	SET_BIT(FLASH->CR, FLASH_CR_PER);
	MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((USR_FLASH_PAGE_NUMBER & 0xFFU) << FLASH_CR_PNB_Pos));
	CLEAR_BIT(FLASH->CR, FLASH_CR_MER1);
	SET_BIT(FLASH->CR, FLASH_CR_STRT);
	while (FLASH->SR & FLASH_SR_BSY)
		;
	SET_BIT(FLASH->ACR, FLASH_ACR_DCEN);
	CLEAR_BIT(FLASH->CR, FLASH_CR_PER);

	CLEAR_BIT(FLASH->SR, FLASH_SR_EOP);
	SET_BIT(FLASH->CR, FLASH_CR_LOCK);
	__enable_irq();
}

/*!
 * \brief flash new data to memory
 *
 * \param data pointer to the recorded data
 * \param address address in flash
 * \param count the number of bytes to be written must be a multiple of 2
 */
__ramfunc void UpdateInternalFlashWrite(unsigned char *data, unsigned int address, unsigned int count)
{
	__disable_irq();
	if (READ_BIT(FLASH->CR, FLASH_CR_LOCK))
	{
		WRITE_REG(FLASH->KEYR, 0x45670123);
		WRITE_REG(FLASH->KEYR, 0xCDEF89AB);
	}
	CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCEN);
	while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
		;
	if (READ_BIT(FLASH->SR, FLASH_SR_EOP))
	{
		SET_BIT(FLASH->SR, FLASH_SR_EOP);
	}
	SET_BIT(FLASH->CR, FLASH_CR_PG);
	CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
	for (uint16_t i = 0; i < count; i += 8)
	{
		//		*(volatile uint64_t *)(address + i) = (((uint64_t)data[i + 1]) << 8) + data[i];
		*(volatile uint64_t *)(address + i) = *(uint64_t *)(data + i);
		while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
			;
		FLASH->SR = FLASH_SR_EOP;
	}
	SET_BIT(FLASH->ACR, FLASH_ACR_DCEN);
	CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

	CLEAR_BIT(FLASH->SR, FLASH_SR_EOP);
	SET_BIT(FLASH->CR, FLASH_CR_LOCK);

	__enable_irq();
}

uint8_t update_check(soft_update_t *su)
{
	volatile uint32_t crc32_calc, update_len;
	memcpy_s((uint8_t *)su, sizeof(soft_update_t), (uint8_t *)AQUA3_FLASH_UPDATE_START, sizeof(soft_update_t));

	crc32_calc = CRC_Crc32((uint32_t *)su, sizeof(soft_update_t) - 4, 0xFFFFFFFF);
	if (crc32_calc != su->crc_of_this_struct)
		return UPDATE_CRC_STRUCT_ERROR;

	if (su->end_add >= su->start_add)
		update_len = su->end_add - su->start_add;
	else
		return UPDATE_LEN_ERROR;

	if (su->end_add >= AQUA3_FLASH_UPDATE_END)
		return UPDATE_END_ADDR_ERROR;

	crc32_calc = CRC_Crc32((uint32_t *)su->start_add, update_len, 0xFFFFFFFF);
	if (crc32_calc != su->crc)
		return UPDATE_CRC_ERROR;

	crc32_calc = CRC_Crc32((uint32_t *)AQUA3_FLASH_APP_START, update_len, 0xFFFFFFFF);
	if (crc32_calc != su->crc)
		return UPDATE_CRC_MISMATCH;

	return UPDATE_APP_IS_ACTUAL;
}

__ramfunc uint8_t write_halfpage(uint32_t addr, uint32_t *data)
{
	uint8_t count = 0;

	volatile uint32_t timeout = UPDATE_FLASH_TIMEOUT;
	while ((FLASH->SR & FLASH_SR_BSY) && timeout--)
		;

#ifdef FLASH_PECR_FPRG
	SET_BIT(FLASH->PECR, FLASH_PECR_FPRG);
	SET_BIT(FLASH->PECR, FLASH_PECR_PROG);
#elseif FLASH_CR_FSTPG
	SET_BIT(FLASH->CR, FLASH_CR_FSTPG);
#endif
	__disable_irq();

	while (count < 16)
	{
		*(__IO uint32_t *)addr = *data;
		data++;
		count++;
	}

	__enable_irq();

	timeout = UPDATE_FLASH_TIMEOUT;
	while ((FLASH->SR & FLASH_SR_BSY) && timeout--)
		;

#ifdef FLASH_PECR_FPRG
	CLEAR_BIT(FLASH->PECR, FLASH_PECR_FPRG);
	CLEAR_BIT(FLASH->PECR, FLASH_PECR_PROG);
#elseif FLASH_CR_FSTPG
	CLEAR_BIT(FLASH->CR, FLASH_CR_FSTPG);
#endif

	return 0;
}

uint8_t write_page(uint32_t page, uint32_t *data)
{
	write_halfpage(page, data);
	write_halfpage(page + FLASH_PAGE_SIZE / 2, data + FLASH_PAGE_SIZE / 2 / 4);
	return 0;
}

uint8_t erase_page(uint32_t addr)
{
	CLEAR_BIT(FLASH->SR, FLASH_SR_OPTVERR);
	//__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); // Clear OPTVERR bit set on virgin samples

	volatile uint32_t timeout = UPDATE_FLASH_TIMEOUT;
	while ((FLASH->SR & FLASH_SR_BSY) && timeout--)
		;

	UpdateInternalFlashErase(addr);

	timeout = UPDATE_FLASH_TIMEOUT;
	while ((FLASH->SR & FLASH_SR_BSY) && timeout--)
		;

#ifdef FLASH_PECR_FPRG
	CLEAR_BIT(FLASH->PECR, FLASH_PECR_FPRG);
	CLEAR_BIT(FLASH->PECR, FLASH_PECR_PROG);
#elseif FLASH_CR_FSTPG
	CLEAR_BIT(FLASH->CR, FLASH_CR_FSTPG);
#endif

	return 0;
}

void update_write_page_to_flash(uint16_t page, uint8_t *data)
{
	//	if (HAL_FLASH_Unlock() == HAL_OK)
	//	{
	//		erase_page(page * FLASH_PAGE_SIZE + FLASH_BASE);
	//		write_page(page * FLASH_PAGE_SIZE + FLASH_BASE, (uint32_t *)data);
	//		HAL_FLASH_Lock();
	//	}
	UpdateInternalFlashErase(FLASH_BASE);
	UpdateInternalFlashWrite((unsigned char *)data, FLASH_BASE, FLASH_PAGE_SIZE);
}

uint8_t update_apply(soft_update_t su)
{
	uint16_t page_start = (AQUA3_FLASH_APP_START - FLASH_BASE) / FLASH_PAGE_SIZE;
	uint16_t page_end = (AQUA3_FLASH_UPDATE_START - FLASH_BASE) / FLASH_PAGE_SIZE;

	//	__HAL_RCC_FLASH_CLK_ENABLE();

	for (uint32_t i = su.start_add; i < su.end_add; i += FLASH_PAGE_SIZE)
	{
		memcpy_s(cache, sizeof(cache), (uint8_t *)i, FLASH_PAGE_SIZE);
		if (page_start >= page_end)
			break;
		update_write_page_to_flash(page_start++, (uint8_t*)cache);
	}

	return 0;
}

void update_clear_header(void)
{
	//	__HAL_RCC_FLASH_CLK_ENABLE();
	erase_page(AQUA3_FLASH_UPDATE_START);
}

void update_clear(void)
{
	for (uint32_t i = AQUA3_FLASH_UPDATE_START; i < AQUA3_FLASH_UPDATE_END; i += FLASH_PAGE_SIZE)
		erase_page(i);
}

void update_cpy(void)
{
	uint32_t len = AQUA3_FLASH_UPDATE_END - AQUA3_FLASH_UPDATE_START - AQUA3_FLASH_UPDATE_HEADER_LEN;

	for (uint32_t i = 0; i < len; i += FLASH_PAGE_SIZE)
	{
		memcpy_s(cache, sizeof(cache), (uint8_t *)(AQUA3_FLASH_APP_START + i), FLASH_PAGE_SIZE);
		erase_page(AQUA3_FLASH_UPDATE_START + AQUA3_FLASH_UPDATE_HEADER_LEN + i);
		write_page(AQUA3_FLASH_UPDATE_START + AQUA3_FLASH_UPDATE_HEADER_LEN + i, (uint32_t *)cache);
	}
}
