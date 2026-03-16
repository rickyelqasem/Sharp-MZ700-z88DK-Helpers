#ifndef MZ700_SCREEN_H
#define MZ700_SCREEN_H

#include <stdint.h>

void mz700_screen_clear(void);
void mz700_screen_write_text(uint8_t x, uint8_t y, const char *text);
void mz700_screen_render_u8(uint8_t x, uint8_t y, uint8_t value);

#endif
