#ifndef CHAR_WRAPPER_H
#define CHAR_WRAPPER_H

#include <stdint.h>

#define CHARSET_ASCII 1
#define CHARSET_DISPLAY_A 2
#define CHARSET_DISPLAY_B 3

#define MZ_COLOUR_BLACK 0
#define MZ_COLOUR_BLUE 1
#define MZ_COLOUR_RED 2
#define MZ_COLOUR_PURPLE 3
#define MZ_COLOUR_GREEN 4
#define MZ_COLOUR_LIGHT_BLUE 5
#define MZ_COLOUR_YELLOW 6
#define MZ_COLOUR_WHITE 7

typedef struct MzTileCell {
    uint8_t charset;
    uint8_t code;
    uint8_t foreground;
    uint8_t background;
} MzTileCell;

void mzClearScreen(uint8_t foreground, uint8_t background);
void mzWriteUiText(uint8_t x, uint8_t y, const char *text, uint8_t foreground, uint8_t background);
void charProcess(uint8_t charset, uint8_t code, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y);
void charArrayProcces(uint8_t charset, const uint8_t *codes, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y);
void sprite4x4(const MzTileCell *tileArray, uint8_t x, uint8_t y);
void sprite8x8(const MzTileCell *tileArray, uint8_t x, uint8_t y);

#endif
