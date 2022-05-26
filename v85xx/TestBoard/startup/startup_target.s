;/**
;* @file    startup_target.s
;* @author  Application Team
;* @version V4.4.0
;* @date    2018-10-08
;* @brief   Target Devices vector table.
;******************************************************************************/

__CHIPINITIAL   EQU    1

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)
        
        EXTERN  __iar_program_start
  IF (__CHIPINITIAL != 0)
        PUBWEAK __CHIP_INIT
  ENDIF
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler                  ; Reset Handler

        DCD     NMI_Handler                    ; NMI Handler
        DCD     HardFault_Handler              ; Hard Fault Handler
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     SVC_Handler                    ; SVCall Handler
        DCD     0                              ; Reserved
        DCD     0                              ; Reserved
        DCD     PendSV_Handler                 ; PendSV Handler
        DCD     SysTick_Handler                ; SysTick Handler

        ; External Interrupts
        DCD     PMU_IRQHandler                 ; PMU,           IRQ0
        DCD     RTC_IRQHandler                 ; RTC,           IRQ1
        DCD     U32K0_IRQHandler               ; UART 32K 0,    IRQ2
        DCD     U32K1_IRQHandler               ; UART 32K 1,    IRQ3
        DCD     I2C_IRQHandler                 ; I2C,           IRQ4
        DCD     SPI1_IRQHandler                ; SPI1,          IRQ5
        DCD     UART0_IRQHandler               ; UART0,         IRQ6
        DCD     UART1_IRQHandler               ; UART1,         IRQ7
        DCD     UART2_IRQHandler               ; UART2,         IRQ8
        DCD     UART3_IRQHandler               ; UART3,         IRQ9
        DCD     UART4_IRQHandler               ; UART4,         IRQ10
        DCD     UART5_IRQHandler               ; UART5,         IRQ11
        DCD     ISO78160_IRQHandler            ; ISO78160,      IRQ12
        DCD     ISO78161_IRQHandler            ; ISO78161,      IRQ13
        DCD     TMR0_IRQHandler                ; Timer 0,       IRQ14
        DCD     TMR1_IRQHandler                ; Timer 1,       IRQ15
        DCD     TMR2_IRQHandler                ; Timer 2,       IRQ16
        DCD     TMR3_IRQHandler                ; Timer 3,       IRQ17
        DCD     PWM0_IRQHandler                ; PWM0,          IRQ18
        DCD     PWM1_IRQHandler                ; PWM1,          IRQ19
        DCD     PWM2_IRQHandler                ; PWM2,          IRQ20
        DCD     PWM3_IRQHandler                ; PWM3,          IRQ21
        DCD     DMA_IRQHandler                 ; DMA,           IRQ22
        DCD     FLASH_IRQHandler               ; FLASH,         IRQ23
        DCD     ANA_IRQHandler                 ; ANA,           IRQ24
        DCD     0                              ; Reserved,      IRQ25
        DCD     0                              ; Reserved,      IRQ26
        DCD     SPI2_IRQHandler                ; SPI2,          IRQ27
        DCD     0                              ; Reserved,      IRQ28
        DCD     0                              ; Reserved,      IRQ29
        DCD     0                              ; Reserved,      IRQ30
        DCD     0                              ; Reserved,      IRQ31
        
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        
        PUBWEAK Reset_Handler
  IF (__CHIPINITIAL != 0)
        SECTION .Reset_Handler_text:CODE:NOROOT:REORDER(2)
Reset_Handler
        LDR     R0, =__CHIP_INIT
        BLX     R0
  ELSE
        SECTION .text:CODE:NOROOT:REORDER(2)
Reset_Handler
  ENDIF
        LDR     R0, =__iar_program_start
        BX      R0
        
        
        PUBWEAK NMI_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
NMI_Handler
        B NMI_Handler
         
        
        PUBWEAK HardFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
HardFault_Handler
        B HardFault_Handler
       
        
        PUBWEAK SVC_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SVC_Handler
        B SVC_Handler
       
        
        PUBWEAK PendSV_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
PendSV_Handler
        B PendSV_Handler
        
        
        PUBWEAK SysTick_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SysTick_Handler
        B SysTick_Handler
        
        
        PUBWEAK PMU_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PMU_IRQHandler
        B PMU_IRQHandler
        
                
        PUBWEAK RTC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTC_IRQHandler
        B RTC_IRQHandler
        
                
        PUBWEAK U32K0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
