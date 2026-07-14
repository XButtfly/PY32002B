/**
 ******************************************************************************
 * @file    sd1516.h
 * @brief   SD-1516 LED 显示屏驱动 — PY32F002Bx5
 * @note    PB0~PB5,PB7 → 7 线 Charlieplexing 接口
 *          1/7 占空比多路复用, 4 位数字 + 电池/频道/MHz 指示灯
 ******************************************************************************
 */

#ifndef SD1516_H
#define SD1516_H

#include "main.h"

/*===========================================================================
  硬件引脚定义 — GPIOB (7 线复用 COM/SEG)
 *===========================================================================*/
#define SD_GPIO_PORT        GPIOB

/* SEG1 (COM0) — rDisplayBuffer bit6 — PB0 */
#define SD_SEG1_PIN         LL_GPIO_PIN_0
#define SD_SEG1_BIT         0x40U

/* SEG2 (COM1) — rDisplayBuffer bit5 — PB1 */
#define SD_SEG2_PIN         LL_GPIO_PIN_1
#define SD_SEG2_BIT         0x20U

/* SEG3 (COM2) — rDisplayBuffer bit4 — PB2 */
#define SD_SEG3_PIN         LL_GPIO_PIN_2
#define SD_SEG3_BIT         0x10U

/* SEG4 (COM3) — rDisplayBuffer bit3 — PB3 */
#define SD_SEG4_PIN         LL_GPIO_PIN_3
#define SD_SEG4_BIT         0x08U

/* SEG5 (COM4) — rDisplayBuffer bit2 — PB4 */
#define SD_SEG5_PIN         LL_GPIO_PIN_4
#define SD_SEG5_BIT         0x04U

/* SEG6 (COM5) — rDisplayBuffer bit1 — PB7 (板子物理走线在PB7) */
#define SD_SEG6_PIN         LL_GPIO_PIN_7
#define SD_SEG6_BIT         0x02U

/* SEG7 (COM6) — rDisplayBuffer bit0 — PB5 (板子物理走线在PB5) */
#define SD_SEG7_PIN         LL_GPIO_PIN_5
#define SD_SEG7_BIT         0x01U

/* 所有显示引脚位掩码 */
#define SD_ALL_PINS         (LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | \
                             LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | \
                             LL_GPIO_PIN_7)

/*===========================================================================
  COM 选通宏 — 将指定引脚切换为推挽输出 (COM 低电平)
 *===========================================================================*/
#define SEG1_SELECT()   LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG1_PIN, LL_GPIO_MODE_OUTPUT)
#define SEG2_SELECT()   LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG2_PIN, LL_GPIO_MODE_OUTPUT)
#define SEG3_SELECT()   LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG3_PIN, LL_GPIO_MODE_OUTPUT)
#define SEG4_SELECT()   LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG4_PIN, LL_GPIO_MODE_OUTPUT)
#define SEG5_SELECT()   LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG5_PIN, LL_GPIO_MODE_OUTPUT)
#define SEG6_SELECT()   LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG6_PIN, LL_GPIO_MODE_OUTPUT)
#define SEG7_SELECT()   LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG7_PIN, LL_GPIO_MODE_OUTPUT)

/*===========================================================================
  数字位段定义 — 编码物理 LED 走线, 不可修改
  LED1(WEI1) 4 位 COM 槽 — 0 1 2 3
 *===========================================================================*/
#define DIS_LED1_0_1    0X2C
#define DIS_LED1_0_2    0X40
#define DIS_LED1_0_3    0X40
#define DIS_LED1_0_4    0X40
#define DIS_LED1_1_1    0X08
#define DIS_LED1_1_2    0X00
#define DIS_LED1_1_3    0X40
#define DIS_LED1_1_4    0X00
#define DIS_LED1_2_1    0X14
#define DIS_LED1_2_2    0X40
#define DIS_LED1_2_3    0X40
#define DIS_LED1_2_4    0X40
#define DIS_LED1_3_1    0X1C
#define DIS_LED1_3_2    0X40
#define DIS_LED1_3_3    0X40
#define DIS_LED1_3_4    0X00
#define DIS_LED1_4_1    0X38
#define DIS_LED1_4_2    0X00
#define DIS_LED1_4_3    0X40
#define DIS_LED1_4_4    0X00
#define DIS_LED1_5_1    0X3C
#define DIS_LED1_5_2    0X40
#define DIS_LED1_5_3    0X00
#define DIS_LED1_5_4    0X00
#define DIS_LED1_6_1    0X3C
#define DIS_LED1_6_2    0X40
#define DIS_LED1_6_3    0X00
#define DIS_LED1_6_4    0X40
#define DIS_LED1_7_1    0X08
#define DIS_LED1_7_2    0X40
#define DIS_LED1_7_3    0X40
#define DIS_LED1_7_4    0X00
#define DIS_LED1_8_1    0X3C
#define DIS_LED1_8_2    0X40
#define DIS_LED1_8_3    0X40
#define DIS_LED1_8_4    0X40
#define DIS_LED1_9_1    0X3C
#define DIS_LED1_9_2    0X40
#define DIS_LED1_9_3    0X40
#define DIS_LED1_9_4    0X00
#define DIS_LED1___1    0X10
#define DIS_LED1___2    0X00
#define DIS_LED1___3    0X00
#define DIS_LED1___4    0X00

