#include "scheduler_hal.h"
#include "rtc.h"

ischeduler_st *_scheduler = 0;

static inline void scheduler_HAL_LPRTC_Init(void)
{
    //main init in cubeMX
    RTC_Init();
}

static inline void scheduler_HAL_LOOPRTC_Init(void)
{
    //main init in cubeMX
    RTC_LooptimInit();
}

static inline void scheduler_HAL_enable_global_irq(void)
{
    RTC_CcIrqEnable(0); //__enable_irq();
}

static inline void scheduler_HAL_disable_global_irq(void)
{
    RTC_CcIrqDisable(0); //__disable_irq();
}

static inline void scheduler_HAL_cc_irq_enable(uint8_t chan)
{
    RTC_CcIrqEnable(chan);
}

static inline void scheduler_HAL_cc_irq_disable(uint8_t chan)
{
    RTC_CcIrqDisable(chan);
}

static inline void scheduler_HAL_loop_irq_enable(uint8_t chan)
{
    RTC_LoopIrqEnable(chan);
}

static inline void scheduler_HAL_loop_irq_disable(uint8_t chan)
{
    RTC_LoopIrqDisable(chan);
}

static inline void scheduler_HAL_cc_set(uint8_t chan, time_t data)
{
    RTC_CcSet(chan, data);
}

static inline time_t scheduler_HAL_cc_get(uint8_t chan)
{
    return RTC_CcGet(chan);
}

static inline time_t scheduler_HAL_cnt_get(uint8_t chan)
{
    return RTC_CntGet(chan);
}

static inline uint8_t scheduler_HAL_check_cc_irq(uint8_t chan)
{
    return RTC_CheckCcIrq(chan);
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
#ifdef USE_RTC_WATIMER
    scheduler_hal_struct.__cc_set = (void (*)(uint8_t, time_t))scheduler_HAL_cc_set;
    scheduler_hal_struct.__cc_get = (time_t(*)(uint8_t))scheduler_HAL_cc_get;
    scheduler_hal_struct.__cnt_get = (time_t(*)(uint8_t))scheduler_HAL_cnt_get;
#else
    scheduler_hal_struct.__cc_set = (void (*)(uint8_t, uint16_t))scheduler_HAL_cc_set;
    scheduler_hal_struct.__cc_get = (uint16_t(*)(uint8_t))scheduler_HAL_cc_get;
    scheduler_hal_struct.__cnt_get = (uint16_t(*)(uint8_t))scheduler_HAL_cnt_get;
#endif //USE_RTC_WATIMER
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
