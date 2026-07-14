# PY32F002Bx5 裸机项目

基于 **PY32F002Bx5** (Cortex-M0+, 24KB Flash, 3KB RAM, 24MHz HSI) 的嵌入式项目。GCC 编译 + Keil AC5 烧录。

## 硬件模块

| 模块 | 引脚 | 说明 |
|------|------|------|
| 板载 LED | PA5 | 低电平亮，PA0 切换闪烁/常亮，PA6 切换开/关 |
| WS2812B 灯带 | PA1 | 5 颗 LED，绿色跑马灯 ~600ms 循环 |
| SD-1516 显示屏 | PB0~PB5, PB7 | 7 线 Charlieplexing 多路复用 |
| ADC 按键 ×10 | PA7 (ADC_IN4) | 电阻分压按键，切换显示屏数字 |

## 快速开始

```cmd
REM 编译 (GCC)
.\build\full_build.cmd

REM 烧录 (Keil uVision5)
REM 打开 PY32002B.uvprojx → F8
```

GCC 工具链: `arm-none-eabi-gcc` (通过 EIDE 或系统 PATH)

## ADC 按键

10 个分压按键 (VCC → 10K → PA7 → 按键 → R_btn → GND):

| 按键 | 电阻 | 显示 |
|:----:|:----:|------|
| 1 | 0Ω | 001.0 MHz |
| 2 | 1K | 002.0 MHz |
| 3 | 2K2 | 003.0 MHz |
| 4 | 4K7 | 004.0 MHz |
| 5 | 6K8 | 005.0 MHz |
| 6 | 10K | 006.0 MHz |
| 7 | 15K | 007.0 MHz |
| 8 | 22K | 008.0 MHz |
| 9 | 33K | 009.0 MHz |
| 10 | 47K | 146.2 MHz (恢复) |


## ⚠️ 重要注意事项

1. **`LL_GPIO_SetPinMode/SetPinPull/SetPinSpeed` 不能传多引脚掩码** — 必须逐引脚调用，否则会破坏 SWD 引脚
2. **`LL_mDelay()` / `HAL_Delay()` 不能用** — 项目无 SysTick (无 `HAL_Init`)，会死循环。用 NOP 循环替代
3. **PY32F002B ADC 引脚映射 ≠ STM32** — PA7=ADC_IN4, PB0=ADC_IN7。PA3/PA4 无 ADC 功能
4. **ADC 采样时间必须显式设置** — 默认 3.5 周期不够，需 `LL_ADC_SAMPLINGTIME_239CYCLES_5`
5. **ADC 校准必须在使能之前** — 配置→校准→延时→使能→延时
6. **PB5 和 PB7 物理走线互换** — 代码已适配

## 编译状态

- **GCC**: ✅ 0 warnings, 0 errors — FLASH 8188B (33%), RAM 476B (15%)
- **Keil AC5**: ✅ 正常烧录调试

## 关键文件

| 文件 | 用途 |
|------|------|
| `Src/main.c` | 主程序 |
| `Src/ws2812.c` | WS2812B 驱动 |
| `Src/sd1516.c` | SD-1516 显示驱动 |
| `Inc/main.h` | 引脚定义 + ADC 阈值 |
| `Inc/sd1516.h` | 显示位段定义 |
| `build/full_build.cmd` | GCC 编译脚本 |
| `PY32002B.uvprojx` | Keil 工程 |
| `HANDOFF.md` | 详细交接文档 |
