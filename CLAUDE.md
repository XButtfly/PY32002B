# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PY32F002Bx5 (Cortex-M0+, 24KB Flash, 3KB RAM) bare-metal embedded project. Dual toolchain: **GCC** (EIDE, primary build) + **Keil AC5** (debug/flash). HSI 24 MHz internal oscillator, no HSE. LED on PA5 (active low), USART1 on PB3(TX)/PB4(RX) @ 115200.

## Build Commands

### GCC (command line, zero-dependency on IDE)
```cmd
.\build\full_build.cmd
```
Compiles all sources + links. Output: `build\PY32002B_Target\Project.elf`.

GCC toolchain: `arm-none-eabi-gcc` (v15.2.1), installed via EIDE.

**Manual compile single file** (standard pattern):
```cmd
arm-none-eabi-gcc -std=c99 -mcpu=cortex-m0plus -mthumb -g3 -Og -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -ffunction-sections -fdata-sections -c -I. -IInc -I..\PY32F002B\Drivers\CMSIS\Include -I..\PY32F002B\Drivers\CMSIS\Device\PY32F0xx\Include -I..\PY32F002B\Drivers\PY32F002B_HAL_Driver\Inc -DUSE_FULL_LL_DRIVER -DPY32F002Bx5 -DHSI_VALUE=24000000U -o build\main.o Src\main.c
```

### GCC (EIDE)
Open VSCode in this folder → EIDE panel → click **Build** (or `Ctrl+Shift+B`).

### Keil uVision5
Open `PY32002B.uvprojx` → **F7** Build → **F8** Download → **Ctrl+F5** Debug. Detailed guide at `docs/keil5-guide.md`.

### Warnings policy
- `-Wno-unused-parameter` and `-Wno-implicit-fallthrough` suppress warnings from Puya vendor HAL/LL headers (not project code).
- `-Wl,--no-warn-rwx-segments` suppresses the LOAD segment RWX warning (RAM must be rwx for Cortex-M0+).
- Target for all builds: **0 warnings, 0 errors**.

## Architecture

### Driver layering (bottom-up)
```
Application (main.c)
    ├── HAL (py32f002b_hal.c, _hal_cortex.c) — Init, Delay, NVIC
    ├── LL  (py32f002b_ll_*.c, py32f002b_ll_*.h) — static-inline peripheral access
    ├── CMSIS Core (core_cm0plus.h) — SCB, SysTick, NVIC registers
    └── CMSIS Device (py32f002bx5.h, system_py32f0xx.h) — peripheral base addresses
```

### Include chain
Every `.c` includes exactly `"main.h"`. That header pulls in:
1. `py32f0xx.h` → `py32f002bx5.h` (device header, selected by `-DPY32F002Bx5`)
2. LL driver headers (bus, rcc, system, cortex, gpio, usart, utils, pwr)
3. `py32f002b_hal_conf.h` + `py32f0xx_hal.h` (HAL layer)
4. `py32f002b_it.h` (interrupt prototypes)
5. Board pin definitions (LED_GPIO_PORT, DEBUG_USART, etc.)

`py32f002b_hal_conf.h` lives in project `Inc/` and **overrides** the one in the driver tree via include path ordering (`-IInc` before `-I../Drivers/.../Inc`). This is how HSI_VALUE, HSE_VALUE, and module enables are configured per-project.

### Clock tree
- Source: **HSI 24 MHz** internal (no HSE crystal)
- Flash latency: 1 wait state (set in `SystemInit()` before clock increase)
- Path: HSI → HSISYS → SYSCLK (24 MHz) → AHB (/1) → APB1 (/1)
- SysTick: 1 ms tick (`SystemCoreClock / 1000`)
- SystemCoreClock updated via `SystemCoreClockUpdate()` which reads RCC CFGR SWS bits

### Custom SystemInit (no Device Pack dependency for GCC)
`Src/system_py32f002b.c` provides its own `SystemInit()` + `SystemCoreClockUpdate()` + frequency tables, independent of the Device Pack's `system_py32f0xx.c`. Includes a 100ms SWD recovery delay and loads HSI/LSI trim values from system memory at `0x1FFF0100` / `0x1FFF0144`.

