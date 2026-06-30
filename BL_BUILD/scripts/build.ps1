param([string]$AppBin = "..\output\bin\AMKN8639_APP.bin", [string]$SxOut, [switch]$BlOnly)
$ScriptDir = Split-Path $MyInvocation.MyCommand.Path
$BLDir = Split-Path $ScriptDir -Parent
$ArmCC = "D:\keil5\ARM\ARMCC\bin\armcc.exe"
$ArmLink = "D:\keil5\ARM\ARMCC\bin\armlink.exe"
$FromElf = "D:\keil5\ARM\ARMCC\bin\fromelf.exe"
$Src = Join-Path $BLDir "src\bootloader.c"
$Obj = Join-Path $BLDir "output\bootloader.o"
$Elf = Join-Path $BLDir "output\bootloader.axf"
$Bin = Join-Path $BLDir "output\bootloader.bin"
$Sct = Join-Path $BLDir "src\bootloader.sct"
$PadBin = Join-Path $BLDir "output\bootloader_pad.bin"
if ($SxOut) { $SxBin = $SxOut } else { $SxBin = Join-Path $BLDir "output\AMKN8639_SX.bin" }

Write-Host "BLDir: $BLDir"
Write-Host "Src: $Src"
Write-Host "AppBin: $AppBin"

if (-not (Test-Path $Src)) { Write-Error "BL source not found: $Src"; exit 1 }
if (-not (Test-Path (Join-Path $BLDir "output"))) { New-Item -ItemType Directory -Force -Path (Join-Path $BLDir "output") | Out-Null }

Write-Host "=== Build Bootloader ===" -ForegroundColor Cyan
& $ArmCC --c99 --cpu=Cortex-M7 --fpu=FPv5_D16 -O2 -c $Src -o $Obj --diag_suppress=1,66,68,111,177,550,993,1296
if ($LASTEXITCODE -ne 0) { Write-Error "armcc failed"; exit 1 }
Write-Host "  armcc OK" -ForegroundColor Green

& $ArmLink --cpu=Cortex-M7 --fpu=FPv5_D16 --scatter=$Sct --strict --no_startup -o $Elf $Obj
if ($LASTEXITCODE -ne 0) { Write-Error "armlink failed"; exit 1 }
Write-Host "  armlink OK" -ForegroundColor Green

& $FromElf --bin $Elf -o $Bin
if ($LASTEXITCODE -ne 0) { Write-Error "fromelf failed"; exit 1 }
$blSize = (Get-Item $Bin).Length
Write-Host "  bootloader.bin: $blSize bytes" -ForegroundColor Green

$padSize = 128 * 1024
if ($blSize -gt $padSize) { Write-Error "Too big: $blSize > $padSize"; exit 1 }
$padData = [byte[]]::new($padSize)
[System.IO.File]::ReadAllBytes($Bin).CopyTo($padData, 0)
[System.IO.File]::WriteAllBytes($PadBin, $padData)

if ($BlOnly) {
    Write-Host "  BL only mode - skip packaging" -ForegroundColor Yellow
    Write-Host "  bootloader.bin: $blSize bytes" -ForegroundColor Green
    Write-Host "=== DONE ===" -ForegroundColor Cyan
    exit 0
}

$appPath = $AppBin
if (-not (Test-Path $appPath)) {
    $appPath = Join-Path $BLDir $AppBin
}
if (-not (Test-Path $appPath)) {
    $appPath = Join-Path $BLDir "..\output\bin\AMKN8639_APP.bin"
}
if (-not (Test-Path $appPath)) { Write-Error "APP.bin missing: $appPath"; exit 1 }
Write-Host "  Using APP: $appPath"

$appData = [System.IO.File]::ReadAllBytes($appPath)
$sxData = $padData + $appData
[System.IO.File]::WriteAllBytes($SxBin, $sxData)
Write-Host "  SX.bin: $($sxData.Length) bytes ($([math]::Round($sxData.Length/1024,1)) KB)" -ForegroundColor Green
Write-Host "=== DONE ===" -ForegroundColor Cyan

