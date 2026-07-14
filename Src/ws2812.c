/**
 ******************************************************************************
 * @file    ws2812.c
 * @brief   WS2812B 位脉冲驱动实现 (24MHz HSI)
 * @note    双编译器支持: GCC (全 24-bit 内联汇编) + Keil AC5 (逐 bit 汇编)
 *
 *   位时序参数 (24MHz 系统时钟, 1 周期 ≈ 41.7ns):
 *     T0H =  9cy (0.38µs)    T0L = 20cy (0.83µs)
 *     T1H = 17cy (0.71µs)    T1L = 12cy (0.50µs)
 *     RES > 50µs (复位码)
 ******************************************************************************
 */

#include "ws2812.h"

/* GPIOA 基址 — IOPORT_BASE = 0x50000000 */
#define _BASE   0x50000000UL

/*===========================================================================
 * @brief  初始化 WS2812B 数据引脚
 * @note   配置 PA1 为高速推挽输出, 发送长复位脉冲清空灯带
 *===========================================================================*/
void ws2812_init(void)
{
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(GPIOA, WS2812_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(GPIOA, WS2812_PIN, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_ResetOutputPin(GPIOA, WS2812_PIN);

    /* 复位脉冲: >50µs LOW, 清空灯带数据 */
    { volatile uint32_t r = 10000U; while (r--) { __NOP(); } }
}

/*===========================================================================
 * @brief  发送 n 颗 LED 的 GRB 颜色数据
 * @param  grb[] 颜色数组 (GRB 格式, MSB 先发)
 * @param  n     LED 数量
 * @note   关全局中断保证时序精度, 发完后输出 >50µs 复位脉冲锁存数据
 *===========================================================================*/
void ws2812_send(const uint32_t grb[], uint32_t n)
{
    uint32_t i;
    uint32_t pin  = WS2812_PIN;     /* 引脚位掩码 0x0002               */

    __disable_irq();                /* 关中断, 保证位脉冲时序精确      */

#if defined(__ARMCC_VERSION)
    /*---- Keil AC5: 逐 bit 内联汇编 (ARM 模式) ----*/
    {
        uint32_t b;
        uint32_t c;
        uint32_t bsrr_addr = _BASE + 0x18U;  /* GPIOA->BSRR = 0x50000018 */
        uint32_t brr_addr  = _BASE + 0x28U;  /* GPIOA->BRR  = 0x50000028 */

        for (i = 0; i < n; i++)
        {
            c = grb[i];
            for (b = 0; b < 24; b++)
            {
                if (c & (0x800000U >> b))           /* MSB 优先         */
                {
                    __asm {
                        STR  pin, [bsrr_addr]       /* PA1 = HIGH      */
                        NOP; NOP; NOP; NOP; NOP     /* 延时 ≈0.71µs    */
                        NOP; NOP; NOP; NOP; NOP
                        NOP; NOP; NOP; NOP; NOP
                        STR  pin, [brr_addr]        /* PA1 = LOW       */
                        NOP; NOP; NOP; NOP; NOP     /* 延时 ≈0.50µs    */
                    }
                }
                else
                {
                    __asm {
                        STR  pin, [bsrr_addr]       /* PA1 = HIGH      */
                        NOP; NOP; NOP; NOP; NOP     /* 延时 ≈0.38µs    */
                        STR  pin, [brr_addr]        /* PA1 = LOW       */
                        NOP; NOP; NOP; NOP; NOP     /* 延时 ≈0.83µs    */
                        NOP; NOP; NOP; NOP; NOP
                        NOP; NOP; NOP; NOP; NOP
                    }
                }
            }
        }
    }
#else
    /*---- GCC: 24-bit 完整循环放入单个内联汇编块 (零 C 层开销) ----*/
    {
        uint32_t base = _BASE;          /* GPIOA 基址 0x50000000        */

    for (i = 0; i < n; i++)
    {
        uint32_t c0 = grb[i];                       /* 当前 LED 颜色   */
        uint32_t nbits = 24;                        /* 位计数器        */

        __ASM volatile (
            ".syntax unified      \n"
        "0: \n"
            "LSLS %[c], #1       \n"               /* MSB → Carry      */
            "BCS  2f             \n"
            /* ---------- 0 码: T0H=9cy T0L=20cy ---------- */
            "STR  %[pin], [%[base], #24] \n"       /* BSRR → PA1=HIGH */
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
            "STR  %[pin], [%[base], #40] \n"       /* BRR  → PA1=LOW  */
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
            "B    3f             \n"
        "2: \n"
            /* ---------- 1 码: T1H=17cy T1L=12cy ---------- */
            "STR  %[pin], [%[base], #24] \n"       /* BSRR → PA1=HIGH */
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
            "STR  %[pin], [%[base], #40] \n"       /* BRR  → PA1=LOW  */
            "NOP  \n" "NOP  \n" "NOP  \n" "NOP  \n"
        "3: \n"
            "SUBS %[n], #1       \n"               /* 位计数--         */
            "BNE  0b             \n"
            : [c] "+l" (c0),
              [n] "+l" (nbits)
            : [pin] "l" (pin),
              [base] "l" (base)
            : "cc"
        );
    }
    }
#endif

    __enable_irq();                 /* 恢复中断                        */

    /* 复位脉冲: >50µs LOW, 锁存灯带数据 */
    { volatile uint32_t r = 2000U; while (r--) { __NOP(); } }
}