U32K0_IRQHandler
        B U32K0_IRQHandler
        
                
        PUBWEAK U32K1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
U32K1_IRQHandler
        B U32K1_IRQHandler
        
                
        PUBWEAK I2C_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C_IRQHandler
        B I2C_IRQHandler
        
                
        PUBWEAK SPI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI1_IRQHandler
        B SPI1_IRQHandler
        
                
        PUBWEAK UART0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART0_IRQHandler
        B UART0_IRQHandler
        
                
        PUBWEAK UART1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART1_IRQHandler
        B UART1_IRQHandler
        
                
        PUBWEAK UART2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART2_IRQHandler
        B UART2_IRQHandler
        
                
        PUBWEAK UART3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART3_IRQHandler
        B UART3_IRQHandler
        
                
        PUBWEAK UART4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART4_IRQHandler
        B UART4_IRQHandler
        
                
        PUBWEAK UART5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART5_IRQHandler
        B UART5_IRQHandler
        
                 
        PUBWEAK ISO78160_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ISO78160_IRQHandler
        B ISO78160_IRQHandler
        
                
        PUBWEAK ISO78161_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ISO78161_IRQHandler
        B ISO78161_IRQHandler
        
                
        PUBWEAK TMR0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TMR0_IRQHandler
        B TMR0_IRQHandler
        
                
        PUBWEAK TMR1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TMR1_IRQHandler
        B TMR1_IRQHandler
        
                
        PUBWEAK TMR2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TMR2_IRQHandler
        B TMR2_IRQHandler
        
                
        PUBWEAK TMR3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TMR3_IRQHandler
        B TMR3_IRQHandler
        
                
        PUBWEAK PWM0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PWM0_IRQHandler
        B PWM0_IRQHandler
        
                
        PUBWEAK PWM1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PWM1_IRQHandler
        B PWM1_IRQHandler
        
                
        PUBWEAK PWM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PWM2_IRQHandler
        B PWM2_IRQHandler
      
                
        PUBWEAK PWM3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PWM3_IRQHandler
        B PWM3_IRQHandler
        
                
        PUBWEAK DMA_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA_IRQHandler
        B DMA_IRQHandler
        
                
        PUBWEAK FLASH_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FLASH_IRQHandler
        B FLASH_IRQHandler
        
                
        PUBWEAK ANA_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ANA_IRQHandler
        B ANA_IRQHandler
        
                
        PUBWEAK SPI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI2_IRQHandler
        B SPI2_IRQHandler
        
        
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Chip init.
;;      1. Load flash configuration
;;      2. Load ANA_REG(B/C/D/E) information
;;      3. IOE6/IOE7 forbidden, disable output, disable input
;;      4. Configure crystal drive capability(ANA_REG4 = 1)
;;      5. VLCD setting 
;;      6. SWT2VDD = 0; SWT2BAT = 1, VDD3OFF = 1
  IF (__CHIPINITIAL != 0)
        SECTION .Reset_Handler_text:CODE:NOROOT(2)
__CHIP_INIT
CONFIG1_START
        ;-------------------------------;
        ;; 1. Load flash configuration
        ; Unlock flash
        LDR     R0, =000FFFE0H
        LDR     R1, =55AAAA55H
        STR     R1, [R0]
        ; Load configure word 0 to 7
        LDR     R0, =00040E00H
        LDR     R1, =20H
        LDR     R2, =000FFFE8H
        LDR     R3, =000FFFF0H
        LDR     R4, =0
        LDR     R7, =0FFH
FLASH_CONF_START_1
        LDR     R5, [R0]
        STR     R4, [R2]
        STR     R5, [R3]
        LDR     R6, [R3]
        ANDS    R5, R7
        ANDS    R6, R7
        CMP     R5, R6
        BNE     FLASH_CONF_AGAIN_1
        ADDS    R4, #4
        ADDS    R0, #4
        CMP     R1, R4
        BEQ     FLASH_CONF_END_1
        B       FLASH_CONF_START_1
FLASH_CONF_AGAIN_1
        LDR     R5, [R0]
        STR     R4, [R2]
        STR     R5, [R3]
        LDR     R6, [R3]
        ANDS    R5, R7
        ANDS    R6, R7
        CMP     R5, R6
FLASH_CONF_WHILELOOP_1
        BNE     FLASH_CONF_WHILELOOP_1
        ADDS    R4, #4
        ADDS    R0, #4
        CMP     R1, R4
        BEQ     FLASH_CONF_END_1
        B       FLASH_CONF_START_1
