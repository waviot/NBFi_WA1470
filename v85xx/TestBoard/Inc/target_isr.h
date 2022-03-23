/**
  * @file    target_isr.h
  * @author  Application Team
  * @version V4.4.0
  * @date    2018-09-27
  * @brief   This file contains the headers of the interrupt handlers.
******************************************************************************/

#ifndef __TARGET_ISR_H
#define __TARGET_ISR_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "target.h"

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void PMU_IRQHandler(void);
void RTC_IRQHandler(void);
void U32K0_IRQHandler(void);
void U32K1_IRQHandler(void);
void I2C_IRQHandler(void);
void SPI1_IRQHandler(void);
void UART0_IRQHandler(void);
void UART1_IRQHandler(void);
void UART2_IRQHandler(void);
void UART3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void ISO78160_IRQHandler(void);
void ISO78161_IRQHandler(void);
void TMR0_IRQHandler(void);
void TMR1_IRQHandler(void);
void TMR2_IRQHandler(void);
void TMR3_IRQHandler(void);
void PWM0_IRQHandler(void);
void PWM1_IRQHandler(void);
void PWM2_IRQHandler(void);
void PWM3_IRQHandler(void);
void DMA_IRQHandler(void);
void FLASH_IRQHandler(void);
void ANA_IRQHandler(void);
void SPI2_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif

/*********************************** END OF FILE ******************************/
