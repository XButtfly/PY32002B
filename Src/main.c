/**
 ******************************************************************************
 * @file    main.c
 * @brief   PY32F002Bx5 主程序 — 板载 LED + WS2812B 跑马灯 + SD-1516 显示屏
 * @note    PA0 (按键): 切换板载 LED 闪烁/常亮
 *          PA6 (按键): 切换板载 LED 开/关
 *          PA1 → WS2812B DIN, 5 颗 LED 绿色跑马灯 (~600ms 循环)
 *          PA7 → ADC_IN4, 10 键分压按键 (0Ω~47K), 切换显示屏数字
 *          PB0~PB5,PB7 → SD-1516 显示屏, 默认显示 "146.2 MHz"
 ******************************************************************************
 */

#include "main.h"
#include "ws2812.h"
#include "sd1516.h"

/*===========================================================================
  ADC 按键 — 初始化
 *===========================================================================*/
static void AdcKeyInit(void)
{
    /* PA7 → 模拟输入 (GPIOA 时钟已在 main 中使能) */
    LL_GPIO_SetPinMode(ADC_KEY_GPIO_PORT, ADC_KEY_PIN, LL_GPIO_MODE_ANALOG);

    /* ADC1 时钟使能 */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);

    /*---- 逐函数配置 ADC 参数 (参照官方示例) ----*/
    LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_SYNC_PCLK_DIV8);        /* 24/8 = 3 MHz  */
    LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);
    LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
    LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_239CYCLES_5);

    /* 规则组: 软件触发, 单次转换 */
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);
    LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_OVERWRITTEN);
    LL_ADC_REG_SetSequencerDiscont(ADC1, LL_ADC_REG_SEQ_DISCONT_DISABLE);
    LL_ADC_REG_SetSequencerChannels(ADC1, ADC_KEY_CHANNEL);     /* PA7 = ADC_IN4 */

    /*---- 校准 (必须在 ADC 未使能时进行) ----*/
    if (LL_ADC_IsEnabled(ADC1) == 0U)
    {
        LL_ADC_StartCalibration(ADC1);
        while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0U) { __NOP(); }
        /* 校准结束→使能 至少 4 ADC 时钟, NOP 替代 LL_mDelay */
        { volatile uint32_t d = 8U; while (d--) { __NOP(); } }
    }

    /*---- 使能 ADC ----*/
    LL_ADC_Enable(ADC1);
    /* ADC 稳定至少 8 个 ADC 时钟 */
    { volatile uint32_t d = 16U; while (d--) { __NOP(); } }
}

/*===========================================================================
  ADC 按键 — 扫描, 返回新确认的按键号 (1-10), 无变化返回 0
  滤波: 4 次滑动平均, 去抖: 3 次连续一致才确认 (~3ms)
  g_adc_raw 暴露最新原始 ADC 值, 供校准/调试用
 *===========================================================================*/
static volatile uint16_t g_adc_raw = 0;  /* 最新 ADC 原始值 (校准时用) */

static uint8_t AdcKeyScan(void)
{
    static uint8_t  confirmed = 0;       /* 上次确认的按键号 (0=无按键)     */
    static uint8_t  cnt       = 0;       /* 连续一致计数                    */
    static uint8_t  prev      = 0;       /* 上次分类结果                    */
    static uint16_t buf[4];              /* 滑动平均 FIFO                   */
    static uint8_t  idx       = 0;       /* FIFO 写指针                     */
    static uint8_t  full      = 0;       /* FIFO 是否已满 4 个              */

    /* 读 ADC (软件触发 → 轮询 EOC) */
    LL_ADC_REG_StartConversion(ADC1);
    while (!LL_ADC_IsActiveFlag_EOC(ADC1)) { __NOP(); }
    uint16_t raw = LL_ADC_REG_ReadConversionData12(ADC1);
    LL_ADC_ClearFlag_EOC(ADC1);
    g_adc_raw = raw;  /* 暴露给外部 (校准/调试) */

    /* 滑动平均 (4 次) */
    buf[idx] = raw;
    if (++idx >= 4) { idx = 0; full = 1; }
    uint16_t adc;
    if (full) {
        uint32_t sum = (uint32_t)buf[0] + buf[1] + buf[2] + buf[3];
        adc = (uint16_t)(sum / 4);
    } else {
        adc = raw;  /* FIFO 未满时直接用原始值, 缩短首次响应延迟 */
    }

    /* 阈值判定 → 按键号 (1-10), 0=无按键 */
    uint8_t key;
    if      (adc <= ADC_THR_KEY1)  key = 1;
    else if (adc <= ADC_THR_KEY2)  key = 2;
    else if (adc <= ADC_THR_KEY3)  key = 3;
    else if (adc <= ADC_THR_KEY4)  key = 4;
    else if (adc <= ADC_THR_KEY5)  key = 5;
    else if (adc <= ADC_THR_KEY6)  key = 6;
    else if (adc <= ADC_THR_KEY7)  key = 7;
    else if (adc <= ADC_THR_KEY8)  key = 8;
    else if (adc <= ADC_THR_KEY9)  key = 9;
    else if (adc <= ADC_THR_KEY10) key = 10;
    else                           key = 0;

    /* 去抖: 连续 3 次读到同一值 → 确认, 不一致 → 清零重计 */
    if (key == prev) {
        if (cnt < 3) cnt++;
        if (cnt == 3 && key != confirmed) {
            confirmed = key;
            return key;  /* 新按键已确认 */
        }
    } else {
        cnt = 0;
    }
    prev = key;
    return 0;  /* 无新按键 */
}

