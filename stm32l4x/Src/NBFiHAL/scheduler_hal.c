#include "scheduler_hal.h"
#include "rtc.h"
#include "tim.h"

ischeduler_st *_scheduler = 0;

static inline void scheduler_HAL_LPRTC_Init(void)
{
    //main init in cubeMX
    RTC_WakeUpPeriodic();
    HAL_LPRTC_Start();
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
    HAL_LPRTC_EnableIt();
}

static inline void scheduler_HAL_cc_irq_disable(uint8_t chan)
{
    HAL_LPRTC_DisableIt();
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
    data = HAL_LPRTC_GetPrescalerS() - (((uint32_t)data * HAL_LPRTC_GetPrescalerS() / SECONDS(1))  % HAL_LPRTC_GetPrescalerS());
    HAL_LPRTC_SetCompare(data);
}

static inline time_t scheduler_HAL_cc_get(uint8_t chan)
{
    return (uint32_t)(HAL_LPRTC_GetPrescalerS() - HAL_LPRTC_GetCompare()) * SECONDS(1) / HAL_LPRTC_GetPrescalerS();
}

static inline time_t scheduler_HAL_cnt_get(uint8_t chan)
{
    static uint32_t oldTime = 0;
    static uint32_t result = 0;
    uint32_t newTime = (uint32_t)(HAL_LPRTC_GetPrescalerS() - HAL_LPRTC_GetCounter()) * SECONDS(1) / HAL_LPRTC_GetPrescalerS();
    if(newTime >= oldTime)
    {
        result += newTime - oldTime;
    }else
    {
        result += newTime + SECONDS(1) - oldTime;
    }
    oldTime = newTime;
    return result & 0xffff;
}

static inline uint8_t scheduler_HAL_check_cc_irq(uint8_t chan)
{
    return HAL_LPRTC_CheckIrq();
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
