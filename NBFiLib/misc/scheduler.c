#include "scheduler.h"




void scheduler_reg_func(uint8_t name, void *fn)
{
  #ifdef WTIMER  
	switch(name)
	{
	case SCHEDULER_GLOBAL_IRQ_ENABLE:
                wtimer_reg_func(WTIMER_GLOBAL_IRQ_ENABLE, fn);
		break;
	case SCHEDULER_GLOBAL_IRQ_DISABLE:
		wtimer_reg_func(WTIMER_GLOBAL_IRQ_DISABLE, fn);
		break;
	case SCHEDULER_CC_IRQ_ENABLE:
		wtimer_reg_func(WTIMER_CC_IRQ_ENABLE, fn);
		break;
	case SCHEDULER_CC_IRQ_DISABLE:
		wtimer_reg_func(WTIMER_CC_IRQ_DISABLE, fn);
		break;
	case SCHEDULER_SET_CC:
		wtimer_reg_func(WTIMER_SET_CC, fn);
		break;
	case SCHEDULER_GET_CC:
		wtimer_reg_func(WTIMER_GET_CC, fn);
		break;
	case SCHEDULER_GET_CNT:
		wtimer_reg_func(WTIMER_GET_CNT, fn);
		break;
	case SCHEDULER_CHECK_CC_IRQ:
		wtimer_reg_func(WTIMER_CHECK_CC_IRQ, fn);
		break;
	default:
		break;
	}
#else
        switch(name)
	{
	case SCHEDULER_GLOBAL_IRQ_ENABLE:
                watimer_reg_func(WATIMER_GLOBAL_IRQ_ENABLE, fn);
		break;
	case SCHEDULER_GLOBAL_IRQ_DISABLE:
		watimer_reg_func(WATIMER_GLOBAL_IRQ_DISABLE, fn);
		break;
	case SCHEDULER_CC_IRQ_ENABLE:
		watimer_reg_func(WATIMER_CC_IRQ_ENABLE, fn);
		break;
	case SCHEDULER_CC_IRQ_DISABLE:
		watimer_reg_func(WATIMER_CC_IRQ_DISABLE, fn);
		break;
	case SCHEDULER_SET_CC:
		watimer_reg_func(WATIMER_SET_CC, fn);
		break;
	case SCHEDULER_GET_CC:
		watimer_reg_func(WATIMER_GET_CC, fn);
		break;
	case SCHEDULER_GET_CNT:
		watimer_reg_func(WATIMER_GET_CNT, fn);
		break;
	case SCHEDULER_CHECK_CC_IRQ:
		watimer_reg_func(WATIMER_CHECK_CC_IRQ, fn);
		break;
	default:
		break;
	}
#endif
}

void scheduler_init()
{
  #ifdef WTIMER
  wtimer_init();
  #else 
  watimer_init();
  #endif
}

void scheduler_irq()
{
  #ifdef WTIMER
  wtimer_cc0_irq();
  #else   
  watimer_irq();
  #endif
}

void scheduler_run_callbacks()
{
  #ifdef WTIMER
  wtimer_runcallbacks();
  #else
  watimer_run_callbacks();
  #endif
}

uint32_t scheduler_curr_time()
{
  #ifdef WTIMER
  return wtimer_state[0].time.cur;
  #else
  return watimer_update_time();
  #endif
}

void scheduler_add_task(struct scheduler_desc *desc, scheduler_desc_handler_t handler, uint8_t relative, uint32_t time)
{
  #ifdef WTIMER
  ScheduleTask(desc, handler, relative, time);
  #else
  watimer_add_callback(desc, handler, (watimer_run_mode_en)relative, time);
  #endif
}

void scheduler_remove_task(struct scheduler_desc *desc)
{
  #ifdef WTIMER
  wtimer0_remove(desc);
  #else
  watimer_remove_callback(desc);
  #endif
}

uint32_t scheduler_current_time()
{
  #ifdef WTIMER
  return wtimer_state[0].time.cur;
  #else
  return watimer_time;
  #endif
}