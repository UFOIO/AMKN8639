# AMKN8639 BL V10 Build - QSPI RCC Reset + HAL-style
param()

$ErrorActionPreference = "Stop"
$BLDir = Split-Path $MyInvocation.MyCommand.Path
$KeilPath = "D:\keil5"
$ArmCC = Join-Path $KeilPath "ARM\ARMCC\bin\armcc.exe"
$ArmLink = Join-Path $KeilPath "ARM\ARMCC\bin\armlink.exe"
$FromElf = Join-Path $KeilPath "ARM\ARMCC\bin\fromelf.exe"

$Src    = Join-Path $BLDir "bl_v10.c"
$Obj    = Join-Path $BLDir "bl_v10.o"
$Elf    = Join-Path $BLDir "bl_v10.axf"
$Bin    = Join-Path $BLDir "bl_v10.bin"
$PadBin = Join-Path $BLDir "bl_v10_pad.bin"
$Sct    = Join-Path $BLDir "bootloader_v2.sct"

Write-Host "=== AMKN8639 BL V10 Build ===" -ForegroundColor Cyan

# Step 1: Compile
Write-Host "[1/3] Compiling bl_v10.c..." -ForegroundColor Yellow
$ArmCCArgs = @(
    "--c99", "--cpu=Cortex-M7", "--fpu=FPv5_D16",
    "-O2", "-c", $Src, "-o", $Obj,
    "--diag_suppress=1,66,68,111,177,550,993,1296,167,174,188,186"
)
$ccOut = & $ArmCC @ArmCCArgs 2>&1
$ccExit = $LASTEXITCODE
if ($ccOut) { $ccOut | ForEach-Object { Write-Host $_ -ForegroundColor DarkGray } }
if (Test-Path $Obj) {
    Write-Host "  [OK] armcc ($((Get-Item $Obj).Length) bytes)" -ForegroundColor Green
} else {
    Write-Error "armcc failed"; exit 1
}

# Step 2: Link
Write-Host "[2/3] Linking..." -ForegroundColor Yellow
& $ArmLink "--cpu=Cortex-M7" "--fpu=FPv5_D16" "--scatter=$Sct" "--strict" "--no_startup" "--no_scanlib" "-o" $Elf $Obj 2>&1
if ($LASTEXITCODE -ne 0) { Write-Error "armlink failed"; exit 1 }
Write-Host "  [OK] armlink" -ForegroundColor Green

# Step 3: Extract binary
Write-Host "[3/3] Extracting binary..." -ForegroundColor Yellow
& $FromElf "--bin" $Elf "-o" $Bin
if ($LASTEXITCODE -ne 0) { Write-Error "fromelf failed"; exit 1 }
$blSize = (Get-Item $Bin).Length
Write-Host "  [OK] bl_v10.bin: $blSize bytes" -ForegroundColor Green

# Pad to 128KB
$padSize = 128 * 1024
if ($blSize -gt $padSize) { Write-Error "BL too big: $blSize > $padSize"; exit 1 }
$padData = [byte[]]::new($padSize)
[System.IO.File]::ReadAllBytes($Bin).CopyTo($padData, 0)
[System.IO.File]::WriteAllBytes($PadBin, $padData)
Write-Host "  [OK] bl_v10_pad.bin: $padSize bytes" -ForegroundColor Green

Write-Host "=== DONE ===" -ForegroundColor Cyan
