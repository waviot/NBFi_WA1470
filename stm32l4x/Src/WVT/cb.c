#include <string.h>
#include "cb.h"
#include "crc.h"
#include "rtc.h"
#include "update.h"

extern uint8_t mode;
//extern uint32_t blinks, update_blinks,
extern uint32_t update_display;
extern struct wtimer_desc led_desc;
extern main_par_t main_par;

uint32_t water7_rfl_patch0(uint32_t data);
uint32_t water7_rfl_patch1(uint32_t data);
uint32_t water7_rfl_patch2(uint32_t data);

errno_t water7set_data(int32_t *data)
{
	meter_set(&data[WATER7_USERDATA_OFFSET], METER_DATA);
	meter_set(&data[WATER7_USERPARAMS_OFFSET], METER_PARAMS);
	return memcpy_s((uint8_t *)&main_par, sizeof(main_par_t), (uint8_t *)&data[WATER7_USERPARAMS_OFFSET * 2], sizeof(main_par_t));
}

errno_t water7get_data(int32_t *data)
{
	meter_get(&data[WATER7_USERDATA_OFFSET], METER_DATA);
	meter_get(&data[WATER7_USERPARAMS_OFFSET], METER_PARAMS);
	return memcpy_s((uint8_t *)&data[WATER7_USERPARAMS_OFFSET * 2], sizeof(main_par_t), (uint8_t *)&main_par, sizeof(main_par_t));
	//WVT_EEPROM_LoadAll(data, EEPROM_WATER7_PARAMS);
}

errno_t water7save_data(int32_t *data)
{
	/// \todo check this
	return WVT_EEPROM_SaveAll(data, EEPROM_WATER7_PARAMS);
}

void get_saved_param(water7_params_str *water7_params_p, meter_params_str *meter_params_p)
{
	/// \todo check this
	//water7get_data((int32_t *)water7_params_p);
	WVT_EEPROM_LoadAll(water7_params_p, EEPROM_WATER7_PARAMS);
}

int32_t update_write_cached(uint32_t addr, uint32_t len, uint8_t *data)
{
	/// \todo check this
	static uint8_t cache[FLASH_PAGE_SIZE];
	static uint16_t page_prev;

	if (addr || len || data)
	{
		uint16_t page = (addr - FLASH_BASE) / FLASH_PAGE_SIZE;
		uint16_t page_next = (addr + len - FLASH_BASE) / FLASH_PAGE_SIZE;

		if (addr < AQUA3_FLASH_UPDATE_START || (addr + len) >= AQUA3_FLASH_UPDATE_END || len > FLASH_PAGE_SIZE)
			return RFL_ERROR;

		if (!page_prev)
		{
			memcpy_s((uint8_t *)cache, sizeof(cache), (uint8_t *)(page * FLASH_PAGE_SIZE + FLASH_BASE), FLASH_PAGE_SIZE);
		}
		else if (page != page_prev)
		{
			update_write_page_to_flash(page_prev, cache);
			memcpy_s((uint8_t *)cache, sizeof(cache), (uint8_t *)(page * FLASH_PAGE_SIZE + FLASH_BASE), FLASH_PAGE_SIZE);
		}

		if (page != page_next)
		{
			uint16_t len_first = FLASH_PAGE_SIZE - addr % FLASH_PAGE_SIZE;
			uint16_t len_second = len - len_first;

			memcpy_s((uint8_t *)&cache[addr % FLASH_PAGE_SIZE], sizeof(cache) - addr % FLASH_PAGE_SIZE, (uint8_t *)data, len_first);
			update_write_page_to_flash(page, cache);
			memcpy_s((uint8_t *)cache, sizeof(cache), (uint8_t *)(page_next * FLASH_PAGE_SIZE + FLASH_BASE), FLASH_PAGE_SIZE);
			memcpy_s((uint8_t *)cache, sizeof(cache), (uint8_t *)&data[len_first], len_second);
		}
		else
		{
			memcpy_s((uint8_t *)&cache[addr % FLASH_PAGE_SIZE], sizeof(cache) - addr % FLASH_PAGE_SIZE, (uint8_t *)data, len);
		}
		page_prev = page_next;
	}
	else if (page_prev)
	{
		//	clearing cache
		update_write_page_to_flash(page_prev, cache);
		page_prev = 0;
	}
	else
		return RFL_ERROR;

	return RFL_ERROR_OK;
}

