$z88dkRoot = "C:\Users\ricky\Downloads\z88dk-win32-2.4\z88dk"
$outputDir = Join-Path $PSScriptRoot "build"

if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

$env:PATH = "$z88dkRoot\bin;$env:PATH"
$env:ZCCCFG = "$z88dkRoot\lib\config"

& "$z88dkRoot\bin\zcc.exe" +mz -O3 -create-app -o "$outputDir\char_browser" "src\main.c" "src\char_wrapper.c"
exit $LASTEXITCODE
