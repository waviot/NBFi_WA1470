#ifndef CB_H_
#define CB_H_

#include "main.h"
#include <stdint.h>
#include "meter.h"
#include "water7.h"

#define LED_BLINK_VOLUME 10000

errno_t water7set_data(int32_t *data);
errno_t water7get_data(int32_t *data);
errno_t water7save_data(int32_t *data);
void get_saved_param(water7_params_str *water7_params_p, meter_params_str *meter_params_p);
int32_t water7_rfl(uint32_t addr, uint32_t len, uint32_t index, uint8_t *data, uint8_t cmd);
void meter_inc_cb(void);

#endif /* CB_H_ */
