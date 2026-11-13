/* RAD-Wolf bring-up: GPL-3.0-or-later. */
#include "radwolf_rad_input.h"
#include <string.h>

static uint32_t key_action(unsigned key)
{
    switch (key) {
        case 'I': return RW_MOVE_FORWARD;
        case 'K': return RW_MOVE_BACK;
        case 'J': return RW_TURN_LEFT;
        case 'L': return RW_TURN_RIGHT;
        case 'Z': return RW_STRAFE_LEFT;
        case 'X': return RW_STRAFE_RIGHT;
        case RW_RAD_COMMODORE: return RW_FIRE;
        case RW_RAD_SPACE: return RW_USE;
        case 'R': case RW_RAD_SHIFT_L: case RW_RAD_SHIFT_R: return RW_RUN;
        case RW_RAD_ESCAPE: return RW_ESCAPE;
        case RW_RAD_RETURN: return RW_CONFIRM;
        case '1': return RW_WEAPON_1;
        case '2': return RW_WEAPON_2;
        case '3': return RW_WEAPON_3;
        case '4': return RW_WEAPON_4;
        case RW_RAD_F1: return RW_MAP;
        default: return 0;
    }
}

void radwolf_rad_input_event(radwolf_rad_input *input, uint32_t event)
{
    if (input == NULL || (event & ~UINT32_C(0x1ff)) != 0)
        return;
    unsigned key = event & 255u;
    if (key >= 'a' && key <= 'z') key -= 'a' - 'A';
    uint8_t mask = (uint8_t)(1u << (key & 7));
    if ((event & 256u) != 0) input->held[key >> 3] |= mask;
    else input->held[key >> 3] &= (uint8_t)~mask;
}

void radwolf_rad_input_joystick(radwolf_rad_input *input, uint8_t cia_value)
{
    if (input != NULL) input->joystick = (uint8_t)(~cia_value & 31u);
}

uint32_t radwolf_rad_input_actions(const radwolf_rad_input *input)
{
    if (input == NULL) return 0;
    uint32_t actions = 0;
    for (unsigned key = 0; key < 256; ++key)
        if ((input->held[key >> 3] & (1u << (key & 7))) != 0)
            actions |= key_action(key);
    static const uint32_t joy_actions[5] = {
        RW_MOVE_FORWARD, RW_MOVE_BACK, RW_TURN_LEFT, RW_TURN_RIGHT, RW_FIRE
    };
    for (unsigned bit = 0; bit < 5; ++bit)
        if ((input->joystick & (1u << bit)) != 0)
            actions |= joy_actions[bit];
    return actions;
}

void radwolf_rad_input_reset(radwolf_rad_input *input)
{
    if (input != NULL) memset(input, 0, sizeof(*input));
}
