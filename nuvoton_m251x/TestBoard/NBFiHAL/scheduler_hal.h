#ifndef SCHEDULER_HAL_H
#define SCHEDULER_HAL_H
#include "scheduler.h"

extern ischeduler_st* _scheduler;

void scheduler_HAL_init();
void scheduler_HAL_reinit();
_Bool scheduler_HAL_can_sleep();
void scheduler_HAL_lock_unlock(uint8_t lock);
void WA_LPTIM_IRQHandler(void);
#endif //SCHEDULER_HAL_H