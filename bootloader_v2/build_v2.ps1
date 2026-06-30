# AMKN8639 Bootloader V2 Build Script
# Compiles bootloader_v2.c with ARMCC (Keil MDK)
# Then merges BL + signed APP into full SX.bin
param(
    [string]$AppBin = "..\output\bin\AMKN8639_APP_SIGNED.bin",
    [string]$KeilPath = "D:\keil5"
)

$ErrorActionPreference = "Stop"
$BLDir = Split-Path $MyInvocation.MyCommand.Path
$ArmCC = Join-Path $KeilPath "ARM\ARMCC\bin\armcc.exe"
$ArmLink = Join-Path $KeilPath "ARM\ARMCC\bin\armlink.exe"
$FromElf = Join-Path $KeilPath "ARM\ARMCC\bin\fromelf.exe"

$Src    = Join-Path $BLDir "bootloader_v2.c"
$Obj    = Join-Path $BLDir "bootloader_v2.o"
$Elf    = Join-Path $BLDir "bootloader_v2.axf"
$Bin    = Join-Path $BLDir "bootloader_v2.bin"
$Sct    = Join-Path $BLDir "bootloader_v2.sct"
$PadBin = Join-Path $BLDir "bootloader_v2_pad.bin"
$SxBin  = Join-Path $BLDir "AMKN8639_SX.bin"

Write-Host "=== AMKN8639 Bootloader V2 Build ===" -ForegroundColor Cyan

# Step 1: Compile
Write-Host "[1/4] Compiling bootloader_v2.c..." -ForegroundColor Yellow
$ArmCCArgs = @(
    "--c99", "--cpu=Cortex-M7", "--fpu=FPv5_D16",
    "-O2", "-c", $Src, "-o", $Obj,
    "--diag_suppress=1,66,68,111,177,550,993,1296"
)
& $ArmCC @ArmCCArgs
if ($LASTEXITCODE -ne 0) { Write-Error "armcc failed"; exit 1 }
Write-Host "  [OK] armcc" -ForegroundColor Green

# Step 2: Link
Write-Host "[2/4] Linking..." -ForegroundColor Yellow
& $ArmLink "--cpu=Cortex-M7" "--fpu=FPv5_D16" "--scatter=$Sct" "--strict" "--no_startup" "--no_scanlib" "-o" $Elf $Obj
if ($LASTEXITCODE -ne 0) { Write-Error "armlink failed"; exit 1 }
Write-Host "  [OK] armlink" -ForegroundColor Green

# Step 3: Extract binary
Write-Host "[3/4] Extracting binary..." -ForegroundColor Yellow
& $FromElf "--bin" $Elf "-o" $Bin
if ($LASTEXITCODE -ne 0) { Write-Error "fromelf failed"; exit 1 }
$blSize = (Get-Item $Bin).Length
Write-Host "  [OK] bootloader_v2.bin: $blSize bytes" -ForegroundColor Green

# Step 4: Pad + Merge
Write-Host "[4/4] Merging BL + APP..." -ForegroundColor Yellow
$padSize = 128 * 1024  # 128KB
if ($blSize -gt $padSize) { Write-Error "BL too big: $blSize > $padSize"; exit 1 }

# Pad bootloader to 128KB
$padData = [byte[]]::new($padSize)
[System.IO.File]::ReadAllBytes($Bin).CopyTo($padData, 0)
[System.IO.File]::WriteAllBytes($PadBin, $padData)
Write-Host "  [OK] bootloader_v2_pad.bin: $padSize bytes" -ForegroundColor Green

# Merge with APP
$appPath = Join-Path $BLDir $AppBin
if (-not (Test-Path $appPath)) {
    Write-Warning "APP signed binary not found: $appPath"
    Write-Host "  Skipping SX.bin generation. Sign APP first with imgtool_keil.py" -ForegroundColor Yellow
} else {
    $appData = [System.IO.File]::ReadAllBytes($appPath)
    $sxData = $padData + $appData
    [System.IO.File]::WriteAllBytes($SxBin, $sxData)
    Write-Host "  [OK] AMKN8639_SX.bin: $($sxData.Length) bytes" -ForegroundColor Green
}

Write-Host "=== DONE ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor White
Write-Host "  1. Sign APP binary:  python scripts\imgtool_keil.py sign"
Write-Host "  2. Re-run build:     .\build_v2.ps1"
Write-Host "  3. Flash SX.bin:     ST-LINK_CLI -P AMKN8639_SX.bin 0x08000000"
