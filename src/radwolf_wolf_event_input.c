/* RAD-Wolf bring-up: map verified RAD keyboard actions into Wolf scan codes. */
#include "radwolf_wolf_event_input.h"
#include "radwolf_input.h"
#include "baremetal/SDL.h"
#include "../upstream/Wolf4SDL/wl_def.h"

extern uint32_t radwolf_wolf_poll_actions(void);

static uint32_t previous_keys;
static int text_mode;
/* Keep short taps captured during screen blits until the menu consumes them. */
static uint32_t raw_events[256];
static unsigned raw_read, raw_write;

void radwolf_wolf_input_event(uint32_t event)
{
    if ((event & ~UINT32_C(0x1ff)) != 0) return;
    unsigned next = (raw_write + 1u) % 256u;
    if (next != raw_read) {
        raw_events[raw_write] = event;
        raw_write = next;
    }
}

void radwolf_wolf_input_text_mode(int enabled)
{
    text_mode = enabled != 0;
    raw_read = raw_write;
    radwolf_wolf_input_set_actions(0);
}

static void apply_raw_event(uint32_t event)
{
    unsigned key = event & 255u;
    int down = (event & 256u) != 0;
    ScanCode scan = sc_None;
    if (key >= 'a' && key <= 'z') key -= 'a' - 'A';
    if (key >= 'A' && key <= 'Z') scan = sc_A + key - 'A';
    else if (key >= '1' && key <= '9') scan = sc_1 + key - '1';
    else if (key == '0') scan = sc_0;
    /* Action keys already have scan codes outside text entry. */
    else if (text_mode) {
        switch (key) {
            case 13: scan = sc_Return; break;
            case 20: scan = sc_BackSpace; break; /* C64 INST/DEL */
            case 19: scan = sc_Home; break;
            case 95: scan = sc_Escape; break; /* C64 left arrow */
            case 29: scan = sc_RightArrow; break;
            case 157: scan = sc_LeftArrow; break;
            case ' ': scan = sc_Space; break;
        }
    }
    if (scan != sc_None) {
        Keyboard[scan] = down;
        if (down) LastScan = scan;
    }
    if (down && key >= 32 && key <= 94) {
        textinput[0] = (char)key;
        textinput[1] = 0;
    }
}

static void set_wolf_key(void *user, enum radwolf_key key, int down)
{
    static const ScanCode scans[RW_KEY_COUNT] = {
        sc_UpArrow, sc_DownArrow, sc_LeftArrow, sc_RightArrow,
        sc_Control, sc_Space, sc_LShift, sc_Alt, sc_Escape, sc_Return,
        sc_1, sc_2, sc_3, sc_4, sc_F1, sc_F2
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
    uint32_t actions = radwolf_wolf_poll_actions();
    radwolf_wolf_input_set_actions(text_mode ? 0 : actions);
    /* One edge per poll preserves down/up taps and character order. */
    if (raw_read != raw_write) {
        uint32_t event = raw_events[raw_read];
        raw_read = (raw_read + 1u) % 256u;
        apply_raw_event(event);
    }
}

void radwolf_wolf_input_reset(void)
{
    radwolf_wolf_input_set_actions(0);
    previous_keys = 0;
    raw_read = raw_write = 0;
    text_mode = 0;
}