int main(void)
{
    /*===================================================================
      状态变量
     *===================================================================*/
    volatile uint32_t g_on    = 1;       /* 板载 LED 总开关 (1=开)          */
    volatile uint32_t g_blink = 1;       /* 板载 LED 闪烁模式 (1=2Hz闪烁)   */

    /*===================================================================
      GPIO 初始化
     *===================================================================*/
    /*---- GPIOA 时钟 (PA5 LED, PA0 按键, PA6 按键, PA1 WS2812) ----*/
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

    /* PA5 — 板载 LED (低电平亮) */
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_NO);
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);

    /* PA0 — 闪烁/常亮 切换按键 (内部上拉, 按下→GND) */
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_UP);

    /* PA6 — 开/关 切换按键 (内部上拉, 按下→GND) */
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_6, LL_GPIO_PULL_UP);

    /*---- ADC 按键初始化 (PA7) ----*/
    AdcKeyInit();

    /*---- WS2812B 初始化 (PA1) ----*/
    ws2812_init();

    /*---- SD-1516 显示屏初始化 (PB0~PB5,PB7) ----*/
    sd1516_init();
    ShowFreq(146200);           /* 显示 "146.2" MHz + 小数点 + MHz 标签   */
    ShowBatFull();              /* 电池图标满格常亮                        */

    /*===================================================================
      运行时状态
     *===================================================================*/
    uint32_t tick      = 0;       /* 主循环节拍 0..499, ~1ms/tick          */
    uint32_t pa0_lock  = 0;       /* PA0 按键去抖锁定计数值                */
    uint32_t pa6_lock  = 0;       /* PA6 按键去抖锁定计数值                */
    uint32_t pa0_prev  = 1;       /* PA0 上一次采样值                      */
    uint32_t pa6_prev  = 1;       /* PA6 上一次采样值                      */
    uint32_t ws_timer  = 0;       /* WS2812 帧间隔计时器 (~1ms/tick)       */
    uint32_t ws_pos    = 0;       /* 当前跑马灯亮灯位置 0..4               */

    /* ADC 按键状态 */
    uint8_t  adc_key_cur = 0;     /* 当前显示的按键号 (0=146.2, 1-9=00X.0) */

    /*===================================================================
      主循环
     *===================================================================*/
    while (1)
    {
        /*---- ADC 按键扫描: 检测到新按键 → 切换显示 ----*/
        {
            uint8_t new_key = AdcKeyScan();
            if (new_key && new_key != adc_key_cur)
            {
                adc_key_cur = new_key;
                ScreenClean();
                if (new_key == 10) {
                    ShowFreq(146200);       /* 恢复默认 146.2 MHz          */
                } else {
                    ShowFreq((unsigned long)new_key * 1000); /* 00X.0     */
                }
                ShowBatFull();              /* 电池图标常亮                */
            }
        }
        /*---- PA0: 下降沿触发 → 切换闪烁模式, 锁定 200ms ----*/
        {
            uint32_t n = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
            if (pa0_lock) { pa0_lock--; }
            else if (pa0_prev == 1U && n == 0U)
            {
                g_blink  = !g_blink;
                pa0_lock = 200;
            }
            pa0_prev = n;
        }

        /*---- PA6: 下降沿触发 → 切换 LED 开关, 锁定 200ms ----*/
        {
            uint32_t n = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_6);
            if (pa6_lock) { pa6_lock--; }
            else if (pa6_prev == 1U && n == 0U)
            {
                g_on     = !g_on;
                pa6_lock = 200;
            }
            pa6_prev = n;
        }

        /*---- 板载 LED: 500ms 周期 = 2Hz 闪烁 / 常亮 / 熄灭 ----*/
        if (!g_on)
        {
            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);     /* 熄灭 (高电平) */
        }
        else if (g_blink && tick >= 250U)
        {
            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);     /* 闪烁-灭半周期  */
        }
        else
        {
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);   /* 亮 (低电平)   */
        }

        /*---- WS2812B 绿色跑马灯: 每 ~120ms 移动一步 ----*/
        ws_timer++;
        if (ws_timer >= 120U)
        {
            ws_timer = 0;
            {
                uint32_t buf[WS2812_COUNT];
                for (uint32_t j = 0; j < WS2812_COUNT; j++)
                    buf[j] = (j == ws_pos) ? WS_DIM_GREEN : WS_OFF; /* 仅当前位亮 */
                ws2812_send(buf, WS2812_COUNT);
                ws_pos++;
                if (ws_pos >= WS2812_COUNT) ws_pos = 0;     /* 循环回起点 */
            }
        }

        /* ~1ms 延时 + 显示屏扫描: 7 步交织, 每步 ~143µs, 整帧 1ms=1000Hz */
        {
            int s;
            for (s = 0; s < 7; s++)
            {
                DisplayPro();
                { volatile uint32_t d = 1143U; while (d--) { __NOP(); } }
            }
        }
        tick++;
        if (tick >= 500U) tick = 0;
    }
}
