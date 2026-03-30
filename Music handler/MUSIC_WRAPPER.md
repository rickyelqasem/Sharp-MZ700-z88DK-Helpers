# MZ-700 Music Wrapper

This project now contains a reusable non-blocking music wrapper for the Sharp MZ-700.

The wrapper is built on top of the low-level `music_engine` driver and is intended to be copied into other z88dk MZ-700 projects.

## Files To Copy Into Another Project

Copy these files:

- `src/mz_music.h`
- `src/mz_music.c`
- `src/music_engine.h`
- `src/music_engine.c`

If you want MIDI playback as well, also copy:

- `src/generated_song.h`
- the converted song source, for example `build/generated_song.c`
- `tools/convert-midi.ps1` if you want to generate new song data from `.mid` files

## What The Wrapper Does

The wrapper gives you two high-level play paths:

1. `mz_music_play_melody(...)`
   Use this when you want to pass in a BPM value, a notes-per-bar value, and a plain C array of notes.

2. `mz_music_play_song(...)`
   Use this when you want to play a MIDI file that has already been converted into C data.

Both play paths are non-blocking. Music keeps advancing only when you call `mz_music_update()` from your main loop, so your game or demo can continue updating the screen, reading input, animating sprites, and so on.

## Important Rule For Multitasking

The music player is cooperative, not interrupt-driven.

That means:

- Call `mz_music_init()` once at startup.
- Call `mz_music_update()` very frequently in your main loop.
- Do not block for long periods without calling `mz_music_update()`.

If your code spends too long drawing text or waiting inside a busy loop, playback will slow down.

## Public API

### Startup And Shutdown

```c
void mz_music_init(void);
void mz_music_shutdown(void);
void mz_music_update(void);
void mz_music_stop(void);
```

### Play A Simple Melody From Notes

```c
unsigned char mz_music_play_melody(
    unsigned int bpm,
    unsigned char notes_per_bar,
    const note_id_t *notes,
    unsigned int note_count,
    unsigned char loop);
```

Parameters:

- `bpm`
  Tempo in beats per minute.
- `notes_per_bar`
  How many equally-sized notes fit in one 4/4 bar.
  Example: `16` means each note is a 16th note.
- `notes`
  Pointer to an array of notes.
- `note_count`
  Number of notes in that array.
- `loop`
  `0` for one-shot playback, `1` to loop forever.

Return value:

- `1` on success
- `0` on invalid input

### Play A Converted MIDI Song

```c
struct mz_music_song {
    unsigned int bpm;
    const struct melody_step *steps;
    unsigned int step_count;
};

unsigned char mz_music_play_song(const struct mz_music_song *song, unsigned char loop);
```

Parameters:

- `song`
  A converted song asset.
- `loop`
  `0` for one-shot playback, `1` to loop forever.

## Note Macros And Pitch Names

The wrapper exposes reusable note macros:

```c
#define MZ_MUSIC_NOTE(octave, pitch) NOTE_MAKE((octave), (pitch))
#define MZ_MUSIC_REST NOTE_REST
```

Pitch constants:

```c
MZ_PITCH_C
MZ_PITCH_CS
MZ_PITCH_D
MZ_PITCH_DS
MZ_PITCH_E
MZ_PITCH_F
MZ_PITCH_FS
MZ_PITCH_G
MZ_PITCH_GS
MZ_PITCH_A
MZ_PITCH_AS
MZ_PITCH_B
```

Example:

```c
MZ_MUSIC_NOTE(4, MZ_PITCH_C)
MZ_MUSIC_NOTE(4, MZ_PITCH_CS)
MZ_MUSIC_NOTE(5, MZ_PITCH_A)
```

## Fastest Way To Play Inline Notes

If you want a call style close to:

```c
playMelody(130, 16, [C, D, C#, C, D, E, A]);
```

use the helper macro:

```c
MZ_MUSIC_PLAY_NOTES(130, 16, 0,
    MZ_MUSIC_NOTE(4, MZ_PITCH_C),
    MZ_MUSIC_NOTE(4, MZ_PITCH_D),
    MZ_MUSIC_NOTE(4, MZ_PITCH_CS),
    MZ_MUSIC_NOTE(4, MZ_PITCH_C),
    MZ_MUSIC_NOTE(4, MZ_PITCH_D),
    MZ_MUSIC_NOTE(4, MZ_PITCH_E),
    MZ_MUSIC_NOTE(4, MZ_PITCH_A));
```

That macro expands into a temporary C array and calls `mz_music_play_melody(...)` for you.

