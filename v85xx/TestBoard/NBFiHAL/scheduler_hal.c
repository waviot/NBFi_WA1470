#include "scheduler_hal.h"
#include "target.h"


#define WA_LOOPTIM			            TMR0
#define WA_LOOPTIM_IRQn	                TMR0_IRQn
#define WA_LOOPTIM_IRQHandler           TMR0_IRQHandler


ischeduler_st* _scheduler = 0;
uint16_t cc_counter = 0;
_Bool loop_irg_first_run = 0;

void scheduler_HAL_LPTIM_Init(void)
{

    RTC_WKUCounterConfig(0x10000, RTC_WKUCNT_2048);
    //RTC_WKUCounterConfig(0xffffff, RTC_WKUCNT_2048);
    //local_timer = RTC_GetWKUCounterValue();
    //PMU_SleepWKUSRC_Config_RTC(PMU_RTCEVT_WKUCNT, 2);

}


void scheduler_HAL_cc_set(uint8_t chan, uint16_t data)
{
    cc_counter = data;
    //RTC_WKUCounterConfig(data, RTC_WKUCNT_2048);
	//hlptim.Instance->CMP = data;

}



uint8_t scheduler_HAL_check_cc_irq(uint8_t chan)
{
  return cc_counter;//RTC->INTSTS & RTC_INTSTS_WKUCNT;
}



uint16_t scheduler_HAL_cc_get(uint8_t chan)
{
	return 0;//RTC->WKUCNT&0xffffff;////hlptim.Instance->CMP;
}

//uint32_t porog,current;
uint16_t scheduler_HAL_cnt_get(uint8_t chan)
{

    //static uint16_t old_current = 0;
    //if(scheduler_HAL_check_cc_irq(0))
    //{
    //    RTC_ClearINTStatus(RTC_INTSTS_WKUCNT);
	//	local_timer += scheduler_HAL_cc_get(0);
    //}
    //current = RTC_GetWKUCounterValue();
    //porog = scheduler_HAL_cc_get(0);
    //if(old_current > current) current = old_current;
    //old_current = current;
    //local_timer /= porog;
    //local_timer *= porog;
    //local_timer += RTC_GetWKUCounterValue();;


    //uint32_t timer = local_timer + current;
    //if(scheduler_HAL_check_cc_irq(0)) timer += porog;
    //else timer += current;//hlptim.Instance->CNT;
    uint16_t timer16 = (uint16_t)RTC_GetWKUCounterValue();
    return timer16;
}



void WA_LPTIM_IRQHandler(void)
{
  if(RTC->INTSTS & RTC_INTSTS_WKUCNT) {
		RTC_ClearINTStatus(RTC_INTSTS_WKUCNT);

  }
  //scheduler_irq();
}



void scheduler_HAL_LOOPTIM_Init(void)
{
    TMR_InitType TMR_InitStruct;
    TMR_DeInit(WA_LOOPTIM);
    TMR_InitStruct.ClockSource = TMR_CLKSRC_INTERNAL;
    TMR_InitStruct.EXTGT = TMR_EXTGT_DISABLE;
    TMR_InitStruct.Period = 13107200/1000 - 1;
    TMR_Init(WA_LOOPTIM, &TMR_InitStruct);

    TMR_INTConfig(WA_LOOPTIM, ENABLE);
    CORTEX_SetPriority_ClearPending_EnableIRQ(WA_LOOPTIM_IRQn, 2);

    TMR_Cmd(WA_LOOPTIM, ENABLE);
    loop_irg_first_run = 0;
}

void WA_LOOPTIM_IRQHandler(void)
{
  if (TMR_GetINTStatus(WA_LOOPTIM))
  {
    TMR_ClearINTStatus(WA_LOOPTIM);
    scheduler_run_callbacks();
    loop_irg_first_run = 1;
    //GPIOBToF_WriteBit(GPIOB, GPIO_Pin_7, !GPIOBToF_ReadOutputDataBit(GPIOB, GPIO_Pin_7));
  }
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
    //PMU_SleepWKUSRC_Config_RTC(RTC->INTEN|PMU_RTCEVT_WKUCNT, 2);
}

void scheduler_HAL_cc_irq_disable(uint8_t chan)
{
    //PMU_SleepWKUSRC_Config_RTC(RTC->INTEN&(~PMU_RTCEVT_WKUCNT), 2);
}

void scheduler_HAL_loop_irq_enable(uint8_t chan)
{
	CORTEX_NVIC_EnableIRQ(WA_LOOPTIM_IRQn);
}

void scheduler_HAL_loop_irq_disable(uint8_t chan)
{
    CORTEX_NVIC_DisableIRQ(WA_LOOPTIM_IRQn);
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

  _scheduler = scheduler_init(&scheduler_hal_struct, 2);

    //HAL_LPTIM_Counter_Start(&hlptim, 0xffff);
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
