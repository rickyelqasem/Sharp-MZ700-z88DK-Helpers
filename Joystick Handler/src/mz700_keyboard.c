#include "mz700_keyboard.h"

#define MZ700_KBD_STROBE (*(volatile uint8_t *)0xE000)
#define MZ700_KBD_READ   (*(volatile uint8_t *)0xE001)

extern void mz700_bank_vram_mmio(void);
extern void mz700_bank_ram(void);

typedef struct {
    uint8_t row;
    uint8_t mask;
    char value;
} mz700_key_entry_t;

static const mz700_key_entry_t key_map[] = {
    {0xF0, 0x01, '\n'},
    {0xF0, 0x02, ':'},
    {0xF0, 0x04, ';'},
    {0xF0, 0x20, '|'},
    {0xF4, 0x10, 'D'},
    {0xF4, 0x08, 'E'},
    {0xF4, 0x04, 'F'},
    {0xF4, 0x02, 'G'},
    {0xF4, 0x01, 'H'},
    {0xF4, 0x20, 'C'},
    {0xF4, 0x40, 'B'},
    {0xF4, 0x80, 'A'},
    {0xF3, 0x01, 'P'},
    {0xF3, 0x02, 'O'},
    {0xF3, 0x04, 'N'},
    {0xF3, 0x08, 'M'},
    {0xF3, 0x10, 'L'},
    {0xF3, 0x20, 'K'},
    {0xF3, 0x40, 'J'},
    {0xF3, 0x80, 'I'},
    {0xF2, 0x01, 'X'},
    {0xF2, 0x02, 'W'},
    {0xF2, 0x04, 'V'},
    {0xF2, 0x08, 'U'},
    {0xF2, 0x10, 'T'},
    {0xF2, 0x20, 'S'},
    {0xF2, 0x40, 'R'},
    {0xF2, 0x80, 'Q'},
    {0xF1, 0x08, ']'},
    {0xF1, 0x10, '['},
    {0xF1, 0x20, '@'},
    {0xF1, 0x40, 'Z'},
    {0xF1, 0x80, 'Y'},
    {0xF5, 0x01, '8'},
    {0xF5, 0x02, '7'},
    {0xF5, 0x04, '6'},
    {0xF5, 0x08, '5'},
    {0xF5, 0x10, '4'},
    {0xF5, 0x20, '3'},
    {0xF5, 0x40, '2'},
    {0xF5, 0x80, '1'},
    {0xF6, 0x04, '9'},
    {0xF6, 0x08, '0'},
    {0xF6, 0x10, ' '},
    {0xF6, 0x20, '-'},
    {0xF6, 0x40, '^'},
    {0xF6, 0x80, '\\'},
    {0xF6, 0x01, '.'},
    {0xF6, 0x02, ','},
    {0xF7, 0x01, '/'},
    {0xF7, 0x02, '?'},
    {0xF7, 0x04, 'L'},
    {0xF7, 0x08, 'R'},
    {0xF7, 0x10, 'D'},
    {0xF7, 0x20, 'U'}
};

static uint8_t mz700_keyboard_read_row(uint8_t row)
{
    uint8_t value;

    mz700_bank_vram_mmio();
    MZ700_KBD_STROBE = row;
    __asm__("nop");
    value = MZ700_KBD_READ;
    mz700_bank_ram();

    return value;
}

char mz700_keyboard_read_key(void)
{
    uint8_t i;
    uint8_t row_value;
    uint8_t current_row;

    current_row = 0xFF;
    row_value = 0xFF;

    for (i = 0; i < (uint8_t)(sizeof(key_map) / sizeof(key_map[0])); ++i) {
        if (key_map[i].row != current_row) {
            current_row = key_map[i].row;
            row_value = mz700_keyboard_read_row(current_row);
        }

        if ((row_value & key_map[i].mask) == 0) {
            return key_map[i].value;
        }
    }

    return 0;
}
