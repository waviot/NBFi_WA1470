
#include "wtimer.h"

#define WTIMER0_MARGIN       0x1000
#define WTIMER1_MARGIN       0x1000
#define WTIMER_LPXOSC_SLEEP  8

struct wtimer_state wtimer_state[2];
struct wtimer_callback * wtimer_pending;


/*
void (* __global_irq_enable)(void);
void (* __global_irq_disable)(void);
void (* __cc_irq_enable)(uint8_t chan);
void (* __cc_irq_disable)(uint8_t chan);
void (* __cc_set)(uint8_t chan, uint16_t data);
uint16_t (* __cc_get)(uint8_t chan);
uint16_t (* __cnt_get)(uint8_t chan);
uint8_t (* __check_cc_irq)(uint8_t chan);
*/

wtimer_HAL_st *wtimer_hal = 0;

/*
void wtimer_reg_func(uint8_t name, void *fn)
{
	switch(name)
	{
	case WTIMER_GLOBAL_IRQ_ENABLE:
		__global_irq_enable = (void(*)(void))fn;
		break;
	case WTIMER_GLOBAL_IRQ_DISABLE:
		__global_irq_disable = (void(*)(void))fn;
		break;
	case WTIMER_CC_IRQ_ENABLE:
		__cc_irq_enable = (void(*)(uint8_t))fn;
		break;
	case WTIMER_CC_IRQ_DISABLE:
		__cc_irq_disable = (void(*)(uint8_t))fn;
		break;
	case WTIMER_SET_CC:
		__cc_set = (void(*)(uint8_t,uint16_t))fn;
		break;
	case WTIMER_GET_CC:
		__cc_get = (uint16_t(*)(uint8_t))fn;
		break;
	case WTIMER_GET_CNT:
		__cnt_get = (uint16_t(*)(uint8_t))fn;
		break;
	case WTIMER_CHECK_CC_IRQ:
		__check_cc_irq = (uint8_t(*)(uint8_t))fn;
		break;
	default:
		break;
	}
}

*/

void wtimer_set_HAL(wtimer_HAL_st *ptr)
{
  wtimer_hal = ptr;
}

void wtimer_cc0_irq(void)
{
	wtimer_hal->__cc_irq_disable(0);
	wtimer0_update();
	wtimer0_schedq();
}

static void wtimer_doinit(uint8_t wakeup)
{
	wtimer_hal->__cc_irq_disable(0);
	wtimer_pending = WTIMER_NULLCB;
	if (wakeup) {
		wtimer_state[0].time.ref = 0;
		wtimer0_update();
	} else {
		wtimer_state[0].time.ref = wtimer_hal->__cnt_get(0);
		wtimer_state[0].time.cur = 0;
		wtimer_state[0].queue = WTIMER_NULLDESC;
	}
	wtimer0_schedq();
	wtimer_hal->__cc_irq_enable(0);
}

void wtimer_init(void)
{
        if(wtimer_hal == 0) while(1); //HAL struct must be configured before library usage 
	wtimer_doinit(0);
}

void wtimer_init_deepsleep(void)
{
	wtimer_doinit(0);
}

// Must be called with (wtimer) interrupts disabled
void wtimer_addcb_core(struct wtimer_callback *desc)
{
	struct wtimer_callback *d = WTIMER_CBPTR(wtimer_pending);
	for (;;) {
		struct wtimer_callback *dn = d->next;
		if (dn == WTIMER_NULLCB)
			break;
		d = dn;
	}
	d->next = (struct wtimer_callback *)desc;
	desc->	next = WTIMER_NULLCB;
}

// Must be called with (wtimer) interrupts disabled
void wtimer0_schedq(void)
{
	while (wtimer_state[0].queue != WTIMER_NULLDESC) {
		int32_t td = wtimer_state[0].time.cur - wtimer_state[0].queue->time;
		if (td >= 0) {
			struct wtimer_callback * d = (struct wtimer_callback *)wtimer_state[0].queue;
			wtimer_state[0].queue = wtimer_state[0].queue->next;
			wtimer_addcb_core(d);
			continue;
		}
		if (td < -(0x10000-WTIMER0_MARGIN))
			break;
		{
			uint16_t nxt = wtimer_state[0].time.ref - (uint16_t)td;
			wtimer_hal->__cc_set(0, nxt);
			wtimer_hal->__cc_irq_enable(0);
		}
		return;
	}
	{
		uint16_t nxt = wtimer_state[0].time.ref + (uint16_t)(0x10000-WTIMER0_MARGIN);
		wtimer_hal->__cc_set(0, nxt);
		wtimer_hal->__cc_irq_enable(0);
	}
}

// Must be called with (wtimer) interrupts disabled
void wtimer0_update(void)
{
	uint16_t t;
	t = wtimer_hal->__cnt_get(0);
	{
		uint16_t t1 =  wtimer_state[0].time.ref;
		wtimer_state[0].time.ref = t;
		t -= t1;
	}
	if (!t)
		return;
	wtimer_state[0].time.cur += t;
}

// Must be called with (wtimer) interrupts disabled
void wtimer0_addcore(struct wtimer_desc *desc)
{
	struct wtimer_desc * d = WTIMER_PTR(wtimer_state[0].queue);
	for (;;) {
		struct wtimer_desc * dn = d->next;
		int32_t td;
		if (dn == WTIMER_NULLDESC)
			break;
		td = desc->time - dn->time;
		if (td < 0)
			break;
		d = dn;
	}
	desc->next = d->next;
	d->next = desc;
}

static uint8_t wtimer_checkexpired(void)
{
	{
		uint16_t t;
		t = wtimer_hal->__cnt_get(0) - wtimer_hal->__cc_get(0);
		if (t < WTIMER0_MARGIN)
			return 1;
	}

	if (wtimer_hal->__check_cc_irq(0) != 0)
        {
          return 1;
        }
        else
        {
          
          return 0;
        }
}

uint8_t wtimer_cansleep(void)
{
	return wtimer_state[1].queue == WTIMER_NULLDESC;
}

typedef void (*handler_t)(struct wtimer_callback *desc);

/*
 * This function is reentrant even though it is not marked reentrant.
 * When marked reentrant, code generation gets worse for SDCC
 * (IE is placed on stack rather than a register)
 */

uint8_t wtimer_runcallbacks(void)
{
	uint8_t ret = 0;
	for (;;) {
		wtimer_hal->__global_irq_disable();
		wtimer0_update();
		wtimer0_schedq();
		for (;;) {
			{
				struct wtimer_callback * d = wtimer_pending;
				if (d != WTIMER_NULLCB) {
					wtimer_pending = d->next;
					wtimer_hal->__global_irq_enable();
					++ret;
					((handler_t)(d->handler))(d);
					wtimer_hal->__global_irq_disable();
					continue;
				}
			}
			{
				uint8_t exp = wtimer_checkexpired();
				wtimer_hal->__global_irq_enable();
				if (exp)
					break;
				return ret;
			}
		}
	}
}

void ScheduleTask(struct wtimer_desc *desc, wtimer_desc_handler_t handler, uint8_t relative, uint32_t time)
{
    wtimer0_remove(desc);
    desc->time = time;
    if(relative)    desc->time += wtimer0_curtime();
    if(handler)     desc->handler = handler;
    wtimer0_addabsolute(desc);
}

uint8_t CheckTask(struct wtimer_desc *desc)
{
    uint8_t status = wtimer0_remove(desc);
    if (status) wtimer0_addabsolute(desc);
    return status;
}

uint8_t wtimer_idle(uint8_t flags)
{
	return 0;
}

