param([string]$AppBin = "..\output\bin\AMKN8639_APP.bin")
$BLDir = Split-Path $MyInvocation.MyCommand.Path
$ArmCC = "D:\keil5\ARM\ARMCC\bin\armcc.exe"
$ArmLink = "D:\keil5\ARM\ARMCC\bin\armlink.exe"
$FromElf = "D:\keil5\ARM\ARMCC\bin\fromelf.exe"
$Src = "$BLDir\bootloader.c"
$Obj = "$BLDir\bootloader.o"
$Elf = "$BLDir\bootloader.axf"
$Bin = "$BLDir\bootloader.bin"
$Sct = "$BLDir\bootloader.sct"
$PadBin = "$BLDir\bootloader_pad.bin"
$SxBin = "..\output\bin\AMKN8639_SX.bin"
Write-Host "=== Build Bootloader ===" -Fore Cyan
& $ArmCC --c99 --cpu=Cortex-M7 --fpu=FPv5_D16 -O2 -c $Src -o $Obj --diag_suppress=1,66,68,111,177,550,993,1296
if ($LASTEXITCODE -ne 0) { Write-Error "armcc failed"; exit 1 }
Write-Host "  armcc OK" -Fore Green
& $ArmLink --cpu=Cortex-M7 --fpu=FPv5_D16 --scatter=$Sct --strict --no_startup --no_scanlib -o $Elf $Obj
if ($LASTEXITCODE -ne 0) { Write-Error "armlink failed"; exit 1 }
Write-Host "  armlink OK" -Fore Green
& $FromElf --bin $Elf -o $Bin
if ($LASTEXITCODE -ne 0) { Write-Error "fromelf failed"; exit 1 }
$blSize = (Get-Item $Bin).Length
Write-Host "  bootloader.bin: $blSize bytes" -Fore Green
$padSize = 128 * 1024
if ($blSize -gt $padSize) { Write-Error "Too big: $blSize > $padSize"; exit 1 }
$padData = [byte[]]::new($padSize)
[System.IO.File]::ReadAllBytes($Bin).CopyTo($padData, 0)
[System.IO.File]::WriteAllBytes($PadBin, $padData)
$appPath = Join-Path $BLDir $AppBin
if (-not (Test-Path $appPath)) { Write-Error "APP.bin missing: $appPath"; exit 1 }
$appData = [System.IO.File]::ReadAllBytes($appPath)
$sxData = $padData + $appData
$sxPath = Join-Path $BLDir $SxBin
[System.IO.File]::WriteAllBytes($sxPath, $sxData)
Write-Host "  SX.bin: $($sxData.Length) bytes" -Fore Green
Write-Host "=== DONE ===" -Fore Cyan