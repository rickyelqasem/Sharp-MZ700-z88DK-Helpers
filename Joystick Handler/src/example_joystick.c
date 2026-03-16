#include <stdint.h>
#include <stdbool.h>
#include <conio.h>

#include "mz700_joystick.h"

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
    case ':':
        return 0x1A;
    case '-':
        return 0x16;
    default:
        return 0x00;
    }
}

static void mz700_put_cell(uint8_t x, uint8_t y, char ch)
{
    uint16_t offset = (uint16_t)y * MZ700_SCREEN_WIDTH + x;
    mz700_bank_vram_mmio();
    MZ700_VRAM[offset] = to_mz_char(ch);
    MZ700_ATTR[offset] = MZ700_ATTR_WHITE_ON_BLACK;
    mz700_bank_ram();
}

static void mz700_write_text(uint8_t x, uint8_t y, const char *text)
{
    while (*text != '\0' && x < MZ700_SCREEN_WIDTH) {
        mz700_put_cell(x, y, *text);
        ++x;
        ++text;
    }
}

static void mz700_clear_screen(void)
{
    uint16_t i;
    for (i = 0; i < (uint16_t)MZ700_SCREEN_WIDTH * MZ700_SCREEN_HEIGHT; ++i) {
        MZ700_VRAM[i] = to_mz_char(' ');
        MZ700_ATTR[i] = MZ700_ATTR_WHITE_ON_BLACK;
    }
}

static void uint8_to_dec3(uint8_t value, char *out)
{
    out[0] = (char)('0' + (value / 100));
    value %= 100;
    out[1] = (char)('0' + (value / 10));
    out[2] = (char)('0' + (value % 10));
    out[3] = '\0';
}

static void render_value(uint8_t x, uint8_t y, uint8_t value)
{
    char buffer[4];
    uint8_to_dec3(value, buffer);
    mz700_write_text(x, y, buffer);
}

static void tiny_delay(void)
{
    volatile uint16_t i;
    for (i = 0; i < 3000; ++i) {
    }
}

int main(void)
{
    uint8_t joy_x;
    uint8_t joy_y;
    uint8_t fire1;
    uint8_t fire2;
    uint8_t joy2_x;
    uint8_t joy2_y;
    uint8_t joy2_fire1;
    uint8_t joy2_fire2;
    int key;

    mz700_clear_screen();
    mz700_write_text(8, 3, "MZ-700 JOYSTICK TEST");
    mz700_write_text(4, 6, "JOY1 X:");
    mz700_write_text(4, 8, "JOY1 Y:");
    mz700_write_text(4, 10, "JOY1 F1:");
    mz700_write_text(4, 12, "JOY1 F2:");
    mz700_write_text(22, 6, "JOY2 X:");
    mz700_write_text(22, 8, "JOY2 Y:");
    mz700_write_text(22, 10, "JOY2 F1:");
    mz700_write_text(22, 12, "JOY2 F2:");
    mz700_write_text(9, 16, "PRESS Q TO EXIT");

    while (1) {
        joy_x = mz700_joystick1_read_x();
        joy_y = mz700_joystick1_read_y();
        fire1 = mz700_joystick1_read_fire1();
        fire2 = mz700_joystick1_read_fire2();
        joy2_x = mz700_joystick2_read_x();
        joy2_y = mz700_joystick2_read_y();
        joy2_fire1 = mz700_joystick2_read_fire1();
        joy2_fire2 = mz700_joystick2_read_fire2();

        render_value(12, 6, joy_x);
        render_value(12, 8, joy_y);
        mz700_write_text(13, 10, fire1 ? "ON " : "OFF");
        mz700_write_text(13, 12, fire2 ? "ON " : "OFF");
        render_value(30, 6, joy2_x);
        render_value(30, 8, joy2_y);
        mz700_write_text(31, 10, joy2_fire1 ? "ON " : "OFF");
        mz700_write_text(31, 12, joy2_fire2 ? "ON " : "OFF");

        key = getk();
        if (key == 'q' || key == 'Q') {
            break;
        }

        tiny_delay();
    }

    mz700_write_text(15, 20, "STOPPED");
    return 0;
}
