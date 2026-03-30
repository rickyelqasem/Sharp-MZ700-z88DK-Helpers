param(
    [string]$MidiPath,
    [string]$SourceOut
)

$ErrorActionPreference = "Stop"

if (-not $MidiPath) {
    $MidiPath = Join-Path $PSScriptRoot "..\src\arp2.mid"
    if (-not (Test-Path $MidiPath)) {
        $MidiPath = Join-Path $PSScriptRoot "..\src\chords2.mid"
        if (-not (Test-Path $MidiPath)) {
            $MidiPath = Join-Path $PSScriptRoot "..\src\chords.mid"
            if (-not (Test-Path $MidiPath)) {
                $MidiPath = Join-Path $PSScriptRoot "..\src\arp.mid"
            }
        }
    }
}

if (-not $SourceOut) {
    $SourceOut = Join-Path $PSScriptRoot "..\build\generated_song.c"
}

function Read-U16([byte[]]$bytes, [int]$pos) {
    return [uint16](($bytes[$pos] * 256) + $bytes[$pos + 1])
}

function Read-U24([byte[]]$bytes, [int]$pos) {
    return [uint32](($bytes[$pos] * 65536) + ($bytes[$pos + 1] * 256) + $bytes[$pos + 2])
}

function Read-U32([byte[]]$bytes, [int]$pos) {
    return [uint32](($bytes[$pos] * 16777216) + ($bytes[$pos + 1] * 65536) + ($bytes[$pos + 2] * 256) + $bytes[$pos + 3])
}

function Read-Vlq([byte[]]$bytes, [ref]$pos) {
    $value = 0

    while ($true) {
        $byte = $bytes[$pos.Value]
        $pos.Value++
        $value = ($value -shl 7) -bor ($byte -band 0x7f)
        if (($byte -band 0x80) -eq 0) {
            return $value
        }
    }
}

function Add-Segment($segments, [int]$note, [int]$duration) {
    if ($duration -le 0) {
        return
    }

    if ($segments.Count -ne 0 -and $segments[$segments.Count - 1].Note -eq $note) {
        $segments[$segments.Count - 1].Duration += $duration
        return
    }

    $segments.Add([pscustomobject]@{
        Note = $note
        Duration = $duration
    }) > $null
}

function Get-ActiveNote($activeNotes) {
    if ($activeNotes.Count -eq 0) {
        return -1
    }

    return [int](($activeNotes.Keys | Measure-Object -Maximum).Maximum)
}

function Set-NoteOn($activeNotes, [int]$note) {
    if ($activeNotes.ContainsKey($note)) {
        $activeNotes[$note]++
    } else {
        $activeNotes[$note] = 1
    }
}

function Set-NoteOff($activeNotes, [int]$note) {
    if (-not $activeNotes.ContainsKey($note)) {
        return
    }

    if ($activeNotes[$note] -le 1) {
        $activeNotes.Remove($note)
    } else {
        $activeNotes[$note]--
    }
}

function Convert-MidiNoteToNoteId([int]$midiNote) {
    while ($midiNote -lt 12) {
        $midiNote += 12
    }

    while ($midiNote -gt 95) {
        $midiNote -= 12
    }

    return $midiNote - 12
}

function Get-GeneratedNoteName([int]$noteId) {
    $pitchNames = @("C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B")
    $octave = 1 + [int][Math]::Floor($noteId / 12.0)

    if ($noteId -lt 0) {
        return "REST"
    }

    return $pitchNames[$noteId % 12] + ([string]$octave)
}

$bytes = [System.IO.File]::ReadAllBytes($MidiPath)
if ([System.Text.Encoding]::ASCII.GetString($bytes, 0, 4) -ne "MThd") {
    throw "Unsupported MIDI header in $MidiPath"
}

