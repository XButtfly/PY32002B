/**
 ******************************************************************************
 * @file    main.h
 * @brief   Application header for PY32F002Bx5
 * @note    Includes all necessary LL/HAL drivers.
 ******************************************************************************
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* MCU header — selects device based on PY32F002Bx5 define */
#include "py32f0xx.h"

/* LL drivers (static-inline peripherals) */
#include "py32f002b_ll_bus.h"
#include "py32f002b_ll_rcc.h"
#include "py32f002b_ll_system.h"
#include "py32f002b_ll_cortex.h"
#include "py32f002b_ll_gpio.h"
#include "py32f002b_ll_exti.h"
#include "py32f002b_ll_usart.h"
#include "py32f002b_ll_utils.h"
#include "py32f002b_ll_pwr.h"
#include "py32f002b_ll_adc.h"

/* HAL layer (tick, delay, init) */
/* py32f002b_hal_conf.h is included by py32f0xx_hal.h automatically —
   the compiler finds our Inc/py32f002b_hal_conf.h first in the include path.
   Include it here with the guard to ensure it is found regardless of order. */
#include "py32f002b_hal_conf.h"
#include "py32f0xx_hal.h"

/* Interrupt handlers */
#include "py32f002b_it.h"

/* ---- Board pin definitions ----
 *
 * ⚠ PY32F002B non-standard SWD pinout (controlled by FLASH_OPTR):
 *    SWCLK = PA14  (standard ARM)
 *    SWDIO = PB6   (NOT PA13 — default SWD_MODE=0 selects PB6)
 *    NRST  = PC0   (default NRST_MODE=0)
 *
 *    Option bytes at 0x1FFF0E80 can remap SWDIO to PC0 and NRST to GPIO.
 *    See fix_opb.cfg in reference template for recovery if SWD is lost.
 *----------------------------------------------------------------------------*/
#define LED_GPIO_PORT           GPIOA
#define LED_PIN                 LL_GPIO_PIN_5       /* PA5 - LED (active low) */

/* ---- Debug USART (reserved for future use) ---- */
#define USART_TX_GPIO_PORT      GPIOB
#define USART_TX_PIN            LL_GPIO_PIN_3       /* PB3 - USART1 TX */
#define USART_RX_GPIO_PORT      GPIOB
#define USART_RX_PIN            LL_GPIO_PIN_4       /* PB4 - USART1 RX */
#define USART_AF                1U

#define DEBUG_USART             USART1
#define DEBUG_USART_BAUDRATE    115200U

/* ---- ADC 按键 (电阻分压, PA7=ADC_IN4) ---- */
#define ADC_KEY_GPIO_PORT       GPIOA
#define ADC_KEY_PIN             LL_GPIO_PIN_7       /* PA7 - ADC IN4 */
#define ADC_KEY_CHANNEL         LL_ADC_CHANNEL_4    /* PA7→IN4 (官方确认) */

/* ADC 按键阈值 (PA7实测校准, 12-bit, VCC=3.3V, R_pullup=10K) */
#define ADC_THR_KEY1             185U   /*    < 186: 0Ω   → 001.0     ADC=0     */
#define ADC_THR_KEY2             555U   /* 186-555: 1K   → 002.0     ADC=367   */
#define ADC_THR_KEY3            1024U   /* 556-1024: 2K2 → 003.0     (理论~739)*/
#define ADC_THR_KEY4            1483U   /* 1025-1483:4K7 → 004.0     ADC=1291  */
#define ADC_THR_KEY5            1741U   /* 1484-1741:6K8 → 005.0     ADC=1663  */
#define ADC_THR_KEY6            1930U   /* 1742-1930:10K → 006.0     ADC=1818  */
#define ADC_THR_KEY7            2137U   /* 1931-2137:15K → 007.0     ADC=2042  */
#define ADC_THR_KEY8            2337U   /* 2138-2337:22K → 008.0     ADC=2232  */
#define ADC_THR_KEY9            2622U   /* 2338-2622:33K → 009.0     ADC=2442  */
#define ADC_THR_KEY10           3450U   /* 2623-3450:47K → 146.2(恢复) ADC=2802 */
                                         /* >3450: 无按键 → 保持当前          */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
