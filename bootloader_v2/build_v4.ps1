# AMKN8639 BL V4 Build - Link ZQLY library
param()

$ErrorActionPreference = "Stop"
$BLDir = Split-Path $MyInvocation.MyCommand.Path
$KeilPath = "D:\keil5"
$ArmCC = Join-Path $KeilPath "ARM\ARMCC\bin\armcc.exe"
$ArmLink = Join-Path $KeilPath "ARM\ARMCC\bin\armlink.exe"
$FromElf = Join-Path $KeilPath "ARM\ARMCC\bin\fromelf.exe"
$Lib = Join-Path $BLDir "STM32H743XX.lib"

$Src    = Join-Path $BLDir "bl_v4.c"
$Obj    = Join-Path $BLDir "bl_v4.o"
$Elf    = Join-Path $BLDir "bl_v4.axf"
$Bin    = Join-Path $BLDir "bl_v4.bin"
$Sct    = Join-Path $BLDir "bootloader_v2.sct"

Write-Host "=== AMKN8639 BL V4 Build (ZQLY lib) ===" -ForegroundColor Cyan

# Step 1: Compile
Write-Host "[1/3] Compiling bl_v4.c..." -ForegroundColor Yellow
$ArmCCArgs = @(
    "--c99", "--cpu=Cortex-M7", "--fpu=FPv5_D16",
    "-O2", "-c", $Src, "-o", $Obj,
    "--diag_suppress=1,66,68,111,177,550,993,1296,167,174,188"
)
& $ArmCC @ArmCCArgs 2>&1
if ($LASTEXITCODE -ne 0) { Write-Error "armcc failed"; exit 1 }
Write-Host "  [OK] armcc" -ForegroundColor Green

# Step 2: Link with ZQLY library
Write-Host "[2/3] Linking with ZQLY library..." -ForegroundColor Yellow
$LinkArgs = @(
    "--cpu=Cortex-M7", "--fpu=FPv5_D16",
    "--scatter=$Sct", "--strict",
    "--no_startup", "--no_scanlib",
    "-o", $Elf, $Obj, $Lib
)
& $ArmLink @LinkArgs 2>&1
if ($LASTEXITCODE -ne 0) { Write-Error "armlink failed"; exit 1 }
Write-Host "  [OK] armlink" -ForegroundColor Green

# Step 3: Extract binary
Write-Host "[3/3] Extracting binary..." -ForegroundColor Yellow
& $FromElf "--bin" $Elf "-o" $Bin
if ($LASTEXITCODE -ne 0) { Write-Error "fromelf failed"; exit 1 }
$blSize = (Get-Item $Bin).Length
Write-Host "  [OK] bl_v4.bin: $blSize bytes" -ForegroundColor Green

Write-Host "=== DONE ===" -ForegroundColor Cyan
