#ifndef _scheduler_H
#define _scheduler_H

#ifdef WTIMER
#include "wtimer.h"
#define scheduler_desc wtimer_desc
#define scheduler_desc_handler_t wtimer_desc_handler_t
#define scheduler_HAL_st wtimer_HAL_st
#else
#include "watimer.h"
#define ABSOLUTE    0
#define RELATIVE    1
#define scheduler_desc watimer_callback_st
#define scheduler_desc_handler_t watimer_callback_func
#define scheduler_HAL_st watimer_HAL_st
#endif

/*
enum scheduler_func_t
{
	SCHEDULER_GLOBAL_IRQ_ENABLE,
	SCHEDULER_GLOBAL_IRQ_DISABLE,
	SCHEDULER_CC_IRQ_ENABLE,
	SCHEDULER_CC_IRQ_DISABLE,
	SCHEDULER_SET_CC,
	SCHEDULER_GET_CC,
	SCHEDULER_GET_CNT,
	SCHEDULER_CHECK_CC_IRQ,
};
*/

void scheduler_set_HAL(scheduler_HAL_st *ptr);

void scheduler_init();
//void scheduler_reg_func(uint8_t name, void *fn);
void scheduler_irq();

void WA1470_HAL_init(uint32_t modem_id);

void scheduler_run_callbacks();
void scheduler_add_task(struct scheduler_desc *desc, scheduler_desc_handler_t handler, uint8_t relative, uint32_t time);
void scheduler_remove_task(struct scheduler_desc *desc);
uint32_t scheduler_current_time();


#endif //