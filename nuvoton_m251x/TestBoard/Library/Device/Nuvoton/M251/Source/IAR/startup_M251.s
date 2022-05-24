;/******************************************************************************
; * @file     startup_M251.s
; * @version  V0.10
; * @brief    CMSIS Cortex-M23 Core Device Startup File for M251
; *
; * SPDX-License-Identifier: Apache-2.0
; * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
;*****************************************************************************/

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        ;EXTERN  HardFault_Handler
        EXTERN  ProcessHardFault
        EXTERN  SystemInit
        PUBLIC  __vector_table
       ; PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA

__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     MemManage_Handler
        DCD     BusFault_Handler
        DCD     UsageFault_Handler
;__vector_table_0x1c
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     0
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

        ; External Interrupts
        DCD     BOD_IRQHandler            ; 0: Brown Out detection
        DCD     IRCTRIM_IRQHandler        ; 1: Internal RC
        DCD     PWRWU_IRQHandler          ; 2: Power down wake up
        DCD     DEFAULT_IRQHandler        ; 3: Reserved
        DCD     CLKFAIL_IRQHandler        ; 4: Clock detection fail
        DCD     DEFAULT_IRQHandler        ; 5: Reserved
        DCD     RTC_IRQHandler            ; 6: Real Time Clock
        DCD     TAMPER_IRQHandler         ; 7: Tamper detection
        DCD     WDT_IRQHandler            ; 8: Watchdog timer
        DCD     WWDT_IRQHandler           ; 9: Window watchdog timer
        DCD     EINT0_IRQHandler          ; 10: External Input 0
        DCD     EINT1_IRQHandler          ; 11: External Input 1
        DCD     EINT2_IRQHandler          ; 12: External Input 2
        DCD     EINT3_IRQHandler          ; 13: External Input 3
        DCD     EINT4_IRQHandler          ; 14: External Input 4
        DCD     EINT5_IRQHandler          ; 15: External Input 5
        DCD     GPA_IRQHandler            ; 16: GPIO Port A
        DCD     GPB_IRQHandler            ; 17: GPIO Port B
        DCD     GPC_IRQHandler            ; 18: GPIO Port C
        DCD     GPD_IRQHandler            ; 19: GPIO Port D
        DCD     GPE_IRQHandler            ; 20: GPIO Port E
        DCD     GPF_IRQHandler            ; 21: GPIO Port F
        DCD     QSPI0_IRQHandler          ; 22: QSPI0
        DCD     SPI0_IRQHandler           ; 23: SPI0
        DCD     BRAKE0_IRQHandler         ; 24: BEAKE0
        DCD     PWM0_P0_IRQHandler        ; 25: PWM0_0
        DCD     PWM0_P1_IRQHandler        ; 26: PWM0_1
        DCD     PWM0_P2_IRQHandler        ; 27: PWM0_2
        DCD     BRAKE1_IRQHandler         ; 28: BREAK1
        DCD     PWM1_P0_IRQHandler        ; 29: PWM1_0
        DCD     PWM1_P1_IRQHandler        ; 30: PWM1_1
        DCD     PWM1_P2_IRQHandler        ; 31: PWM1_2
        DCD     TMR0_IRQHandler           ; 32: Timer 0
        DCD     TMR1_IRQHandler           ; 33: Timer 1
        DCD     TMR2_IRQHandler           ; 34: Timer 2
        DCD     TMR3_IRQHandler           ; 35: Timer 3
        DCD     UART0_IRQHandler          ; 36: UART0
        DCD     UART1_IRQHandler          ; 37: UART1
        DCD     I2C0_IRQHandler           ; 38: I2C0
        DCD     I2C1_IRQHandler           ; 39: I2C1
        DCD     PDMA_IRQHandler           ; 40: Peripheral DMA
        DCD     DAC_IRQHandler            ; 41: DAC
        DCD     EADC_INT0_IRQHandler      ; 42: EADC interrupt source 0
        DCD     EADC_INT1_IRQHandler      ; 43: EADC interrupt source 1
        DCD     ACMP01_IRQHandler         ; 44: ACMP0 and ACMP1
        DCD     BPWM0_IRQHandler          ; 45: BPWM0
        DCD     EADC_INT2_IRQHandler      ; 46: EADC interrupt source 2
        DCD     EADC_INT3_IRQHandler      ; 47: EADC interrupt source 3
        DCD     UART2_IRQHandler          ; 48: UART2
        DCD     DEFAULT_IRQHandler        ; 49: Reserved
        DCD     USCI0_IRQHandler          ; 50: USCI0
        DCD     SPI1_IRQHandler           ; 51: SPI1
        DCD     USCI1_IRQHandler          ; 52: USCI1
        DCD     USBD_IRQHandler           ; 53: USB device
        DCD     BPWM1_IRQHandler          ; 54: BPWM1
        DCD     PSIO_IRQHandler           ; 55: PSIO
        DCD     DEFAULT_IRQHandler        ; 56: Reserved
        DCD     CRPT_IRQHandler           ; 57: CRPT
        DCD     SC0_IRQHandler            ; 58: Smart Card0
        DCD     DEFAULT_IRQHandler        ; 59: Reserved
        DCD     USCI2_IRQHandler          ; 60: USCI2
        DCD     LCD_IRQHandler            ; 61: LCD
        DCD     OPA_IRQHandler            ; 62: OPA
        DCD     TK_IRQHandler             ; 63: TK
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
Reset_Handler
        ; Unlock Register
        LDR     R0, =0x40000100
        LDR     R1, =0x59
        STR     R1, [R0]
        LDR     R1, =0x16
        STR     R1, [R0]
        LDR     R1, =0x88
        STR     R1, [R0]

        LDR     R0, =SystemInit
        BLX     R0

        ; Init POR
        LDR     R2, =0x40000024
        LDR     R1, =0x00005AA5
        STR     R1, [R2]

	    LDR     R2, =0x400001EC
        STR     R1, [R2]

        ; Lock register
        LDR     R0, =0x40000100
        MOVS    R1, #0
        STR     R1, [R0]

        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
