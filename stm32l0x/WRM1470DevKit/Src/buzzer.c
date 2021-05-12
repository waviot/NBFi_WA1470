#include "stm32l0xx_hal_conf.h"


#define BUZZER_TIMER			            TIM6
#define BUZZER_TIMER_IRQn		            TIM6_IRQn
#define BUZZER_TIMER_RCC_ENABLE 	        __HAL_RCC_TIM6_CLK_ENABLE
#define BUZZER_TIMER_RCC_DISABLE 	        __HAL_RCC_TIM6_CLK_DISABLE
#define BUZZER_TIMER_TIM_FREQ		        4000// 2000 //500Hz
#define BUZZER_TIMER_IRQHandler             TIM6_IRQHandler


#define ADC9_Port 	    GPIOB
#define ADC9_Pin 		GPIO_PIN_1


static TIM_HandleTypeDef hbuzzertim;

_Bool buzzer_enabled = 0;

void BUZZER_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = ADC9_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ADC9_Port, &GPIO_InitStruct);


    BUZZER_TIMER_RCC_ENABLE();

    hbuzzertim.Instance = BUZZER_TIMER;
    hbuzzertim.Init.Prescaler = SystemCoreClock / BUZZER_TIMER_TIM_FREQ;
    hbuzzertim.Init.Period = 1;
    hbuzzertim.Init.ClockDivision = 0;
    hbuzzertim.Init.CounterMode = TIM_COUNTERMODE_UP;

    HAL_TIM_Base_Init(&hbuzzertim);
    HAL_TIM_Base_Start_IT(&hbuzzertim);

    BUZZER_TIMER_RCC_ENABLE();
    HAL_NVIC_SetPriority(BUZZER_TIMER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(BUZZER_TIMER_IRQn);

    buzzer_enabled = 1;

}

void BUZZER_Deinit(void)
{

    HAL_NVIC_DisableIRQ(BUZZER_TIMER_IRQn);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = ADC9_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ADC9_Port, &GPIO_InitStruct);

    BUZZER_TIMER_RCC_DISABLE();

    buzzer_enabled = 0;

}


void BUZZER_Enable(_Bool en)
{
    if(en) BUZZER_Init();
    else BUZZER_Deinit();
}

void BUZZER_Set_Freq(uint32_t freq)
{
    hbuzzertim.Init.Prescaler = SystemCoreClock/(freq*4);
    HAL_TIM_Base_Init(&hbuzzertim);
}

void BUZZER_TIMER_IRQHandler(void)
{

    static GPIO_PinState zoomer = GPIO_PIN_RESET;

 	if(__HAL_TIM_GET_FLAG(&hbuzzertim, TIM_FLAG_UPDATE) != RESET){
		if(__HAL_TIM_GET_IT_SOURCE(&hbuzzertim, TIM_IT_UPDATE) != RESET){
			__HAL_TIM_CLEAR_IT(&hbuzzertim, TIM_IT_UPDATE);

        HAL_GPIO_WritePin(ADC9_Port, ADC9_Pin,  zoomer);

        if(zoomer == GPIO_PIN_RESET) zoomer = GPIO_PIN_SET;
        else zoomer = GPIO_PIN_RESET;

		}
	}
}