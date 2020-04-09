
#ifndef RADIO_H_
#define RADIO_H_

#include "wa1470_hal.h"
#include "scheduler_hal.h"
#include "nbfi_hal.h"

void radio_init(void);
void radio_switch_to_from_short_range(_Bool en);

#endif /* RADIO_H_ */