HardFault_Handler
        MOV     R0, LR
        MRS     R1, MSP
        MRS     R2, PSP
        LDR     R3, =ProcessHardFault
        BLX     R3
        BX      R0

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B SVC_Handler

        ;PUBWEAK DebugMon_Handler
        ;SECTION .text:CODE:REORDER:NOROOT(1)
;DebugMon_Handler
        ;B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B SysTick_Handler

        PUBWEAK  BOD_IRQHandler
        PUBWEAK  IRCTRIM_IRQHandler
        PUBWEAK  PWRWU_IRQHandler
        PUBWEAK  CLKFAIL_IRQHandler
        PUBWEAK  RTC_IRQHandler
        PUBWEAK  TAMPER_IRQHandler
        PUBWEAK  WDT_IRQHandler
        PUBWEAK  WWDT_IRQHandler
        PUBWEAK  EINT0_IRQHandler
        PUBWEAK  EINT1_IRQHandler
        PUBWEAK  EINT2_IRQHandler
        PUBWEAK  EINT3_IRQHandler
        PUBWEAK  EINT4_IRQHandler
        PUBWEAK  EINT5_IRQHandler
        PUBWEAK  GPA_IRQHandler
        PUBWEAK  GPB_IRQHandler
        PUBWEAK  GPC_IRQHandler
        PUBWEAK  GPD_IRQHandler
        PUBWEAK  GPE_IRQHandler
        PUBWEAK  GPF_IRQHandler
        PUBWEAK  QSPI0_IRQHandler
        PUBWEAK  SPI0_IRQHandler
        PUBWEAK  BRAKE0_IRQHandler
        PUBWEAK  PWM0_P0_IRQHandler
        PUBWEAK  PWM0_P1_IRQHandler
        PUBWEAK  PWM0_P2_IRQHandler
        PUBWEAK  BRAKE1_IRQHandler
        PUBWEAK  PWM1_P0_IRQHandler
        PUBWEAK  PWM1_P1_IRQHandler
        PUBWEAK  PWM1_P2_IRQHandler
        PUBWEAK  TMR0_IRQHandler
        PUBWEAK  TMR1_IRQHandler
        PUBWEAK  TMR2_IRQHandler
        PUBWEAK  TMR3_IRQHandler
        PUBWEAK  UART0_IRQHandler
        PUBWEAK  UART1_IRQHandler
        PUBWEAK  I2C0_IRQHandler
        PUBWEAK  I2C1_IRQHandler
        PUBWEAK  PDMA_IRQHandler
        PUBWEAK  DAC_IRQHandler
        PUBWEAK  EADC_INT0_IRQHandler
        PUBWEAK  EADC_INT1_IRQHandler
        PUBWEAK  ACMP01_IRQHandler
        PUBWEAK  BPWM0_IRQHandler
        PUBWEAK  EADC_INT2_IRQHandler
        PUBWEAK  EADC_INT3_IRQHandler
        PUBWEAK  UART2_IRQHandler
        PUBWEAK  USCI0_IRQHandler
        PUBWEAK  SPI1_IRQHandler
        PUBWEAK  USCI1_IRQHandler
        PUBWEAK  USBD_IRQHandler
        PUBWEAK  BPWM1_IRQHandler
        PUBWEAK  PSIO_IRQHandler
        PUBWEAK  CRPT_IRQHandler
        PUBWEAK  SC0_IRQHandler
        PUBWEAK  USCI2_IRQHandler
        PUBWEAK  LCD_IRQHandler
        PUBWEAK  OPA_IRQHandler
        PUBWEAK  TK_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)

