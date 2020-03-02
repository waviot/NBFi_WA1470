#ifndef WA1470_HAL_H
#define WA1470_HAL_H


void WA1470_HAL_init(uint32_t modem_id);

void WA1470_HAL_reg_data_received_callback(void*);
void WA1470_HAL_reg_tx_finished_callback(void*);

#endif //WA1470_HAL_H