/*===========================================================================
  LED2(WEI2) 5 位 COM 槽 — 1 2 3 4 5 (大号中间数字)
 *===========================================================================*/
#define DIS_LED2_0_1    0X14
#define DIS_LED2_0_2    0X20
#define DIS_LED2_0_3    0X20
#define DIS_LED2_0_4    0X20
#define DIS_LED2_0_5    0X20
#define DIS_LED2_1_1    0X04
#define DIS_LED2_1_2    0X00
#define DIS_LED2_1_3    0X20
#define DIS_LED2_1_4    0X00
#define DIS_LED2_1_5    0X00
#define DIS_LED2_2_1    0X08
#define DIS_LED2_2_2    0X20
#define DIS_LED2_2_3    0X20
#define DIS_LED2_2_4    0X20
#define DIS_LED2_2_5    0X20
#define DIS_LED2_3_1    0X0C
#define DIS_LED2_3_2    0X20
#define DIS_LED2_3_3    0X20
#define DIS_LED2_3_4    0X00
#define DIS_LED2_3_5    0X20
#define DIS_LED2_4_1    0X1C
#define DIS_LED2_4_2    0X00
#define DIS_LED2_4_3    0X20
#define DIS_LED2_4_4    0X00
#define DIS_LED2_4_5    0X00
#define DIS_LED2_5_1    0X1C
#define DIS_LED2_5_2    0X20
#define DIS_LED2_5_3    0X00
#define DIS_LED2_5_4    0X00
#define DIS_LED2_5_5    0X20
#define DIS_LED2_6_1    0X1C
#define DIS_LED2_6_2    0X20
#define DIS_LED2_6_3    0X00
#define DIS_LED2_6_4    0X20
#define DIS_LED2_6_5    0X20
#define DIS_LED2_7_1    0X04
#define DIS_LED2_7_2    0X20
#define DIS_LED2_7_3    0X20
#define DIS_LED2_7_4    0X00
#define DIS_LED2_7_5    0X00
#define DIS_LED2_8_1    0X1C
#define DIS_LED2_8_2    0X20
#define DIS_LED2_8_3    0X20
#define DIS_LED2_8_4    0X20
#define DIS_LED2_8_5    0X20
#define DIS_LED2_9_1    0X1C
#define DIS_LED2_9_2    0X20
#define DIS_LED2_9_3    0X20
#define DIS_LED2_9_4    0X00
#define DIS_LED2_9_5    0X20
#define DIS_LED2___1    0X08
#define DIS_LED2___2    0X00
#define DIS_LED2___3    0X00
#define DIS_LED2___4    0X00
#define DIS_LED2___5    0X00
#define DIS_LED2_V_1    0X14        /* V                      */
#define DIS_LED2_V_2    0X00
#define DIS_LED2_V_3    0X20
#define DIS_LED2_V_4    0X20
#define DIS_LED2_V_5    0X20
#define DIS_LED2_H_1    0X1C        /* H                      */
#define DIS_LED2_H_2    0X00
#define DIS_LED2_H_3    0X20
#define DIS_LED2_H_4    0X20
#define DIS_LED2_H_5    0X00

/*===========================================================================
  LED3(WEI3) 4 位 COM 槽 — 0 1 2 3 (小号数字)
 *===========================================================================*/
#define DIS_LED3_0_1    0X02
#define DIS_LED3_0_2    0X0A
#define DIS_LED3_0_3    0X04
#define DIS_LED3_0_4    0X18
#define DIS_LED3_1_1    0X00
#define DIS_LED3_1_2    0X00
#define DIS_LED3_1_3    0X00
#define DIS_LED3_1_4    0X18
#define DIS_LED3_2_1    0X02
#define DIS_LED3_2_2    0X06
#define DIS_LED3_2_3    0X04
#define DIS_LED3_2_4    0X10
#define DIS_LED3_3_1    0X02
#define DIS_LED3_3_2    0X04
#define DIS_LED3_3_3    0X04
#define DIS_LED3_3_4    0X18
#define DIS_LED3_4_1    0X00
#define DIS_LED3_4_2    0X0C
#define DIS_LED3_4_3    0X00
#define DIS_LED3_4_4    0X18
#define DIS_LED3_5_1    0X02
#define DIS_LED3_5_2    0X0C
#define DIS_LED3_5_3    0X04
#define DIS_LED3_5_4    0X08
#define DIS_LED3_6_1    0X02
#define DIS_LED3_6_2    0X0E
#define DIS_LED3_6_3    0X04
#define DIS_LED3_6_4    0X08
#define DIS_LED3_7_1    0X00
#define DIS_LED3_7_2    0X00
#define DIS_LED3_7_3    0X04
#define DIS_LED3_7_4    0X18
#define DIS_LED3_8_1    0X02
#define DIS_LED3_8_2    0X0E
#define DIS_LED3_8_3    0X04
#define DIS_LED3_8_4    0X18
#define DIS_LED3_9_1    0X02
#define DIS_LED3_9_2    0X0C
#define DIS_LED3_9_3    0X04
#define DIS_LED3_9_4    0X18
#define DIS_LED3___1    0X00
#define DIS_LED3___2    0X04
#define DIS_LED3___3    0X00
#define DIS_LED3___4    0X00
#define DIS_LED3_F_1    0X00        /* F                      */
#define DIS_LED3_F_2    0X0E
#define DIS_LED3_F_3    0X04
#define DIS_LED3_F_4    0X00

