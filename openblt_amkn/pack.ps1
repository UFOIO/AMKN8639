# AMKN8639 Pack: BL(openblt_amkn) + APP -> SX.bin
$BLBin = "$PSScriptRoot\Boot\openblt_amkn.bin"
$AppBin = "..\output\bin\AMKN8639_APP.bin"
$SxBin  = "..\output\bin\AMKN8639_SX.bin"
$BLPad  = "$PSScriptRoot\Boot\bl_pad.bin"

$ErrorActionPreference = "Stop"

if (-not (Test-Path $BLBin)) { Write-Error "BL missing: $BLBin"; exit 1 }
if (-not (Test-Path $AppBin)) { Write-Error "APP missing: $AppBin"; exit 1 }

$blSize = (Get-Item $BLBin).Length
$appSize = (Get-Item $AppBin).Length
Write-Host "=== AMKN8639 Pack ===" -Fore Cyan
Write-Host "  BL  : $BLBin ($blSize bytes)" -Fore Yellow
Write-Host "  APP : $AppBin ($appSize bytes)" -Fore Yellow

$padSize = 128 * 1024  # 128KB
if ($blSize -gt $padSize) { Write-Error "BL too big: $blSize > $padSize"; exit 1 }

$padData = [byte[]]::new($padSize)
[System.IO.File]::ReadAllBytes($BLBin).CopyTo($padData, 0)
[System.IO.File]::WriteAllBytes($BLPad, $padData)
Write-Host "  BL padded to 128KB" -Fore Green

$blData = [System.IO.File]::ReadAllBytes($BLPad)
$appData = [System.IO.File]::ReadAllBytes($AppBin)
$sxData = $blData + $appData
[System.IO.File]::WriteAllBytes($SxBin, $sxData)
Write-Host "  SX.bin: $($sxData.Length) bytes" -Fore Green
Write-Host "=== DONE ===" -Fore Cyan