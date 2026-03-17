#include "char_wrapper.h"

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25

#define DISPLAY_BASE ((volatile uint8_t *)0xD000)
#define COLOUR_BASE ((volatile uint8_t *)0xD800)

static uint8_t mzEncodeUiChar(uint8_t ch);
static uint8_t mzEncodeAsciiChar(uint8_t ch);
static uint8_t mzBuildColour(uint8_t charset, uint8_t foreground, uint8_t background);
static void mzPutRawCode(uint8_t x, uint8_t y, uint8_t code, uint8_t colour);

static uint8_t mzEncodeUiChar(uint8_t ch)
{
    if (ch == ' ') {
        return 0x00;
    }

    if (ch >= '0' && ch <= '9') {
        return (uint8_t)(ch - 16);
    }

    if (ch >= 'A' && ch <= 'Z') {
        return (uint8_t)(ch - 64);
    }

    if (ch >= 'a' && ch <= 'z') {
        return (uint8_t)(ch + 32);
    }

    switch (ch) {
    case '!':
        return 0x61;
    case '"':
        return 0x62;
    case '#':
        return 0x63;
    case '$':
        return 0x64;
    case '%':
        return 0x65;
    case '&':
        return 0x66;
    case '\'':
        return 0x67;
    case '(':
        return 0x68;
    case ')':
        return 0x69;
    case '*':
        return 0x6A;
    case '+':
        return 0x6B;
    case ',':
        return 0x6C;
    case '-':
        return 0x6D;
    case '.':
        return 0x6E;
    case '/':
        return 0x6F;
    case ':':
        return 0x70;
    case ';':
        return 0x71;
    case '<':
        return 0x72;
    case '=':
        return 0x73;
    case '>':
        return 0x74;
    case '?':
        return 0x75;
    default:
        return 0x00;
    }
}

static uint8_t mzEncodeAsciiChar(uint8_t ch)
{
    return mzEncodeUiChar(ch);
}

static uint8_t mzBuildColour(uint8_t charset, uint8_t foreground, uint8_t background)
{
    uint8_t colour;

    colour = (uint8_t)((((foreground & 0x07) << 4) | (background & 0x07)) & 0x77);
    if (charset == CHARSET_DISPLAY_B) {
        colour |= 0x80;
    }

    return colour;
}

static void mzPutRawCode(uint8_t x, uint8_t y, uint8_t code, uint8_t colour)
{
    uint16_t offset;

    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        return;
    }

    offset = ((uint16_t)y * SCREEN_WIDTH) + x;
    DISPLAY_BASE[offset] = code;
    COLOUR_BASE[offset] = colour;
}

void mzClearScreen(uint8_t foreground, uint8_t background)
{
    uint16_t i;
    uint8_t colour;

    colour = mzBuildColour(CHARSET_DISPLAY_A, foreground, background);

    for (i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); ++i) {
        DISPLAY_BASE[i] = mzEncodeUiChar(' ');
        COLOUR_BASE[i] = colour;
    }
}

void mzWriteUiText(uint8_t x, uint8_t y, const char *text, uint8_t foreground, uint8_t background)
{
    uint8_t px;
    uint8_t py;
    uint8_t colour;

    px = x;
    py = y;
    colour = mzBuildColour(CHARSET_DISPLAY_A, foreground, background);

    while (*text != 0) {
        if (*text == '\n') {
            ++py;
            px = x;
        } else {
            mzPutRawCode(px, py, mzEncodeUiChar((uint8_t)*text), colour);
            ++px;
        }
        ++text;
    }
}

void charProcess(uint8_t charset, uint8_t code, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y)
{
    uint8_t displayCode;
    uint8_t colour;

    if (charset < CHARSET_ASCII || charset > CHARSET_DISPLAY_B) {
        return;
    }

    if (charset == CHARSET_ASCII) {
        displayCode = mzEncodeAsciiChar(code);
        colour = mzBuildColour(CHARSET_DISPLAY_A, foreground, background);
    } else {
        displayCode = code;
        colour = mzBuildColour(charset, foreground, background);
    }

    mzPutRawCode(x, y, displayCode, colour);
}

void charArrayProcces(uint8_t charset, const uint8_t *codes, uint8_t foreground, uint8_t background, uint8_t x, uint8_t y)
{
    if (codes == 0) {
        return;
    }

    while (*codes != 0) {
        charProcess(charset, *codes, foreground, background, x, y);
        ++codes;
        ++x;

        if (x >= SCREEN_WIDTH) {
            break;
        }
    }
}

void sprite4x4(const MzTileCell *tileArray, uint8_t x, uint8_t y)
{
    uint8_t row;
    uint8_t col;

    if (tileArray == 0) {
        return;
    }

    for (row = 0; row < 4; ++row) {
        for (col = 0; col < 4; ++col) {
            charProcess(
                tileArray[(row * 4) + col].charset,
                tileArray[(row * 4) + col].code,
                tileArray[(row * 4) + col].foreground,
                tileArray[(row * 4) + col].background,
                (uint8_t)(x + col),
                (uint8_t)(y + row)
            );
        }
    }
}

void sprite8x8(const MzTileCell *tileArray, uint8_t x, uint8_t y)
{
    uint8_t row;
    uint8_t col;

    if (tileArray == 0) {
        return;
    }

    for (row = 0; row < 8; ++row) {
        for (col = 0; col < 8; ++col) {
            charProcess(
                tileArray[(row * 8) + col].charset,
                tileArray[(row * 8) + col].code,
                tileArray[(row * 8) + col].foreground,
                tileArray[(row * 8) + col].background,
                (uint8_t)(x + col),
                (uint8_t)(y + row)
            );
        }
    }
}
