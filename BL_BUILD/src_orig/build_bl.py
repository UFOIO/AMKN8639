#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
build_bl.py - 单独编译 AMKN8639 Bootloader (Phase 3.2)
  - 用 armcc + armlink 编译 startup_boot.s + bootloader.c
  - 不动 Keil .uvprojx，独立验证 BL 编译可行性
  - 失败可回滚（未污染任何文件）

用法：python build_bl.py
"""
import os
import subprocess
import sys
import io
from pathlib import Path

# Force UTF-8 stdout
if hasattr(sys.stdout, "buffer"):
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding="utf-8", errors="replace")

# ===== 路径 =====
PROJECT_ROOT = Path(r"C:\Users\gjt\Desktop\项目工程\2026内蒙轻型机库项目\软件\机库工控机\机库电设文档\AMKNxxxx_FreeRTOS_V1.31.02_20260501")
BOOT_DIR     = PROJECT_ROOT / "targets" / "AMKN8639" / "Boot"
OUTPUT_DIR   = BOOT_DIR / "output"

ARMCC_BIN    = Path(r"D:/keil5/ARM/ARMCC/bin")
ARMCC        = ARMCC_BIN / "armcc.exe"
ARMASM       = ARMCC_BIN / "armasm.exe"
ARMLINK      = ARMCC_BIN / "armlink.exe"
FROMELF      = ARMCC_BIN / "fromelf.exe"

# 源文件
STARTUP_S    = BOOT_DIR / "startup_boot.s"
BOOTLOADER_C = BOOT_DIR / "bootloader.c"
W25Q64_C     = BOOT_DIR / "w25q64.c"     # Modify 2026.6.23: P1 W25Q64 驱动
SCAT         = BOOT_DIR / "bootloader.scat"

# 输出文件
STARTUP_O    = OUTPUT_DIR / "startup_boot.o"
BOOTLOADER_O = OUTPUT_DIR / "bootloader.o"
W25Q64_O     = OUTPUT_DIR / "w25q64.o"
AXF          = OUTPUT_DIR / "AMKN8639_BL.axf"
BIN          = OUTPUT_DIR / "AMKN8639_BL.bin"
MAP          = OUTPUT_DIR / "AMKN8639_BL.map"

# Include 路径 (Phase 3.2 v2 极简: 只 BOOT_DIR)
INCLUDES = [
    BOOT_DIR,
    Path(r"D:/keil5/ARM/ARMCC/include"),
]

# 库文件
# Phase 3.2 v2: 不依赖 .lib, 全部寄存器直写
# LIB_PATH = PROJECT_ROOT / "Libraries"
# H743_LIB = LIB_PATH / "STM32H743XX_V131_20260415.lib"

# 宏定义 (Phase 3.2 v2 极简, 不引产品宏)
DEFINES = [
    "STM32H743XX",          # 通用 H743 标识
]

# ARMCC 参数
ARMCC_COMMON = [
    "--cpu=Cortex-M7",
    "--fpu=VFPv4_D16",     # H743 单精度 FPU (armcc 5.05 最高只支持 VFPv4)
    "--apcs=/hardfp",     # 硬浮点 ABI
    "-O2",                # 优化
    "-g",                 # 调试信息
    "--split_sections",   # 优化 section 放置
    "--diag_suppress=1295",  # 抑制 deprecated 警告
]
ARMCC_INCLUDES = ["-I" + str(p) for p in INCLUDES]
ARMCC_DEFINES  = ["-D" + d for d in DEFINES]

# 链接参数
ARMLINK_COMMON = [
    "--cpu=Cortex-M7",
    "--fpu=VFPv4_D16",
    "--scatter", str(SCAT),
    "--entry=ResetHndlr",           # 显式入口 (0x08000000 是 initial SP 数据, 不是指令)
    "--library_type=standardlib",
    "--strict",                     # 严格检查
    "--no_remove",                  # 保留未引用 section（debug 用）
    "--summary_stderr",             # 摘要输出到 stderr
    "--info=summarysizes",          # 大小摘要
    "--info=stack",                 # 栈信息
    "--map",                        # 生成 map
    "--list", str(OUTPUT_DIR / "AMKN8639_BL_list.txt"),
]

# ===== 工具函数 =====
def run(cmd, name):
    print(f"\n========== {name} ==========")
    print(" ".join(f'"{c}"' if " " in str(c) else str(c) for c in cmd))
    r = subprocess.run(cmd, capture_output=True, text=True, errors="replace", encoding="gbk")
    print(r.stdout)
    if r.returncode != 0:
        print(f"!! {name} FAILED, returncode={r.returncode}")
        print("STDERR:", r.stderr)
        sys.exit(1)
    return r

# ===== 编译流程 =====
def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    # 1) armasm 编译 startup_boot.s
    #    ARMASM 不支持 -D，--pd 用于传宏，但启动文件不用这些宏所以不传
    asm_cmd = [
        str(ARMASM),
        *ARMCC_INCLUDES,
        "--cpu=Cortex-M7",
        "--fpu=VFPv4_D16",
        "-g",
        "-o", str(STARTUP_O),
        str(STARTUP_S),
    ]
    run(asm_cmd, "ARMASM startup_boot.s")

    # 2) armcc 编译 bootloader.c
    c_cmd = [
        str(ARMCC),
        *ARMCC_INCLUDES,
        *ARMCC_DEFINES,
        *ARMCC_COMMON,
        "-c",                  # 只编译不链接
        "-o", str(BOOTLOADER_O),
        str(BOOTLOADER_C),
    ]
    run(c_cmd, "ARMCC bootloader.c")

    # 2b) armcc 编译 w25q64.c (Modify 2026.6.23: P1)
    if W25Q64_C.exists():
        w25_cmd = [
            str(ARMCC),
            *ARMCC_INCLUDES,
            *ARMCC_DEFINES,
            *ARMCC_COMMON,
            "-c",
            "-o", str(W25Q64_O),
            str(W25Q64_C),
        ]
        run(w25_cmd, "ARMCC w25q64.c")

    # 3) armlink 链接 (Phase 3.2 v2: 不连 .lib, 全部寄存器直写)
    objects = [str(STARTUP_O), str(BOOTLOADER_O)]
    if W25Q64_C.exists():
        objects.append(str(W25Q64_O))
    link_cmd = [
        str(ARMLINK),
        *ARMLINK_COMMON,
        *objects,
        "-o", str(AXF),
    ]
    run(link_cmd, "ARMLINK")

    # 4) fromelf 生成 .bin
    bin_cmd = [
        str(FROMELF),
        "--bin",
        "-o", str(BIN),
        str(AXF),
    ]
    run(bin_cmd, "FROMELF .bin")

    # 5) 检查大小（必须 < 64KB = 65536 bytes）
    bin_size = BIN.stat().st_size
    print(f"\n========== SIZE CHECK ==========")
    print(f"  Bootloader .bin size: {bin_size} bytes ({bin_size/1024:.1f} KB)")
    print(f"  Limit: 65536 bytes (64 KB)")
    if bin_size > 65536:
        print("!! OVERFLOW: Bootloader > 64KB! Must reduce code size.")
        sys.exit(2)
    else:
        print(f"  OK: {65536 - bin_size} bytes free ({100*(65536-bin_size)/65536:.1f}%)")

    # 6) 自动复制到 BIN/ 目录 (Modify 2026.6.23)
    print(f"\n========== COLLECT TO BIN/ ==========")
    collect_script = BOOT_DIR.parent / "Tools" / "collect_bins.py"
    if collect_script.exists():
        r = subprocess.run(
            [sys.executable, str(collect_script)],
            cwd=str(PROJECT_ROOT),
        )
        if r.returncode not in (0, 1):  # 0=成功, 1=无源 (warn, 不致命)
            print(f"!! collect_bins.py 返回 {r.returncode}")
    else:
        print(f"!! 收集脚本不存在: {collect_script}")

if __name__ == "__main__":
    main()
