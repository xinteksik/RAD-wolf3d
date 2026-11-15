/* Exercise the firmware keyboard bridge with Wolf4SDL's actual scan codes. */
#include "baremetal/SDL.h"
#include "../upstream/Wolf4SDL/wl_def.h"
#include "radwolf_wolf_event_input.h"
#include "radwolf_input.h"
#include <assert.h>
#include <stdio.h>

boolean Keyboard[sc_Last];
ScanCode LastScan;
char textinput[TEXTINPUTSIZE];
int px, py;
byte fontcolor, backcolor;
static uint32_t actions;
static unsigned script[256], script_count, script_pos;
static uint32_t ticks;

void VW_MeasurePropString(const char *text, word *w, word *h)
{
    *w = (word)(strlen(text) * 6);
    *h = 8;
}
void VWB_DrawPropString(const char *text) { (void)text; }
SDL_Surface *screenBuffer;
void VH_UpdateScreen(SDL_Surface *surface) { (void)surface; }
uint32_t radwolf_wolf_milliseconds(void) { ticks += 15; return ticks; }
void radwolf_wolf_delay(uint32_t ms) { (void)ms; }
void IN_ClearTextInput(void) { memset(textinput, 0, sizeof(textinput)); }
void IN_ClearKeysDown(void) {
    memset(Keyboard, 0, sizeof(Keyboard));
    LastScan = sc_None;
}
void ReadAnyControl(ControlInfo *ci)
{
    assert(script_pos < script_count);
    radwolf_wolf_input_event(script[script_pos++]);
    radwolf_wolf_input_poll();
    memset(ci, 0, sizeof(*ci));
    ci->dir = dir_None;
    if (Keyboard[sc_UpArrow]) ci->dir = dir_North;
    if (Keyboard[sc_LeftArrow]) ci->dir = dir_West;
}
static void tap(unsigned key)
{
    script[script_count++] = key | 256u;
    script[script_count++] = key;
}
uint32_t radwolf_wolf_poll_actions(void) { return actions; }

static void edge(unsigned key, int down)
{
    radwolf_wolf_input_event(key | (down ? 256u : 0u));
    radwolf_wolf_input_poll();
}

int main(void)
{
    /* Both confirmations and releases, including taps captured during blits. */
    radwolf_wolf_input_event('Y' | 256u);
    radwolf_wolf_input_event('Y');
    radwolf_wolf_input_poll();
    assert(Keyboard[sc_Y] && LastScan == sc_Y);
    radwolf_wolf_input_poll();
    assert(!Keyboard[sc_Y]);
    edge('n', 1);
    assert(Keyboard[sc_N]);
    edge('N', 0);
    assert(!Keyboard[sc_N]);

    actions = RW_MOVE_FORWARD | RW_TURN_LEFT | RW_USE | RW_CAPTURE;
    radwolf_wolf_input_poll();
    assert(Keyboard[sc_UpArrow] && Keyboard[sc_Space] && Keyboard[sc_F2]);
    radwolf_wolf_input_text_mode(1);
    assert(!Keyboard[sc_UpArrow] && !Keyboard[sc_Space] && !Keyboard[sc_F2]);
    const char *name = "IJKL R P 0123456789";
    for (const char *c = name; *c; ++c) {
        textinput[0] = 0;
        edge((unsigned)*c, 1);
        assert(textinput[0] == *c && textinput[1] == 0);
        assert(!Keyboard[sc_UpArrow] && !Keyboard[sc_LeftArrow]);
        assert(!Keyboard[sc_F2] && !Keyboard[sc_Control]);
        edge((unsigned)*c, 0);
    }
    textinput[0] = 0;
    edge(20, 1);
    assert(LastScan == sc_BackSpace && !textinput[0]);
    edge(20, 0);
    edge(13, 1);
    assert(LastScan == sc_Return && Keyboard[sc_Return]);
    edge(13, 0);
    edge(95, 1);
    assert(LastScan == sc_Escape && Keyboard[sc_Escape] && !textinput[0]);
    edge(95, 0);
    radwolf_wolf_input_text_mode(0);
    radwolf_wolf_input_poll();
    assert(Keyboard[sc_UpArrow] && Keyboard[sc_F2]);
    radwolf_wolf_input_reset();
    /* Run the actual engine name-entry dialog, with rendering stubbed out. */
    const char *typed = "IJKL R P 19";
    for (const char *c = typed; *c; ++c) tap((unsigned)*c);
    tap(20);
    tap('5');
    tap(13);
    char saved[32] = "untouched";
    assert(US_LineInput(0, 0, saved, "", true, 31, 250));
    assert(strcmp(saved, "IJKL R P 15") == 0);
    script_count = script_pos = 0;
    tap('X');
    tap(95);
    assert(!US_LineInput(0, 0, saved, "OLD", true, 31, 250));
    assert(strcmp(saved, "IJKL R P 15") == 0);
    puts("Menu keyboard and engine line-input tests passed");
    return 0;
}
