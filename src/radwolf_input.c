/* RAD-Wolf bring-up: GPL-3.0-or-later. Original adapter implementation. */
#include "radwolf_input.h"

void radwolf_input_apply(radwolf_input_state *state, uint32_t event)
{
    if (state == 0) return;
    unsigned key = event & 0xffu;
    const int down = (event & 0x100u) != 0;
    if (key >= 'a' && key <= 'z') key -= ('a' - 'A');
    uint32_t action = 0;
    switch (key) {
        case 'W': action = RW_MOVE_FORWARD; break;
        case 'S': action = RW_MOVE_BACK; break;
        case 'A': action = RW_TURN_LEFT; break;
        case 'D': action = RW_TURN_RIGHT; break;
        case 'Q': action = RW_STRAFE_LEFT; break;
        case 'E': action = RW_STRAFE_RIGHT; break;
        case ' ': action = RW_USE; break;
        case 'F': action = RW_FIRE; break;
        case 'R': action = RW_RUN; break;
        case 27: action = RW_ESCAPE; break;
        case 13: action = RW_CONFIRM; break;
        case '1': action = RW_WEAPON_1; break;
        case '2': action = RW_WEAPON_2; break;
        case '3': action = RW_WEAPON_3; break;
        case '4': action = RW_WEAPON_4; break;
        default: break;
    }
    if (down) state->active |= action;
    else state->active &= ~action;
}

void radwolf_input_sync_wolf(uint32_t *previous_keys, uint32_t actions,
                             radwolf_key_event_fn event, void *user)
{
    static const uint32_t bindings[RW_KEY_COUNT] = {
        RW_MOVE_FORWARD, RW_MOVE_BACK,
        RW_TURN_LEFT | RW_STRAFE_LEFT, RW_TURN_RIGHT | RW_STRAFE_RIGHT,
        RW_FIRE, RW_USE, RW_RUN, RW_STRAFE_LEFT | RW_STRAFE_RIGHT,
        RW_ESCAPE, RW_CONFIRM, RW_WEAPON_1, RW_WEAPON_2,
        RW_WEAPON_3, RW_WEAPON_4
    };
    if (previous_keys == 0 || event == 0)
        return;
    uint32_t keys = 0;
    for (unsigned i = 0; i < RW_KEY_COUNT; ++i)
        if ((actions & bindings[i]) != 0)
            keys |= UINT32_C(1) << i;
    uint32_t changed = keys ^ *previous_keys;
    *previous_keys = keys;
    for (unsigned i = 0; i < RW_KEY_COUNT; ++i)
        if ((changed & (UINT32_C(1) << i)) != 0)
            event(user, (enum radwolf_key)i,
                  (keys & (UINT32_C(1) << i)) != 0);
}