## Example: Manual Melody Array

```c
#include "mz_music.h"

static const note_id_t intro[] = {
    MZ_MUSIC_NOTE(4, MZ_PITCH_C),
    MZ_MUSIC_NOTE(4, MZ_PITCH_D),
    MZ_MUSIC_NOTE(4, MZ_PITCH_E),
    MZ_MUSIC_NOTE(4, MZ_PITCH_G)
};

void main(void)
{
    unsigned char running = 1;

    mz_music_init();
    mz_music_play_melody(130, 16, intro, sizeof(intro) / sizeof(intro[0]), 1);

    while (running) {
        mz_music_update();

        /* your input, game logic, and screen updates go here */
    }

    mz_music_shutdown();
}
```

## Example: Converted MIDI Song

```c
#include "generated_song.h"
#include "mz_music.h"

void main(void)
{
    mz_music_init();

    mz_music_play_song(&generated_song, 0);

    while (1) {
        mz_music_update();

        /* your other program work goes here */
    }
}
```

## MIDI Workflow

Do not try to load and parse a raw MIDI file at runtime on the MZ-700 side.

The intended workflow is:

1. Convert the `.mid` file on the PC using `tools/convert-midi.ps1`
2. Compile the generated C source into your MZ-700 program
3. Call `mz_music_play_song(...)`

### Is BPM Auto-Detected For MIDI?

Yes.

The converter reads the MIDI tempo meta event and writes the correct BPM into the generated `struct mz_music_song`.

### Is Notes-Per-Bar Auto-Detected For MIDI?

Effectively yes.

You do not pass `notes_per_bar` to `mz_music_play_song(...)`.
The converter already turns MIDI timing into playback step lengths, so the runtime wrapper does not need an extra bar-division parameter.

### Practical Meaning

- `mz_music_play_melody(...)`
  You provide `bpm` and `notes_per_bar`.
- `mz_music_play_song(...)`
  BPM and note lengths are already embedded in the converted asset.

## Limits And Rules

- `mz_music_play_melody(...)` currently supports up to `256` notes per call.
- `notes_per_bar` currently supports `1` to `16`.
- `16` is the finest supported grid for inline melodies because the engine stores durations in 16th-note units.
- MIDI songs are not limited by the wrapper scratch buffer because they use generated song data directly.

## Error Handling

You can inspect the last wrapper error:

```c
unsigned char mz_music_get_last_error(void);
```

Values:

- `MZ_MUSIC_ERROR_NONE`
- `MZ_MUSIC_ERROR_BAD_NOTES_PER_BAR`
- `MZ_MUSIC_ERROR_TOO_MANY_NOTES`
- `MZ_MUSIC_ERROR_BAD_SONG`

Useful case:

```c
if (!mz_music_play_melody(...)) {
    if (mz_music_get_last_error() == MZ_MUSIC_ERROR_BAD_NOTES_PER_BAR) {
        /* handle bad rhythm grid */
    }
}
```

## Optional Runtime Helpers

These can be useful for status displays:

```c
unsigned char mz_music_is_playing(void);
unsigned char mz_music_get_mode(void);
note_id_t mz_music_get_current_note(void);
unsigned int mz_music_get_bpm(void);
unsigned int mz_music_get_step_index(void);
unsigned int mz_music_get_step_count(void);
unsigned char mz_music_get_last_notes_per_bar(void);
const char *mz_music_note_name(note_id_t note);
```

## Preview Notes From A Keyboard

For keyboard-controlled preview:

```c
void mz_music_preview_note(note_id_t note);
void mz_music_preview_off(void);
```

These are useful for on-screen piano or tracker-style editors.

## Build Integration

Add these sources to your z88dk build:

```text
src/main.c
src/mz_music.c
src/music_engine.c
build/generated_song.c   <- only if using MIDI assets
```

Add include paths:

```text
-Isrc
-Ibuild
```

## Recommended Usage Pattern

Use this order:

1. Initialize the wrapper once.
2. Start a melody or MIDI song.
3. Call `mz_music_update()` every loop.
4. Continue your game logic and drawing normally.
5. Stop or replace the song whenever needed.
6. Shut the wrapper down before exit.

## Summary

Use `mz_music_play_melody(...)` when you want a simple C array of notes and a uniform bar grid.

Use `mz_music_play_song(...)` when you want a converted MIDI tune with BPM and note timing already embedded.

For MIDI playback, BPM and timing are auto-detected during conversion, so the user does not need to pass them separately at runtime.
