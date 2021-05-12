#ifndef METER_H_
#define METER_H_

#include <stdint.h>
#include <time.h>

#define METER_FLASH_PARAMS (DATA_EEPROM_BASE + METER_FLASH_DATA_LEN)
#define METER_FLASH_DATA (DATA_EEPROM_BASE)
#define METER_FLASH_DATA_LEN 2048
#define METER_FLASH_PARAMS_LEN 256

#define TIM_CLOCK (32768 / 2)
#define CALL_VOLUME 50
#define CALL_MAGIC_NUM (int64_t)((float)TIM_CLOCK * ((float)CALL_VOLUME / 1000.0) * 3600.0)

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif //MIN

#define MKL_TO_L(x) ((x) / 1000000)
#define MKL_TO_100ML(x) ((x) / 100000)
#define L_TO_MKL(x) ((x)*1000000)

#define SECONDS_IN_HOUR 3600

#define SIZE_OF_CALIB_Q 7

#define GUARD_TIME_MS 60000

typedef enum
{
	METER_DIR_DOWN,
	METER_DIR_UP,
} meter_dir_enum;

typedef enum
{
	METER_FLOW_FORWARD,
	METER_FLOW_BACKWARD,
	METER_FLOW_DIFF,
	METER_FLOW_SIZE
} meter_flow_enum;

typedef enum
{
	METER_DISABLE,
	METER_ENABLE,
} meter_state_enum;

typedef enum
{
	METER_PARAMS,
	METER_DATA,
	METER_STATE,
} meter_get_set_enum;

typedef enum
{
	METER_ERROR_NONE = 0x00,
	METER_ERROR_LEAK = 0x01,
	METER_ERROR_BREAK = 0x02,
	METER_ERROR_FROST = 0x04,
	METER_ERROR_SENSOR = 0x08
} meter_flow_error;

typedef struct
{
	int32_t liter[METER_FLOW_SIZE];
	int64_t microliter[METER_FLOW_SIZE];
	uint32_t flow;
	time_t timestamp;
	uint8_t errors;
	uint8_t cntErrLeak;
	uint8_t cntErrBreak;
	uint8_t cntErrFrost;
	uint8_t cntErrSensor;
	int32_t liter24Hour[METER_FLOW_SIZE];
	uint32_t crc;
} meter_data_str;

typedef struct
{
	int32_t QInc[SIZE_OF_CALIB_Q];	 //mkl per impulse * second
	int64_t QTimes[SIZE_OF_CALIB_Q]; //ps delta tof
	int32_t QBreak;					 //liter per hour
	int32_t timeToBreak;			 //time s of break
	int32_t QLeak;					 //liter per hour
	int32_t timeToLeak;				 //time s of leak
	int32_t timeToFrost;			 //time s of frost
	int32_t tempToFrost;			 //temperature to frost
	int32_t zeroThreshold;			 //zero flow threshold
	uint32_t ErrorsMask;
} meter_params_str;

#ifdef USE_HALL_METER
typedef struct
{
	uint8_t lock_data;
	uint8_t prev_flow[2], get_flow, flow, period, min_period, direction, lock_data;
	uint16_t hall_state[2], hall_state_prev[2], hall_error[2];
	uint16_t max[2], min[2];
	uint16_t inc[2];
	uint32_t Q_time[2], Q_volume[2];
	uint32_t arr;
	uint32_t Q_flow_time, Q_flow_volume;
	uint32_t timestamp_acc;
} meter_state_str;
#endif // USE_HALL_METER

static inline int32_t Abs32(int32_t value) //https://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs
{
    int const mask = value >> sizeof(int32_t) * 8 - 1;
    return (value + mask) ^ mask;
}

void meter_get(void *data, meter_get_set_enum type);
void meter_set(void *data, meter_get_set_enum type);
meter_params_str *meter_GetParams(void);
void meter_init(meter_params_str *init_params, void fn(void));
void meter_save_data(void);
int64_t MeterGetTotalVolumeMkl(void);
int64_t MeterGetReverseVolumeMkl(void);
void MeterDTofOnePulse(int64_t dTofPs, time_t deltaTimeMs);
uint8_t MeterEverySecHandler(time_t timeNow);
uint8_t MeterErrorGet(void);
uint8_t MeterErrorHandler(uint8_t errors);
void MeterClearErrorsMask(void);
uint32_t meter_irq_handler(void);

#endif
