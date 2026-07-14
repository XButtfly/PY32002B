/**
 ******************************************************************************
 * @file     startup_py32f002bxx.S
 * @brief    PY32F002Bxx vector table and startup for GCC (arm-none-eabi)
 * @note     Clean rewrite — CMSIS copy.table/zero.table init, no stack fill.
 *           Compile with: -x assembler-with-cpp
 ******************************************************************************
 */

                .syntax  unified
                .arch    armv6-m

/*----------------------------------------------------------------------------
  Vector table (placed in .vectors section, mapped to 0x08000000)
 *----------------------------------------------------------------------------*/
                .section .vectors
                .align   2
                .globl   __Vectors
                .globl   __Vectors_End
                .globl   __Vectors_Size

__Vectors:
                .long    __StackTop                          /*  0  Initial SP      */
                .long    Reset_Handler                       /*  1  Reset            */
                .long    NMI_Handler                         /*  2  NMI              */
                .long    HardFault_Handler                   /*  3  HardFault        */
                .long    0                                   /*  4  Reserved         */
                .long    0                                   /*  5  Reserved         */
                .long    0                                   /*  6  Reserved         */
                .long    0                                   /*  7  Reserved         */
                .long    0                                   /*  8  Reserved         */
                .long    0                                   /*  9  Reserved         */
                .long    0                                   /* 10  Reserved         */
                .long    SVC_Handler                         /* 11  SVCall           */
                .long    0                                   /* 12  Reserved         */
                .long    0                                   /* 13  Reserved         */
                .long    PendSV_Handler                      /* 14  PendSV           */
                .long    SysTick_Handler                     /* 15  SysTick          */

                /* ---- External interrupts (peripheral) ---- */
                .long    0                                   /*  0 Reserved          */
                .long    0                                   /*  1 Reserved          */
                .long    0                                   /*  2 Reserved          */
                .long    FLASH_IRQHandler                    /*  3 FLASH             */
                .long    RCC_IRQHandler                      /*  4 RCC               */
                .long    EXTI0_1_IRQHandler                  /*  5 EXTI0-1           */
                .long    EXTI2_3_IRQHandler                  /*  6 EXTI2-3           */
                .long    EXTI4_15_IRQHandler                 /*  7 EXTI4-15          */
                .long    0                                   /*  8 Reserved          */
                .long    0                                   /*  9 Reserved          */
                .long    0                                   /* 10 Reserved          */
                .long    0                                   /* 11 Reserved          */
                .long    ADC_COMP_IRQHandler                 /* 12 ADC + COMP        */
                .long    TIM1_BRK_UP_TRG_COM_IRQHandler      /* 13 TIM1              */
                .long    TIM1_CC_IRQHandler                  /* 14 TIM1 CC           */
                .long    0                                   /* 15 Reserved          */
                .long    0                                   /* 16 Reserved          */
                .long    LPTIM1_IRQHandler                   /* 17 LPTIM1            */
                .long    0                                   /* 18 Reserved          */
                .long    TIM14_IRQHandler                    /* 19 TIM14             */
                .long    0                                   /* 20 Reserved          */
                .long    0                                   /* 21 Reserved          */
                .long    0                                   /* 22 Reserved          */
                .long    I2C1_IRQHandler                     /* 23 I2C1              */
                .long    0                                   /* 24 Reserved          */
                .long    SPI1_IRQHandler                     /* 25 SPI1              */
                .long    0                                   /* 26 Reserved          */
                .long    USART1_IRQHandler                   /* 27 USART1            */
                .long    0                                   /* 28 Reserved          */
                .long    0                                   /* 29 Reserved          */
                .long    0                                   /* 30 Reserved          */
                .long    0                                   /* 31 Reserved          */

__Vectors_End:
                .equ     __Vectors_Size, __Vectors_End - __Vectors
                .size    __Vectors, . - __Vectors

