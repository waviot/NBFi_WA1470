#include "gpio.h"
#include "scheduler_hal.h"
#include "spi.h"
#include "wa1470_hal.h"

#ifdef WA1470_LOG
//#include "log.h"
#endif //WA1470_LOG

static inline void wa1470_HAL_GPIO_Init(void)
{
    //main init in cubeMX
}

static inline void wa1470_HAL_SPI_Init(void)
{
    //main init in cubeMX
}

inline void WA_EXT_IRQHandler(void)
{
    //main init in cubeMX
    wa1470_isr();
}

static inline void wa1470_HAL_spi_rx(uint8_t *pData, uint16_t Size)
{
    SPI_Rx(pData, Size);
}

static inline void wa1470_HAL_spi_tx(uint8_t *pData, uint16_t Size)
{
    SPI_Tx(pData, Size);
}

static inline void wa1470_HAL_enable_pin_irq(void)
{
    GPIO_EnablePinIrq();
}

static inline void wa1470_HAL_disable_pin_irq(void)
{
    GPIO_DisablePinIrq();
}

static inline void wa1470_HAL_chip_enable(void)
{
    GPIO_ChipEnable();
}

static inline void wa1470_HAL_chip_disable(void)
{
    GPIO_ChipDisable();
}

static inline uint8_t wa1470_HAL_get_irq_pin_state(void)
{
    return GPIO_GetIrqPinState();
}

static inline void wa1470_HAL_spi_write_cs(uint8_t state)
{
    GPIO_SetSpiState(SPI_DEVICE_WA1470, state);
}

#define NOP_DELAY_MS_TICK 1600
static inline void NOP_Delay(uint32_t i)
{
    while (i--)
        asm volatile("nop");
}

static inline void NOP_Delay_ms(uint32_t val)
{
    while (val--)
        NOP_Delay(NOP_DELAY_MS_TICK);
}

static inline void wa1470_HAL_bpsk_pin_send(uint8_t *data, uint16_t len, uint16_t bitrate)
{
    wa1470_bpsk_pin_tx_finished();
}

static wa1470_HAL_st wa1470_hal_struct = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void wa1470_HAL_reg_data_received_callback(void *fn)
{
    wa1470_hal_struct.__wa1470_data_received = (void (*)(uint8_t *, uint8_t *))fn;
}

void wa1470_HAL_reg_tx_finished_callback(void *fn)
{
    wa1470_hal_struct.__wa1470_tx_finished = (void (*)(void))fn;
}

void log_send_strWA(const char *str)
{
	printf_s("%s",str);
    printf_s("\n");
}

void wa1470_HAL_init()
{

    wa1470_HAL_GPIO_Init();

    wa1470_HAL_SPI_Init();

    wa1470_hal_struct.__wa1470_enable_pin_irq = &wa1470_HAL_enable_pin_irq;
    wa1470_hal_struct.__wa1470_disable_pin_irq = &wa1470_HAL_disable_pin_irq;
    wa1470_hal_struct.__wa1470_chip_enable = &wa1470_HAL_chip_enable;
    wa1470_hal_struct.__wa1470_chip_disable = &wa1470_HAL_chip_disable;
    wa1470_hal_struct.__wa1470_get_irq_pin_state = &wa1470_HAL_get_irq_pin_state;
    wa1470_hal_struct.__spi_rx = &wa1470_HAL_spi_rx;
    wa1470_hal_struct.__spi_tx = &wa1470_HAL_spi_tx;
    wa1470_hal_struct.__spi_cs_set = &wa1470_HAL_spi_write_cs;
    wa1470_hal_struct.__wa1470_nop_dalay_ms = &NOP_Delay_ms;
    wa1470_hal_struct.__wa1470_send_to_bpsk_pin = &wa1470_HAL_bpsk_pin_send;
#ifdef WA1470_LOG
    wa1470_hal_struct.__wa1470_log_send_str = &log_send_strWA;
#endif //WA1470_LOG
    wa1470_init(WA1470_SEND_BY_I_Q_MODULATOR, 0, &wa1470_hal_struct, _scheduler);
}
