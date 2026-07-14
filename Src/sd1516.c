/**
 ******************************************************************************
 * @file    sd1516.c
 * @brief   SD-1516 LED 显示屏驱动实现
 * @note    1/7 占空比多路复用扫描, 所有引脚在 GPIOB 上
 *          DisplayPro() 需在主循环中周期性调用 (~1ms/次, 整帧 7ms)
 ******************************************************************************
 */

#include "sd1516.h"

/*===========================================================================
  全局显示缓冲区 — 7 字节, 每个对应一个 COM 扫描槽
  每 bit 控制一个 SEG 线: bit6=SEG1, bit5=SEG2, ... bit0=SEG7
 *===========================================================================*/
unsigned char rDisplayBuffer[7];

/*----------------------------------------------------------------------------
  数字位段查找表 (与 SD-1516 物理走线对应, 不可修改)
 *----------------------------------------------------------------------------*/
static const unsigned char rNumberTable_1[11][4] = {
    {DIS_LED1_0_1, DIS_LED1_0_2, DIS_LED1_0_3, DIS_LED1_0_4}, /* 0 */
    {DIS_LED1_1_1, DIS_LED1_1_2, DIS_LED1_1_3, DIS_LED1_1_4}, /* 1 */
    {DIS_LED1_2_1, DIS_LED1_2_2, DIS_LED1_2_3, DIS_LED1_2_4}, /* 2 */
    {DIS_LED1_3_1, DIS_LED1_3_2, DIS_LED1_3_3, DIS_LED1_3_4}, /* 3 */
    {DIS_LED1_4_1, DIS_LED1_4_2, DIS_LED1_4_3, DIS_LED1_4_4}, /* 4 */
    {DIS_LED1_5_1, DIS_LED1_5_2, DIS_LED1_5_3, DIS_LED1_5_4}, /* 5 */
    {DIS_LED1_6_1, DIS_LED1_6_2, DIS_LED1_6_3, DIS_LED1_6_4}, /* 6 */
    {DIS_LED1_7_1, DIS_LED1_7_2, DIS_LED1_7_3, DIS_LED1_7_4}, /* 7 */
    {DIS_LED1_8_1, DIS_LED1_8_2, DIS_LED1_8_3, DIS_LED1_8_4}, /* 8 */
    {DIS_LED1_9_1, DIS_LED1_9_2, DIS_LED1_9_3, DIS_LED1_9_4}, /* 9 */
    {DIS_LED1___1, DIS_LED1___2, DIS_LED1___3, DIS_LED1___4}, /* _ */
};

static const unsigned char rNumberTable_2[13][5] = {
    {DIS_LED2_0_1, DIS_LED2_0_2, DIS_LED2_0_3, DIS_LED2_0_4, DIS_LED2_0_5}, /* 0 */
    {DIS_LED2_1_1, DIS_LED2_1_2, DIS_LED2_1_3, DIS_LED2_1_4, DIS_LED2_1_5}, /* 1 */
    {DIS_LED2_2_1, DIS_LED2_2_2, DIS_LED2_2_3, DIS_LED2_2_4, DIS_LED2_2_5}, /* 2 */
    {DIS_LED2_3_1, DIS_LED2_3_2, DIS_LED2_3_3, DIS_LED2_3_4, DIS_LED2_3_5}, /* 3 */
    {DIS_LED2_4_1, DIS_LED2_4_2, DIS_LED2_4_3, DIS_LED2_4_4, DIS_LED2_4_5}, /* 4 */
    {DIS_LED2_5_1, DIS_LED2_5_2, DIS_LED2_5_3, DIS_LED2_5_4, DIS_LED2_5_5}, /* 5 */
    {DIS_LED2_6_1, DIS_LED2_6_2, DIS_LED2_6_3, DIS_LED2_6_4, DIS_LED2_6_5}, /* 6 */
    {DIS_LED2_7_1, DIS_LED2_7_2, DIS_LED2_7_3, DIS_LED2_7_4, DIS_LED2_7_5}, /* 7 */
    {DIS_LED2_8_1, DIS_LED2_8_2, DIS_LED2_8_3, DIS_LED2_8_4, DIS_LED2_8_5}, /* 8 */
    {DIS_LED2_9_1, DIS_LED2_9_2, DIS_LED2_9_3, DIS_LED2_9_4, DIS_LED2_9_5}, /* 9 */
    {DIS_LED2___1, DIS_LED2___2, DIS_LED2___3, DIS_LED2___4, DIS_LED2___5}, /* _ */
    {DIS_LED2_V_1, DIS_LED2_V_2, DIS_LED2_V_3, DIS_LED2_V_4, DIS_LED2_V_5}, /* V */
    {DIS_LED2_H_1, DIS_LED2_H_2, DIS_LED2_H_3, DIS_LED2_H_4, DIS_LED2_H_5}, /* H */
};