/*----------------------------------------------------------------------------
  Reset Handler
 *----------------------------------------------------------------------------*/
                .thumb
                .section .text
                .align   2

                .thumb_func
                .type    Reset_Handler, %function
                .globl   Reset_Handler
                .fnstart
Reset_Handler:
                cpsid    i

                /* ---- Copy .data from FLASH to RAM ---- */
                ldr      r4, =__copy_table_start__
                ldr      r5, =__copy_table_end__

1:              cmp      r4, r5
                bge      1f
                ldr      r1, [r4]          /* LMA (source in FLASH)     */
                ldr      r2, [r4, #4]      /* VMA (dest in RAM)        */
                ldr      r3, [r4, #8]      /* word count               */
                lsls     r3, r3, #2        /* -> byte count             */

2:              subs     r3, #4
                blt      2f
                ldr      r0, [r1, r3]
                str      r0, [r2, r3]
                b        2b
2:
                adds     r4, #12
                b        1b
1:

                /* ---- Zero-fill .bss ---- */
                ldr      r3, =__zero_table_start__
                ldr      r4, =__zero_table_end__

3:              cmp      r3, r4
                bge      3f
                ldr      r1, [r3]          /* VMA (dest in RAM)        */
                ldr      r2, [r3, #4]      /* word count               */
                lsls     r2, r2, #2        /* -> byte count             */
                movs     r0, #0

4:              subs     r2, #4
                blt      4f
                str      r0, [r1, r2]
                b        4b
4:
                adds     r3, #8
                b        3b
3:

                /* ---- Stack fill for overflow detection ----
                 * Fill entire stack with 0xDEADBEEF so HardFault_Handler
                 * can measure peak stack usage by scanning for the first
                 * overwritten word.
                 */
                ldr      r1, =__StackLimit
                ldr      r2, =__StackTop
                ldr      r3, =0xDEADBEEF

5:              cmp      r1, r2
                bhs      5f
                str      r3, [r1]
                adds     r1, #4
                b        5b
5:

                /* ---- Call system init + main ---- */
                bl       SystemInit
                bl       _start             /* -> calls main() via libc */

                .fnend
                .size    Reset_Handler, . - Reset_Handler

/*----------------------------------------------------------------------------
  Default handler — infinite loop with BKPT for debugger
 *----------------------------------------------------------------------------*/
                .thumb_func
                .type    Default_Handler, %function
                .weak    Default_Handler
                .fnstart
Default_Handler:
                bkpt     #0
                b        .
                .fnend
                .size    Default_Handler, . - Default_Handler

/* ---- Macro: alias handler to Default_Handler ---- */
                .macro   Set_Default_Handler  Handler_Name
                .weak    \Handler_Name
                .set     \Handler_Name, Default_Handler
                .endm

/* Systick — override in C */
                Set_Default_Handler  SysTick_Handler

/* System exceptions */
                Set_Default_Handler  NMI_Handler
                Set_Default_Handler  HardFault_Handler
                Set_Default_Handler  SVC_Handler
                Set_Default_Handler  PendSV_Handler

/* Peripheral interrupts */
                Set_Default_Handler  FLASH_IRQHandler
                Set_Default_Handler  RCC_IRQHandler
                Set_Default_Handler  EXTI0_1_IRQHandler
                Set_Default_Handler  EXTI2_3_IRQHandler
                Set_Default_Handler  EXTI4_15_IRQHandler
                Set_Default_Handler  ADC_COMP_IRQHandler
                Set_Default_Handler  TIM1_BRK_UP_TRG_COM_IRQHandler
                Set_Default_Handler  TIM1_CC_IRQHandler
                Set_Default_Handler  LPTIM1_IRQHandler
                Set_Default_Handler  TIM14_IRQHandler
                Set_Default_Handler  I2C1_IRQHandler
                Set_Default_Handler  SPI1_IRQHandler
                Set_Default_Handler  USART1_IRQHandler

                .end
