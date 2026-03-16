#include "mz700_joystick.h"

extern uint8_t mz700_joy1_x(void);
extern uint8_t mz700_joy1_y(void);
extern uint8_t mz700_joy1_fire1(void);
extern uint8_t mz700_joy1_fire2(void);
extern uint8_t mz700_joy2_x(void);
extern uint8_t mz700_joy2_y(void);
extern uint8_t mz700_joy2_fire1(void);
extern uint8_t mz700_joy2_fire2(void);

uint8_t mz700_joystick1_read_x(void)
{
    return mz700_joy1_x();
}

uint8_t mz700_joystick1_read_y(void)
{
    return mz700_joy1_y();
}

uint8_t mz700_joystick1_read_fire1(void)
{
    return mz700_joy1_fire1();
}

uint8_t mz700_joystick1_read_fire2(void)
{
    return mz700_joy1_fire2();
}

uint8_t mz700_joystick2_read_x(void)
{
    return mz700_joy2_x();
}

uint8_t mz700_joystick2_read_y(void)
{
    return mz700_joy2_y();
}

uint8_t mz700_joystick2_read_fire1(void)
{
    return mz700_joy2_fire1();
}

uint8_t mz700_joystick2_read_fire2(void)
{
    return mz700_joy2_fire2();
}
