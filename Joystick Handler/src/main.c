#include <stdint.h>

#include "mz700_keyboard.h"
#include "mz700_joystick.h"
#include "mz700_screen.h"

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
    char key;
    char key_text[2];

    key_text[0] = ' ';
    key_text[1] = '\0';

    mz700_screen_clear();
    mz700_screen_write_text(8, 3, "MZ-700 JOYSTICK TEST");
    mz700_screen_write_text(4, 6, "JOY1 X:");
    mz700_screen_write_text(4, 8, "JOY1 Y:");
    mz700_screen_write_text(4, 10, "JOY1 F1:");
    mz700_screen_write_text(4, 12, "JOY1 F2:");
    mz700_screen_write_text(22, 6, "JOY2 X:");
    mz700_screen_write_text(22, 8, "JOY2 Y:");
    mz700_screen_write_text(22, 10, "JOY2 F1:");
    mz700_screen_write_text(22, 12, "JOY2 F2:");
    mz700_screen_write_text(14, 14, "KEY:");
    mz700_screen_write_text(9, 16, "PRESS Q TO EXIT");

    while (1) {
        joy_x = mz700_joystick1_read_x();
        joy_y = mz700_joystick1_read_y();
        fire1 = mz700_joystick1_read_fire1();
        fire2 = mz700_joystick1_read_fire2();
        joy2_x = mz700_joystick2_read_x();
        joy2_y = mz700_joystick2_read_y();
        joy2_fire1 = mz700_joystick2_read_fire1();
        joy2_fire2 = mz700_joystick2_read_fire2();

        mz700_screen_render_u8(12, 6, joy_x);
        mz700_screen_render_u8(12, 8, joy_y);
        mz700_screen_write_text(13, 10, fire1 ? "ON " : "OFF");
        mz700_screen_write_text(13, 12, fire2 ? "ON " : "OFF");
        mz700_screen_render_u8(30, 6, joy2_x);
        mz700_screen_render_u8(30, 8, joy2_y);
        mz700_screen_write_text(31, 10, joy2_fire1 ? "ON " : "OFF");
        mz700_screen_write_text(31, 12, joy2_fire2 ? "ON " : "OFF");

        key = mz700_keyboard_read_key();
        key_text[0] = (key >= 32 && key <= 126) ? key : ' ';
        mz700_screen_write_text(19, 14, key_text);

        if (key == 'q' || key == 'Q') {
            break;
        }

        tiny_delay();
    }

    mz700_screen_write_text(15, 20, "STOPPED");
    return 0;
}
