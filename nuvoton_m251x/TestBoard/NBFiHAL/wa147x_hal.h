#ifndef wa147x_HAL_H
#define wa147x_HAL_H

#ifdef WA1471
  #include "wa1471.h"
#else
  #include "wa1470.h"
#endif

void wa147x_HAL_init();
void wa147x_HAL_reg_data_received_callback(void*);
void wa147x_HAL_reg_tx_finished_callback(void*);
void WA_EXT_IRQHandler();

#endif //wa147x_HAL_H