# AMKN8639 BL V6 Build
param(
    [string]$KeilPath = "D:\keil5"
)

$ErrorActionPreference = "Stop"
$BLDir = Split-Path $MyInvocation.MyCommand.Path
$ArmCC = Join-Path $KeilPath "ARM\ARMCC\bin\armcc.exe"
$ArmLink = Join-Path $KeilPath "ARM\ARMCC\bin\armlink.exe"
$FromElf = Join-Path $KeilPath "ARM\ARMCC\bin\fromelf.exe"

$Src    = Join-Path $BLDir "bl_v6.c"
$Obj    = Join-Path $BLDir "bl_v6.o"
$Elf    = Join-Path $BLDir "bl_v6.axf"
$Bin    = Join-Path $BLDir "bl_v6.bin"
$PadBin = Join-Path $BLDir "bl_v6_pad.bin"
$Sct    = Join-Path $BLDir "bootloader_v2.sct"

Write-Host "=== AMKN8639 BL V6 Build ===" -ForegroundColor Cyan

# Step 1: Compile
Write-Host "[1/3] Compiling bl_v6.c..." -ForegroundColor Yellow
$ArmCCArgs = @(
    "--c99", "--cpu=Cortex-M7", "--fpu=FPv5_D16",
    "-O2", "-c", $Src, "-o", $Obj,
    "--diag_suppress=1,66,68,111,177,550,993,1296,167,174,188"
)
& $ArmCC @ArmCCArgs 2>&1
if ($LASTEXITCODE -ne 0) { Write-Error "armcc failed"; exit 1 }
Write-Host "  [OK] armcc" -ForegroundColor Green

# Step 2: Link
Write-Host "[2/3] Linking..." -ForegroundColor Yellow
& $ArmLink "--cpu=Cortex-M7" "--fpu=FPv5_D16" "--scatter=$Sct" "--strict" "--no_startup" "--no_scanlib" "-o" $Elf $Obj
if ($LASTEXITCODE -ne 0) { Write-Error "armlink failed"; exit 1 }
Write-Host "  [OK] armlink" -ForegroundColor Green

# Step 3: Extract binary
Write-Host "[3/3] Extracting binary..." -ForegroundColor Yellow
& $FromElf "--bin" $Elf "-o" $Bin
if ($LASTEXITCODE -ne 0) { Write-Error "fromelf failed"; exit 1 }
$blSize = (Get-Item $Bin).Length
Write-Host "  [OK] bl_v6.bin: $blSize bytes" -ForegroundColor Green

# Pad to 128KB
$padSize = 128 * 1024
if ($blSize -gt $padSize) { Write-Error "BL too big: $blSize > $padSize"; exit 1 }
$padData = [byte[]]::new($padSize)
[System.IO.File]::ReadAllBytes($Bin).CopyTo($padData, 0)
[System.IO.File]::WriteAllBytes($PadBin, $padData)
Write-Host "  [OK] bl_v6_pad.bin: $padSize bytes" -ForegroundColor Green

Write-Host "=== DONE ===" -ForegroundColor Cyan