FLASH_CONF_END_1
        ; Load configure word 8 to 11
        LDR     R1, =30H
FLASH_CONF_START_2
        LDR     R5, [R0]
        STR     R4, [R2]        
        STR     R5, [R3]
        LDR     R6, [R3]
        MOV     R8, R5
        LDR     R7, =1FFH
        ANDS    R7, R5
        MOV     R9, R7
        LSRS    R5, R5, #15
        LDR     R7, =3FFH
        ANDS    R5, R7
        LSLS    R5, R5, #9
        MOV     R7, R9
        ADDS    R7, R7, R5
        MOV     R9, R7
        MOV     R5, R8
        LSRS    R5, R5, #31
        LDR     R7, =1
        ANDS    R5, R7
        LSLS    R5, R5, #19
        MOV     R7, R9
        ADDS    R7, R7, R5
        LDR     R5, =0FFFFFH
        ANDS    R6, R6, R5
        ANDS    R7, R7, R5
        CMP     R6, R7
        BNE     FLASH_CONF_AGAIN_2
        ADDS    R4, #4
        ADDS    R0, #4
        CMP     R1, R4
        BEQ     FLASH_CONF_END_2
        B       FLASH_CONF_START_2
FLASH_CONF_AGAIN_2
        LDR     R5, [R0]
        STR     R4, [R2]        
        STR     R5, [R3]
        LDR     R6, [R3]
        MOV     R8, R5
        LDR     R7, =1FFH
        ANDS    R7, R5
        MOV     R9, R7
        LSRS    R5, R5, #15
        LDR     R7, =3FFH
        ANDS    R5, R7
        LSLS    R5, R5, #9
        MOV     R7, R9
        ADDS    R7, R7, R5
        MOV     R9, R7
        MOV     R5, R8
        LSRS    R5, R5, #31
        LDR     R7, =1
        ANDS    R5, R7
        LSLS    R5, R5, #19
        MOV     R7, R9
        ADDS    R7, R7, R5
        LDR     R5, =0FFFFFH
        ANDS    R6, R6, R5
        ANDS    R7, R7, R5
        CMP     R6, R7
FLASH_CONF_WHILELOOP_2
        BNE     FLASH_CONF_WHILELOOP_2
        ADDS    R0, #4
        ADDS    R4, #4
        CMP     R1, R4
        BEQ     FLASH_CONF_END_2
        B       FLASH_CONF_START_2
FLASH_CONF_END_2  
        ; Lock flash
        LDR     R0, =000FFFE0H
        LDR     R1, =0
        STR     R1, [R0]
        ;-------------------------------;
        ;; 2. Load ANA_REG(B/C/D/E) information
CONFIG2_START
        LDR     R4, =4001422CH
        LDR     R5, =40014230H
        LDR     R6, =40014234H
        LDR     R7, =40014238H
        LDR     R0, =40DC0H
        LDR     R0, [R0]
        LDR     R1, =40DC4H
        LDR     R1, [R1]
        ADDS    R2, R0, R1
        ADDS    R2, #0FFFFFFFFH
        MVNS    R2, R2
        LDR     R3, =40DCCH
        LDR     R3, [R3]
        CMP     R3, R2
        BEQ     ANADAT_CHECKSUM1_OK
        B       ANADAT_CHECKSUM1_ERR
ANADAT_CHECKSUM1_OK  
        ; ANA_REGB
        LDR     R1, =0FFH
        ANDS    R1, R0
        STR     R1, [R4]
        ; ANA_REGC
        LDR     R1, =0FF00H
        ANDS    R1, R0
        LSRS    R1, R1, #8
        STR     R1, [R5]
        ; ANA_REGD
        LDR     R1, =0FF0000H
        ANDS    R1, R0
        LSRS    R1, R1, #16
        STR     R1, [R6]
        ; ANA_REGE
        LDR     R1, =0FF000000H
        ANDS    R1, R0
        LSRS    R1, R1, #24
        STR     R1, [R7]
        B       CONFIG3_START
ANADAT_CHECKSUM1_ERR
        LDR     R0, =40DD0H
        LDR     R0, [R0]
        LDR     R1, =40DD4H
        LDR     R1, [R1]
        ADDS    R2, R0, R1
        ADDS    R2, #0FFFFFFFFH
        MVNS    R2, R2
        LDR     R3, =40DDCH
        LDR     R3, [R3]
        CMP     R3, R2
        BEQ     ANADAT_CHECKSUM2_OK
        B       ANADAT_CHECKSUM2_ERR
