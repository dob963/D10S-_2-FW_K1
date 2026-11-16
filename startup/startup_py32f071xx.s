/*
 * Startup indipendente per PY32F071 / Cortex-M0+.
 * Tabella vettori e nomi IRQ verificati rispetto al startup pubblico Puya
 * PY32F071xx (repository Armel/F4HWN, revisione 3261ca7).
 * Implementazione riscritta per questo progetto; nessun sorgente assembly
 * upstream è stato copiato.
 *
 * Licenze di riferimento: il startup Puya consultato dichiara BSD-3-Clause
 * e copyright Puya Semiconductor (2023) e STMicroelectronics (2016).
 */
.syntax unified
.cpu cortex-m0plus
.thumb

.extern _estack
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss
.extern SystemInit
.extern __libc_init_array
.extern main

.global Reset_Handler
.global Default_Handler
.type Reset_Handler, %function
.type Default_Handler, %function

.section .text.Reset_Handler,"ax",%progbits
.thumb_func
Reset_Handler:
    ldr r0, =_estack
    mov sp, r0
    bl SystemInit

    /* Copia .data dalla FLASH alla RAM, parola per parola. */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0]
    str r3, [r1]
    adds r0, #4
    adds r1, #4
    b 1b

    /* Azzera .bss. */
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bcs 4f
    str r3, [r1]
    adds r1, #4
    b 3b

4:
    bl __libc_init_array
    bl main
5:
    b 5b
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.thumb_func
Default_Handler:
6:
    b 6b
.size Default_Handler, .-Default_Handler

/* Core exception and PY32F071 external interrupt vectors. */
.section .isr_vector,"a",%progbits
.align 7
.global g_pfnVectors
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word 0, 0, 0, 0, 0, 0, 0
    .word SVC_Handler
    .word 0, 0
    .word PendSV_Handler
    .word SysTick_Handler
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_1_IRQHandler
    .word EXTI2_3_IRQHandler
    .word EXTI4_15_IRQHandler
    .word LCD_IRQHandler
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_3_IRQHandler
    .word DMA1_Channel4_5_6_7_IRQHandler
    .word ADC_COMP_IRQHandler
    .word TIM1_BRK_UP_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word TIM6_LPTIM1_DAC_IRQHandler
    .word TIM7_IRQHandler
    .word TIM14_IRQHandler
    .word TIM15_IRQHandler
    .word TIM16_IRQHandler
    .word TIM17_IRQHandler
    .word I2C1_IRQHandler
    .word I2C2_IRQHandler
    .word SPI1_IRQHandler
    .word SPI2_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word USART3_IRQHandler
    .word 0
    .word USB_IRQHandler

/* Ogni handler può essere sostituito da una definizione forte nel firmware. */
.macro WEAK_DEFAULT name
    .weak \name
    .thumb_set \name, Default_Handler
.endm
WEAK_DEFAULT NMI_Handler
WEAK_DEFAULT HardFault_Handler
WEAK_DEFAULT SVC_Handler
WEAK_DEFAULT PendSV_Handler
WEAK_DEFAULT SysTick_Handler
WEAK_DEFAULT WWDG_IRQHandler
WEAK_DEFAULT PVD_IRQHandler
WEAK_DEFAULT RTC_IRQHandler
WEAK_DEFAULT FLASH_IRQHandler
WEAK_DEFAULT RCC_IRQHandler
WEAK_DEFAULT EXTI0_1_IRQHandler
WEAK_DEFAULT EXTI2_3_IRQHandler
WEAK_DEFAULT EXTI4_15_IRQHandler
WEAK_DEFAULT LCD_IRQHandler
WEAK_DEFAULT DMA1_Channel1_IRQHandler
WEAK_DEFAULT DMA1_Channel2_3_IRQHandler
WEAK_DEFAULT DMA1_Channel4_5_6_7_IRQHandler
WEAK_DEFAULT ADC_COMP_IRQHandler
WEAK_DEFAULT TIM1_BRK_UP_TRG_COM_IRQHandler
WEAK_DEFAULT TIM1_CC_IRQHandler
WEAK_DEFAULT TIM2_IRQHandler
WEAK_DEFAULT TIM3_IRQHandler
WEAK_DEFAULT TIM6_LPTIM1_DAC_IRQHandler
WEAK_DEFAULT TIM7_IRQHandler
WEAK_DEFAULT TIM14_IRQHandler
WEAK_DEFAULT TIM15_IRQHandler
WEAK_DEFAULT TIM16_IRQHandler
WEAK_DEFAULT TIM17_IRQHandler
WEAK_DEFAULT I2C1_IRQHandler
WEAK_DEFAULT I2C2_IRQHandler
WEAK_DEFAULT SPI1_IRQHandler
WEAK_DEFAULT SPI2_IRQHandler
WEAK_DEFAULT USART1_IRQHandler
WEAK_DEFAULT USART2_IRQHandler
WEAK_DEFAULT USART3_IRQHandler
WEAK_DEFAULT USB_IRQHandler
