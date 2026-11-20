/* GPL-3.0-or-later. Host smoke test of the actual dual bare-metal engine.
 * Run only with separately supplied game data in a temporary SD:RADWOLF.
 * Hardware is stubbed; engine/cache/input/render/audio adapters are real.
 */
#include "wl_def.h"
#include "radwolf_audio_config.h"
#include "radwolf_wolf_event_input.h"
#include <setjmp.h>
#include <time.h>
extern void radwolf_wolf_main(void);
extern void radwolf_wl1_NewGame(int difficulty, int episode);
extern void radwolf_wl1_SetupGameLevel(void);
extern void radwolf_wl1_DrawPlayScreen(void);
extern void radwolf_wl1_ThreeDRefresh(void);
extern objtype *radwolf_wl1_player;
extern gametype radwolf_wl1_gamestate;
extern boolean radwolf_wl1_Keyboard[];
extern void radwolf_wl1_radwolf_wolf_input_poll(void);
extern void radwolf_wl1_radwolf_wolf_input_reset(void);
static jmp_buf menu_ready;
static int testing_map;
static unsigned frames;
static uint32_t framebuffer[320 * 200];
uint32_t *DG_ScreenBuffer = framebuffer;
uint64_t soundFirstuSecTick;
uint8_t soundRingBuffer[16384];
static uint8_t lookup[256];
const uint8_t *mahoneyLUT = lookup;
unsigned radwolf_sid2_address, radwolf_sid2_model;
unsigned radwolf_sid1_address = 0xd418, radwolf_sid1_model;
const unsigned char *radwolf_sid2_lut;
unsigned radwolf_sid1_boot_model(void) { return 8580; }
int radwolf_sid1_initialize(unsigned base, unsigned model)
{ radwolf_sid1_address = base + 0x18; radwolf_sid1_model = model; return 1; }
int radwolf_sid2_initialize(unsigned base, unsigned model)
{ (void)base; (void)model; return 0; }
void disableInterrupts(void) { }
void enableInterrupts(void) { }
void startSIDSamplePlayer(void) { }
void stopSIDSamplePlayer(void) { }
uint64_t GetuSec(void)
{
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return (uint64_t)now.tv_sec * 1000000u + now.tv_nsec / 1000u;
}
void radwolf_test_check_menu(void)
{
    if (testing_map) return;
    FILE *file = fopen("SD:RADWOLF/WOLFBOOT.LOG", "r");
    if (!file) return;
    char line[256];
    int ready = 0;
    while (fgets(line, sizeof(line), file))
        if (strstr(line, "menu: ready")) ready = 1;
    fclose(file);
    if (ready) longjmp(menu_ready, 1);
}
static int present(void) { ++frames; return 0; }
int (*functionAddress[])(void) = {NULL, present};
int main(int argc, char **argv)
{
    assert(argc == 2);
    int shareware = strcmp(argv[1], "WL1") == 0;
    for (unsigned i = 0; i < 256; ++i) lookup[i] = i;
    if (setjmp(menu_ready) == 0) radwolf_wolf_main();
    testing_map = 1;
    assert(frames > 0);
    if (shareware) {
        radwolf_wl1_radwolf_wolf_input_reset();
        radwolf_wolf_input_event(256u | 'A');
        radwolf_wl1_radwolf_wolf_input_poll();
        assert(radwolf_wl1_Keyboard[sc_A]);
        assert(!Keyboard[sc_A]);
        radwolf_wl1_NewGame(gd_easy, 0);
        radwolf_wl1_SetupGameLevel();
        assert(radwolf_wl1_player != NULL);
        assert(radwolf_wl1_gamestate.mapon == 0);
        radwolf_wl1_DrawPlayScreen();
        radwolf_wl1_ThreeDRefresh();
    } else {
        radwolf_wolf_input_reset();
        radwolf_wolf_input_event(256u | 'A');
        radwolf_wolf_input_poll();
        assert(Keyboard[sc_A]);
        assert(!radwolf_wl1_Keyboard[sc_A]);
        NewGame(gd_easy, 0);
        SetupGameLevel();
        assert(player != NULL);
        assert(gamestate.mapon == 0);
        DrawPlayScreen();
        ThreeDRefresh();
    }
    unsigned distinct = 0;
    for (unsigned i = 1; i < 320u * 200u; ++i)
        if (framebuffer[i] != framebuffer[0]) ++distinct;
    assert(distinct > 1000);
    FILE *capture = fopen("first-map.rgb", "wb");
    assert(capture != NULL);
    assert(fwrite(framebuffer, sizeof(framebuffer), 1, capture) == 1);
    assert(fclose(capture) == 0);
    printf("%s: actual menu, routed keyboard input, E1M1 and rendered frame passed (%u frames)\n",
           argv[1], frames);
    return 0;
}
