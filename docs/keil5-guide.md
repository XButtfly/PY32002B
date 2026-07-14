# Keil uVision5 操作流程 — PY32F002Bx5

## 环境要求

| 组件 | 路径/版本 |
|------|-----------|
| Keil MDK | `D:\Keil5` (v5.25, ARM Compiler 5.06) |
| Puya Device Pack | Puya.PY32F0xx_DFP.1.1.9 (已安装) |
| 调试器 | ST-Link (SWD) |
| 串口工具 | 任意串口助手, 115200-8-N-1 |

## 一、打开项目

```
1. 双击 D:\zl\demo1\PY32002B.uvprojx
2. Keil uVision5 自动启动并加载项目
```

项目树结构：
```
Target 1
├── Startup
│   └── startup_py32f002bx5.s    (Device Pack 提供)
├── User
│   ├── main.c                    (LED + printf + USART 回显)
│   ├── py32f002b_it.c            (SysTick / HardFault)
│   ├── system_py32f002b.c        (时钟初始化 + 频率表)
│   └── syscalls.c                (newlib 桩, _write → USART1)
└── Drivers
    ├── py32f002b_hal.c           (HAL 核心)
    ├── py32f002b_hal_flash.c     (Flash 操作)
    ├── py32f002b_hal_cortex.c    (NVIC / SysTick)
    ├── py32f002b_ll_utils.c      (LL 工具)
    ├── py32f002b_ll_usart.c      (USART LL 驱动)
    └── py32f002b_ll_rcc.c        (RCC LL 驱动)
```

## 二、编译构建

| 操作 | 快捷键 | 说明 |
|------|--------|------|
| 编译当前文件 | `Ctrl+F7` | 只编译当前 .c |
| 构建整个项目 | `F7` | 编译所有修改过的文件 + 链接 |
| 全部重新构建 | 菜单 → Project → Rebuild all target files | 强制全部重编 |

**构建输出应看到：**
```
Build target 'Target 1'
compiling main.c...
compiling py32f002b_it.c...
...
linking...
Program Size: Code=xxxx RO-data=xxxx RW-data=xxxx ZI-data=xxxx
"PY32002B.axf" - 0 Error(s), 0 Warning(s).
```

> ⚠️ **警告抑制**：AC5 编译器已配置 `--diag_suppress=1 --diag_suppress=1295`（对应 EIDE eide.yml 中的配置）来抑制库文件的已知诊断。

## 三、烧录 (Download)

```
菜单 → Flash → Download   (或工具栏 Download 按钮 F8)
```

烧录流程：
1. Keil 通过 ST-Link 连接芯片 (SWD 接口)
2. 自动擦除 Flash 对应扇区
3. 写入 `PY32002B.axf` → `0x08000000`
4. 校验写入内容
5. 状态栏显示 "Flash Download Finished"

**连接检查清单：**
- [ ] ST-Link 通过 SWD (SWCLK/SWDIO) 连接目标板
- [ ] 目标板已供电 (3.3V)
- [ ] Keil → Options for Target → Debug → Use: ST-Link Debugger
- [ ] Keil → Options for Target → Utilities → Use Target Driver for Flash Programming

## 四、调试 (Debug)

### 启动调试
```
菜单 → Debug → Start/Stop Debug Session   (或工具栏 按钮 Ctrl+F5)
```

### 常用调试操作

| 操作 | 快捷键/方式 | 功能 |
|------|------------|------|
| 运行 | `F5` | 全速运行 |
| 暂停 | `Esc` | 暂停执行 |
| 单步进入 | `F11` | Step Into (进入函数) |
| 单步跳过 | `F10` | Step Over (不进入函数) |
| 单步返回 | `Ctrl+F11` | Step Out (跳出当前函数) |
| 运行到光标 | `Ctrl+F10` | Run to Cursor Line |
| 设置断点 | 行号左侧点击 | 红色圆点, F9 切换 |
| 复位 | 工具栏 RST 按钮 | 复位 MCU |

### 调试窗口

| 窗口 | 菜单路径 | 用途 |
|------|----------|------|
| Watch 1/2 | View → Watch Windows | 监视变量值 |
| Memory | View → Memory Windows | 查看内存/寄存器 |
| Peripheral | View → System Viewer | 外设寄存器 (通过 SVD) |
| Serial | View → Serial Windows | UART 输出 (软件仿真用) |
| Disassembly | View → Disassembly Window | 反汇编 |
| Call Stack | View → Call Stack Window | 调用栈 |
| Registers | 左侧 Project 区域底部 | 内核寄存器 R0-R15 |