BOD_IRQHandler
IRCTRIM_IRQHandler
PWRWU_IRQHandler
CLKFAIL_IRQHandler
RTC_IRQHandler
TAMPER_IRQHandler
WDT_IRQHandler
WWDT_IRQHandler
EINT0_IRQHandler
EINT1_IRQHandler
EINT2_IRQHandler
EINT3_IRQHandler
EINT4_IRQHandler
EINT5_IRQHandler
GPA_IRQHandler
GPB_IRQHandler
GPC_IRQHandler
GPD_IRQHandler
GPE_IRQHandler
GPF_IRQHandler
QSPI0_IRQHandler
SPI0_IRQHandler
BRAKE0_IRQHandler
PWM0_P0_IRQHandler
PWM0_P1_IRQHandler
PWM0_P2_IRQHandler
BRAKE1_IRQHandler
PWM1_P0_IRQHandler
PWM1_P1_IRQHandler
PWM1_P2_IRQHandler
TMR0_IRQHandler
TMR1_IRQHandler
TMR2_IRQHandler
TMR3_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
PDMA_IRQHandler
DAC_IRQHandler
EADC_INT0_IRQHandler
EADC_INT1_IRQHandler
ACMP01_IRQHandler
BPWM0_IRQHandler
EADC_INT2_IRQHandler
EADC_INT3_IRQHandler
UART2_IRQHandler
USCI0_IRQHandler
SPI1_IRQHandler
USCI1_IRQHandler
USBD_IRQHandler
BPWM1_IRQHandler
PSIO_IRQHandler
CRPT_IRQHandler
SC0_IRQHandler
USCI2_IRQHandler
LCD_IRQHandler
OPA_IRQHandler
TK_IRQHandler
DEFAULT_IRQHandler
    B DEFAULT_IRQHandler


;int32_t SH_DoCommand(int32_t n32In_R0, int32_t n32In_R1, int32_t *pn32Out_R0)
          PUBWEAK SH_DoCommand
          SECTION .text:CODE:REORDER:ROOT(2)
SH_DoCommand
                IMPORT      SH_Return

                BKPT    0xAB                ; Wait ICE or HardFault
                LDR     R3, =SH_Return
		PUSH    {R3 ,lr}
                BLX     R3                  ; Call SH_Return. The return value is in R0
		POP     {R3 ,PC}            ; Return value = R0

        END
