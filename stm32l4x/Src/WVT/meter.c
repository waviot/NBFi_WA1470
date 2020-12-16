#include <string.h>
#include <stdbool.h>

//#include "libmfcrc.h"
#include "meter.h"
#include "meter_defaults.h"
#include "main.h"
#include "rtc.h"
#include "water7.h"

void (*_inc_cb)(void);

#ifdef USE_HALL_METER
static meter_state_str _state = {0};
#endif // USE_HALL_METER
static meter_data_str _data = {0};
static meter_params_str _params = {0};
static const meter_params_str _params_default =
	{
		{K1, K2, K3, K4, K5, K6, K7},
		{Q1, Q2, Q3, Q4, Q5, Q6, Q7},
		K5,
		3600,
		K2,
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
#ifdef USE_HALL_METER
	case METER_STATE:
		memcpy_s((uint8_t *)data, (uint8_t *)&_state, sizeof(meter_state_str));
		break;
#endif
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
#ifdef USE_HALL_METER
	case METER_STATE:
		memcpy_s((uint8_t *)&_state, (uint8_t *)data, sizeof(meter_state_str));
		break;
#endif
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

#ifdef USE_WVT_ULTRASOUND_METER
void MeterDTofOnePulse(int64_t dTofPs, time_t deltaTimeMs)
{
	uint8_t IsReverse = CalibAndLinIsReverse(dTofPs, &_params);
	int32_t mklPS = CalibAndLinGetVolumeLinear(dTofPs, &_params);
	int32_t deltaMkl = Abs32(mklPS) * deltaTimeMs / MS_IN_SECOND / SECONDS_IN_HOUR;
	if (deltaTimeMs < GUARD_TIME_MS)
	{
		//_data.microliter[IsReverse] += deltaMkl;
		_data.microliter[IsReverse] += (-2) * IsReverse * deltaMkl + deltaMkl;
	}
	_inc_cb();
}
#endif //USE_WVT_ULTRASOUND_METER

#if defined(MAX35101) || defined(MAX35103) || defined(MAX35104)
void MeterDTofOnePulse(int64_t dTofPs, int32_t deltaTimeMs)
{
	uint8_t IsReverse = CalibAndLinIsReverse(dTofPs, &_params);
	int32_t mklPS = CalibAndLinGetVolumeLinear(dTofPs, &_params);
	int32_t deltaMkl = mklPS * deltaTimeMs / MS_IN_SECOND;
	_data.microliter[IsReverse] += deltaMkl;
	_data.microliter[METER_FLOW_DIFF] += (-2) * IsReverse * deltaMkl + deltaMkl;
}
#endif // defined(MAX35101) || defined(MAX35103) || defined(MAX35104)

#ifdef USE_HALL_METER
void tx(uint8_t data)
{
	LPUART1->TDR = data;
	while (!(LPUART1->ISR & UART_FLAG_TC))
		;
}

void tx_uint16(uint16_t data)
{
	tx(data & 0xff);
	tx(data >> 8);
}

void CountUp(void)
{
	if (_state.direction == METER_DIR_UP && !_state.get_flow)
		_state.get_flow = 1;

	_state.direction ^= 1;
	_state.hall_state_prev[0] = _state.hall_state[0];

	_state.min_period = MIN(_state.min_period, _state.period);
	_state.period = 0;

	if (_state.arr >= _params.min_period)
		_state.arr = _params.max_period;

	if (!_state.lock_data)
	{
		_data.microliter[_state.flow] += _state.inc[_state.flow];
		if (_data.microliter[_state.flow] >= 1000000)
		{
			_data.liter[_state.flow]++;
			_data.microliter[_state.flow] -= 1000000;
		}
	}

	_state.Q_flow_volume += _state.inc[_state.flow];
	_state.Q_volume[_state.flow] += _state.inc[_state.flow];
	if (_state.Q_volume[_state.flow] >= CALL_VOLUME * 1000)
	{
		uint8_t i;
		for (i = 0; _state.Q_time[_state.flow] < _params.Q_times[i]; i++)
			;
		_state.inc[_state.flow] = _params.Q_inc[i];
		_state.Q_volume[_state.flow] = _state.Q_time[_state.flow] = 0;
	}
	_inc_cb();
}

void meter_test_hall(uint32_t num)
{
	if (_state.hall_state[num] < 100 || _state.hall_state[num] > 3000)
		_state.hall_error[num] = 1;
	else
		_state.hall_error[num] = 0;
}

uint32_t meter_irq_handler(void)
{
	static uint32_t i, period;

	if (i >= period)
	{
		i = 0;

		Set_MAGN1_PWR;
		if (_state.get_flow == 1)
			Set_MAGN2_PWR;

		hw_rtc_disable_wakeup();
		hw_adc_prepeare();

		if (_state.period < _params.max_period_point)
			_state.period++;

		uint32_t arr = _state.arr > (_params.min_period >> 2) ? _state.arr & ~((uint32_t)0x3) : _state.arr;

		_state.Q_time[_state.flow] += arr;

		_state.Q_flow_time += arr;
		if (_state.Q_flow_time & TIM_CLOCK)
		{
			_data.flow = 3600 * _state.Q_flow_volume;
			_state.Q_flow_time = _state.Q_flow_volume = 0;
		}

		_state.timestamp_acc += arr;
		if (_state.timestamp_acc & TIM_CLOCK)
		{
			_state.timestamp_acc &= TIM_CLOCK - 1;
			_data.timestamp++;
		}

		volatile uint32_t wait = 50;
		while (wait--)
			;

		hw_adc_get_data(&_state.hall_state[1], &_state.hall_state[0]);
		hw_adc_disable();

		Clr_MAGN1_PWR;
		Clr_MAGN2_PWR;

		_state.max[0] = MAX(_state.max[0], _state.hall_state[0]);
		_state.min[0] = MIN(_state.min[0], _state.hall_state[0]);

		meter_test_hall(0);
		if (_state.get_flow)
		{
			meter_test_hall(1);
			if (_state.get_flow == 1)
			{
				_state.prev_flow[1] = _state.prev_flow[0];
				if (_state.hall_state[1] > ((_state.max[0] + _state.min[0]) >> 1))
					_state.prev_flow[0] = METER_FLOW_FORWARD;
				else
					_state.prev_flow[0] = METER_FLOW_BACKWARD;
				_state.min[0] = 0xFFFF;
				_state.max[0] = 0;
				_state.get_flow = 0;
				if (_state.prev_flow[0] == _state.prev_flow[1])
					_state.flow = _state.prev_flow[0];
			}
			else
				_state.get_flow--;
		}

		if (_state.direction == METER_DIR_DOWN)
		{
			if (_state.hall_state[0] > _state.hall_state_prev[0] + _params.threshold)
				CountUp();
			else if (_state.hall_state[0] < _state.hall_state_prev[0])
				_state.hall_state_prev[0] = _state.hall_state[0];
		}
		else if (_state.direction == METER_DIR_UP)
		{

			if (_state.hall_state[0] < _state.hall_state_prev[0] - _params.threshold)
				CountUp();
			else if (_state.hall_state[0] > _state.hall_state_prev[0])
				_state.hall_state_prev[0] = _state.hall_state[0];
		}

		if (_state.min_period < _params.nominal_period_point)
		{
			uint16_t delta = (_state.arr >> 1) + 1;
			if (_state.arr > delta + _params.max_period)
			{
				_state.min_period = 0xFF;
				_state.arr = _state.arr - delta;
			}
			else
			{
				_state.min_period = 0xFF;
				_state.arr = _params.max_period;
			}
		}
		else if (_state.period > _params.nominal_period_point + _params.delta_period_point)
		{
			if ((_state.arr < _params.min_period))
			{
				_state.min_period = 0xFF;
				_state.arr = _state.arr + (_state.arr >> 5) + 1;
				_state.period = _params.nominal_period_point + _params.delta_period_point;
			}
		}

		if (_params.uart_debug == METER_ENABLE)
		{
			tx_uint16(_state.hall_state[0]);
			tx_uint16(_state.hall_state[1]);
			tx(_state.direction << 1 | _state.flow);
			tx_uint16(0xAA55);
		}

		if (_state.arr > (_params.min_period >> 2))
		{
			hw_rtc_set_period(_state.arr >> 2);
			period = 3;
		}
		else
		{
			hw_rtc_set_period(_state.arr);
			period = 0;
		}
	}
	else
		i++;

	return _state.arr;
}
#endif //USE_HALL_METER
