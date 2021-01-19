#include <string.h>
#include <stdbool.h>

//#include "libmfcrc.h"
#include "meter.h"
#include "meter_defaults.h"
#include "main.h"
#include "rtc.h"
#include "water7.h"

void (*_inc_cb)(void);

static meter_data_str _data = {0};
static meter_params_str _params = {0};
static const meter_params_str _params_default =
	{
		{0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0},
		0,
		3600,
		0,
		7200, //time s of leak
		3600, //time s of frost
		5,	  //temperature to frost
		100,  //zero flow threshold
		0x0,  //none errors
};

void meter_get(void *data, meter_get_set_enum type)
{
	switch (type)
	{
	case METER_PARAMS:
		memcpy_s((uint8_t *)data, sizeof(meter_params_str), (uint8_t *)&_params, sizeof(meter_params_str));
		break;
	case METER_DATA:
		_data.microliter[METER_FLOW_DIFF] = _data.microliter[METER_FLOW_FORWARD] - _data.microliter[METER_FLOW_BACKWARD];
		for (uint8_t i = 0; i < METER_FLOW_SIZE; i++)
		{
			_data.liter[i] = _data.microliter[i] / 1000000;
		}
		_data.timestamp = RTC_GetSeconds();
		memcpy_s((uint8_t *)data, sizeof(meter_data_str), (uint8_t *)&_data, sizeof(meter_data_str));
		break;
	default:
		break;
	}
}

void meter_set(void *data, meter_get_set_enum type)
{
	switch (type)
	{
	case METER_PARAMS:
		memcpy_s((uint8_t *)&_params, sizeof(meter_params_str), (uint8_t *)data, sizeof(meter_params_str));
		break;
	case METER_DATA:
		memcpy_s((uint8_t *)&_data, sizeof(meter_data_str), (uint8_t *)data, sizeof(meter_data_str));
		break;
	default:
		break;
	}
}

meter_params_str *meter_GetParams(void)
{
	return &_params;
}

void meter_init(meter_params_str *init_params, void fn(void))
{
	/// \todo fix this
	//	_inc_cb = fn;

	if (init_params)
	{
		memcpy_s((uint8_t *)&_params, sizeof(meter_params_str), (uint8_t *)init_params, sizeof(meter_params_str));
	}
	else
		memcpy_s((uint8_t *)&_params, sizeof(meter_params_str), (uint8_t *)&_params_default, sizeof(meter_params_str));

	/// \todo check this
	meter_data_str meter_data_last;
	if (WVT_EERPROM_ReadMeterData(&meter_data_last)) //loaded
	{
		memcpy_s((uint8_t *)&_data, sizeof(meter_data_str), (uint8_t *)&meter_data_last, sizeof(meter_data_str));
	}
}

void meter_save_data(void)
{
	WVT_EERPROM_WriteMeterData(&_data);
}

int64_t MeterGetTotalVolumeMkl(void)
{
	return _data.microliter[METER_FLOW_DIFF];
}

int64_t MeterGetReverseVolumeMkl(void)
{
	return _data.microliter[METER_FLOW_BACKWARD];
}

uint8_t MeterEverySecHandler(time_t timeNow)
{
	uint8_t errors = 0;/// \todo add check errors function
	MeterErrorHandler(errors);
	return errors;
}

uint8_t MeterErrorGet(void)
{
	return _data.errors;
}

uint8_t MeterErrorHandler(uint8_t errors)
{
	uint8_t result = 0;
	if ((_data.errors & errors) != errors)
	{
		SET_BIT(_data.errors, errors);
		//if new error detected
		if (errors & _params.ErrorsMask)
		{
			//and we can send
			if (READ_BIT(_data.errors, METER_ERROR_LEAK) && (_data.cntErrLeak < UINT8_MAX - 1))
			{
				_data.cntErrLeak++;
				SendErrors(_data.errors);
				//				Water7PushEvent(EVENT_LEAK, _data.cntErrLeak);
				result = _data.cntErrLeak;
			}
			if (READ_BIT(_data.errors, METER_ERROR_BREAK) && (_data.cntErrBreak < UINT8_MAX - 1))
			{
				_data.cntErrBreak++;
				SendErrors(_data.errors);
				//				Water7PushEvent(EVENT_BREAK, _data.cntErrBreak);
				result = _data.cntErrBreak;
			}
			if (READ_BIT(_data.errors, METER_ERROR_FROST) && (_data.cntErrFrost < UINT8_MAX - 1))
			{
				_data.cntErrFrost++;
				SendErrors(_data.errors);
				//				Water7PushEvent(EVENT_FROST, _data.cntErrFrost);
				result = _data.cntErrFrost;
			}
			/// \todo tune count errors of sensor
			if (READ_BIT(_data.errors, METER_ERROR_SENSOR) && (_data.cntErrSensor < 3)) //UINT8_MAX - 1))
			{
				_data.cntErrSensor++;
				SendErrors(_data.errors);
				//				Water7PushEvent(EVENT_FROST, _data.cntErrFrost);
				result = _data.cntErrSensor;
			}
			CLEAR_BIT(_params.ErrorsMask, errors);
		}
	}
	return result;
}
void MeterClearErrorsMask(void)
{
	_params.ErrorsMask = 0x0;
	_data.cntErrFrost = 0;
	_data.cntErrBreak = 0;
	_data.cntErrLeak = 0;
	_data.cntErrSensor = 0;
}