ANADAT_CHECKSUM2_OK  
        ; ANA_REGB
        LDR     R1, =0FFH
        ANDS    R1, R0
        STR     R1, [R4]
        ; ANA_REGC
        LDR     R1, =0FF00H
        ANDS    R1, R0
        LSRS    R1, R1, #8
        STR     R1, [R5]
        ; ANA_REGD
        LDR     R1, =0FF0000H
        ANDS    R1, R0
        LSRS    R1, R1, #16
        STR     R1, [R6]
        ; ANA_REGE
        LDR     R1, =0FF000000H
        ANDS    R1, R0
        LSRS    R1, R1, #24
        STR     R1, [R7]
        B       CONFIG3_START 
ANADAT_CHECKSUM2_ERR
        B       ANADAT_CHECKSUM2_ERR
        ;-------------------------------;
        ;; 3. IOE6/IOE7 forbidden, disable output, disable input
        ; Disable output
CONFIG3_START
        LDR     R2, =0C0H
        LDR     R0, =40000080H
        LDR     R1, [R0]
        ORRS    R1, R2
        STR     R1, [R0]
        ; Disable input
        LDR     R0, =40000084H
        LDR     R1, [R0]
        BICS    R1, R2
        STR     R1, [R0]
        ;-------------------------------;
        ;; 4. Configure crystal drive capability(ANA_REG4 = 1)
CONFIG4_START
        LDR     R1, =40014210H
        LDR     R0, =1
        STR     R0, [R1]
        ;-------------------------------;
        ;; 5. VLCD setting  
        ; Get LCD LDO trim value 1, R0
CONFIG5_START
        LDR     R2, =40D90H
        LDR     R0, [R2]
        ; Get VLCD setting 1, R1
        LDR     R2, =40D94H
        LDR     R1, [R2]
        ; Calculate checksum 1, R3
        ADDS    R3, R0, R1
        MVNS    R3, R3
        ; Compare checksum 1
        LDR     R2, =40D98H
        LDR     R4, [R2]
        CMP     R3, R4
        BEQ     VLCD_CHECKSUM1_OK
        B       VLCD_CHECKSUM1_ERR
VLCD_CHECKSUM1_OK 
       LSLS    R1, R1, #1 
        ; Write VLCD setting to ANA->REG6
        LDR     R2, =40014218H
        LDR     R3, [R2]        
        LDR     R4, =1EH
        BICS    R3, R4
        ANDS    R1, R4
        ORRS    R3, R1
        STR     R3, [R2]
        B       CONFIG6_START
VLCD_CHECKSUM1_ERR
        ; Get LCD LDO trim value 2, R0
        LDR     R2, =40D9CH
        LDR     R0, [R2]
        ; Get VLCD setting 2, R1
        LDR     R2, =40DA0H
        LDR     R1, [R2]        
        ; Calculate checksum 2, R3
        ADDS    R3, R0, R1
        MVNS    R3, R3
        ; Compare checksum 2
        LDR     R2, =40DA4H
        LDR     R4, [R2]
        CMP     R3, R4
        BEQ     VLCD_CHECKSUM2_OK
        B       VLCD_CHECKSUM2_ERR
VLCD_CHECKSUM2_OK
        LSLS    R1, R1, #1 
        ; Write VLCD setting to ANA->REG6
        LDR     R2, =40014218H
        LDR     R3, [R2]   
        LDR     R4, =1EH
        BICS    R3, R4
        ANDS    R1, R4
        ORRS    R3, R1
        STR     R3, [R2]
        B       CONFIG6_START        
VLCD_CHECKSUM2_ERR      
        BX      LR
        ;-------------------------------;
        ;; 6. SWT2VDD = 0; SWT2BAT =1, VDD3OFF = 1
CONFIG6_START
        ; SWT2VDD = 0, ANA_REG7 = 0
        LDR     R0, =4001421CH
        LDR     R1, =0H
        STR     R1, [R0]
        ; SWT2BAT = 1, VDD3OFF = 1, ANA_REGA[6:0] = 0x0A
        LDR     R2, =0AH
        LDR     R3, =7FH
        LDR     R0, =40014228H
        LDR     R1, [R0]
        BICS    R1, R3
        ORRS    R1, R2
        STR     R1, [R0]
        BX      LR
  ENDIF        
        END

/*********************************** END OF FILE ******************************/
