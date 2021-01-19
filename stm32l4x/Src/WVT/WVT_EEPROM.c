/*!
 * \file WVT_EEPROM.c
 * \author Sergei Savkin (ssavkin@waviot.ru)
 * \brief save data to eeprom or flash or backup registers
 * \version 0.1
 * \date 10-07-2020
 *
 * \copyright WAVIoT 2020
 *
 */
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "crc.h"
#include "main.h"
#include "nbfi_types.h"
#include "rtc.h"
#include "water7.h"
#include "update.h"
#include "WVT_EEPROM.h"

/*!
 * \brief Chose last sector
 * !!! Be careful the user area should be in another bank than the code !!!
 *
 */
#define FLASH_USER_START_ADDR ADDR_FLASH_PAGE_62                          /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR (FLASH_USER_START_ADDR + FLASH_PAGE_SIZE - 1) /* End @ of user Flash area */

#define SIZE_NBFI sizeof(nbfi_settings_t)
#define SIZE_METER sizeof(meter_params_str)
#define SIZE_WATER7 WATER7_PAR_LENGTH * 4 //sizeof(water7_params_str)
#define SIZE_ALL (SIZE_NBFI + SIZE_METER + SIZE_WATER7)

/**
  * @brief user data struct definition
  */
typedef __PACKED_UNION
{
    __PACKED_STRUCT
    {
        uint8_t nbfiSetting[SIZE_NBFI];
        uint8_t meterParams[SIZE_METER];
        uint8_t water7Params[SIZE_WATER7];
    };
    uint8_t buffer[SIZE_ALL];
}
EEPROM_DataTypeDef;

uint32_t WVT_EERPROM_CalcMeterCrc(meter_data_str *data);
/*!
 * \brief Load data from flash
 *
 * \param data destination pointer
 * \param type what settings do you need?
 * \return true if memory is empty
 */
uint32_t WVT_EEPROM_LoadAll(void *data, enum SettingsType type)
{
    /// \todo check this
    bool isEmpty = true;

    switch (type)
    {
    case EEPROM_NBFI_SETTING:
    {
        for (uint32_t *i = (uint32_t *)FLASH_USER_START_ADDR; i < (uint32_t *)(FLASH_USER_START_ADDR + SIZE_NBFI); i++)
        {
            if ((*i != 0xffffffff) && (*i != 0))
            {
                isEmpty = false;
            }
        }
        if (isEmpty == true)
        {
            return isEmpty;
        }
        memcpy_s(data, SIZE_NBFI, ((const void *)FLASH_USER_START_ADDR), SIZE_NBFI);
        break;
    }
    case EEPROM_METER_PARAMS:
    {
        memcpy_s(data, SIZE_METER, ((const void *)(FLASH_USER_START_ADDR + SIZE_NBFI)), SIZE_METER);
        break;
    }
    case EEPROM_WATER7_PARAMS:
    {
        for (uint32_t *i = (uint32_t *)FLASH_USER_START_ADDR + SIZE_NBFI + SIZE_METER; i < (uint32_t *)(FLASH_USER_START_ADDR + SIZE_WATER7); i++)
        {
            if ((*i != 0xffffffff) && (*i != 0))
            {
                isEmpty = false;
            }
        }
        if (isEmpty == true)
        {
            return isEmpty;
        }
        memcpy_s(data, SIZE_WATER7, ((const void *)(FLASH_USER_START_ADDR + SIZE_NBFI + SIZE_METER)), SIZE_WATER7);
        break;
    }
    default:
        break;
    };
    return isEmpty;
}

/*!
 * \brief Save settings to flash
 *
 * \param data pointer of settings
 * \param type settings type
 */
errno_t WVT_EEPROM_SaveAll(void *data, enum SettingsType type)
{
    static EEPROM_DataTypeDef oldData = {0};
    errno_t result = 0;
    /// \todo check this on time to execution
    result |= memcpy_s((void *)oldData.buffer, SIZE_ALL, ((const void *)FLASH_USER_START_ADDR), SIZE_ALL);
    switch (type)
    {
    case EEPROM_NBFI_SETTING:
    {
        /// \todo check this
        result |= memcpy_s((void *)oldData.nbfiSetting, sizeof(oldData.nbfiSetting), (const void *)data, SIZE_NBFI);
        break;
    }
    case EEPROM_METER_PARAMS:
    {
        result |= memcpy_s((void *)oldData.meterParams, sizeof(oldData.meterParams), (const void *)data, SIZE_METER);
        break;
    }
    case EEPROM_WATER7_PARAMS:
    {
        result |= memcpy_s((void *)oldData.water7Params, sizeof(oldData.water7Params), (const void *)data, SIZE_WATER7);
        break;
    }
    default:
        break;
    };
    /// \todo check this
    UpdateInternalFlashErase(USR_FLASH_PAGE_NUMBER);
    UpdateInternalFlashWrite((unsigned char *)oldData.nbfiSetting, FLASH_USER_START_ADDR, SIZE_ALL); /// \todo check for overflow and underflow
    return result;
}

/*!
 * \brief calculate crc32 from meter data
 *
 * \param data
 * \return uint32_t
 */
uint32_t WVT_EERPROM_CalcMeterCrc(meter_data_str *data)
{
    /// \todo fix this, because not right calculation
    return CRC_Crc32((uint32_t *)data, sizeof(meter_data_str) - 4, 0xFFFFFFFF);
}
/*!
 * \brief read data from backup registers
 *
 * \param data pointer of data
 */
bool WVT_EERPROM_ReadMeterData(meter_data_str *data)
{
    uint32_t tempData = 0;
    for (uint32_t i = 0; i < sizeof(meter_data_str) / sizeof(uint32_t); i++)
    {
        tempData = RTC_BackupRead(i);
        //        memcpy_s((void *)(data + i * sizeof(uint32_t)), (void const *)&tempData, sizeof(uint32_t));
        *((uint32_t *)data + i) = tempData;
    }
    if (data->crc == WVT_EERPROM_CalcMeterCrc(data))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 * \brief save data to backup registers
 *
 * \param data to be saved
 */
void WVT_EERPROM_WriteMeterData(meter_data_str *data)
{
    data->crc = WVT_EERPROM_CalcMeterCrc(data);

    for (uint32_t i = 0; i < sizeof(meter_data_str) / sizeof(uint32_t); i++)
    {
        uint32_t *ptrValue = (uint32_t *)data + i;
        RTC_BackupWrite(*ptrValue, i); //+ i * sizeof(uint32_t)))
    }
}