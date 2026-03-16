$ErrorActionPreference = "Stop"

$z88dkRoot = "C:\Users\ricky\Downloads\z88dk-win32-2.4\z88dk"
$env:PATH = "$z88dkRoot\bin;$env:PATH"
$env:ZCCCFG = "$z88dkRoot\lib\config"

if (-not (Test-Path -LiteralPath "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

& zcc +mz -O3 -create-app -o build\joytest src\main.c src\mz700_screen.c src\mz700_keyboard.c src\mz700_joystick.c src\joy_read.asm
