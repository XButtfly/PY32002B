/**
 ******************************************************************************
 * @file    system_py32f002b.c
 * @brief   System initialization for PY32F002Bx5 (Cortex-M0+)
 * @note    Provides SystemInit() + SystemCoreClockUpdate() + frequency tables.
 *          Includes SWD recovery delay (100 ms) — critical when SWD pins
 *          (PA14/SWCLK, PB6/SWDIO) are reused as GPIO in the application.
 *
 *          ⚠ PY32F002B non-standard SWD pinout:
 *             SWCLK = PA14 (standard)
 *             SWDIO = PB6  (NOT PA13 — controlled by FLASH_OPTR SWD_MODE bit)
 ******************************************************************************
 */

#include "main.h"

/*----------------------------------------------------------------------------
  System memory trim addresses (loaded at factory)
 *----------------------------------------------------------------------------*/
#define HSI_TRIM_ADDR   ((uint16_t *)0x1FFF0100U)  /* HSI calibration    */
#define LSI_TRIM_ADDR   ((uint16_t *)0x1FFF0144U)  /* LSI calibration    */

/* HSI trim uses lower 13 bits in HSITRIM[12:0], masked by RCC_ICSCR_HSI_TRIM */
#define HSI_TRIM_MASK   0x1FFFU

/* LSI trim uses 9 bits in LSITRIM[8:0], masked by RCC_ICSCR_LSI_TRIM */
#define LSI_TRIM_MASK   0x01FFU

/*----------------------------------------------------------------------------
  Frequency tables — required by the LL RCC driver (ll_rcc.c)
 *----------------------------------------------------------------------------*/
const uint32_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint32_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};
const uint32_t HSIFreqTable[8]   = {0U, 0U, 0U, 0U, 24000000U, 0U, 0U, 0U};

/*----------------------------------------------------------------------------
  Global
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = HSI_VALUE;

/*----------------------------------------------------------------------------
  Local helper — CPU cycle delay (~24 MHz calibrated)
 *----------------------------------------------------------------------------*/
#ifndef SWD_DELAY
static void SWD_DelayMs(uint32_t ms)
{
    /* ~3 cycles per iteration at 24 MHz → ms * 8000 iterations */
    __IO uint32_t count = ms * (SystemCoreClock / 3U / 1000U);
    while (count--)
    {
        __NOP();
    }
}
#endif /* SWD_DELAY */

/*----------------------------------------------------------------------------
  SystemInit — called from startup before main()
 *----------------------------------------------------------------------------*/
void SystemInit(void)
{
    uint16_t trim_val;

    /* 0. Set SystemCoreClock NOW — SystemInit runs before __main in microlib,
     *    so .data is not yet initialized. SystemCoreClock could be garbage. */
    SystemCoreClock = HSI_VALUE;

    /* 1. Flash latency: 1 wait state (safe for all frequencies) */
    FLASH->ACR |= FLASH_ACR_LATENCY;

    /* 2. Load HSI trim from system memory */
    trim_val = *HSI_TRIM_ADDR & HSI_TRIM_MASK;
    if (trim_val != 0x0000U && trim_val != 0xFFFFU)
    {
        RCC->ICSCR = (RCC->ICSCR & ~RCC_ICSCR_HSI_TRIM) | (trim_val << RCC_ICSCR_HSI_TRIM_Pos);
    }

    /* 3. Load LSI trim from system memory */
    trim_val = *LSI_TRIM_ADDR & LSI_TRIM_MASK;
    if (trim_val != 0x0000U && trim_val != 0x01FFU)
    {
        RCC->ICSCR = (RCC->ICSCR & ~RCC_ICSCR_LSI_TRIM) | (trim_val << RCC_ICSCR_LSI_TRIM_Pos);
    }

    /* 4. Vector table in Flash */
    SCB->VTOR = FLASH_BASE;

    /* 5. DBGMCU — keep SWD alive during STOP mode
     *    RCC_APBENR1 @ 0x40021018, DBGMCUEN = bit 22 = 0x00400000
     *    DBGMCU_CR    @ 0x40015804, DBG_STOP = bit 1  = 0x00000002 */
    (*(volatile uint32_t *)0x40021018UL) |= 0x00400000UL;
    (*(volatile uint32_t *)0x40015804UL) |= 0x00000002UL;

    /* 6. SWD recovery delay — gives debugger a 100 ms window to connect
     *    before application code potentially repurposes SWD pins.
     *    Define SWD_DELAY to skip this if you intentionally need SWD pins as GPIO
     *    and accept that SWD will be unavailable after startup. */
#ifndef SWD_DELAY
    SWD_DelayMs(100);
#endif /* SWD_DELAY */

}

/*----------------------------------------------------------------------------
  SystemCoreClockUpdate — re-evaluate SystemCoreClock from hardware registers
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate(void)
{
    uint32_t sws;
    uint32_t hpre;
    uint32_t tmp;

    sws = (RCC->CFGR & RCC_CFGR_SWS_Msk) >> RCC_CFGR_SWS_Pos;

    switch (sws)
    {
    case 0x00:  /* HSISYS */
        tmp = HSI_VALUE;
        break;
    case 0x01:  /* HSE */
        tmp = HSE_VALUE;
        break;
    case 0x03:  /* LSI */
        tmp = LSI_VALUE;
        break;
    case 0x02:  /* LSE */
        tmp = LSE_VALUE;
        break;
    default:
        tmp = HSI_VALUE;
        break;
    }

    hpre = (RCC->CFGR & RCC_CFGR_HPRE_Msk) >> RCC_CFGR_HPRE_Pos;
    if (hpre >= 0x08U)
    {
        /* AHB prescaler: 0x08 = /2, 0x09 = /4, ..., 0x0F = /512
           encoded as 2^(HPRE-7) */
        tmp >>= (hpre - 7U);
    }

    SystemCoreClock = tmp;
}
