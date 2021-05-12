#ifndef METER_DEFAULTS_H_
#define METER_DEFAULTS_H_

#include "main.h"

#define MIN_PERIOD				(TIM_CLOCK / 8 - 1)
#define MAX_PERIOD				(TIM_CLOCK / 1000 - 1)
#define MAX_PERIOD_POINT		20
#define NOMINAL_PERIOD_POINT	4
#define DELTA_PERIOD_POINT		1
#define THRESHOLD				100

#endif
