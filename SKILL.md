# AMKN8639 Bootloader & OTA 开发指南

## 项目概况

AMKN8639 基于 STM32H743，BL (Bootloader) 实现远程无人化固件升级。
**当前状态: BL V6.0 生产就绪 ✅**  (2026-06-30)

## 文件结构
```
targets/AMKN8639/
  bootloader/
    bootloader.c       -- BL 源码 (单文件，纯寄存器，无 HAL 依赖)
    bootloader.sct     -- Keil 链接脚本 (BL @ 0x08000000, 128KB)
    build.ps1          -- 编译+打包脚本 (BL + APP = SX.bin)
    bootloader.bin     -- BL 固件产物 (~2KB)
    bootloader_pad.bin -- 128KB 对齐
  ota_app/
    ota_trigger.h      -- APP 侧 OTA 触发代码 (已弃用, 现用内联代码)
    ota_config.h       -- OTA 配置
  scripts/
    ota_host.py        -- PC 上位机 OTA 烧录工具 (已测试通过)
    rk_ota_agent.py    -- RK3568 远程升级代理
    xcp_test.py        -- XCP 协议测试工具
  output/bin/
    AMKN8639_APP.bin   -- APP 固件 (Keil 编译产物)
    AMKN8639_SX.bin    -- BL+APP 合并固件 (BL 128KB + APP)
```

## 内存布局
```
0x08000000  +-----------------+
            |  BL  (128KB)    |  Sector 0  (写保护)
0x08020000  +-----------------+
            |  APP (~1.2MB)   |  Sector 1-10 (OTA 可更新)
0x081E0000  +-----------------+  (BL 自升级暂存区, 128KB)
0x08200000  +-----------------+  (Flash 末端)
```

## BL V6.0 特性 (已验证)

| 特性 | 状态 | 说明 |
|------|------|------|
| HSI 64MHz | ✅ | 不依赖外部晶振 |
| UART1 115200 | ✅ | PA9(TX) PA10(RX), 仅输出 |
| LED 指示 (PB7) | ✅ | 低电平亮, 开机 10s 快闪等待 XCP |
| 内部 Flash 驱动 | ✅ | 解锁/擦除/写入, 已验证 |
| XCP-lite 协议 | ✅ | 5 条命令, 完整验证 |
| APP 校验 (SP+PC) | ✅ | SP 0x2xxx/0x0xxx, PC 0x0802xxxx |
| APP→BL 后门 | ✅ | AT+OTA → BKP0R=0x424C4F54 → Reset |
| BL 自升级框架 | 🔶 | 代码已就位, 待独立测试 |
| QSPI 外部 Flash | ❌ | 硬件差异 (W25Q64), BL 不用 |

## APP 集成 (已完成)

**关键文件修改:**
- `source/user_app.c` (line ~1165): AT+OTA handler 改为 XCP 路径
  ```c
  // XCP OTA: write BKP magic + reset, BL will enter XCP mode
  Garage_PushEvt("OTA=ENTER_XCP", 0);
  Delay_ms(200);
  *(volatile INT32U *)0x58004000 = 0x424C4F54;  // BKP0R
  __dsb(0xF);
  SysLib_Ctrl(CMD_SYSLIB_RESET, 0);
  ```
- `libapp/at.c` (line ~1723): 冗余 handler (已同步更新)

## XCP-lite 协议

### 命令包格式
```
[0xAA] [LEN] [CMD] [DATA...] [CKSUM]
```
- LEN = CMD(1) + DATA(N) + CKSUM(1) = N+2
- CKSUM = XOR(CMD, DATA...)   **不包含 LEN**

