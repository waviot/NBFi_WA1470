#include "scheduler_hal.h"
#include "NuMicro.h"


#define WA_LPTIM			            TIMER0
#define WA_LPTIM_MODULE			        TMR0_MODULE
#define WA_LPTIM_CLKSEL                 CLK_CLKSEL1_TMR0SEL_LXT
#define WA_LPTIM_IRQn			        TMR0_IRQn
#define WA_LPTIM_IRQHandler             TMR0_IRQHandler

#define WA_LOOPTIM_FREQ                   1000
#define WA_LOOPTIM			            TIMER1
#define WA_LOOPTIM_MODULE			    TMR1_MODULE
#define WA_LOOPTIM_CLKSEL               CLK_CLKSEL1_TMR1SEL_HIRC
#define WA_LOOPTIM_IRQn			        TMR1_IRQn
#define WA_LOOPTIM_IRQHandler           TMR1_IRQHandler


ischeduler_st* _scheduler = 0;


uint16_t cc_counter = 0;

_Bool loop_irg_first_run = 0;

void scheduler_HAL_LPTIM_Init(void)
{

    CLK_EnableModuleClock(WA_LPTIM_MODULE);
    CLK_SetModuleClock(WA_LPTIM_MODULE, WA_LPTIM_CLKSEL, 0);

    WA_LPTIM->CTL = TIMER_CONTINUOUS_MODE;

    TIMER_SET_PRESCALE_VALUE(WA_LPTIM, 32);
    WA_LPTIM->CMP = 0x00FFFF;

    TIMER_EnableWakeup(WA_LPTIM);
    TIMER_EnableInt(WA_LPTIM);
    NVIC_EnableIRQ(WA_LPTIM_IRQn);
    NVIC_SetPriority(WA_LPTIM_IRQn, 2);
    TIMER_Start(WA_LPTIM);

}

void scheduler_HAL_cc_set(uint8_t chan, uint16_t data)
{
    if((WA_LPTIM->CNT&0xFFFF) > (data + 0x8000))
    {
        WA_LPTIM->CMP = ((WA_LPTIM->CNT&0xFF0000)|data) + 0x10000;
    }
    else WA_LPTIM->CMP = ((WA_LPTIM->CMP&0xFF0000)|data);

}




uint16_t scheduler_HAL_cc_get(uint8_t chan)
{
	return WA_LPTIM->CMP&0xffff;
}

uint8_t scheduler_HAL_check_cc_irq(uint8_t chan)
{
  return TIMER_GetIntFlag(WA_LPTIM);
}


uint16_t scheduler_HAL_cnt_get(uint8_t chan)
{
    return WA_LPTIM->CNT&0xffff;;
}



void WA_LPTIM_IRQHandler(void)
{
    TIMER_ClearWakeupFlag(WA_LPTIM);
    TIMER_ClearIntFlag(WA_LPTIM);
    scheduler_irq();
}



void scheduler_HAL_LOOPTIM_Init(void)
{
    CLK_EnableModuleClock(WA_LOOPTIM_MODULE);
    CLK_SetModuleClock(WA_LOOPTIM_MODULE, WA_LOOPTIM_CLKSEL, 0);
    if (TIMER_Open(WA_LOOPTIM, TIMER_PERIODIC_MODE, WA_LOOPTIM_FREQ) != WA_LOOPTIM_FREQ)
    {
        while(1);
    }

    TIMER_EnableInt(WA_LOOPTIM);
    NVIC_EnableIRQ(WA_LOOPTIM_IRQn);
    NVIC_SetPriority(WA_LOOPTIM_IRQn, 2);
    TIMER_Start(WA_LOOPTIM);
    loop_irg_first_run = 0;
}

void WA_LOOPTIM_IRQHandler(void)
{
    TIMER_ClearIntFlag(WA_LOOPTIM);
    scheduler_run_callbacks();
    loop_irg_first_run = 1;

}


void scheduler_HAL_enable_global_irq(void)
{
  __enable_irq();
}

void scheduler_HAL_disable_global_irq(void)
{
  __disable_irq();
}


void scheduler_HAL_cc_irq_enable(uint8_t chan)
{
    NVIC_EnableIRQ(WA_LPTIM_IRQn);
}

void scheduler_HAL_cc_irq_disable(uint8_t chan)
{
    NVIC_DisableIRQ(WA_LPTIM_IRQn);
}

void scheduler_HAL_loop_irq_enable(uint8_t chan)
{
	NVIC_EnableIRQ(WA_LOOPTIM_IRQn);
}

void scheduler_HAL_loop_irq_disable(uint8_t chan)
{
    NVIC_DisableIRQ(WA_LOOPTIM_IRQn);
}


scheduler_HAL_st scheduler_hal_struct = {0,0,0,0,0,0,0,0,0,0};

void scheduler_HAL_init()
{
  scheduler_hal_struct.__global_irq_enable = (void(*)(void))scheduler_HAL_enable_global_irq;
  scheduler_hal_struct.__global_irq_disable = (void(*)(void))scheduler_HAL_disable_global_irq;
  scheduler_hal_struct.__cc_irq_enable = (void(*)(uint8_t))scheduler_HAL_cc_irq_enable;
  scheduler_hal_struct.__cc_irq_disable = (void(*)(uint8_t))scheduler_HAL_cc_irq_disable;
  scheduler_hal_struct.__loop_irq_enable = (void(*)(void))scheduler_HAL_loop_irq_enable;
  scheduler_hal_struct.__loop_irq_disable = (void(*)(void))scheduler_HAL_loop_irq_disable;
  scheduler_hal_struct.__cc_set = (void(*)(uint8_t,uint16_t))scheduler_HAL_cc_set;
  scheduler_hal_struct.__cc_get = (uint16_t(*)(uint8_t))scheduler_HAL_cc_get;
  scheduler_hal_struct.__cnt_get = (uint16_t(*)(uint8_t))scheduler_HAL_cnt_get;
  scheduler_hal_struct.__check_cc_irq = (uint8_t(*)(uint8_t))scheduler_HAL_check_cc_irq;

  scheduler_HAL_LPTIM_Init();

  _scheduler = scheduler_init(&scheduler_hal_struct, 1);

  scheduler_HAL_LOOPTIM_Init();

}

void scheduler_HAL_reinit()
{
    scheduler_HAL_LOOPTIM_Init();
}

_Bool scheduler_HAL_can_sleep()
{
    return loop_irg_first_run && scheduler_can_sleep();
}

void scheduler_HAL_lock_unlock(uint8_t lock)
{
     if(lock)  scheduler_hal_struct.__loop_irq_disable();
     else    scheduler_hal_struct.__loop_irq_enable();
}