$headerLength = Read-U32 $bytes 4
$format = Read-U16 $bytes 8
$trackCount = Read-U16 $bytes 10
$division = Read-U16 $bytes 12
$pos = 8 + $headerLength
$tracks = New-Object System.Collections.ArrayList
$tempoUsPerQuarter = $null

for ($trackIndex = 0; $trackIndex -lt $trackCount; $trackIndex++) {
    $tag = [System.Text.Encoding]::ASCII.GetString($bytes, $pos, 4)
    $trackLength = Read-U32 $bytes ($pos + 4)
    if ($tag -ne "MTrk") {
        throw "Invalid MIDI track tag at byte offset $pos"
    }

    $trackBytes = New-Object byte[] $trackLength
    [Array]::Copy($bytes, $pos + 8, $trackBytes, 0, $trackLength)
    $pos += 8 + $trackLength

    $trackEvents = New-Object System.Collections.ArrayList
    $trackPos = 0
    $absoluteTime = 0
    $runningStatus = -1
    $trackEndTime = 0
    $noteOnCount = 0

    while ($trackPos -lt $trackBytes.Length) {
        $trackPosRef = [ref]$trackPos
        $absoluteTime += Read-Vlq $trackBytes $trackPosRef
        $trackPos = $trackPosRef.Value
        $trackEndTime = $absoluteTime

        $status = $trackBytes[$trackPos]
        if ($status -lt 0x80) {
            if ($runningStatus -lt 0) {
                throw "Running status used before a MIDI status byte"
            }
            $status = $runningStatus
        } else {
            $trackPos++
            $runningStatus = $status
        }

        if ($status -eq 0xff) {
            $metaType = $trackBytes[$trackPos]
            $trackPos++
            $trackPosRef = [ref]$trackPos
            $metaLength = Read-Vlq $trackBytes $trackPosRef
            $trackPos = $trackPosRef.Value

            if ($metaType -eq 0x51 -and $metaLength -eq 3) {
                $tempoValue = Read-U24 $trackBytes $trackPos
                if ($null -eq $tempoUsPerQuarter) {
                    $tempoUsPerQuarter = $tempoValue
                }
            } elseif ($metaType -eq 0x2f) {
                $trackPos += $metaLength
                break
            }

            $trackPos += $metaLength
            continue
        }

        if ($status -eq 0xf0 -or $status -eq 0xf7) {
            $trackPosRef = [ref]$trackPos
            $sysExLength = Read-Vlq $trackBytes $trackPosRef
            $trackPos = $trackPosRef.Value + $sysExLength
            continue
        }

        $eventType = $status -band 0xf0

        if ($eventType -eq 0x80 -or $eventType -eq 0x90 -or $eventType -eq 0xa0 -or $eventType -eq 0xb0 -or $eventType -eq 0xe0) {
            $note = $trackBytes[$trackPos]
            $velocity = $trackBytes[$trackPos + 1]
            $trackPos += 2

            if ($eventType -eq 0x90 -and $velocity -gt 0) {
                $trackEvents.Add([pscustomobject]@{
                    Time = $absoluteTime
                    Kind = "note_on"
                    SortOrder = 1
                    Note = [int]$note
                }) > $null
                $noteOnCount++
            } elseif ($eventType -eq 0x80 -or ($eventType -eq 0x90 -and $velocity -eq 0)) {
                $trackEvents.Add([pscustomobject]@{
                    Time = $absoluteTime
                    Kind = "note_off"
                    SortOrder = 0
                    Note = [int]$note
                }) > $null
            }

            continue
        }

        if ($eventType -eq 0xc0 -or $eventType -eq 0xd0) {
            $trackPos++
            continue
        }

        throw ("Unsupported MIDI status 0x{0:X2}" -f $status)
    }

    $tracks.Add([pscustomobject]@{
        Events = $trackEvents
        EndTime = $trackEndTime
        NoteOnCount = $noteOnCount
    }) > $null
}

if ($null -eq $tempoUsPerQuarter) {
    $tempoUsPerQuarter = 500000
}

