#ifndef WATER7_H_
#define WATER7_H_

#define SECONDS_PER_MINUTE 60 //	seconds per minute
#define MINUTES_PER_HOUR 60	  //	minutes per hour
#define HOURS_PER_DAY 24	  //	hours per day
#define DAYS_PER_WEEK 7		  //	day per week
#define DAYS_PER_MONTH 30	  //	day per month
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * HOURS_PER_DAY)

#define WATER7_USERDATA_OFFSET 0
#define WATER7_USERPARAMS_OFFSET 64
#define WATER7_PARAMS_OFFSET ((64 + 128) - 1)

#define WATER7_PAR_LENGTH 320	  //	parameters array length (must be equal or bigger than sizeof(_AQUA_TAGS_TO_SAVE) / 4 and request length)
#define WATER7_PAR_COUNT 15		  //	parameters count (max 64)
#define WATER7_PAR_OFFSET_NOT24 3 //	one parameter for only every hour
#define WATER7_INDEX_TIME 11
#define WATER7_INDEX_24H_PRAMETER (WATER7_PAR_COUNT - WATER7_PAR_OFFSET_NOT24) //	parameters count (max 64)
#define WATER7_PAR_COUNT_PLUS_24HOUR 10										   //	parameters count (max 64)
#define WATER7_BYTE_PER_PAR (1 + 4 + 2 + 12)								   //	maximum byte parameter in regular sends
#define WATER7_REGULAR_BUF_SIZE 128											   //	regular buffer size

#define WATER7_CALLBACK_BUF_SIZE 256								  //	callbak buffer size
#define WATER7_CALLBACK_PAR_SIZE ((WATER7_CALLBACK_BUF_SIZE - 5) / 4) //	posible parameters count in one answer packet

#define WATER7_FASTDL_TIMEOUT (60 * 2)
#define WATER7_ACCEL_REGULAR 1

#define WATER7_MAX_EVENT_COUNT 20
#define WATER7_CRX_TIMEOUT (60 * 5)
/*
#define WATER7_TIME_PERIOD_FAST_DL		420			//	time switch too cose connection in seconds
#define WATER7_TIME_IN_FAST_DL			75			//	close connection wait time in milliseconds
#define WATER7_ACCEL_REGULAR			1			//	acceleration regular send in times
#define WATER7_SWITCH_WAIT_TIME			30
#define WATER7_AQUA_CLAIBRATION_TIME	(30 * 60)	//	time in calibraton mode after excess max flow in seconds
*/
/*	Link test
#define TIME_PERIOD_FAST_DL	 	1	 	//	time switch too cose connection in seconds
#define TIME_IN_FAST_DL		 	75		//	close connection wait time in milliseconds
#define ACCEL_REGULAR			30		//	acceleration regular send in times
#define SWITCH_WAIT_TIME		30
#define AQUA_CLAIBRATION_TIME	2 * 60	//	30 * 60	//	time in calibraton mode after excess max flow in seconds
*/

enum water7_send_period_t
{
	UNDEFINED_PERIOD = 0,
	ONEHOUR = 3600,
	TWOHOURS = 7200,
	FOURHOURS = 14400,
	SIXHOURS = 21600,
	TWELVEHOURS = 43200,
	ONEDAY = 86400,
	TWODAYS = 172800,
	FOURDAYS = 345600,
	ONEWEEK = 604800,
	TWOWEEKS = 1209600,
	THREEWEEKS = 1814400,
	FOURWEEKS = 2419200,
	ONEMONTH = 2678400,
	TWOMONTHS = 5356800,
	THREEMONTHS = 8035200,
	PERIOD_OFF = 0xffffffff,
};

enum water7_cmd_t
{
	CMD_REGULAR = 0x80,
	CMD_ERR = 0x40,
	CMD_MULTIREAD = 0x03,
	CMD_SINGLEPRESS = 0x06,
	CMD_SINGLEREAD = 0x07,
	CMD_MULTIPRESS = 0x10,
	CMD_ECHO = 0x19,
	CMD_EVENT = 0x20,
	CMD_CTRL = 0x27,
	CMD_RLF = 0x29,
};