### Startup (GCC)
`Startup/startup_py32f002bxx.S` — custom vector table + CMSIS copy-table/zero-table init (`__copy_table_start__` / `__zero_table_start__`). Calls `SystemInit` → `_start` (libc init) → `main`. All peripheral IRQs weak-linked to `Default_Handler` (BKPT + infinite loop).

### Startup (Keil)
Uses the Device Pack's `startup_py32f002bx5.s` referenced via `$$Device:PY32F002Bx5$`. Not part of the project repo — resolved at build time from the pack.

### syscalls.c — cross-compiler printf retargeting
```c
#if defined(__ARMCC_VERSION)
  int fputc(int ch, FILE *f);  // Keil microlib: printf → fputc
#else
  int _write(int fd, const char *buf, int len);  // GCC newlib-nano: printf → _write
#endif
```
Route: `printf()` → USART1 TX (PB3). Keil build uses **microlib** (`<useUlib>1</useUlib>` in uvprojx).

### Memory map
| Region | Start | Size | Notes |
|--------|-------|------|-------|
| Flash | 0x08000000 | 24 KB (0x6000) | Vector table at base |
| SRAM | 0x20000000 | 3 KB (0xC00) | Stack at top, 512 B |
| HSI trim | 0x1FFF0100 | 2 B | Factory calibration |
| LSI trim | 0x1FFF0144 | 2 B | Factory calibration |

Linker script: `Linker/PY32F002Bx5.ld` — asserts `__StackLimit >= __HeapLimit` to catch RAM overflow.

## Key Design Decisions

1. **Project-specific HAL config in Inc/** — not in the driver tree. Modifying `py32f002b_hal_conf.h` in `Inc/` configures the HAL per-project without touching vendor files.
2. **Separate Startup/ for GCC** — the GCC startup and linker script are project-owned (not from Device Pack) for full control over init sequence.
3. **Library sources via relative path** — `../PY32F002B/Drivers/...` keeps the SDK outside the repo, referenced read-only.
4. **Dual toolchain, both active** — GCC/EIDE for fast VSCode-native builds, Keil AC5 for ST-Link flash + hardware debug (peripheral viewer, SVD integration). Both produce functional firmware with 0 warnings.
5. **Zero-warning baseline** — vendor-library warnings suppressed via flags so project-code warnings stand out.

## Files to Not Modify

- `../PY32F002B/Drivers/**` — Puya SDK, treat as read-only. If driver bugs found, work around in application code.
- `.pack/Puya/**` — Device Pack cache, managed by EIDE.
- `build/Target 1/**` — Keil build artifacts managed by EIDE.

## Adding a New Peripheral

1. Include its LL header in `Inc/main.h`
2. Add the driver `.c` to both EIDE (`eide.yml` → virtualFolder → Drivers) and Keil (`PY32002B.uvprojx` → Groups → Drivers)
3. Update `build/full_build.cmd` with the new `.o` in both compile and link steps
4. If it needs HAL config, add `#define HAL_xxx_MODULE_ENABLED` in `Inc/py32f002b_hal_conf.h`

## Pin Mappings

**!!! PY32F002B non-standard SWD pinout** (controlled by FLASH_OPTR bits 13-14):
- SWCLK = **PA14** (standard)
- SWDIO = **PB6** (NOT PA13! Default: OB_SWD_PB6_NRST_PC0)
- NRST = **PC0**
- If SWD lost, use Connect-under-Reset (requires NRST connected) or see reference template's `fix_opb.cfg` at `..\PY32F002B\Templates\PY32F002Bxx_Templates_LL\MDK-ARM\fix_opb.cfg`.

| Function | Port | Pin | AF | Notes |
|----------|------|-----|----|-------|
| LED | PA5 | LL_GPIO_PIN_5 | — | Output, active low |
| USART1 TX | PB3 | LL_GPIO_PIN_3 | AF1 | To USB-TTL RX |
| USART1 RX | PB4 | LL_GPIO_PIN_4 | AF1 | From USB-TTL TX |
| SWCLK | PA14 | — | AF0 | Debug, do not use |
| SWDIO | PB6 | — | AF0 | Debug (default), do not use |
| NRST | PC0 | — | — | Reset (default) |