$noteTrack = $tracks | Sort-Object NoteOnCount -Descending | Select-Object -First 1
if ($null -eq $noteTrack -or $noteTrack.NoteOnCount -eq 0) {
    throw "No note data found in $MidiPath"
}

$steps = New-Object System.Collections.ArrayList
$noteOns = @($noteTrack.Events | Where-Object { $_.Kind -eq "note_on" } | Sort-Object Time)
if ($noteOns.Count -eq 0) {
    throw "No note-on events found in $MidiPath"
}

for ($noteIndex = 0; $noteIndex -lt $noteOns.Count; $noteIndex++) {
    $startTime = [int]$noteOns[$noteIndex].Time

    if ($noteIndex + 1 -lt $noteOns.Count) {
        $endTime = [int]$noteOns[$noteIndex + 1].Time
    } else {
        $matchingNoteOff = $noteTrack.Events |
            Where-Object { $_.Kind -eq "note_off" -and $_.Note -eq $noteOns[$noteIndex].Note -and [int]$_.Time -gt $startTime } |
            Sort-Object Time |
            Select-Object -First 1
        if ($null -ne $matchingNoteOff) {
            $endTime = [int]$matchingNoteOff.Time
        } else {
            $endTime = [int]$noteTrack.EndTime
        }
    }

    if ($endTime -le $startTime) {
        $endTime = $startTime + 1
    }

    $durationTicks = $endTime - $startTime
    $length16ths = [int][Math]::Round(($durationTicks * 4.0) / $division, 0, [System.MidpointRounding]::AwayFromZero)
    if ($durationTicks -gt 0 -and $length16ths -lt 1) {
        $length16ths = 1
    }

    if ($length16ths -le 0) {
        continue
    }

    $noteId = Convert-MidiNoteToNoteId ([int]$noteOns[$noteIndex].Note)

    while ($length16ths -gt 255) {
        $steps.Add([pscustomobject]@{
            Note = $noteId
            Length = 255
        }) > $null
        $length16ths -= 255
    }

    $steps.Add([pscustomobject]@{
        Note = $noteId
        Length = $length16ths
    }) > $null
}

if ($steps.Count -eq 0) {
    $steps.Add([pscustomobject]@{
        Note = -1
        Length = 1
    }) > $null
}

$bpm = [int][Math]::Round(60000000.0 / $tempoUsPerQuarter, 0, [System.MidpointRounding]::AwayFromZero)
if ($bpm -lt 1) {
    $bpm = 120
}

$sourceDir = Split-Path -Parent $SourceOut
New-Item -ItemType Directory -Force -Path $sourceDir | Out-Null

$sourceLines = New-Object System.Collections.Generic.List[string]
$sourceLines.Add(("/* Auto-generated from src/{0} by tools/convert-midi.ps1 */" -f (Split-Path -Leaf $MidiPath)))
$sourceLines.Add('#include "generated_song.h"')
$sourceLines.Add("")
$sourceLines.Add("static const struct melody_step generated_song_steps[] = {")
foreach ($step in $steps) {
    $sourceLines.Add(("    {{ {0}, {1} }}, /* {2} */" -f $step.Note, $step.Length, (Get-GeneratedNoteName ([int]$step.Note))))
}
$sourceLines.Add("};")
$sourceLines.Add("")
$sourceLines.Add("const struct mz_music_song generated_song = {")
$sourceLines.Add(("    {0}u," -f $bpm))
$sourceLines.Add("    generated_song_steps,")
$sourceLines.Add("    sizeof(generated_song_steps) / sizeof(generated_song_steps[0])")
$sourceLines.Add("};")
$sourceLines.Add("")

[System.IO.File]::WriteAllLines($SourceOut, $sourceLines, [System.Text.Encoding]::ASCII)

Write-Output ("Converted {0} to {1} melody steps at {2} BPM." -f (Split-Path -Leaf $MidiPath), $steps.Count, $bpm)