### 常用断点技巧

**HardFault 调试** — `py32f002b_it.c` 中已内置 `__BKPT(0)`：
```c
void HardFault_Handler(void)
{
    __BKPT(0);   // ← 发生 HardFault 时自动停在这里
    while (1) { }
}
```
此时查看 **Call Stack + Registers** 窗口，SP 指向异常栈帧，可分析故障原因。

**条件断点** — 右键断点 → Condition：
```
ch == '\r'     // 只在收到回车时停
count >= 100   // 计数器达到 100 次
```

## 五、串口输出验证

1. 用 USB-TTL 连接目标板串口：
   - TX → PB4 (RX of MCU)
   - RX → PB3 (TX of MCU)
   - GND → GND

2. 打开串口助手，配置：
   ```
   波特率: 115200 | 数据位: 8 | 停止位: 1 | 校验: None
   ```

3. 复位 MCU 或重新上电，应看到启动信息：
   ```
   ========================================
     PY32F002Bx5 — Keil + EIDE + GCC
     SystemCoreClock: 24000000 Hz
     USART1: PB3(TX) PB4(RX) @ 115200 baud
     LED: PA5 (active low, 500 ms toggle)
     Echo mode: type any key, it echoes back
   ========================================
   ```

4. 在串口助手输入任意字符 → 立即回显，`\r` 自动补 `\n`。

## 六、项目配置速查

### Options for Target (Alt+F7) 关键配置

| 选项卡 | 配置项 | 值 |
|--------|--------|-----|
| **Target** | Xtal | 12.0 MHz (无关, HSI 内部) |
| | IROM1 | Start: 0x8000000, Size: 0x6000 |
| | IRAM1 | Start: 0x20000000, Size: 0xC00 |
| **C/C++** | Define | `PY32F002Bx5, USE_FULL_LL_DRIVER` |
| | Include Paths | `.\Inc` ; `..\PY32F002B\Drivers\CMSIS\Include` ; `..\PY32F002B\Drivers\CMSIS\Device\PY32F0xx\Include` ; `..\PY32F002B\Drivers\PY32F002B_HAL_Driver\Inc` |
| | C99 Mode | ✅ |
| | GNU extensions | ✅ |
| | One ELF Section per Function | ✅ |
| | Optimization | Level 0 (-O0) |
| | Warnings | All |
| | Misc Controls | `--diag_suppress=1 --diag_suppress=1295` |
| **Debug** | Use | ST-Link Debugger |
| **Utilities** | Use Target Driver | ✅ |

## 七、VSCode ↔ Keil 双工具链协作

| 用途 | 工具 | 
|------|------|
| 日常编辑、代码导航、IntelliSense | **VSCode** + EIDE |
| GCC 构建验证、CI | **EIDE** (Build 按钮) |
| 芯片烧录、硬件调试 | **Keil uVision5** |
| 备用调试 | VSCode `F5` (Cortex-Debug + OpenOCD) |

**协作流程：**
```
VSCode 编辑代码 → EIDE (GCC Build) 验证无警告
    → Keil F7 编译 → F8 烧录 → Ctrl+F5 调试
```

## 八、故障排查

| 现象 | 检查项 |
|------|--------|
| "未找到ulink2/me设备" / "Internal DLL Error" | Debug 选项卡未切换到 ST-Link: **Alt+F7 → Debug → 右侧下拉选 "ST-Link Debugger"** |
| "Flash Download Failed" / "Target DLL cancelled" | Utilities 选项卡: **Alt+F7 → Utilities → 下拉选 "ST-Link Debugger"** |
| "No ST-Link found" | ST-Link USB 连接、驱动安装 |
| "Flash Download Failed" (SWD) | SWD 接线 (SWCLK/SWDIO)、目标板供电 |
| "Cannot Access Memory" | MCU 是否处于低功耗/读保护状态 |
| HardFault 触发 | BKPT 处查看 Call Stack + SP 寄存器 |
| 串口无输出 | 接线 (TX→RX, RX→TX, GND)、波特率 115200 |
| printf 无输出 | Keil: 检查 Use MicroLIB = ✅ (已完成); GCC: 确认 syscalls.c 加入编译 |