### 命令列表
| CMD | 名称 | 数据 | 超时 | 说明 |
|-----|------|------|------|------|
| 0x01 | Erase | uint32 addr (Little-Endian) | 10s | 擦除 128KB 扇区 |
| 0x02 | Write | uint32 addr + uint32 data | 3s | 写入 32 位字 |
| 0x03 | Boot  | 无 | 3s | 跳转 APP |
| 0x04 | Info  | 无 | 1s | 查询 BL 版本/APP 状态 |
| 0x05 | LED   | 无 | 1s | LED 闪 3 次 (调试) |

### ACK 格式
```
[0xAA] [0x02] [STATUS] [CKSUM]
```
- STATUS: 0=成功, 1=失败, 0xFE=未知命令, 0xFF=数据错误

### Info 响应 (CMD 0x04 → ACK 0x84)
```
[0xAA][0x0A][0x84][VER][00][FLASH_SIZE_H][FLASH_SIZE_L][00][00][APP_VALID][CKSUM]
```

## 编译方法

```powershell
# 1. 编译 BL (Keil ARMCC v5)
cd targets/AMKN8639/bootloader
powershell -File build.ps1

# 2. 编译 APP (Keil UV4)
"D:\keil5\UV4\UV4.exe" -j0 -b AMKN8639.uvprojx

# 3. 打包 SX.bin = BL(128KB pad) + APP
powershell -File build.ps1  # 自动完成
```

## 烧录方法

```powershell
# STM32CubeProgrammer CLI (ST-Link)
# 1. 全片擦除
STM32_Programmer_CLI.exe -c port=SWD freq=4000 -e all

# 2. 烧录 SX.bin + 复位运行
STM32_Programmer_CLI.exe -c port=SWD freq=4000 -w output/bin/AMKN8639_SX.bin 0x08000000 --skipErase -rst
```

## OTA 升级流程 (已验证)

### 流程 A: PC 直接升级
```
1. 板子上电 → BL 等待 10s
2. python scripts/ota_host.py COM46 flash app.bin
   - 自动发 0xAA 进入 XCP
   - Erase → Write → Boot
```

### 流程 B: APP 触发升级 (RK3568 远程)
```
1. RK3568 发 "AT+OTA\r\n" → STM32 UART1
2. APP 处理 → 写 BKP0R=0x424C4F54 → Reset
3. BL 启动, 检测到魔法值 → 进入 XCP 模式
4. RK3568 通过 XCP 协议烧录新固件
5. 发 Boot 命令启动新 APP
```

### 流程 C: RK3568 完整自动化
```bash
# 使用 rk_ota_agent.py
python3 rk_ota_agent.py --uart /dev/ttyS1 --fw http://server/firmware.bin
python3 rk_ota_agent.py --uart /dev/ttyS1 --fw local.bin
```

## BL 自升级流程 (待验证)
```
1. RK3568 将新 BL 先烧到临时区 (0x081E0000)
2. 发送 BL upgrade magic (0x424C5550) → Reset
3. BL 启动 → 检测到 Upgrade magic → 复制自身到 ITCM RAM
4. 在 ITCM 中执行: 擦除 Sector 0 → 从 0x081E0000 搬移 128KB → 复位
```

## 注意事项

1. **不要动硬件 GPIO**: 继电器、电机等由 APP 控制，BL 只操作 PB7(LED) 和 UART1
2. **BL 独立编译**: 使用 Keil ARMCC v5，不依赖 HAL 库 (`--no_startup --no_scanlib`)
3. **BKP 寄存器**: 依赖 VBAT 供电（电池），如无电池则复位后丢失
4. **Flash 写保护**: BL 写完固件后锁定 Flash (flash_lock)
5. **串口**: COM46 (ST-Link VCP), 115200-8-N-1
6. **ST-Link SN**: E1007200D0D2139393740544
7. **APP 路径**: `output/bin/AMKN8639_APP.bin` (Keil UV4 编译)
8. **SX.bin 路径**: `output/bin/AMKN8639_SX.bin`
9. **APP 有效检查**: SP 0x20000000~0x24080000 或 0x00000000~0x00010000, PC 0x08020000~0x08200000 且 bit0=1
