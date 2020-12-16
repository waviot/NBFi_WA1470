#ifndef METER_DEFAULTS_H_
#define METER_DEFAULTS_H_

#include "main.h"

#define MIN_PERIOD				(TIM_CLOCK / 8 - 1)
#define MAX_PERIOD				(TIM_CLOCK / 1000 - 1)
#define MAX_PERIOD_POINT		20
#define NOMINAL_PERIOD_POINT	4
#define DELTA_PERIOD_POINT		1
#define THRESHOLD				100

#if defined BAIKAL_B
//	baikal b, nominal 7169
//	liter / hour
#define Q1 0
#define Q2 10
#define Q3 30
#define Q4 714
#define Q5 954
#define Q6 1104
#define Q7 3000
//	microliter
#define K1 -9500 //  -2.5%
#define K2 -8500 //  -2%
#define K3 -8000 //  -1.5%
#define K4 -7400 //  -1%
#define K5 -5700 //  -1%
#define K6 -4700 //  -0.3%
#define K7 -3000 //  -0.3%

#elif defined BAIKAL_C
//	baikal c, nominal 7169
#define Q1 19
#define Q2 35
#define Q3 45
#define Q4 65
#define Q5 85
#define Q6 155

#define K1 7025 //  -2%
#define K2 7025 //  -2%
#define K3 7025 //  -2%
#define K4 7061 //  -1.5%
#define K5 7097 //  -1%
#define K6 7097 //  -1%
#define K7 7147 //  -0.3%

#elif defined BAIKAL_B_80_15

#define Q1 50
#define Q2 70
#define Q3 90
#define Q4 500
#define Q5 1000
#define Q6 2000

#define K1 5683
#define K2 5715
#define K3 5742
#define K4 5771
#define K5 5771
#define K6 5900
#define K7 5900

#elif defined BAIKAL_B_130_20

#define Q1 30
#define Q2 100
#define Q3 300
#define Q4 500
#define Q5 1000
#define Q6 2000

#define K1 11230
#define K2 11230
#define K3 11174
#define K4 11174
#define K5 11342
#define K6 11342
#define K7 11342

#elif defined BAIKAL_C_130_20

#define Q1 30
#define Q2 100
#define Q3 300
#define Q4 500
#define Q5 1000
#define Q6 2000

#define K1 11010
#define K2 10900
#define K3 11010
#define K4 11174
#define K5 11230
#define K6 11230
#define K7 11230

#elif defined HABAR_B
//	habarovsk b
//	liter / hour
#define Q1 50
#define Q2 70
#define Q3 90
#define Q4 500
#define Q5 1000
#define Q6 2000
//	microliter
#define K1 7983
#define K2 8025
#define K3 8250
#define K4 8292
#define K5 8292
#define K6 8091
#define K7 8091

#elif defined ACCU_FLOW_B
#define Q1 50
#define Q2 200
#define Q3 500
#define Q4 1000
#define Q5 2000
#define Q6 2500

#define K1 8440
#define K2 8322
#define K3 8450
#define K4 8549
#define K5 8549
#define K6 8549
#define K7 8549
#endif

#endif
