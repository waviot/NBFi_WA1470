#include "scheduler_hal.h"
#include "lptim.h"
#include "rtc.h"
#include "tim.h"

ischeduler_st *_scheduler = 0;

static inline void scheduler_HAL_LPRTC_Init(void)
{
    //main init in cubeMX
    RTC_WakeUpPeriodic();
    HAL_LPTIM_Start();
}

static inline void scheduler_HAL_LOOPRTC_Init(void)
{
    //main init in cubeMX
    HAL_TIM6_Start();
}

static inline void scheduler_HAL_enable_global_irq(void)
{
    __enable_irq();
}

static inline void scheduler_HAL_disable_global_irq(void)
{
    __disable_irq();
}

static inline void scheduler_HAL_cc_irq_enable(uint8_t chan)
{
    HAL_LPTIM_EnableIt();
}

static inline void scheduler_HAL_cc_irq_disable(uint8_t chan)
{
    HAL_LPTIM_DisableIt();
}

static inline void scheduler_HAL_loop_irq_enable(uint8_t chan)
{
    HAL_TIM6_EnableIt();
}

static inline void scheduler_HAL_loop_irq_disable(uint8_t chan)
{
    HAL_TIM6_DisableIt();
}

static inline void scheduler_HAL_cc_set(uint8_t chan, uint16_t data)
{
    HAL_LPTIM_SetCompare(data);
}

static inline time_t scheduler_HAL_cc_get(uint8_t chan)
{
    return HAL_LPTIM_GetCompare();
}

static inline time_t scheduler_HAL_cnt_get(uint8_t chan)
{
    return HAL_LPTIM_GetCounter();
}

static inline uint8_t scheduler_HAL_check_cc_irq(uint8_t chan)
{
    return HAL_LPTIM_CheckIrq();
}

scheduler_HAL_st scheduler_hal_struct = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void scheduler_HAL_init()
{

    scheduler_hal_struct.__global_irq_enable = (void (*)(void))scheduler_HAL_enable_global_irq;
    scheduler_hal_struct.__global_irq_disable = (void (*)(void))scheduler_HAL_disable_global_irq;
    scheduler_hal_struct.__cc_irq_enable = (void (*)(uint8_t))scheduler_HAL_cc_irq_enable;
    scheduler_hal_struct.__cc_irq_disable = (void (*)(uint8_t))scheduler_HAL_cc_irq_disable;
    scheduler_hal_struct.__loop_irq_enable = (void (*)(void))scheduler_HAL_loop_irq_enable;
    scheduler_hal_struct.__loop_irq_disable = (void (*)(void))scheduler_HAL_loop_irq_disable;
    scheduler_hal_struct.__cc_set = (void (*)(uint8_t, uint16_t))scheduler_HAL_cc_set;
    scheduler_hal_struct.__cc_get = (uint16_t(*)(uint8_t))scheduler_HAL_cc_get;
    scheduler_hal_struct.__cnt_get = (uint16_t(*)(uint8_t))scheduler_HAL_cnt_get;
    scheduler_hal_struct.__check_cc_irq = (uint8_t(*)(uint8_t))scheduler_HAL_check_cc_irq;

    _scheduler = scheduler_init(&scheduler_hal_struct);

    scheduler_HAL_LPRTC_Init();
    scheduler_HAL_LOOPRTC_Init();
}

void scheduler_HAL_lock_unlock(uint8_t lock)
{
    if (lock)
        scheduler_hal_struct.__loop_irq_disable();
    else
        scheduler_hal_struct.__loop_irq_enable();
}
