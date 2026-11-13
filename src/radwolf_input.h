/* RAD-Wolf bring-up: GPL-3.0-or-later. Original adapter implementation. */
#ifndef RADWOLF_INPUT_H
#define RADWOLF_INPUT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Actions independent of Wolf4SDL keycode / SDL translation. */
enum radwolf_action {
    RW_MOVE_FORWARD = 1u << 0,
    RW_MOVE_BACK = 1u << 1,
    RW_TURN_LEFT = 1u << 2,
    RW_TURN_RIGHT = 1u << 3,
    RW_STRAFE_LEFT = 1u << 4,
    RW_STRAFE_RIGHT = 1u << 5,
    RW_FIRE = 1u << 6,
    RW_USE = 1u << 7,
    RW_RUN = 1u << 8,
    RW_ESCAPE = 1u << 9,
    RW_CONFIRM = 1u << 10,
    RW_WEAPON_1 = 1u << 11,
    RW_WEAPON_2 = 1u << 12,
    RW_WEAPON_3 = 1u << 13,
    RW_WEAPON_4 = 1u << 14,
    RW_MAP = 1u << 15
};

typedef struct radwolf_input_state {
    uint32_t active;
} radwolf_input_state;

/*
 * Event format follows readKeyDoom(): low byte keycode, bit 8 = press.
 * The bare-metal RAD adapter maps only Doom-style IJKL movement, Z/X
 * strafing, and discrete C64 action keys. Joystick input stays disabled.
 */
void radwolf_input_apply(radwolf_input_state *state, uint32_t event);

/* Logical keys, independent of SDL and the C64 matrix. */
enum radwolf_key {
    RW_KEY_UP, RW_KEY_DOWN, RW_KEY_LEFT, RW_KEY_RIGHT,
    RW_KEY_FIRE, RW_KEY_USE, RW_KEY_RUN, RW_KEY_STRAFE,
    RW_KEY_ESCAPE, RW_KEY_CONFIRM, RW_KEY_1, RW_KEY_2, RW_KEY_3, RW_KEY_4,
    RW_KEY_MAP, RW_KEY_COUNT
};
typedef void (*radwolf_key_event_fn)(void *user, enum radwolf_key key, int down);

/* previous_keys starts at zero. Dispatch only changed key levels. Dispatching
 * actions=0 releases all held keys (e.g. on disconnect/focus loss). Callback
 * must consume synchronously; callers serialize access and must not reenter. */
void radwolf_input_sync_wolf(uint32_t *previous_keys, uint32_t actions,
                             radwolf_key_event_fn event, void *user);
#ifdef __cplusplus
}
#endif
#endif
