#ifndef wa1470_HAL_H
#define wa1470_HAL_H

#include "wa1470.h"

void wa1470_HAL_init();

void wa1470_HAL_reg_data_received_callback(void*);
void wa1470_HAL_reg_tx_finished_callback(void*);
void WA_EXT_IRQHandler();

#endif //wa1470_HAL_H