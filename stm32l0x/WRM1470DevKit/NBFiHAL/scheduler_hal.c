#include "scheduler_hal.h"
#include "stm32l0xx_hal_conf.h"

static LPTIM_HandleTypeDef hlptim;
static TIM_HandleTypeDef hlooptim;


#define WA_LPTIM			LPTIM1
#define WA_LPTIM_IRQn			LPTIM1_IRQn
#define WA_LPTIM_RCC_ENABLE 	        __HAL_RCC_LPTIM1_CLK_ENABLE
#define WA_LPTIM_RCC_DISABLE 	        __HAL_RCC_LPTIM1_CLK_DISABLE
#define WA_LPTIM_IRQHandler             LPTIM1_IRQHandler


#define WA_LOOPTIM			TIM7
#define WA_LOOPTIM_IRQn		        TIM7_IRQn
#define WA_LOOPTIM_RCC_ENABLE 	        __HAL_RCC_TIM7_CLK_ENABLE
#define WA_LOOPTIM_RCC_DISABLE 	        __HAL_RCC_TIM7_CLK_DISABLE
#define WA_LOOPTIM_TIM_FREQ		2000
#define WA_LOOPTIM_IRQHandler           TIM7_IRQHandler

ischeduler_st* _scheduler = 0;


void scheduler_HAL_LPTIM_Init(void)
{

    WA_LPTIM_RCC_ENABLE();

    hlptim.Instance = WA_LPTIM;
    hlptim.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
    hlptim.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
    hlptim.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    HAL_LPTIM_Init(&hlptim);

    HAL_NVIC_SetPriority(WA_LPTIM_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(WA_LPTIM_IRQn);
}

void WA_LPTIM_IRQHandler(void)
{

  if (__HAL_LPTIM_GET_FLAG(&hlptim, LPTIM_FLAG_CMPM) != RESET) {
                __HAL_LPTIM_CLEAR_FLAG(&hlptim, LPTIM_FLAG_CMPM);

                scheduler_irq();
  }
}

void scheduler_HAL_LOOPTIM_Init(void)
{

    WA_LOOPTIM_RCC_ENABLE();

    hlooptim.Instance = WA_LOOPTIM;
    hlooptim.Init.Prescaler = SystemCoreClock / WA_LOOPTIM_TIM_FREQ;
    hlooptim.Init.Period = 1;
    hlooptim.Init.ClockDivision = 0;
    hlooptim.Init.CounterMode = TIM_COUNTERMODE_UP;

    HAL_TIM_Base_Init(&hlooptim);
    HAL_TIM_Base_Start_IT(&hlooptim);

    WA_LOOPTIM_RCC_ENABLE();
    HAL_NVIC_SetPriority(WA_LOOPTIM_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(WA_LOOPTIM_IRQn);

}

void WA_LOOPTIM_IRQHandler(void)
{
 	if(__HAL_TIM_GET_FLAG(&hlooptim, TIM_FLAG_UPDATE) != RESET){
		if(__HAL_TIM_GET_IT_SOURCE(&hlooptim, TIM_IT_UPDATE) != RESET){
			__HAL_TIM_CLEAR_IT(&hlooptim, TIM_IT_UPDATE);

                     scheduler_run_callbacks();

		}
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
	__HAL_LPTIM_ENABLE_IT(&hlptim, LPTIM_IT_CMPM);
}

void scheduler_HAL_cc_irq_disable(uint8_t chan)
{
	__HAL_LPTIM_DISABLE_IT(&hlptim, LPTIM_IT_CMPM);
}

void scheduler_HAL_loop_irq_enable(uint8_t chan)
{
	HAL_NVIC_EnableIRQ(WA_LOOPTIM_IRQn);
}

void scheduler_HAL_loop_irq_disable(uint8_t chan)
{
	HAL_NVIC_DisableIRQ(WA_LOOPTIM_IRQn);
}


void scheduler_HAL_cc_set(uint8_t chan, uint16_t data)
{
	hlptim.Instance->CMP = data;
}

uint16_t scheduler_HAL_cc_get(uint8_t chan)
{
	return hlptim.Instance->CMP;
}


uint16_t scheduler_HAL_cnt_get(uint8_t chan)
{
  //static uint16_t prev = 0;
  uint16_t timer = (uint16_t) hlptim.Instance->CNT;

  //if((timer < prev) && ((prev - timer) < 10000))
  //{
  //  return prev;
  //}
  //prev = timer;
  return timer;
}

uint8_t scheduler_HAL_check_cc_irq(uint8_t chan)
{
  return (__HAL_LPTIM_GET_IT_SOURCE(&hlptim, LPTIM_IT_CMPM) != RESET) && __HAL_LPTIM_GET_FLAG(&hlptim, LPTIM_IT_CMPM);
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

  _scheduler = scheduler_init(&scheduler_hal_struct, 1);

  scheduler_HAL_LPTIM_Init();
  HAL_LPTIM_Counter_Start(&hlptim, 0xffff);
  scheduler_HAL_LOOPTIM_Init();
}


void scheduler_HAL_lock_unlock(uint8_t lock)
{
     if(lock)  scheduler_hal_struct.__loop_irq_disable();
     else    scheduler_hal_struct.__loop_irq_enable();
}