static const unsigned char rNumberTable_3[12][4] = {
    {DIS_LED3_0_1, DIS_LED3_0_2, DIS_LED3_0_3, DIS_LED3_0_4}, /* 0 */
    {DIS_LED3_1_1, DIS_LED3_1_2, DIS_LED3_1_3, DIS_LED3_1_4}, /* 1 */
    {DIS_LED3_2_1, DIS_LED3_2_2, DIS_LED3_2_3, DIS_LED3_2_4}, /* 2 */
    {DIS_LED3_3_1, DIS_LED3_3_2, DIS_LED3_3_3, DIS_LED3_3_4}, /* 3 */
    {DIS_LED3_4_1, DIS_LED3_4_2, DIS_LED3_4_3, DIS_LED3_4_4}, /* 4 */
    {DIS_LED3_5_1, DIS_LED3_5_2, DIS_LED3_5_3, DIS_LED3_5_4}, /* 5 */
    {DIS_LED3_6_1, DIS_LED3_6_2, DIS_LED3_6_3, DIS_LED3_6_4}, /* 6 */
    {DIS_LED3_7_1, DIS_LED3_7_2, DIS_LED3_7_3, DIS_LED3_7_4}, /* 7 */
    {DIS_LED3_8_1, DIS_LED3_8_2, DIS_LED3_8_3, DIS_LED3_8_4}, /* 8 */
    {DIS_LED3_9_1, DIS_LED3_9_2, DIS_LED3_9_3, DIS_LED3_9_4}, /* 9 */
    {DIS_LED3___1, DIS_LED3___2, DIS_LED3___3, DIS_LED3___4}, /* _ */
    {DIS_LED3_F_1, DIS_LED3_F_2, DIS_LED3_F_3, DIS_LED3_F_4}, /* F */
};

static const unsigned char rNumberTable_4[12][4] = {
    {DIS_LED4_0_1, DIS_LED4_0_2, DIS_LED4_0_3, DIS_LED4_0_4}, /* 0 */
    {DIS_LED4_1_1, DIS_LED4_1_2, DIS_LED4_1_3, DIS_LED4_1_4}, /* 1 */
    {DIS_LED4_2_1, DIS_LED4_2_2, DIS_LED4_2_3, DIS_LED4_2_4}, /* 2 */
    {DIS_LED4_3_1, DIS_LED4_3_2, DIS_LED4_3_3, DIS_LED4_3_4}, /* 3 */
    {DIS_LED4_4_1, DIS_LED4_4_2, DIS_LED4_4_3, DIS_LED4_4_4}, /* 4 */
    {DIS_LED4_5_1, DIS_LED4_5_2, DIS_LED4_5_3, DIS_LED4_5_4}, /* 5 */
    {DIS_LED4_6_1, DIS_LED4_6_2, DIS_LED4_6_3, DIS_LED4_6_4}, /* 6 */
    {DIS_LED4_7_1, DIS_LED4_7_2, DIS_LED4_7_3, DIS_LED4_7_4}, /* 7 */
    {DIS_LED4_8_1, DIS_LED4_8_2, DIS_LED4_8_3, DIS_LED4_8_4}, /* 8 */
    {DIS_LED4_9_1, DIS_LED4_9_2, DIS_LED4_9_3, DIS_LED4_9_4}, /* 9 */
    {DIS_LED4___1, DIS_LED4___2, DIS_LED4___3, DIS_LED4___4}, /* _ */
    {DIS_LED4_F_1, DIS_LED4_F_2, DIS_LED4_F_3, DIS_LED4_F_4}, /* F */
};

