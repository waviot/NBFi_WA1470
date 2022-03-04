#ifndef SETTINGS_H_
#define SETTINGS_H_

typedef enum
{   UART_MODE_ATCOMMANDS        = 1,
    UART_MODE_TRANSPARENT       = 2,
    UART_MODE_APPENDD3          = 3,
    UART_MODE_NBFI_UNUSED       = 4,
    UART_MODE_UNDEFINED         = 0
}uart_mode_t;

typedef struct
{
    uart_mode_t uart_mode;
    uint32_t uart_bitrate;
}global_settings_t;

extern uint32_t available_bitrates[];

extern global_settings_t  global_settings;

_Bool set_uart_bitrate(uint32_t bitrate);
void load_global_settings();
void save_global_settings();


#endif  //SETTINGS_H_


