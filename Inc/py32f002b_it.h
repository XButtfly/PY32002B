/**
 ******************************************************************************
 * @file    py32f002b_it.h
 * @brief   Interrupt handler prototypes for PY32F002Bx5
 ******************************************************************************
 */

#ifndef __PY32F002B_IT_H
#define __PY32F002B_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* System exception handlers */
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* Peripheral interrupt handlers — declare only what you implement */
/* (All others fall through to Default_Handler in startup) */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F002B_IT_H */
