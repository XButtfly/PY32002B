/**
 ******************************************************************************
 * @file    py32f002b_it.c
 * @brief   Interrupt service routines for PY32F002Bx5
 * @note    HardFault_Handler captures stacked frame, MSP/PSP, EXC_RETURN,
 *          ICSR, fault count, and stack watermark to 0x20000BA0.
 *          SysTick_Handler drives HAL_IncTick() for HAL_Delay.
 ******************************************************************************
 */

#include "main.h"

/*----------------------------------------------------------------------------
  HardFault Diagnostic Data Structure (64 bytes at 0x20000BA0)
  On Cortex-M0+, all faults escalate to HardFault.
  The stacked frame is the primary diagnostic.
 *----------------------------------------------------------------------------*/
typedef struct
{
    volatile uint32_t stacked_r0;      /* +0  (0x00): R0                       */
    volatile uint32_t stacked_r1;      /* +4  (0x04): R1                       */
    volatile uint32_t stacked_r2;      /* +8  (0x08): R2                       */
    volatile uint32_t stacked_r3;      /* +12 (0x0C): R3                       */
    volatile uint32_t stacked_r12;     /* +16 (0x10): R12                      */
    volatile uint32_t stacked_lr;      /* +20 (0x14): LR at fault              */
    volatile uint32_t stacked_pc;      /* +24 (0x18): PC (faulting instruction)*/
    volatile uint32_t stacked_xpsr;    /* +28 (0x1C): xPSR                     */
    volatile uint32_t msp;             /* +32 (0x20): MSP at handler entry     */
    volatile uint32_t psp;             /* +36 (0x24): PSP at handler entry     */
    volatile uint32_t exc_return;      /* +40 (0x28): EXC_RETURN value         */
    volatile uint32_t icsr;            /* +44 (0x2C): SCB->ICSR                */
    volatile uint32_t fault_count;     /* +48 (0x30): times HardFault entered  */
    volatile uint32_t stack_watermark; /* +52 (0x34): ~unused stack words left */
    volatile uint32_t reserved[3];     /* +56..+64: padding to 64 bytes        */
} HardFaultInfo_t;

#define HARDFAULT_INFO    ((volatile HardFaultInfo_t *)0x20000BA0UL)
#define HARDFAULT_FRAME   ((volatile uint32_t *)0x20000BE0UL)
#define STACK_FILL_PATTERN  0xDEADBEEFUL

/*----------------------------------------------------------------------------
  SysTick — 1 ms tick for HAL_Delay()
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/*----------------------------------------------------------------------------
  HardFault — crash diagnostics
  On entry, the processor has stacked R0-R3, R12, LR, PC, xPSR.
  We capture these + MSP/PSP + EXC_RETURN + ICSR + stack usage.
  Then BKPT halts the debugger; if no debugger attached, LOCKUP.
 *----------------------------------------------------------------------------*/
void HardFault_Handler(void)
{
    volatile HardFaultInfo_t *diag = HARDFAULT_INFO;
    volatile uint32_t *frame = HARDFAULT_FRAME;
    volatile uint32_t msp_val;
    volatile uint32_t psp_val;
    volatile uint32_t lr_val;
    uint32_t i;

    /* Read MSP, PSP. Derive EXC_RETURN-like lr_val:
     *   Handler mode  -> 0xFFFFFFF1
     *   Thread + MSP  -> 0xFFFFFFF9
     *   Thread + PSP  -> 0xFFFFFFFD
     * Bit 2 tells us which SP holds the stacked frame. */
    msp_val = __get_MSP();
    psp_val = __get_PSP();
    {
        uint32_t ipsr = __get_IPSR();
        if (ipsr != 0U)
        {
            lr_val = 0xFFFFFFF1UL;   /* Handler mode */
        }
        else if (__get_CONTROL() & 0x2U)
        {
            lr_val = 0xFFFFFFFDUL;   /* Thread mode, PSP */
        }
        else
        {
            lr_val = 0xFFFFFFF9UL;   /* Thread mode, MSP */
        }
    }

    diag->msp        = msp_val;
    diag->psp        = psp_val;
    diag->exc_return = lr_val;
    diag->icsr       = SCB->ICSR;

    /* Increment fault count (clamp at 1000) */
    {
        uint32_t count = diag->fault_count;
        if (count > 1000U) { count = 0U; }
        diag->fault_count = count + 1U;
    }

    /* Copy 8-word stacked frame from the active stack pointer.
     * EXC_RETURN bit 2: 0 = MSP was used, 1 = PSP was used. */
    {
        volatile uint32_t *sp = ((lr_val & 0x4U) != 0U)
                              ? (volatile uint32_t *)psp_val
                              : (volatile uint32_t *)msp_val;

        for (i = 0; i < 8; i++) { frame[i] = sp[i]; }

        diag->stacked_r0   = sp[0];
        diag->stacked_r1   = sp[1];
        diag->stacked_r2   = sp[2];
        diag->stacked_r3   = sp[3];
        diag->stacked_r12  = sp[4];
        diag->stacked_lr   = sp[5];
        diag->stacked_pc   = sp[6];
        diag->stacked_xpsr = sp[7];
    }

    /* Stack watermark — requires GCC linker symbols + stack fill.
     * On ARMCC (Keil), the startup is from Device Pack and doesn't
     * support stack fill, so this is skipped. */
#if !defined(__ARMCC_VERSION)
    {
        extern uint32_t __StackLimit;
        extern uint32_t __StackTop;
        volatile uint32_t *p = (volatile uint32_t *)&__StackLimit;
        volatile uint32_t *end = (volatile uint32_t *)&__StackTop;
        uint32_t remaining = 0U;
        while (p < end)
        {
            if (*p != STACK_FILL_PATTERN) { break; }
            remaining++;
            p++;
        }
        diag->stack_watermark = ~remaining;
    }
#else
    diag->stack_watermark = 0U;   /* not available with Keil startup */
#endif

    /* Halt for debugger */
    __BKPT(0);
    while (1) { }
}

/*----------------------------------------------------------------------------
  Other system exceptions — weak-linked to Default_Handler in startup
 *----------------------------------------------------------------------------*/
/* NMI_Handler, SVC_Handler, PendSV_Handler are handled by startup defaults */

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    __BKPT(0);
    while (1) { }
}
#endif /* USE_FULL_ASSERT */
