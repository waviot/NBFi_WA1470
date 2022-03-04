#include <stm32l0xx_hal.h>
#include <string.h>
#include "settings.h"
#include "rs485_uart.h"

global_settings_t  global_settings =
{
    UART_MODE_ATCOMMANDS,
    115200
};


uint32_t available_bitrates[] = {300, 1200, 9600, 19200,38400,57600,115200,0};


_Bool set_uart_bitrate(uint32_t bitrate)
{
    uint8_t i = 0;
    while(available_bitrates[i] && bitrate != available_bitrates[i])
        i++;
    if(available_bitrates[i] == 0) return 0;
    if(global_settings.uart_bitrate != bitrate)
    {
        global_settings.uart_bitrate = bitrate;
        //RS485_UART_deinit();
        //RS485_UART_init();
    }
    return 1;
}


#define EEPROM_INT_global_settings DATA_EEPROM_BASE


void load_global_settings()
{

    if(((global_settings_t*)(EEPROM_INT_global_settings))->uart_mode != UART_MODE_UNDEFINED)
        memcpy((void*)&global_settings, ((const void*)EEPROM_INT_global_settings), sizeof(global_settings_t));
}

void save_global_settings()
{
    if(HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) return;
    for(uint8_t i = 0; i != sizeof(global_settings_t); i++)
    {
	if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_INT_global_settings + i, ((uint8_t *)(&global_settings))[i]) != HAL_OK) break;
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
}



void RS485_UART_check_placeholder(char c)
{
    static char placeholder[] = "whoisondutytoday";
    static uint8_t ptr = 0;

    if(placeholder[ptr] == c)
    {
        if(++ptr == sizeof(placeholder)-1)
        {
            ptr = 0;
            if(global_settings.uart_mode == UART_MODE_TRANSPARENT)
            {
               global_settings.uart_mode = UART_MODE_ATCOMMANDS;
            }
        }
    } else ptr = 0;

}

