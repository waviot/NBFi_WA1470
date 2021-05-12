#ifndef SCHEDULER_HAL_H
#define SCHEDULER_HAL_H
#include "scheduler.h"

extern ischeduler_st* _scheduler;

void scheduler_HAL_init();
void scheduler_HAL_lock_unlock(uint8_t lock);

#endif //SCHEDULER_HAL_H