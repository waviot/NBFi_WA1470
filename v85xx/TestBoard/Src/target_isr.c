/**
  * @file    target_isr.c
  * @author  Application Team
  * @version V4.4.0
  * @date    2018-09-27
  * @brief   Main Interrupt Service Routines.
******************************************************************************/

#include "target_isr.h"
#include "main.h"
#include "scheduler_hal.h"
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
  while(1)
  {}
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  while(1)
  {}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/**
  * @brief  This function handles PMU interrupt request.
  * @param  None
  * @retval None
  */

void PMU_IRQHandler(void)
{
    WA_EXT_IRQHandler();
}

/**
  * @brief  This function handles RTC interrupt request.
  * @param  None
  * @retval None
  */

void RTC_IRQHandler(void)
{
  WA_LPTIM_IRQHandler();
  RTC_ClearINTStatus(RTC_INTSTS_WKUMIN|RTC_INTSTS_WKUSEC);

  //toggle_pin(0);
  //GPIOBToF_WriteBit(GPIOB, GPIO_Pin_7, !GPIOBToF_ReadOutputDataBit(GPIOB, GPIO_Pin_7));
}

/**
  * @brief  This function handles U32K0 interrupt request.
  * @param  None
  * @retval None
  */
void U32K0_IRQHandler(void)
{
}

/**
  * @brief  This function handles U32K1 interrupt request.
  * @param  None
  * @retval None
  */
void U32K1_IRQHandler(void)
{
}

/**
  * @brief  This function handles I2C interrupt request.
  * @param  None
  * @retval None
  */
void I2C_IRQHandler(void)
{
}

/**
  * @brief  This function handles SPI1 interrupt request.
  * @param  None
  * @retval None
  */
void SPI1_IRQHandler(void)
{
}

/**
  * @brief  This function handles UART0 interrupt request.
  * @param  None
  * @retval None
  */
void UART0_IRQHandler(void)
{
}

/**
  * @brief  This function handles UART1 interrupt request.
  * @param  None
  * @retval None
  */
void UART1_IRQHandler(void)
{
}

/**
  * @brief  This function handles UART2 interrupt request.
  * @param  None
  * @retval None
  */
void UART2_IRQHandler(void)
{
}

/**
  * @brief  This function handles UART3 interrupt request.
  * @param  None
  * @retval None
  */
void UART3_IRQHandler(void)
{
}

/**
  * @brief  This function handles UART4 interrupt request.
  * @param  None
  * @retval None
  */
void UART4_IRQHandler(void)
{
}

/**
  * @brief  This function handles UART5 interrupt request.
  * @param  None
  * @retval None
  */
void UART5_IRQHandler(void)
{
}

/**
  * @brief  This function handles ISO78160 interrupt request.
  * @param  None
  * @retval None
  */
void ISO78160_IRQHandler(void)
{
}

/**
  * @brief  This function handles ISO78161 interrupt request.
  * @param  None
  * @retval None
  */
void ISO78161_IRQHandler(void)
{
}

/**
  * @brief  This function handles TMR0 interrupt request.
  * @param  None
  * @retval None
  */
/*void TMR0_IRQHandler(void)
{
  if (TMR_GetINTStatus(TMR0))
  {
    TMR_ClearINTStatus(TMR0);
    GPIOBToF_WriteBit(GPIOB, GPIO_Pin_7, !GPIOBToF_ReadOutputDataBit(GPIOB, GPIO_Pin_7));
  }
}*/

/**
  * @brief  This function handles TMR1 interrupt request.
  * @param  None
  * @retval None
  */
void TMR1_IRQHandler(void)
{
}

/**
  * @brief  This function handles TMR2 interrupt request.
  * @param  None
  * @retval None
  */
void TMR2_IRQHandler(void)
{
}

/**
  * @brief  This function handles TMR3 interrupt request.
  * @param  None
  * @retval None
  */
void TMR3_IRQHandler(void)
{
}

/**
  * @brief  This function handles PWM0 interrupt request.
  * @param  None
  * @retval None
  */
void PWM0_IRQHandler(void)
{
}

/**
  * @brief  This function handles PWM1 interrupt request.
  * @param  None
  * @retval None
  */
void PWM1_IRQHandler(void)
{
}

/**
  * @brief  This function handles PWM2 interrupt request.
  * @param  None
  * @retval None
  */
void PWM2_IRQHandler(void)
{
}

/**
  * @brief  This function handles PWM3 interrupt request.
  * @param  None
  * @retval None
  */
void PWM3_IRQHandler(void)
{
}

/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  */
void DMA_IRQHandler(void)
{
}

/**
  * @brief  This function handles FLASH interrupt request.
  * @param  None
  * @retval None
  */
void FLASH_IRQHandler(void)
{
}

/**
  * @brief  This function handles ANA interrupt request.
  * @param  None
  * @retval None
  */
void ANA_IRQHandler(void)
{
}

/**
  * @brief  This function handles SPI2 interrupt request.
  * @param  None
  * @retval None
  */
void SPI2_IRQHandler(void)
{
}

/*********************************** END OF FILE ******************************/
