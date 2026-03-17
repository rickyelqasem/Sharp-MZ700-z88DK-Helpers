#include <conio.h>
#include <stdint.h>

#include "char_wrapper.h"

static void tinyDelay(void);
static void waitForKeyRelease(void);
static uint8_t chooseCharset(void);
static void writeHex8(uint8_t x, uint8_t y, uint8_t value);
static void writeDec3(uint8_t x, uint8_t y, uint8_t value);
static void initExampleMiniSprite(void);
static void initExampleSprite(void);
static void drawExampleMiniSprite(void);
static void drawExampleSprite(void);
static const char *colourName(uint8_t colour);
static const char *charsetName(uint8_t charset);
static void drawStaticUi(void);
static void updateUi(uint8_t charset, uint8_t code, uint8_t foreground, uint8_t background);

static const char *const COLOUR_NAMES[] = {
    "BLACK",
    "BLUE",
    "RED",
    "PURPLE",
    "GREEN",
    "LT BLUE",
    "YELLOW",
    "WHITE"
};

static MzTileCell EXAMPLE_MINI_TILE[16];
static MzTileCell EXAMPLE_TILE[64];

static void tinyDelay(void)
{
    uint16_t i;

    for (i = 0; i < 3000; ++i) {
    }
}

static void waitForKeyRelease(void)
{
    while (getk() != 0) {
        tinyDelay();
    }
}

