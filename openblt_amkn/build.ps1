# AMKN8639 OpenBLT Bootloader Build Script
param(
    [string]$KeilPath = "D:\keil5"
)

$ErrorActionPreference = "Stop"
$BLDir = $PSScriptRoot + "\Boot"
$ArmCC   = Join-Path $KeilPath "ARM\ARMCC\bin\armcc.exe"
$ArmLink = Join-Path $KeilPath "ARM\ARMCC\bin\armlink.exe"
$FromElf = Join-Path $KeilPath "ARM\ARMCC\bin\fromelf.exe"

$Sct     = Join-Path $BLDir "amkn8639_bl.sct"
$ObjDir  = Join-Path $BLDir "obj"
$Elf     = Join-Path $BLDir "openblt_amkn.axf"
$Bin     = Join-Path $BLDir "openblt_amkn.bin"

if (-not (Test-Path $ObjDir)) { New-Item -ItemType Directory $ObjDir | Out-Null }

$CommonFlags = @(
    "--c99", "--cpu=Cortex-M7", "--fpu=FPv5_D16",
    "-O1",
    "-I$BLDir",
    "-I$BLDir\lib",
    "-I$BLDir\lib\CMSIS\Include",
    "-I$BLDir\lib\CMSIS\Device\ST\STM32H7xx\Include",
    "-I$BLDir\lib\STM32H7xx_HAL_Driver\Inc",
    "-DSTM32H743xx", "-DUSE_FULL_LL_DRIVER",
    "-DHSE_VALUE=25000000",
    "--diag_suppress=1,66,68,111,177,550,993,1296"
)

$Sources = @(
    "main.c", "boot.c", "asserts.c", "backdoor.c", "com.c",
    "cop.c", "file.c", "infotable.c", "net.c", "xcp.c",
    "cpu.c", "cpu_comp.c", "flash.c", "nvm.c", "rs232.c",     "lib\system_stm32h7xx.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_cortex.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_flash.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_flash_ex.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_gpio.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_rcc.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_rcc_ex.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_pwr.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_pwr_ex.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_dma.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_dma_ex.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_mdma.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_tim.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_tim_ex.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_exti.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_ll_usart.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_ll_gpio.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_ll_rcc.c",
    "lib\STM32H7xx_HAL_Driver\Src\stm32h7xx_ll_utils.c"
)

$ASMSources = @("startup_stm32h743xx.s")

Write-Host "=== AMKN8639 OpenBLT Build ===" -ForegroundColor Cyan

# Step 1: Compile
Write-Host "[1/3] Compiling $($Sources.Count) C files..." -ForegroundColor Yellow
$ObjFiles = @()
foreach ($src in $Sources) {
    $name = [System.IO.Path]::GetFileNameWithoutExtension($src)
    $obj = Join-Path $ObjDir "$name.o"
    $fullSrc = Join-Path $BLDir $src
    Write-Host "  $src"
    & $ArmCC @CommonFlags -c $fullSrc -o $obj
    if ($LASTEXITCODE -ne 0) { Write-Error "FAILED: $src"; exit 1 }
    $ObjFiles += $obj
}
Write-Host "  [OK] All compiled" -ForegroundColor Green

# Step 2: Assemble startup
Write-Host "[2/3] Assembling startup..." -ForegroundColor Yellow
foreach ($src in $ASMSources) {
    $name = [System.IO.Path]::GetFileNameWithoutExtension($src)
    $obj = Join-Path $ObjDir "$name.o"
    $fullSrc = Join-Path $BLDir $src
    & $ArmCC @CommonFlags -c $fullSrc -o $obj
    if ($LASTEXITCODE -ne 0) { Write-Error "FAILED: $src"; exit 1 }
    $ObjFiles += $obj
}

# Step 3: Link
Write-Host "[3/3] Linking..." -ForegroundColor Yellow
& $ArmLink "--cpu=Cortex-M7" "--fpu=FPv5_D16" "--entry=Reset_Handler" "--scatter=$Sct" "--strict" "--no_startup" "--no_scanlib" "-o" $Elf $ObjFiles
if ($LASTEXITCODE -ne 0) { Write-Error "Link failed"; exit 1 }

& $FromElf "--bin" $Elf "-o" $Bin
$blSize = (Get-Item $Bin).Length
Write-Host "=== Build Complete ===" -ForegroundColor Cyan
Write-Host "  openblt_amkn.bin : $blSize bytes" -ForegroundColor Green