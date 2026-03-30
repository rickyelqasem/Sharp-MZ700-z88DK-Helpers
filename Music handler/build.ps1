$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$sourceZ88dkRoot = "C:\Users\ricky\OneDrive\devwork\8bit games\sharp mz700\z88dk-win32-2.4\z88dk"
$z88dkRoot = Join-Path $projectRoot "z88dk_toolchain"

if (-not (Test-Path $z88dkRoot)) {
    New-Item -ItemType Junction -Path $z88dkRoot -Target $sourceZ88dkRoot | Out-Null
}

$buildDir = Join-Path $projectRoot "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$demoMidi = Join-Path $projectRoot "src\arp2.mid"
if (-not (Test-Path $demoMidi)) {
    $demoMidi = Join-Path $projectRoot "src\chords2.mid"
    if (-not (Test-Path $demoMidi)) {
        $demoMidi = Join-Path $projectRoot "src\chords.mid"
        if (-not (Test-Path $demoMidi)) {
            $demoMidi = Join-Path $projectRoot "src\arp.mid"
        }
    }
}

$converter = Join-Path $projectRoot "tools\convert-midi.ps1"
& $converter `
    -MidiPath $demoMidi `
    -SourceOut (Join-Path $buildDir "generated_song.c")

Push-Location $projectRoot
try {
    $projectRootShort = (cmd /c "for %I in (.) do @echo %~sI").Trim()
    $z88dkRootShort = Join-Path $projectRootShort "z88dk_toolchain"
    $zcc = Join-Path $z88dkRootShort "bin\zcc.exe"

    if (-not (Test-Path $zcc)) {
        throw "zcc.exe was not found at $zcc"
    }

    $env:ZCCCFG = Join-Path $z88dkRootShort "lib\config"
    $env:PATH = (Join-Path $z88dkRootShort "bin") + ";" + $env:PATH

    Pop-Location
    Push-Location $projectRootShort

    & $zcc `
        +mz `
        -clib=ansi `
        -subtype=wav `
        -create-app `
        -pragma-need=ansiterminal `
        -Isrc `
        -Ibuild `
        -O3 `
        -SO3 `
        "src\main.c" `
        "src\mz_music.c" `
        "src\music_engine.c" `
        "build\generated_song.c" `
        -o "build\mz700_music_demo"
} finally {
    Pop-Location
}
