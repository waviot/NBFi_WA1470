#include <stdio.h>
#include "NuMicro.h"
#include "radio.h"


#define CLK_HIRC    1
#define CLK_HXT     0
#define CLK_SOURCE  CLK_HIRC
#define PLL_CLOCK   FREQ_48MHZ

#if defined (__GNUC__) && !defined(__ARMCC_VERSION) && defined(OS_USE_SEMIHOSTING)
    extern void initialise_monitor_handles(void);
#endif

/*void TMR0_IRQHandler(void)
{
    // Clear wake up flag
    TIMER_ClearWakeupFlag(TIMER0);
    // Clear interrupt flag
    TIMER_ClearIntFlag(TIMER0);
}*/

/*---------------------------------------------------------------------------------------------------------*/
/*              Enable the external 32768Hz XTAL Clock     */
/*---------------------------------------------------------------------------------------------------------*/
void LXT_Enable(void)
{
    /* Set X32_OUT(PF.4) and X32_IN(PF.5) to input mode to prevent leakage */
    PF->MODE &= ~(GPIO_MODE_MODE4_Msk | GPIO_MODE_MODE5_Msk);

    /* Enable external 32768Hz XTAL */
    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);

    /* Disable digital input path of analog pin X32_OUT to prevent leakage */
    GPIO_DISABLE_DIGITAL_PATH(PF, BIT4 | BIT5);
}

void SYS_Init(void)
{
	SYS_UnlockReg();
	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

	/* Waiting for Internal RC clock ready */
	CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

	/* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

	LXT_Enable();

	/* Enable GPIO clock */
	CLK_EnableModuleClock(GPA_MODULE);
	CLK_EnableModuleClock(GPB_MODULE);
	CLK_EnableModuleClock(GPC_MODULE);
	CLK_EnableModuleClock(GPD_MODULE);
	CLK_EnableModuleClock(GPF_MODULE);

	SystemCoreClockUpdate();

	CLK_EnableSysTick(CLK_CLKSEL0_STCLKSEL_HCLK, SystemCoreClock / 1000 - 1);

	SYS_LockReg();
}


void GPIO_Init()
{
    //set all GPIOs to push-pull 0 level
    PA->DOUT = 0;
    PB->DOUT = 0;
    PC->DOUT = 0;
    PD->DOUT = 0;
    PF->DOUT = 0;
    PA->MODE = 0xAAAAAAAA;
    PB->MODE = 0xAAAAAAAA;
    PC->MODE = 0xAAAAAAAA;
    PD->MODE = 0xAAAAAAAA;
    PF->MODE = 0xAAAAAAAA;
}


void nbfi_receive_complete(uint8_t * data, uint16_t length)
{
    NBFi_Send(data, length);
}


void toggle_pin(struct scheduler_desc *desc)
{
   static uint8_t pin = 0;
   if(pin)
   {
       pin = 0;
       PB2 = 0;
   }
   else
   {
       pin = 1;
       PB2 = 1;
   }
}

int main(void)
{

  SYS_Init();

  GPIO_Init();

  radio_init();

  GPIO_SetMode(PB, BIT2, GPIO_MODE_OUTPUT);

  struct scheduler_desc pin_desc;

  scheduler_add_task(&pin_desc, toggle_pin, RUN_CONTINUOSLY_RELATIVE, SECONDS(1));

  SYS_UnlockReg();

  while (1)
  {
    NBFI_Main_Level_Loop();
    if (NBFi_can_sleep() && scheduler_can_sleep())
    {
        SYS_UnlockReg();
        CLK_PowerDown();
        SYS_LockReg();
    }
    else
    {
        SYS_UnlockReg();
        CLK_Idle();
        SYS_LockReg();
    }

  }


}