/*===========================================================================
 * @brief  将所有显示引脚复位为输入/hi-Z (无上拉)
 * @note   LL_SetPinMode/SetPinPull 内部用 Pin*Pin 运算,
 *         多引脚组合掩码会产生错误值! 必须逐引脚单独调用!
 *===========================================================================*/
static void DisplayInit(void)
{
    /* 逐引脚设为输入 + 无上下拉 (不能合并掩码!) */
    LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG1_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG2_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG3_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG4_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG5_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG6_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(SD_GPIO_PORT, SD_SEG7_PIN, LL_GPIO_MODE_INPUT);

    LL_GPIO_SetPinPull(SD_GPIO_PORT, SD_SEG1_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(SD_GPIO_PORT, SD_SEG2_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(SD_GPIO_PORT, SD_SEG3_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(SD_GPIO_PORT, SD_SEG4_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(SD_GPIO_PORT, SD_SEG5_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(SD_GPIO_PORT, SD_SEG6_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(SD_GPIO_PORT, SD_SEG7_PIN, LL_GPIO_PULL_NO);

    /* ResetOutputPin 直接写 BRR 寄存器, 多引脚掩码安全 */
    LL_GPIO_ResetOutputPin(SD_GPIO_PORT, SD_ALL_PINS);
}

/*===========================================================================
 * @brief  根据 rDisplayBuffer[com] 逐位点亮对应 SEG 线
 * @param  dat  当前 COM 槽的段码字节
 *===========================================================================*/
static void SetDisplayData(unsigned char dat)
{
    if (dat & SD_SEG1_BIT) { SEG1_SELECT(); LL_GPIO_SetOutputPin(SD_GPIO_PORT, SD_SEG1_PIN); }
    if (dat & SD_SEG2_BIT) { SEG2_SELECT(); LL_GPIO_SetOutputPin(SD_GPIO_PORT, SD_SEG2_PIN); }
    if (dat & SD_SEG3_BIT) { SEG3_SELECT(); LL_GPIO_SetOutputPin(SD_GPIO_PORT, SD_SEG3_PIN); }
    if (dat & SD_SEG4_BIT) { SEG4_SELECT(); LL_GPIO_SetOutputPin(SD_GPIO_PORT, SD_SEG4_PIN); }
    if (dat & SD_SEG5_BIT) { SEG5_SELECT(); LL_GPIO_SetOutputPin(SD_GPIO_PORT, SD_SEG5_PIN); }
    if (dat & SD_SEG6_BIT) { SEG6_SELECT(); LL_GPIO_SetOutputPin(SD_GPIO_PORT, SD_SEG6_PIN); }
    if (dat & SD_SEG7_BIT) { SEG7_SELECT(); LL_GPIO_SetOutputPin(SD_GPIO_PORT, SD_SEG7_PIN); }
}

/*===========================================================================
 * @brief  初始化 SD-1516 显示屏
 * @note   使能 GPIOB 时钟, 全部引脚设为输入/hi-Z
 *===========================================================================*/
void sd1516_init(void)
{
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    DisplayInit();
    ScreenClean();
}

/*===========================================================================
 * @brief  显示扫描主函数 — 每次调用推进一个 COM 扫描步
 * @note   需在主循环中周期性调用 (~1ms/次 → 整帧 7ms, 143Hz 无闪烁)
 *         调用后 PB3 被设为输入以兼容按键检测
 *===========================================================================*/
void DisplayPro(void)
{
    static unsigned char index = 0;

    DisplayInit();                          /* 全部引脚复位为 hi-Z              */

    switch (index)
    {
        case 0: SEG1_SELECT(); break;       /* COM0: PB0 选通为输出            */
        case 1: SEG2_SELECT(); break;       /* COM1: PB1                       */
        case 2: SEG3_SELECT(); break;       /* COM2: PB2                       */
        case 3: SEG4_SELECT(); break;       /* COM3: PB3                       */
        case 4: SEG5_SELECT(); break;       /* COM4: PB4                       */
        case 5: SEG6_SELECT(); break;       /* COM5: PB7 (SEG6→PB7物理)       */
        case 6: SEG7_SELECT(); break;       /* COM6: PB5 (SEG7→PB5物理)       */
        default: index = 0; break;
    }
    SetDisplayData(rDisplayBuffer[index]);  /* 点亮当前 COM 槽的 SEG 段        */

    if (++index > 6) index = 0;
}

/*===========================================================================
 * @brief  清空显示缓冲区 (全灭)
 *===========================================================================*/
void ScreenClean(void)
{
    rDisplayBuffer[0] = 0;
    rDisplayBuffer[1] = 0;
    rDisplayBuffer[2] = 0;
    rDisplayBuffer[3] = 0;
    rDisplayBuffer[4] = 0;
    rDisplayBuffer[5] = 0;
    rDisplayBuffer[6] = 0;
}

/*===========================================================================
 * @brief  显示频率数值 (MHz 格式: XXX.X)
 * @param  khz  频率值 (单位 kHz, 如 146200 显示为 "146.2")
 * @note   同时点亮 DIS_MHz 和 DIS_DOT 指示灯
 *===========================================================================*/
void ShowFreq(unsigned long khz)
{
    unsigned char d1, d2, d3, d4;

    /* 先全清, 避免残留 CHB/CHA/电池/上次数字等 */
    ScreenClean();

    d1 = (unsigned char)(khz / 100000);             /* 百位 */
    d2 = (unsigned char)((khz % 100000) / 10000);   /* 十位 */
    d3 = (unsigned char)((khz % 10000) / 1000);     /* 个位 */
    d4 = (unsigned char)((khz % 1000) / 100);       /* 十分位 */

    /* 点亮指示灯 */
    rDisplayBuffer[1] |= DIS_MHz_POS1;              /* MHz 标签 */
    rDisplayBuffer[4] |= DIS_DOT_POS4;              /* 小数点    */

    /* 写入各位数字 */
    rDisplayBuffer[0] |= (rNumberTable_1[d1][0]) | (rNumberTable_3[d3][0]);
    rDisplayBuffer[1] |= (rNumberTable_1[d1][1]) | (rNumberTable_2[d2][0]);
    rDisplayBuffer[2] |= (rNumberTable_1[d1][2]) | (rNumberTable_2[d2][1]) | (rNumberTable_3[d3][1]);
    rDisplayBuffer[3] |= (rNumberTable_1[d1][3]) | (rNumberTable_2[d2][2]) | (rNumberTable_3[d3][2]) | (rNumberTable_4[d4][0]);
    rDisplayBuffer[4] |= (rNumberTable_2[d2][3]) | (rNumberTable_3[d3][3]) | (rNumberTable_4[d4][1]);
    rDisplayBuffer[5] |= (rNumberTable_2[d2][4]) | (rNumberTable_4[d4][2]);
    rDisplayBuffer[6] |= rNumberTable_4[d4][3];
}

/*===========================================================================
 * @brief  电池图标满格常亮 (3 条电量 + 边框)
 *===========================================================================*/
void ShowBatFull(void)
{
    rDisplayBuffer[2] |= DIS_J1_POS2;       /* 电池框 */
    rDisplayBuffer[5] |= DIS_J2_POS5;
    rDisplayBuffer[5] |= DIS_J3_POS5;
    rDisplayBuffer[6] |= DIS_J4_POS6;
    rDisplayBuffer[6] |= DIS_K6_POS6;       /* 电量条 3/3 */
    rDisplayBuffer[6] |= DIS_K7_POS6;
    rDisplayBuffer[6] |= DIS_K8_POS6;
}

/*===========================================================================
 * @brief  启动画面 — 显示 "1 H" (LED3='1', LED2='H')
 *===========================================================================*/
void ShowStartup(void)
{
    ScreenClean();
    rDisplayBuffer[0] = (rNumberTable_3[1][0]);
    rDisplayBuffer[1] = (rNumberTable_2[12][0]);            /* H 的 COM 槽 1 */
    rDisplayBuffer[2] = (rNumberTable_2[12][1]) | (rNumberTable_3[1][1]);
    rDisplayBuffer[3] = (rNumberTable_2[12][2]) | (rNumberTable_3[1][2]);
    rDisplayBuffer[4] = (rNumberTable_2[12][3]) | (rNumberTable_3[1][3]);
    rDisplayBuffer[5] = (rNumberTable_2[12][4]);
}