static uint8_t chooseCharset(void)
{
    uint8_t key;

    mzClearScreen(MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(4, 4, "CHAR WRAPPER DEMO", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(4, 7, "PRESS 1 FOR ASCII", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(4, 9, "PRESS 2 FOR DISPLAY A", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(4, 11, "PRESS 3 FOR DISPLAY B", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);

    for (;;) {
        key = getk();
        if (key == '1') {
            waitForKeyRelease();
            return CHARSET_ASCII;
        }
        if (key == '2') {
            waitForKeyRelease();
            return CHARSET_DISPLAY_A;
        }
        if (key == '3') {
            waitForKeyRelease();
            return CHARSET_DISPLAY_B;
        }
    }
}

static void writeHex8(uint8_t x, uint8_t y, uint8_t value)
{
    static const char HEX[] = "0123456789ABCDEF";
    char text[3];

    text[0] = HEX[(value >> 4) & 0x0F];
    text[1] = HEX[value & 0x0F];
    text[2] = 0;
    mzWriteUiText(x, y, text, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
}

static void writeDec3(uint8_t x, uint8_t y, uint8_t value)
{
    char text[4];

    text[0] = (char)('0' + (value / 100));
    text[1] = (char)('0' + ((value / 10) % 10));
    text[2] = (char)('0' + (value % 10));
    text[3] = 0;
    mzWriteUiText(x, y, text, MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
}

static void initExampleMiniSprite(void)
{
    static uint8_t initialized = 0;
    uint8_t i;

    if (initialized != 0) {
        return;
    }

    for (i = 0; i < 16; ++i) {
        EXAMPLE_MINI_TILE[i].charset = CHARSET_ASCII;
        EXAMPLE_MINI_TILE[i].code = 0x2E;
        EXAMPLE_MINI_TILE[i].foreground = MZ_COLOUR_LIGHT_BLUE;
        EXAMPLE_MINI_TILE[i].background = MZ_COLOUR_BLACK;
    }

    EXAMPLE_MINI_TILE[5].code = 0x59;
    EXAMPLE_MINI_TILE[5].foreground = MZ_COLOUR_YELLOW;
    EXAMPLE_MINI_TILE[5].background = MZ_COLOUR_RED;

    EXAMPLE_MINI_TILE[6].code = 0x59;
    EXAMPLE_MINI_TILE[6].foreground = MZ_COLOUR_YELLOW;
    EXAMPLE_MINI_TILE[6].background = MZ_COLOUR_RED;

    EXAMPLE_MINI_TILE[9].code = 0x59;
    EXAMPLE_MINI_TILE[9].foreground = MZ_COLOUR_YELLOW;
    EXAMPLE_MINI_TILE[9].background = MZ_COLOUR_GREEN;

    EXAMPLE_MINI_TILE[10].code = 0x59;
    EXAMPLE_MINI_TILE[10].foreground = MZ_COLOUR_YELLOW;
    EXAMPLE_MINI_TILE[10].background = MZ_COLOUR_GREEN;

    initialized = 1;
}

static void initExampleSprite(void)
{
    static uint8_t initialized = 0;
    uint8_t i;

    if (initialized != 0) {
        return;
    }

    for (i = 0; i < 64; ++i) {
        EXAMPLE_TILE[i].charset = CHARSET_ASCII;
        EXAMPLE_TILE[i].code = 0x2E;
        EXAMPLE_TILE[i].foreground = MZ_COLOUR_BLUE;
        EXAMPLE_TILE[i].background = MZ_COLOUR_BLACK;
    }

    EXAMPLE_TILE[11].code = 0x47; EXAMPLE_TILE[11].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[12].code = 0x47; EXAMPLE_TILE[12].foreground = MZ_COLOUR_GREEN;

    EXAMPLE_TILE[18].code = 0x47; EXAMPLE_TILE[18].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[19].code = 0x47; EXAMPLE_TILE[19].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[20].code = 0x47; EXAMPLE_TILE[20].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[21].code = 0x47; EXAMPLE_TILE[21].foreground = MZ_COLOUR_GREEN;

    EXAMPLE_TILE[25].code = 0x47; EXAMPLE_TILE[25].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[26].code = 0x47; EXAMPLE_TILE[26].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[27].code = 0x59; EXAMPLE_TILE[27].foreground = MZ_COLOUR_YELLOW; EXAMPLE_TILE[27].background = MZ_COLOUR_RED;
    EXAMPLE_TILE[28].code = 0x59; EXAMPLE_TILE[28].foreground = MZ_COLOUR_YELLOW; EXAMPLE_TILE[28].background = MZ_COLOUR_RED;
    EXAMPLE_TILE[29].code = 0x47; EXAMPLE_TILE[29].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[30].code = 0x47; EXAMPLE_TILE[30].foreground = MZ_COLOUR_GREEN;

    EXAMPLE_TILE[33].code = 0x47; EXAMPLE_TILE[33].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[34].code = 0x59; EXAMPLE_TILE[34].foreground = MZ_COLOUR_YELLOW; EXAMPLE_TILE[34].background = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[35].code = 0x52; EXAMPLE_TILE[35].foreground = MZ_COLOUR_RED; EXAMPLE_TILE[35].background = MZ_COLOUR_YELLOW;
    EXAMPLE_TILE[36].code = 0x52; EXAMPLE_TILE[36].foreground = MZ_COLOUR_RED; EXAMPLE_TILE[36].background = MZ_COLOUR_YELLOW;
    EXAMPLE_TILE[37].code = 0x59; EXAMPLE_TILE[37].foreground = MZ_COLOUR_YELLOW; EXAMPLE_TILE[37].background = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[38].code = 0x47; EXAMPLE_TILE[38].foreground = MZ_COLOUR_GREEN;

    EXAMPLE_TILE[41].code = 0x47; EXAMPLE_TILE[41].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[42].code = 0x47; EXAMPLE_TILE[42].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[43].code = 0x59; EXAMPLE_TILE[43].foreground = MZ_COLOUR_YELLOW; EXAMPLE_TILE[43].background = MZ_COLOUR_BLUE;
    EXAMPLE_TILE[44].code = 0x59; EXAMPLE_TILE[44].foreground = MZ_COLOUR_YELLOW; EXAMPLE_TILE[44].background = MZ_COLOUR_BLUE;
    EXAMPLE_TILE[45].code = 0x47; EXAMPLE_TILE[45].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[46].code = 0x47; EXAMPLE_TILE[46].foreground = MZ_COLOUR_GREEN;

    EXAMPLE_TILE[50].code = 0x47; EXAMPLE_TILE[50].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[51].code = 0x47; EXAMPLE_TILE[51].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[52].code = 0x47; EXAMPLE_TILE[52].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[53].code = 0x47; EXAMPLE_TILE[53].foreground = MZ_COLOUR_GREEN;

    EXAMPLE_TILE[59].code = 0x47; EXAMPLE_TILE[59].foreground = MZ_COLOUR_GREEN;
    EXAMPLE_TILE[60].code = 0x47; EXAMPLE_TILE[60].foreground = MZ_COLOUR_GREEN;

    initialized = 1;
}

static void drawExampleMiniSprite(void)
{
    initExampleMiniSprite();
    sprite4x4(EXAMPLE_MINI_TILE, 35, 1);
}

static void drawExampleSprite(void)
{
    initExampleSprite();
    mzWriteUiText(27, 8, "TILE", MZ_COLOUR_YELLOW, MZ_COLOUR_BLACK);
    sprite8x8(EXAMPLE_TILE, 24, 10);
}

static const char *colourName(uint8_t colour)
{
    return COLOUR_NAMES[colour & 0x07];
}

static const char *charsetName(uint8_t charset)
{
    if (charset == CHARSET_ASCII) {
        return "ASCII";
    }
    if (charset == CHARSET_DISPLAY_A) {
        return "DISPLAY A";
    }
    return "DISPLAY B";
}

static void drawStaticUi(void)
{
    mzClearScreen(MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    drawExampleMiniSprite();
    mzWriteUiText(2, 1, "CHAR WRAPPER API DEMO", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 3, "SET", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 5, "HEX", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(14, 5, "DEC", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 18, "FG", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 19, "BG", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 21, "A NEXT  Z PREVIOUS", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 22, "N FG COLOUR  M BG COLOUR", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 23, "1 ASCII  2 DISP A  3 DISP B", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(2, 24, "Q QUIT", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    drawExampleSprite();
}

static void updateUi(uint8_t charset, uint8_t code, uint8_t foreground, uint8_t background)
{
    mzWriteUiText(7, 3, "            ", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(7, 3, charsetName(charset), MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    writeHex8(7, 5, code);
    writeDec3(19, 5, code);
    mzWriteUiText(5, 18, "            ", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(5, 18, colourName(foreground), MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(5, 19, "            ", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(5, 19, colourName(background), MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);

    mzWriteUiText(2, 16, "API CALL BELOW            ", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    charProcess(charset, code, foreground, background, 20, 11);
}

int main(void)
{
    uint8_t charset;
    uint8_t code;
    uint8_t foreground;
    uint8_t background;
    uint8_t key;

    charset = chooseCharset();
    code = 0;
    foreground = MZ_COLOUR_WHITE;
    background = MZ_COLOUR_BLACK;

    drawStaticUi();
    updateUi(charset, code, foreground, background);

    for (;;) {
        for (;;) {
            key = getk();
            if (key != 0) {
                break;
            }
        }

        if (key == 'a' || key == 'A') {
            ++code;
        } else if (key == 'z' || key == 'Z') {
            --code;
        } else if (key == 'n' || key == 'N') {
            foreground = (uint8_t)((foreground + 1) & 0x07);
        } else if (key == 'm' || key == 'M') {
            background = (uint8_t)((background + 1) & 0x07);
        } else if (key == '1') {
            charset = CHARSET_ASCII;
            code = 0;
        } else if (key == '2') {
            charset = CHARSET_DISPLAY_A;
            code = 0;
        } else if (key == '3') {
            charset = CHARSET_DISPLAY_B;
            code = 0;
        } else if (key == 'q' || key == 'Q') {
            break;
        }

        updateUi(charset, code, foreground, background);
        waitForKeyRelease();
    }

    mzClearScreen(MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);
    mzWriteUiText(10, 12, "END OF TEST", MZ_COLOUR_WHITE, MZ_COLOUR_BLACK);

    return 0;
}
