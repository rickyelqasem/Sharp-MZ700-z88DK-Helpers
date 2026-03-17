# Character Rom Wrapper

Small Sharp MZ-700 character wrapper for z88dk, with a demo app.

## Build

Run:

```powershell
.\build.ps1
```

This uses the local z88dk install at:

`C:\Users\ricky\Downloads\z88dk-win32-2.4\z88dk`

## API

Include [src/char_wrapper.h](c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/char_wrapper.h) and call:

```c
charProcess(CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK, 10, 5);
```

Parameters:

- `charset`: `CHARSET_ASCII`, `CHARSET_DISPLAY_A`, or `CHARSET_DISPLAY_B`
- `code`: character code
- `foreground`: 0..7 foreground colour
- `background`: 0..7 background colour
- `x`, `y`: 40x25 screen coordinates

Helper functions:

- `mzClearScreen(fg, bg)`
- `mzWriteUiText(x, y, text, fg, bg)`
- `charArrayProcces(charset, codes, fg, bg, x, y)`
- `sprite4x4(tileArray, x, y)`
- `sprite8x8(tileArray, x, y)`

Tile type:

```c
typedef struct MzTileCell {
    uint8_t charset;
    uint8_t code;
    uint8_t foreground;
    uint8_t background;
} MzTileCell;
```

Example:

```c
static const uint8_t title_codes[] = { 0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x00 };
charArrayProcces(CHARSET_ASCII, title_codes, MZ_COLOUR_WHITE, MZ_COLOUR_BLUE, 4, 2);
```

`charArrayProcces()` expects a `0x00` terminator at the end of the code array.

`sprite4x4()` expects an array of 16 `MzTileCell` entries laid out row-by-row for a 4x4 tile.

`sprite8x8()` expects an array of 64 `MzTileCell` entries laid out row-by-row for an 8x8 tile.

## Demo Controls

- `1`: ASCII set
- `2`: display set A
- `3`: display set B
- `A`: next code
- `Z`: previous code
- `N`: next foreground colour
- `M`: next background colour
- `Q`: quit

## Notes

- `CHARSET_DISPLAY_B` sets `D7` in colour RAM to select the second display character set.
- Foreground uses `D6-D4`, background uses `D2-D0`, following the MZ-700 colour VRAM layout.
