# Character Wrapper Usage

This project provides a small reusable API for drawing Sharp MZ-700 characters directly into screen RAM with z88dk.

The wrapper is intended for projects that want to place characters on the MZ-700 screen without manually handling:

- display RAM at `$D000`
- colour RAM at `$D800`
- display set A versus display set B
- foreground and background colour packing into the MZ-700 colour byte
- basic ASCII-to-display conversion for common printable ASCII characters

The core API lives in [src/char_wrapper.h](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/char_wrapper.h) and [src/char_wrapper.c](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/char_wrapper.c).

## What The Wrapper Does

The wrapper writes one screen cell per character to:

- character VRAM at `$D000`
- colour VRAM at `$D800`

It supports three character-set modes:

- `CHARSET_ASCII`
- `CHARSET_DISPLAY_A`
- `CHARSET_DISPLAY_B`

It also packs the MZ-700 colour byte correctly:

- bit `D7`: set for display set B
- bits `D6-D4`: foreground colour
- bits `D2-D0`: background colour

## Files To Include

In another project, compile both of these files:

- [src/char_wrapper.h](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/char_wrapper.h)
- [src/char_wrapper.c](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/char_wrapper.c)

Then include the header in your own C file:

```c
#include "char_wrapper.h"
```

## Public Functions

The wrapper currently exposes these useful functions:

```c
void mzClearScreen(uint8_t foreground, uint8_t background);
void mzWriteUiText(uint8_t x, uint8_t y, const char *text, uint8_t foreground, uint8_t background);
void charProcess(uint8_t charset, uint8_t code, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y);
void charArrayProcces(uint8_t charset, const uint8_t *codes, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y);
void sprite4x4(const MzTileCell *tileArray, uint8_t x, uint8_t y);
void sprite8x8(const MzTileCell *tileArray, uint8_t x, uint8_t y);
```

The wrapper also exposes this tile-cell type:

```c
typedef struct MzTileCell {
    uint8_t charset;
    uint8_t code;
    uint8_t foreground;
    uint8_t background;
} MzTileCell;
```

## Character Set Values

The `charset` argument accepts these values:

- `CHARSET_ASCII` = `1`
- `CHARSET_DISPLAY_A` = `2`
- `CHARSET_DISPLAY_B` = `3`

### Meaning Of Each Mode

`CHARSET_ASCII`

- Use this when your code value is a normal ASCII byte such as `0x41` for `A`.
- The wrapper converts common printable ASCII into the display code needed by the MZ-700.

`CHARSET_DISPLAY_A`

- Use this when your code value is already a raw MZ-700 display-set-A code.
- The wrapper writes that code directly to screen RAM.

`CHARSET_DISPLAY_B`

- Use this when your code value is already a raw MZ-700 display-set-B code.
- The wrapper writes the code directly and sets bit `D7` in the colour byte so set B is selected.

## Colour Values

Foreground and background colour arguments both use values `0` to `7`.

Use the named constants from [src/char_wrapper.h](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/char_wrapper.h) rather than raw numbers when possible.

- `MZ_COLOUR_BLACK` = `0`
- `MZ_COLOUR_BLUE` = `1`
- `MZ_COLOUR_RED` = `2`
- `MZ_COLOUR_PURPLE` = `3`
- `MZ_COLOUR_GREEN` = `4`
- `MZ_COLOUR_LIGHT_BLUE` = `5`
- `MZ_COLOUR_YELLOW` = `6`
- `MZ_COLOUR_WHITE` = `7`

Example:

```c
charProcess(CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLUE, 10, 5);
```

This draws:

- ASCII `A`
- white foreground
- blue background
- at screen position `x=10`, `y=5`

## Screen Coordinates

The MZ-700 screen is treated as:

- width: `40`
- height: `25`

Coordinates are zero-based:

- top-left corner is `0,0`
- rightmost visible column is `39`
- bottom visible row is `24`

Example positions:

- `0,0` = top-left
- `10,5` = column 10, row 5
- `39,24` = bottom-right

If `x` or `y` is outside the valid screen range, the wrapper does nothing for that character.

