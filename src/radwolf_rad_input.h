/* RAD-Wolf bring-up: GPL-3.0-or-later. */
#ifndef RADWOLF_RAD_INPUT_H
#define RADWOLF_RAD_INPUT_H
#include "radwolf_input.h"

#ifdef __cplusplus
extern "C" {
#endif

/* readKeyDoom keyTable codes; event bit 8 means down. */
enum radwolf_rad_key {
    RW_RAD_SHIFT_L = 1, RW_RAD_COMMODORE = 4, RW_RAD_SHIFT_R = 6,
    RW_RAD_RETURN = 13, RW_RAD_DOWN = 17, RW_RAD_SPACE = 32,
    RW_RAD_ESCAPE = 95, RW_RAD_UP = 145, RW_RAD_LEFT = 157,
    RW_RAD_RIGHT = 29
};

typedef struct radwolf_rad_input {
    uint8_t held[32];
    uint8_t joystick;
} radwolf_rad_input;

/* Zero initialization means no input. Keyboard and joystick are combined. */
void radwolf_rad_input_event(radwolf_rad_input *input, uint32_t event);
/* CIA joystick byte: bits 0..4 up/down/left/right/fire, active low.
 * Caller must supply an isolated port-2 reading, not a keyboard matrix row. */
void radwolf_rad_input_joystick(radwolf_rad_input *input, uint8_t cia_value);
uint32_t radwolf_rad_input_actions(const radwolf_rad_input *input);
void radwolf_rad_input_reset(radwolf_rad_input *input);
#ifdef __cplusplus
}
#endif
#endif
