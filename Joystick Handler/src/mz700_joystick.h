#ifndef MZ700_JOYSTICK_H
#define MZ700_JOYSTICK_H

#include <stdint.h>

uint8_t mz700_joystick1_read_x(void);
uint8_t mz700_joystick1_read_y(void);
uint8_t mz700_joystick1_read_fire1(void);
uint8_t mz700_joystick1_read_fire2(void);
uint8_t mz700_joystick2_read_x(void);
uint8_t mz700_joystick2_read_y(void);
uint8_t mz700_joystick2_read_fire1(void);
uint8_t mz700_joystick2_read_fire2(void);

#endif
