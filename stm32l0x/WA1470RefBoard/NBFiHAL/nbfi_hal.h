#ifndef NBFI_HAL_H
#define NBFI_HAL_H
#include "nbfi.h"

void nbfi_HAL_init(const nbfi_settings_t* settings, nbfi_dev_info_t* info);
uint32_t nbfi_HAL_measure_valtage_or_temperature(uint8_t val);

#endif //NBFI_HAL_H