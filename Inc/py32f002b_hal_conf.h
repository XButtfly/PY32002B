/**
 ******************************************************************************
 * @file    py32f002b_hal_conf.h
 * @brief   HAL configuration for PY32F002Bx5
 * @note    This board uses HSI (24 MHz internal) only — no HSE crystal.
 ******************************************************************************
 */

#ifndef __PY32F002B_HAL_CONF_H
#define __PY32F002B_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
  Module Selection
 *----------------------------------------------------------------------------*/
#define HAL_MODULE_ENABLED
/* #define HAL_FLASH_MODULE_ENABLED */
#define HAL_CORTEX_MODULE_ENABLED

/* ---- HAL includes (conditional on module selection) ---- */
/* py32f002b_hal_def.h must come first — it defines HAL_StatusTypeDef */
#include "py32f002b_hal_def.h"

#ifdef HAL_MODULE_ENABLED
  #include "py32f0xx_hal.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
  #include "py32f002b_hal_flash.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "py32f002b_hal_cortex.h"
#endif

/*----------------------------------------------------------------------------
  Oscillator Values (Hz)
 *----------------------------------------------------------------------------*/
#define HSE_VALUE    24000000U    /* Not present on this board; kept for API compat */
#define HSI_VALUE    24000000U    /* Internal 24 MHz — actual clock source */
#define LSI_VALUE       32768U    /* Internal low-speed oscillator              */
#define LSE_VALUE       32768U    /* Not present on this board; kept for compat */

/* HSI divider table: { /1, /2, /3, /4, /5, /6, /7, /8 } */
#define HSITRIM_DIV1  24000000U
#define HSITRIM_DIV2  12000000U
#define HSITRIM_DIV3   8000000U
#define HSITRIM_DIV4   6000000U
#define HSITRIM_DIV5   4800000U
#define HSITRIM_DIV6   4000000U
#define HSITRIM_DIV7   3428572U  /* ~3.43 MHz */
#define HSITRIM_DIV8   3000000U

/*----------------------------------------------------------------------------
  System
 *----------------------------------------------------------------------------*/
#define  VDD_VALUE              3300U    /* mV */
#define  TICK_INT_PRIORITY         0U    /* SysTick priority (0 = highest on M0+) */

/*----------------------------------------------------------------------------
  Assert (enable for debugging, disable for release)
 *----------------------------------------------------------------------------*/
/* #define USE_FULL_ASSERT    1U */

#ifdef USE_FULL_ASSERT
  #define assert_param(expr)  ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr)  ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F002B_HAL_CONF_H */