enum water7_cmd_ctrl_t
{
	CTRL_SETFASTDL,
	CTRL_RESETFASTDL,
	CTRL_RESET,
	CTRL_SAVE,
	CTRL_CLEAR_ERRORS,
	CTRL_NOCTRL = 0xFFFF - 1,
};

enum water7_func_list_t
{
	WATER7_FUNC_GET_DATA,
	WATER7_FUNC_SET_DATA,
	WATER7_FUNC_SAVE_DATA,
	WATER7_FUNC_RFL,
};

enum rfl_cmd_t
{
	RFL_CMD_WRITE_HEX_INDEX,
	RFL_CMD_WRITE_HEX,
	RFL_CMD_READ_HEX,
	RFL_CMD_CLEAR_CACHE,
	RFL_CMD_CLEAR_INDEX,
	RFL_CMD_CHECK_UPDATE,
	RFL_CMD_GET_CRC,
	RFL_CMD_SOFT_RESET,
	RFL_CMD_MASS_ERASE,
	RFL_CMD_CPY_ACTUAL,
	RFL_CMD_GET_INDEX,
	RFL_CMD_GET_VERSION,
	RFL_CMD_EXEC_PATCH0,
	RFL_CMD_EXEC_PATCH1,
	RFL_CMD_EXEC_PATCH2,
};

enum rfl_error_t
{
	RFL_ERROR_NO_CMD = -3,
	RFL_ERROR,
	RFL_ERROR_INDEX_OK,
	RFL_ERROR_OK,
};

typedef struct
{
	uint32_t send_flag[2];
	uint32_t send_hour_flag[2];
	uint32_t period;
	uint32_t mesNum;
	uint32_t reserved[6];
	uint32_t par_max[WATER7_PAR_COUNT];
	uint8_t BufPrev[WATER7_REGULAR_BUF_SIZE];
	uint32_t length_buf;
	//	uint32_t crc;
} water7_params_str;

typedef struct
{
	uint16_t events[WATER7_MAX_EVENT_COUNT][2];
	uint16_t events_to_send;
	uint32_t period_prev;

	int32_t parameters_array[WATER7_PAR_LENGTH];
	uint32_t PeriodTable[HOURS_PER_DAY];
	int32_t RegularMas[WATER7_PAR_COUNT][HOURS_PER_DAY];
	int32_t Regular24Hour[WATER7_PAR_OFFSET_NOT24][HOURS_PER_DAY];
	int32_t PrevRegularMas[WATER7_PAR_COUNT];
	int32_t PrevRegular[WATER7_PAR_COUNT];
	int32_t PrevMinute[WATER7_PAR_COUNT];
	int32_t MasCount;

	int32_t SecondsRegular;
	uint8_t Seconds;
	uint8_t Minutes;
	uint8_t Hours;
	uint8_t Days;
	uint8_t Days4month;

} water7_state_str;

water7_params_str *Water7GetParams(void);
void Water7Init(water7_params_str *params);
uint8_t Water7Loop(void);
void Water7OneSec(struct tm newTime);
uint8_t Water7isCanSleep(void);
void Water7RXcallback(uint8_t *data, uint16_t length);
void Water7SendEvent(uint16_t event, uint16_t payload);
void Water7PushFunc(uint8_t func_type, errno_t func(int32_t *));
void Water7PushFuncRfl(uint8_t func_type, int32_t func(uint32_t addr, uint32_t len, uint32_t index, uint8_t *data, uint8_t cmd));
void Water7PushEvent(uint16_t event, uint16_t payload);
void Water7ForceEvent(void);
void Water7ForceData(void);
void Water7fastdl_on(void);
//void Water7fastdl_off(struct wtimer_desc *desc);

#endif // AQUA_H_INCLUDED
