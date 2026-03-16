# Sharp MZ-700 Input Working Notes

This note captures the exact path that produced a working MZ-1X03 joystick reader and a working direct keyboard reader in this repo so future MZ-700 input work can start from the correct assumptions.

## What finally worked

- Use the original machine-language joystick routines from [joy.asm](c:/Users/ricky/OneDrive/SharpMZ700/joyex/joy.asm) as literally as possible.
- Do not rewrite the axis logic into a "cleaner" equivalent unless there is hard proof it is correct on hardware.
- Keep the original self-modifying behavior:
  - `JA1` loads `A=4EH`
  - `JA2` loads `A=56H`
  - `LD (LP4+2),A` patches the operand byte of `BIT 0,(HL)` so the shared loop really becomes `BIT 1,(HL)` or `BIT 2,(HL)`
- Keep the original switch logic too:
  - `SW1` uses `A=4EH`
  - `SW2` uses `A=56H`
  - The routine patches both `LP7+1` and `LPB+1`

## Important hardware detail

- On the MZ-700, upper memory is bank-switched.
- Before touching joystick MMIO at `0xE002` / `0xE008`, switch upper memory to `VRAM/MMIO` via I/O port `0xE3`.
- Before touching keyboard MMIO at `0xE000` / `0xE001`, also switch upper memory to `VRAM/MMIO` via I/O port `0xE3`.
- After the joystick routine finishes, switch upper memory back to RAM via I/O port `0xE1`.
- After a direct keyboard scan finishes, switch upper memory back to RAM via I/O port `0xE1`.
- The same applies to direct VRAM writes at `0xD000` / `0xD800`.

Practical rule:

- bracket MMIO / VRAM access with `bank to VRAM/MMIO` and `bank back to RAM`

## Keyboard and joystick together

- The MZ-700 keyboard and joystick both live in the same upper MMIO area.
- z88dk `getk()` for target `+mz` uses the ROM monitor key routine at `$001B`, not the same direct MMIO path as the joystick code.
- Mixing ROM keyboard reads with direct joystick MMIO reads caused misleading behavior in this project.
- The reliable fix was to stop using `getk()` and add a direct keyboard scanner using the 8255 PPI:
  - write the row strobe to `0xE000`
  - wait one `NOP`
  - read the row value from `0xE001`
  - a key is pressed when its bit is `0`

Practical rule:

- for apps that use the MZ-1X03 joystick, prefer direct keyboard scanning over `getk()`

## z88dk integration detail

- The original routine returns its result in register `A`.
- z88dk C expects an 8-bit return value in `L` / `HL`.
- Therefore the assembly wrapper must:
  - call the original routine
  - restore RAM bank
  - move `A` into `L`
  - clear `H`
  - return

## Files that now contain the working implementation

- [src/joy_read.asm](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/joystick%20test/v1.1/src/joy_read.asm)
- [src/mz700_joystick.h](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/joystick%20test/v1.1/src/mz700_joystick.h)
- [src/mz700_joystick.c](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/joystick%20test/v1.1/src/mz700_joystick.c)
- [src/mz700_keyboard.h](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/joystick%20test/v1.1/src/mz700_keyboard.h)
- [src/mz700_keyboard.c](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/joystick%20test/v1.1/src/mz700_keyboard.c)
- [src/mz700_screen.h](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/joystick%20test/v1.1/src/mz700_screen.h)
- [src/mz700_screen.c](/c:/Users/ricky/OneDrive/devwork/8bit%20games/sharp%20mz700/joystick%20test/v1.1/src/mz700_screen.c)

## Public wrapper API

```c
#include "mz700_joystick.h"
#include "mz700_keyboard.h"
#include "mz700_screen.h"

uint8_t x = mz700_joystick1_read_x();
uint8_t y = mz700_joystick1_read_y();
uint8_t fire1 = mz700_joystick1_read_fire1();
uint8_t fire2 = mz700_joystick1_read_fire2();

uint8_t joy2_x = mz700_joystick2_read_x();
uint8_t joy2_y = mz700_joystick2_read_y();
uint8_t joy2_fire1 = mz700_joystick2_read_fire1();
uint8_t joy2_fire2 = mz700_joystick2_read_fire2();

char key = mz700_keyboard_read_key();

mz700_screen_clear();
mz700_screen_write_text(0, 0, "HELLO");
mz700_screen_render_u8(0, 1, x);
```

## Joystick 2 mapping

- `JB1` is joystick 2 X axis
- `JB2` is joystick 2 Y axis
- `SW3` is joystick 2 fire button 1
- `SW4` is joystick 2 fire button 2

The working wrapper now exposes all of these through `mz700_joystick.h`.

## Keyboard wrapper notes

- `mz700_keyboard_read_key()` performs direct row scanning through `0xE000` / `0xE001`
- it currently returns a practical single-key value for test/game use
- cursor keys are currently represented as:
  - `L` for LEFT
  - `R` for RIGHT
  - `U` for UP
  - `D` for DOWN
- this is why joystick motion can appear as `L/R/U/D` if the same matrix lines are observed during testing

## Screen wrapper notes

- `mz700_screen.c` owns the direct VRAM writes and MZ display-code conversion
- punctuation mapping matters on the MZ-700
- an incorrect character map caused visible errors like `-` showing as the wrong glyph
- the validated punctuation mapping was taken from the earlier working MZ platform code

## What not to do next time

- Do not replace the working joystick code with an inferred directional sampler.
- Do not assume reading `0xE002` / `0xE008` works without bank switching.
- Do not assume reading `0xE000` / `0xE001` works without bank switching.
- Do not assume "return in A" is enough for z88dk C.
- Do not mix `getk()` with direct joystick MMIO and expect reliable simultaneous input.
- Do not treat tiny `+/-1` changes around center as proof the read path is correct.

## Good prompt for future sessions

Use the working MZ-700 input wrappers in this repo.
The joystick implementation is based on a near-literal port of `joy.asm`, including the self-modifying `LP4+2` patching and MZ-700 upper-bank switching (`0xE3` to VRAM/MMIO, `0xE1` back to RAM).
Do not use `getk()` when mixing keyboard and MZ-1X03 joystick input; use the direct keyboard wrapper instead.
Preserve the current wrapper ABI in `src/mz700_joystick.h`, `src/mz700_keyboard.h`, and `src/mz700_screen.h`.