/*===========================================================================
  LED4(WEI4) 4 位 COM 槽 — 3 4 5 6 (最右数字, 带小数点)
 *===========================================================================*/
#define DIS_LED4_0_1    0X02
#define DIS_LED4_0_2    0X02
#define DIS_LED4_0_3    0X0D
#define DIS_LED4_0_4    0X02
#define DIS_LED4_1_1    0X00
#define DIS_LED4_1_2    0X00
#define DIS_LED4_1_3    0X04
#define DIS_LED4_1_4    0X02
#define DIS_LED4_2_1    0X02
#define DIS_LED4_2_2    0X00
#define DIS_LED4_2_3    0X09
#define DIS_LED4_2_4    0X06
#define DIS_LED4_3_1    0X02
#define DIS_LED4_3_2    0X00
#define DIS_LED4_3_3    0X05
#define DIS_LED4_3_4    0X06
#define DIS_LED4_4_1    0X00
#define DIS_LED4_4_2    0X02
#define DIS_LED4_4_3    0X04
#define DIS_LED4_4_4    0X06
#define DIS_LED4_5_1    0X02
#define DIS_LED4_5_2    0X02
#define DIS_LED4_5_3    0X05
#define DIS_LED4_5_4    0X04
#define DIS_LED4_6_1    0X02
#define DIS_LED4_6_2    0X02
#define DIS_LED4_6_3    0X0D
#define DIS_LED4_6_4    0X04
#define DIS_LED4_7_1    0X00
#define DIS_LED4_7_2    0X00
#define DIS_LED4_7_3    0X05
#define DIS_LED4_7_4    0X02
#define DIS_LED4_8_1    0X02
#define DIS_LED4_8_2    0X02
#define DIS_LED4_8_3    0X0D
#define DIS_LED4_8_4    0X06
#define DIS_LED4_9_1    0X02
#define DIS_LED4_9_2    0X02
#define DIS_LED4_9_3    0X05
#define DIS_LED4_9_4    0X06
#define DIS_LED4___1    0X00
#define DIS_LED4___2    0X00
#define DIS_LED4___3    0X00
#define DIS_LED4___4    0X04
#define DIS_LED4_F_1    0X00        /* F                      */
#define DIS_LED4_F_2    0X02
#define DIS_LED4_F_3    0X09
#define DIS_LED4_F_4    0X04

/*===========================================================================
  图标位段 — 电池框/电池条/频道/MHz/小数点
 *===========================================================================*/
/* 电池图标边框 */
#define DIS_J1_POS2     0X01    /* rDisplayBuffer[2] */
#define DIS_J2_POS5     0X40    /* rDisplayBuffer[5] */
#define DIS_J3_POS5     0X10    /* rDisplayBuffer[5] */
#define DIS_J4_POS6     0X10    /* rDisplayBuffer[6] */

/* 电池电量条 (3 级) */
#define DIS_K6_POS6     0X40    /* rDisplayBuffer[6] 满格 */
#define DIS_K7_POS6     0X20    /* rDisplayBuffer[6] 中格 */
#define DIS_K8_POS6     0X08    /* rDisplayBuffer[6] 低格 */

/* 信道/频率 指示灯 */
#define DIS_CHA_POS4    0X40    /* rDisplayBuffer[4] CHA */
#define DIS_CHB_POS1    0X01    /* rDisplayBuffer[1] CHB */
#define DIS_MHz_POS1    0X02    /* rDisplayBuffer[1] MHz */
#define DIS_DOT_POS4    0X01    /* rDisplayBuffer[4] 小数点 */
#define DIS_PDU_POS0    0X01    /* rDisplayBuffer[0] PDU */

/*===========================================================================
  全局变量 & 函数声明
 *===========================================================================*/
extern unsigned char rDisplayBuffer[7];     /* 显示缓冲区, 每字节对应一个 COM 扫描槽 */

void sd1516_init(void);                     /* 初始化 7 个 GPIO 为输入/hi-Z            */
void DisplayPro(void);                      /* 扫描一步 (每次调用推进一个 COM)          */
void ScreenClean(void);                     /* 清空显示缓冲区                          */
void ShowFreq(unsigned long khz);           /* 显示频率数字 + MHz + 小数点            */
void ShowBatFull(void);                     /* 电池图标满格常亮                        */
void ShowStartup(void);                     /* 启动画面                                */

#endif