int32_t water7_rfl(uint32_t addr, uint32_t len, uint32_t index, uint8_t *data, uint8_t cmd)
{
	/// \todo fix this
	static uint32_t _index;
	soft_update_t su;
	int32_t ret;

	switch (cmd)
	{
	case RFL_CMD_WRITE_HEX:
		return update_write_cached(addr, len, data);
	case RFL_CMD_WRITE_HEX_INDEX:
		if (index != _index + 1 || update_write_cached(addr, len, data) == RFL_ERROR)
			return _index;
		_index = index;
		return RFL_ERROR_INDEX_OK;
	case RFL_CMD_READ_HEX:
		update_write_cached(0, 0, 0);
		if (addr > FLASH_BASE && (addr + len) < (FLASH_BASE + 128 * 1024))
		{
			memcpy_s(data, len, (uint8_t *)addr, len);
			return RFL_ERROR_OK;
		}
		return RFL_ERROR;
	case RFL_CMD_CLEAR_CACHE:
		return update_write_cached(0, 0, 0);
	case RFL_CMD_CHECK_UPDATE:
		update_write_cached(0, 0, 0);
		return update_check(&su);
	case RFL_CMD_GET_CRC:
		update_write_cached(0, 0, 0);
		if (addr > FLASH_BASE && (addr + len) < (FLASH_BASE + 128 * 1024))
			return CRC_Crc32((uint32_t *)addr, len, 0xFFFFFFFF);
		return RFL_ERROR;
	case RFL_CMD_SOFT_RESET:
		update_write_cached(0, 0, 0);
		NVIC_SystemReset();
		break;
	case RFL_CMD_MASS_ERASE:
		update_clear();
		return RFL_ERROR_OK;
	case RFL_CMD_CPY_ACTUAL:
		update_cpy();
		return RFL_ERROR_OK;
	case RFL_CMD_GET_INDEX:
		return _index;
	case RFL_CMD_CLEAR_INDEX:
		_index = 0;
		return RFL_ERROR_OK;
	case RFL_CMD_GET_VERSION:
		ret = HW_VERSION;
		ret <<= 8;
		ret |= HW_SUB_VERSION;
		ret <<= 8;
		ret |= FW_VERSION;
		ret <<= 8;
		ret |= FW_SUB_VERSION;
		return ret;
	case RFL_CMD_EXEC_PATCH0:
		return water7_rfl_patch0(addr);
	case RFL_CMD_EXEC_PATCH1:
		return water7_rfl_patch1(addr);
	case RFL_CMD_EXEC_PATCH2:
		return water7_rfl_patch2(addr);
	default:
		break;
	}
	return RFL_ERROR_NO_CMD;
}

void meter_inc_cb(void)
{
//	if (main_par.work_mode == MODE_TEST)
//	{
//		static uint32_t prev, mod;
//		meter_data_str meter_data;
//		meter_get(&meter_data, METER_DATA);
//
//		if (meter_data.microliter[METER_FLOW_FORWARD] >= prev)
//			mod += meter_data.microliter[METER_FLOW_FORWARD] - prev;
//		else
//			mod += meter_data.microliter[METER_FLOW_FORWARD] + (1000000 - prev);
//		prev = meter_data.microliter[METER_FLOW_FORWARD];
//
//		// if (mod > LED_BLINK_VOLUME)
//		// {
//		// 	mod -= LED_BLINK_VOLUME;
//		// 	update_blinks = 1;
//		// 	blinks++;
//		// }
//	}
//	// if (update_display >= UPDATE_DISPLAY_TIMEOUT)
//	// 	update_display = 0;
//	// else
//	// 	update_display = 1;
}

uint32_t water7_rfl_patch0(uint32_t data)
{
	volatile uint32_t tmp = data;
	tmp += 1;
	return tmp;
}

uint32_t water7_rfl_patch1(uint32_t data)
{
	volatile uint32_t tmp = data;
	tmp += 2;
	return tmp;
}

uint32_t water7_rfl_patch2(uint32_t data)
{
	volatile uint32_t tmp = data;
	tmp += 4;
	return tmp;
}
