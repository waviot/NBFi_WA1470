#ifndef WATIMER_H
#define WATIMER_H


extern volatile uint32_t watimer_time;


typedef enum{RUN_SINGLE_ABSOLUTE, RUN_SINGLE_RELATIVE, RUN_CONTINUOSLY_RELATIVE} watimer_run_mode_en;


struct watimer_callback_st;


typedef void(*watimer_callback_func)(struct watimer_callback_st *desc);


struct watimer_callback_st
{
	watimer_run_mode_en	level;
	uint32_t 		period;
	uint32_t		timer;
	watimer_callback_func func;
	uint8_t 	timeout;
};


#define MAXCALLBACKS    20


#define SECONDS(x) ((uint32_t)(x) * 1000)
#define MILLISECONDS(x) (x)

enum watimer_func_t
{
	WATIMER_GLOBAL_IRQ_ENABLE,
	WATIMER_GLOBAL_IRQ_DISABLE,
	WATIMER_CC_IRQ_ENABLE,
	WATIMER_CC_IRQ_DISABLE,
	WATIMER_SET_CC,
	WATIMER_GET_CC,
	WATIMER_GET_CNT,
	WATIMER_CHECK_CC_IRQ,
};


void watimer_init(void);
void watimer_reg_func(uint8_t name, void *fn);
void watimer_irq(void);
void watimer_run_callbacks();
void watimer_add_callback(struct watimer_callback_st *desc, watimer_callback_func cb, watimer_run_mode_en run_level, uint32_t period);
void watimer_remove_callback(struct watimer_callback_st *desc);
_Bool watimer_check_callback(struct watimer_callback_st *desc);
uint32_t watimer_update_time();


#endif //WATIMER_H