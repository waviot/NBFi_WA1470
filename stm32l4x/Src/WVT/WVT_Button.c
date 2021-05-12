/*!
 * \file WVT_Button.c
 * \author Sergei Savkin (ssavkin@waviot.ru)
 * \brief User button handler and logic
 * \version 0.1
 * \date 10-07-2020
 *
 * \copyright WAVIoT 2020
 *
 */
#include "WVT_Button.h"
#include "gpio.h"
#include "rtc.h"
#include "scheduler_hal.h"
#include "water7.h"

/*!
 * \brief simple time filter from contact glitch
 *
 */
#define CONTACT_FILTER_TIME_MS 50

/*!
//  * \brief time to understand long button press in seconds
 *
 */
#define LONG_TIME_PRESS_S 2

/*!
 * \brief time capture for simple filter from previous IRQ
 *
 */
time_t TimeMsOld = 0;

/*!
 * \brief If button pressed then true
 *
 */
bool IsButtonPressed = false;

/*!
 * \brief for scheduler long press detection
 *
 */
struct scheduler_desc checkButton_desc;

static void ButtonShortPressHandler(void);
static void ButtonLongPressHandler(void);

/*!
 * \brief task for check long press button
 *
 * \param desc scheduler param
 */
void checkButton(struct scheduler_desc *desc)
{
	//check long press
	if (GPIO_IsButtonPressed())
	{
		if (IsButtonPressed == true) //not released
		{
			ButtonLongPressHandler();
		}
	}
}

/*!
 * \brief Callback from GPIO IRQ
 * \example
 * void GPIO_EXTI7_Callback(void)
 * {
 *  ButtonHandler();
 * }
 */
void ButtonHandler(void)
{
	time_t timeMsNow = RTC_GetAbsMilliseconds();
	if (GPIO_IsButtonPressed())
	{
		if ((IsButtonPressed == false)&&(timeMsNow - TimeMsOld > CONTACT_FILTER_TIME_MS))
		{
            scheduler_add_task(&checkButton_desc, checkButton, RELATIVE, SECONDS(LONG_TIME_PRESS_S));
            IsButtonPressed = true;
		}
	}
	else
	{
		if (IsButtonPressed == true)
		{
			if (timeMsNow - TimeMsOld > CONTACT_FILTER_TIME_MS)
			{
				ButtonShortPressHandler();
				scheduler_remove_task(&checkButton_desc);
			}
		}
		IsButtonPressed = false;
	}
	TimeMsOld = timeMsNow;
}

/*!
 * \brief short release action
 *
 */
static void ButtonShortPressHandler(void)
{
}

/*!
 * \brief long press action
 *
 */
static void ButtonLongPressHandler(void)
{
	//send data to nb-fi
	Water7ForceData();
}