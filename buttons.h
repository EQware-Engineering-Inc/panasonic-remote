#ifndef __BUTTONS_H__
#define __BUTTONS_H__

enum button {
    BTN_NA = 0xFFFF,
    POWER = 0x3D,
    EJECT = 0x01,
    STOP = 0x00,
    PLAY = 0x0A,
    PAUSE = 0x06,
    VSS = 0x9E,
    FF = 0x03,
    RW = 0x02,
    FORWARD = 0x4A,
    BACKWARD = 0x49,
    MENU = 0x80,
    UP = 0x85,
    DOWN = 0x86,
    LEFT = 0x87,
    RIGHT = 0x88,
    ENTER = 0x82,
    SUBTITLE = 0x91,
    ACTION = 0x56,
    TITLE = 0x9B,
};

extern volatile enum button current_button;

void init_buttons();
void wait_for_unpress();
void enable_button_interrupt();

#endif // __BUTTONS_H__
