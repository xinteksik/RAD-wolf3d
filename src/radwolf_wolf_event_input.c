/* RAD-Wolf bring-up: map verified RAD keyboard actions into Wolf scan codes. */
#include "radwolf_wolf_event_input.h"
#include "radwolf_input.h"
#include "baremetal/SDL.h"
#include "../upstream/Wolf4SDL/wl_def.h"

extern uint32_t radwolf_wolf_poll_actions(void);

static uint32_t previous_keys;

static void set_wolf_key(void *user, enum radwolf_key key, int down)
{
    static const ScanCode scans[RW_KEY_COUNT] = {
        sc_UpArrow, sc_DownArrow, sc_LeftArrow, sc_RightArrow,
        sc_Control, sc_Space, sc_LShift, sc_Alt, sc_Escape, sc_Return,
        sc_1, sc_2, sc_3, sc_4
    };
    (void)user;
    if (key >= RW_KEY_COUNT)
        return;
    Keyboard[scans[key]] = down != 0;
    if (down)
        LastScan = scans[key];
}

void radwolf_wolf_input_set_actions(uint32_t actions)
{
    radwolf_input_sync_wolf(&previous_keys, actions, set_wolf_key, NULL);
}

void radwolf_wolf_input_poll(void)
{
    radwolf_wolf_input_set_actions(radwolf_wolf_poll_actions());
}

void radwolf_wolf_input_reset(void)
{
    radwolf_wolf_input_set_actions(0);
    previous_keys = 0;
}
