/**
  * @file    main.c
  * @author  Application Team
  * @version V4.4.0
  * @date    2018-09-27
  * @brief   Main program body.
******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "radio.h"


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Clock_Init:
              - PLLL input clock    : External 32K crystal
              - PLLL frequency      : 26M
              - AHB Clock source    : PLLL
              - AHB Clock frequency : 26M (PLLL divided by 1)
              - APB Clock frequency : 13M (AHB Clock divided by 2)
  * @param  None
  * @retval None
  */
void Clock_Init(void)
{
  CLK_InitTypeDef CLK_Struct;

  CLK_Struct.ClockType = CLK_TYPE_AHBSRC \
                        |CLK_TYPE_PLLL   \
                        |CLK_TYPE_HCLK   \
                        |CLK_TYPE_PCLK;
  CLK_Struct.AHBSource      = CLK_AHBSEL_LSPLL;
  CLK_Struct.PLLL.Frequency = CLK_PLLL_26_2144MHz;
  CLK_Struct.PLLL.Source    = CLK_PLLLSRC_XTALL;
  CLK_Struct.PLLL.State     = CLK_PLLL_ON;
  CLK_Struct.HCLK.Divider   = 1;
  CLK_Struct.PCLK.Divider   = 2;
  CLK_ClockConfig(&CLK_Struct);
}

void RTC_Init()
{
  RTC_WKUSecondsConfig(1);
  PMU_SleepWKUSRC_Config_RTC(PMU_RTCEVT_WKUSEC, 2);
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void toggle_pin(struct scheduler_desc *desc)
{
     static uint8_t pin = 0;
     if(!pin) pin = 1;
     else pin = 0;
     GPIO_InitType GPIO_InitStruct;
       /* IOB7, CMOS output mode, output low */
      GPIOBToF_ResetBits(GPIOB, GPIO_Pin_7);
      GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUTPUT_CMOS;
      GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
      GPIOBToF_Init(GPIOB, &GPIO_InitStruct);
    GPIOBToF_WriteBit(GPIOB, GPIO_Pin_7, pin/*!GPIOBToF_ReadOutputDataBit(GPIOB, GPIO_Pin_7)*/);
}

uint8_t go_to_sleep()
{
  GPIO_InitType GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_FORBIDDEN;
  GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_All;
  //GPIOA_Init(GPIOA, &GPIO_InitStruct);
  //GPIOBToF_Init(GPIOB, &GPIO_InitStruct);
  //GPIOBToF_Init(GPIOC, &GPIO_InitStruct);
  //GPIOBToF_Init(GPIOD, &GPIO_InitStruct);
  //GPIOBToF_Init(GPIOE, &GPIO_InitStruct);
  GPIOBToF_Init(GPIOF, &GPIO_InitStruct);

  PMU_LowPWRTypeDef LowPower_InitStruct;
  LowPower_InitStruct.AHBPeriphralDisable = PMU_AHB_LCD|PMU_AHB_CRYPT|PMU_AHB_DMA;//PMU_AHB_ALL;
  LowPower_InitStruct.APBPeriphralDisable = PMU_APB_ALL;
  LowPower_InitStruct.BGPPower            = PMU_BGPPWR_OFF;
  LowPower_InitStruct.COMP1Power          = PMU_COMP1PWR_OFF;
  LowPower_InitStruct.COMP2Power          = PMU_COMP2PWR_OFF;
  LowPower_InitStruct.LCDPower            = PMU_LCDPWER_OFF;
  LowPower_InitStruct.AVCCPower           = PMU_AVCCPWR_ON;
  LowPower_InitStruct.TADCPower           = PMU_TADCPWR_OFF;
  LowPower_InitStruct.VDCINDetector       = PMU_VDCINDET_DISABLE;
  LowPower_InitStruct.VDDDetector         = PMU_VDDDET_DISABLE;
  return  PMU_EnterSleep_LowPower(&LowPower_InitStruct);
}





int main(void)
{

  Clock_Init();

  radio_init();

  RTC_Init();

  struct scheduler_desc pin_desc;

  scheduler_add_task(&pin_desc, toggle_pin, RUN_CONTINUOSLY_RELATIVE, SECONDS(5));


  while (1)
  {
      /* Disable Watch Dog Timer */
      if(scheduler_HAL_can_sleep()&&0)
      {
          WDT_Disable();
          __disable_irq();
          if(!go_to_sleep())
          {
            CLK_AHBPeriphralCmd(CLK_AHBPERIPHRAL_ALL, ENABLE);
            CLK_APBPeriphralCmd(CLK_APBPERIPHRAL_ALL, ENABLE);
              //Clock_Init();
            scheduler_HAL_reinit();
          }
          __enable_irq();
      }
      else
      {
        WDT_Clear();
      }

    WDT_Clear();
  }
}

#ifndef  ASSERT_NDEBUG
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_errhandler error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_errhandler error line source number
  * @retval None
  */
void assert_errhandler(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*********************************** END OF FILE ******************************/
