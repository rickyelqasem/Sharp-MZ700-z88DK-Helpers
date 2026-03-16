#include "mz700_screen.h"

#define MZ700_SCREEN_WIDTH 40
#define MZ700_SCREEN_HEIGHT 25
#define MZ700_VRAM ((volatile uint8_t *)0xD000)
#define MZ700_ATTR ((volatile uint8_t *)0xD800)
#define MZ700_ATTR_WHITE_ON_BLACK 0x70

extern void mz700_bank_vram_mmio(void);
extern void mz700_bank_ram(void);

static uint8_t to_mz_char(char ch)
{
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
    case ' ':
        return 0x00;
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
        return 0x6B;
    case '+':
        return 0x6A;
    case ',':
        return 0x2F;
    case '-':
        return 0x2A;
    case '.':
        return 0x2E;
    case '/':
        return 0x2D;
    case ':':
        return 0x4F;
    case ';':
        return 0x2C;
    case '<':
        return 0x51;
    case '=':
        return 0x2B;
    case '>':
        return 0x57;
    case '?':
        return 0x49;
    case '@':
        return 0x55;
    case '[':
        return 0x52;
    case '\\':
        return 0x59;
    case ']':
        return 0x54;
    case '^':
        return 0x50;
    case '_':
        return 0x3C;
    case '|':
        return 0x79;
    default:
        return 0x00;
    }
}

static void mz700_screen_put_cell(uint8_t x, uint8_t y, char ch)
{
    uint16_t offset = (uint16_t)y * MZ700_SCREEN_WIDTH + x;

    mz700_bank_vram_mmio();
    MZ700_VRAM[offset] = to_mz_char(ch);
    MZ700_ATTR[offset] = MZ700_ATTR_WHITE_ON_BLACK;
    mz700_bank_ram();
}

static void uint8_to_dec3(uint8_t value, char *out)
{
    out[0] = (char)('0' + (value / 100));
    value %= 100;
    out[1] = (char)('0' + (value / 10));
    out[2] = (char)('0' + (value % 10));
    out[3] = '\0';
}

void mz700_screen_clear(void)
{
    uint16_t i;

    mz700_bank_vram_mmio();
    for (i = 0; i < (uint16_t)MZ700_SCREEN_WIDTH * MZ700_SCREEN_HEIGHT; ++i) {
        MZ700_VRAM[i] = to_mz_char(' ');
        MZ700_ATTR[i] = MZ700_ATTR_WHITE_ON_BLACK;
    }
    mz700_bank_ram();
}

void mz700_screen_write_text(uint8_t x, uint8_t y, const char *text)
{
    while (*text != '\0' && x < MZ700_SCREEN_WIDTH) {
        mz700_screen_put_cell(x, y, *text);
        ++x;
        ++text;
    }
}

void mz700_screen_render_u8(uint8_t x, uint8_t y, uint8_t value)
{
    char buffer[4];

    uint8_to_dec3(value, buffer);
    mz700_screen_write_text(x, y, buffer);
}