## How To Use `charProcess()`

`charProcess()` draws one character at one screen position.

Signature:

```c
void charProcess(uint8_t charset, uint8_t code, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y);
```

Parameters:

- `charset`: one of the three charset constants
- `code`: the character code to draw
- `foreground`: colour of the character itself
- `background`: colour behind the character
- `x`: screen column
- `y`: screen row

Example 1: draw one ASCII letter

```c
charProcess(CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK, 10, 5);
```

This draws `A` at `10,5`.

Example 2: draw one raw display-A code

```c
charProcess(CHARSET_DISPLAY_A, 0x80, MZ_COLOUR_YELLOW, MZ_COLOUR_BLUE, 12, 8);
```

This draws raw display-set-A code `0x80` at `12,8`.

Example 3: draw one raw display-B code

```c
charProcess(CHARSET_DISPLAY_B, 0x80, MZ_COLOUR_RED, MZ_COLOUR_BLACK, 14, 8);
```

This draws raw display-set-B code `0x80` at `14,8`.

## How To Use `charArrayProcces()`

`charArrayProcces()` draws multiple characters in a row starting at the supplied `x,y`.

Signature:

```c
void charArrayProcces(uint8_t charset, const uint8_t *codes, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y);
```

Parameters:

- `charset`: one of the three charset constants
- `codes`: pointer to an array of character codes
- `foreground`: colour of the character itself
- `background`: colour behind the character
- `x`: starting screen column
- `y`: screen row

### Important Rule

The `codes` array must end with `0x00`.

That means the function treats `0x00` as the terminator and stops drawing when it reaches it.

Example:

```c
static const uint8_t hello_codes[] = {
    0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x00
};

charArrayProcces(
    CHARSET_ASCII,
    hello_codes,
    MZ_COLOUR_WHITE,
    MZ_COLOUR_BLUE,
    4,
    10
);
```

That draws `HELLO` starting at `4,10`.

### How It Moves Across The Screen

`charArrayProcces()`:

- starts at the given `x,y`
- draws one character
- increments `x`
- draws the next character
- stops if it reaches the array terminator `0x00`
- also stops if `x` reaches the right side of the screen

It does not currently:

- wrap to the next line
- interpret newline codes
- allow `0x00` to be drawn as a visible character inside the array

## How To Use `sprite8x8()`

## How To Use `sprite4x4()`

`sprite4x4()` draws one 4x4 tile made from 16 character cells.

Signature:

```c
void sprite4x4(const MzTileCell *tileArray, uint8_t x, uint8_t y);
```

Parameters:

- `tileArray`: pointer to 16 `MzTileCell` entries
- `x`: top-left screen column of the tile
- `y`: top-left screen row of the tile

Each cell in the tile contains 4 fields:

- `charset`
- `code`
- `foreground`
- `background`

The cells are stored row-by-row:

- entries `0..3` are the first row
- entries `4..7` are the second row
- entries `12..15` are the last row

Example:

```c
static const MzTileCell mini_tile[16] = {
    { CHARSET_ASCII, 0x47, MZ_COLOUR_GREEN, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x47, MZ_COLOUR_GREEN, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x52, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x52, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x52, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x52, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x47, MZ_COLOUR_GREEN, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x59, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x47, MZ_COLOUR_GREEN, MZ_COLOUR_BLACK }
};

sprite4x4(mini_tile, 10, 10);
```

That draws the top-left corner of the 4x4 tile at screen coordinate `10,10`.

## How To Use `sprite8x8()`

`sprite8x8()` draws one 8x8 tile made from 64 character cells.

Signature:

```c
void sprite8x8(const MzTileCell *tileArray, uint8_t x, uint8_t y);
```

Parameters:

- `tileArray`: pointer to 64 `MzTileCell` entries
- `x`: top-left screen column of the tile
- `y`: top-left screen row of the tile

Each cell in the tile contains 4 fields:

- `charset`
- `code`
- `foreground`
- `background`

The cells are stored row-by-row:

- entries `0..7` are the first row
- entries `8..15` are the second row
- entries `56..63` are the last row

Example:

```c
static const MzTileCell demo_tile[64] = {
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x43, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x43, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x43, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x43, MZ_COLOUR_RED, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x42, MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x20, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK },
    { CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK }
};

sprite8x8(demo_tile, 24, 10);
```

That draws the top-left corner of the 8x8 tile at screen coordinate `24,10`.

## How To Use `mzClearScreen()`

`mzClearScreen()` fills the full 40x25 screen with spaces using the requested colours.

Signature:

```c
void mzClearScreen(uint8_t foreground, uint8_t background);
```

Example:

```c
mzClearScreen(MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
```

## How To Use `mzWriteUiText()`

`mzWriteUiText()` is a helper for writing normal C strings as text.

Signature:

```c
void mzWriteUiText(uint8_t x, uint8_t y, const char *text, uint8_t foreground, uint8_t background);
```

Use this for labels, menus, status text, or debug messages.

Example:

```c
mzWriteUiText(2, 2, "SCORE", MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK);
```

Notes:

- this is intended for UI text, not raw display-table exploration
- it uses the wrapper's internal ASCII mapping for common printable characters
- newline `\n` is supported and moves to the next line at the original start column

## Small Complete Example

This is a simple example of using the wrapper from another program:

```c
#include <stdint.h>
#include "char_wrapper.h"

static const uint8_t title_codes[] = {
    0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x00
};

int main(void)
{
    mzClearScreen(MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);

    mzWriteUiText(2, 2, "WRAPPER TEST", MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK);

    charProcess(CHARSET_ASCII, 0x41, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK, 10, 5);

    charProcess(CHARSET_DISPLAY_A, 0x80, MZ_COLOUR_GREEN, MZ_COLOUR_BLACK, 12, 5);

    charProcess(CHARSET_DISPLAY_B, 0x80, MZ_COLOUR_RED, MZ_COLOUR_BLACK, 14, 5);

    charArrayProcces(
        CHARSET_ASCII,
        title_codes,
        MZ_COLOUR_WHITE,
        MZ_COLOUR_BLUE,
        4,
        10
    );

    for (;;) {
    }

    return 0;
}
```

## Build Example

If you are compiling your own app with z88dk, include both your source file and the wrapper source file:

```powershell
zcc +mz -O3 -create-app -o build\myapp src\main.c src\char_wrapper.c
```

If you use the local wrapper project as-is, [build.ps1](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/build.ps1) already compiles:

- [src/main.c](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/main.c)
- [src/char_wrapper.c](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/z88DK%20Sharp%20Helpers/Character%20Rom%20Wrapper/src/char_wrapper.c)

## Current Limitations

These are the practical limitations of the current wrapper implementation:

- ASCII conversion currently covers common printable characters, not the entire documented MZ-700 ASCII table
- `charArrayProcces()` requires a `0x00` terminator
- `charArrayProcces()` cannot currently draw `0x00` as part of the array
- `charArrayProcces()` draws on one row only and stops at the right edge
- `sprite4x4()` assumes the tile array contains exactly 16 cells
- `sprite8x8()` assumes the tile array contains exactly 64 cells
- no clipping or wrapping beyond basic bounds checking
- no full font/character metadata lookup is included

## Recommended Usage Style

Use this wrapper in the following way:

- use `mzClearScreen()` for scene setup
- use `mzWriteUiText()` for labels and readable UI strings
- use `charProcess()` for single special characters or direct character placement
- use `charArrayProcces()` for short sequences of known codes
- use `sprite4x4()` for 4x4 character-based tiles
- use `sprite8x8()` for 8x8 character-based tiles
- use `CHARSET_DISPLAY_A` or `CHARSET_DISPLAY_B` when you already know the raw MZ display codes you want
- use `CHARSET_ASCII` for common readable text-like characters when the local mapping is enough

## Future Improvements

Good future additions would be:

- a length-based version of `charArrayProcces()` so `0x00` can be drawn
- optional line wrapping
- a fuller ASCII-to-display mapping covering more of the MZ-700 table
- helpers for drawing framed boxes, labels, and tables
- a text writer that mixes raw display codes and UI strings more flexibly
