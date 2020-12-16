#ifndef NBFI_HAL_H
#define NBFI_HAL_H
#include "nbfi.h"

#ifdef NBFI_AT_SERVER
#include "nbfi_at_server.h"
#endif // NBFI_AT_SERVER

void nbfi_HAL_init(const nbfi_settings_t *settings, nbfi_dev_info_t *info);
#endif //NBFI_HAL_H