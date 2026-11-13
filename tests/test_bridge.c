#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "radwolf_frame.h"
#include "radwolf_input.h"
#include "radwolf_platform.h"
#include "radwolf_video.h"
#include "radwolf_rad_input.h"

typedef struct fake_keys {
    uint32_t held;
    unsigned events;
} fake_keys;

static void fake_key_event(void *user, enum radwolf_key key, int down)
{
    fake_keys *keys = user;
    uint32_t mask = UINT32_C(1) << key;
    assert(((keys->held & mask) != 0) != down);
    if (down) keys->held |= mask;
    else keys->held &= ~mask;
    ++keys->events;
}

typedef struct fake_video {
    radwolf_video *video;
    unsigned calls;
    uint32_t expected;
    int fail;
} fake_video;

static int fake_present(void *user, const uint32_t *rgb, size_t pitch)
{
    fake_video *fake = user;
    ++fake->calls;
    assert(pitch == RADWOLF_WIDTH);
    assert(rgb[0] == fake->expected);
    assert(rgb[RADWOLF_WIDTH * RADWOLF_HEIGHT - 1] == fake->expected);
    /* Reentrant callbacks must not overwrite the frame being consumed. */
    assert(radwolf_video_present(fake->video, NULL, 0, NULL) == -1);
    assert(rgb[0] == fake->expected);
    return fake->fail;
}

typedef struct fake_platform {
    uint64_t now;
    uint64_t slept;
    const char *logged;
} fake_platform;

static uint64_t fake_now(void *user)
{
    return ((fake_platform *) user)->now;
}

static void fake_sleep(void *user, uint64_t duration_us)
{
    ((fake_platform *) user)->slept += duration_us;
}

static void fake_log(void *user, const char *message)
{
    ((fake_platform *) user)->logged = message;
}

void test_files(void);
void test_map(void);
void test_gray(void);
void test_surface(void);

int main(void)
{
    test_files();
    test_map();
    test_gray();
    test_surface();
    static uint8_t src[RADWOLF_HEIGHT][RADWOLF_WIDTH + 5];
    static uint32_t dst[RADWOLF_HEIGHT][RADWOLF_WIDTH + 3];
    static radwolf_rgb pal[RADWOLF_PALETTE_SIZE];
    uint8_t raw[RADWOLF_PALETTE_SIZE * 4] = {0};

    raw[4] = 0x12; raw[5] = 0x34; raw[6] = 0x56;
    assert(radwolf_palette_from_rgba(pal, raw, 4) == 0);
    assert(pal[1].r == 0x12 && pal[1].g == 0x34 && pal[1].b == 0x56);
    src[0][0] = 1;
    src[199][319] = 1;
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 323; x++) dst[y][x] = 0xDEADBEEF;
    assert(radwolf_blit_indexed_320x200(&dst[0][0], 323, &src[0][0], 325, pal) == 0);
    assert(dst[0][0] == 0x00123456);
    assert(dst[199][319] == 0x00123456);
    assert(dst[0][1] == 0);
    assert(dst[0][320] == 0xDEADBEEF);
    assert(radwolf_blit_indexed_320x200(0, 323, &src[0][0], 325, pal) == -1);
    assert(radwolf_frame_last_error() == RADWOLF_FRAME_NULL_DST);
    assert(radwolf_blit_indexed_320x200(&dst[0][0], 319, &src[0][0], 325, pal) == -1);
    assert(radwolf_frame_last_error() == RADWOLF_FRAME_DST_PITCH);

    static uint8_t rgb24[RADWOLF_HEIGHT][RADWOLF_WIDTH * 3 + 4];
    memset(rgb24, 0xEE, sizeof(rgb24));
    assert(radwolf_blit_indexed_to_rgb24(&rgb24[0][0],
            RADWOLF_WIDTH * 3 + 4, &src[0][0], RADWOLF_WIDTH + 5, pal) == 0);
    assert(rgb24[0][0] == 0x12 && rgb24[0][1] == 0x34 && rgb24[0][2] == 0x56);
    assert(rgb24[199][319*3] == 0x12 && rgb24[199][319*3+2] == 0x56);
    assert(rgb24[0][RADWOLF_WIDTH*3] == 0xEE);
    assert(radwolf_blit_indexed_to_rgb24(0, RADWOLF_WIDTH*3,
            &src[0][0], RADWOLF_WIDTH+5, pal) == -1);
    assert(radwolf_blit_indexed_to_rgb24(&rgb24[0][0], RADWOLF_WIDTH*3-1,
            &src[0][0], RADWOLF_WIDTH+5, pal) == -1);

    radwolf_input_state state = {0};
    radwolf_input_apply(&state, ('W' | 0x100));
    assert(state.active & RW_MOVE_FORWARD);
    radwolf_input_apply(&state, ('w' | 0x100));
    radwolf_input_apply(&state, 'W');
    assert((state.active & RW_MOVE_FORWARD) == 0);
    radwolf_input_apply(&state, (' ' | 0x100));
    assert(state.active & RW_USE);
    radwolf_input_apply(&state, ' ');
    assert(!(state.active & RW_USE));
    uint32_t previous_keys = 0;
    fake_keys keys = {0, 0};
    radwolf_input_sync_wolf(&previous_keys, RW_MOVE_FORWARD | RW_FIRE,
                            fake_key_event, &keys);
    assert(keys.events == 2);
    assert(keys.held == ((1u << RW_KEY_UP) | (1u << RW_KEY_FIRE)));
    radwolf_input_sync_wolf(&previous_keys, RW_MOVE_FORWARD | RW_FIRE,
                            fake_key_event, &keys);
    assert(keys.events == 2);
    radwolf_input_sync_wolf(&previous_keys, RW_STRAFE_LEFT | RW_TURN_LEFT,
                            fake_key_event, &keys);
    assert(keys.held == ((1u << RW_KEY_LEFT) | (1u << RW_KEY_STRAFE)));
    radwolf_input_sync_wolf(&previous_keys, RW_TURN_LEFT, fake_key_event, &keys);
    assert(keys.held == (1u << RW_KEY_LEFT));
    radwolf_input_sync_wolf(&previous_keys, 0, fake_key_event, &keys);
    assert(keys.held == 0 && previous_keys == 0);
    radwolf_input_apply(&state, 27 | 0x100);
    radwolf_input_apply(&state, '4' | 0x100);
    assert(state.active == (RW_ESCAPE | RW_WEAPON_4));
    radwolf_rad_input rad_input = {{0}, 0};
    radwolf_rad_input_event(&rad_input, RW_RAD_UP | 256);
    radwolf_rad_input_event(&rad_input, 'I' | 256);
    radwolf_rad_input_event(&rad_input, RW_RAD_UP);
    assert(radwolf_rad_input_actions(&rad_input) == RW_MOVE_FORWARD);
    radwolf_rad_input_event(&rad_input, 'i');
    assert(radwolf_rad_input_actions(&rad_input) == 0);
    radwolf_rad_input_event(&rad_input, RW_RAD_SHIFT_L | 256);
    radwolf_rad_input_event(&rad_input, RW_RAD_SHIFT_R | 256);
    radwolf_rad_input_event(&rad_input, RW_RAD_SHIFT_L);
    assert(radwolf_rad_input_actions(&rad_input) == RW_RUN);
    radwolf_rad_input_event(&rad_input, RW_RAD_SHIFT_R);
    radwolf_rad_input_event(&rad_input, RW_RAD_COMMODORE | 256);
    radwolf_rad_input_joystick(&rad_input, 0xEE); /* up and fire */
    radwolf_rad_input_event(&rad_input, RW_RAD_COMMODORE);
    assert(radwolf_rad_input_actions(&rad_input) == (RW_MOVE_FORWARD | RW_FIRE));
    radwolf_rad_input_joystick(&rad_input, 0xFF);
    assert(radwolf_rad_input_actions(&rad_input) == 0);
    radwolf_rad_input_event(&rad_input, RW_RAD_RETURN | 256);
    radwolf_rad_input_event(&rad_input, RW_RAD_ESCAPE | 256);
    radwolf_rad_input_event(&rad_input, RW_RAD_SPACE | 256);
    radwolf_rad_input_event(&rad_input, '3' | 256);
    assert(radwolf_rad_input_actions(&rad_input) ==
           (RW_CONFIRM | RW_ESCAPE | RW_USE | RW_WEAPON_3));
    radwolf_rad_input_reset(&rad_input);
    radwolf_rad_input_event(&rad_input, 0x10000 | 'W' | 256);
    assert(radwolf_rad_input_actions(&rad_input) == 0);
    radwolf_rad_input_event(&rad_input, RW_RAD_F1 | 256);
    assert(radwolf_rad_input_actions(&rad_input) == RW_MAP);
    uint32_t map_previous = 0;
    fake_keys map_keys = {0, 0};
    radwolf_input_sync_wolf(&map_previous, RW_MAP, fake_key_event, &map_keys);
    assert(map_keys.held == (UINT32_C(1) << RW_KEY_MAP));
    radwolf_rad_input_event(&rad_input, RW_RAD_F1);
    radwolf_input_sync_wolf(&map_previous, radwolf_rad_input_actions(&rad_input), fake_key_event, &map_keys);
    assert(map_keys.held == 0 && map_keys.events == 2);

    fake_platform fake = {100, 0, 0};
    radwolf_platform platform;
    const radwolf_platform_ops ops = {fake_now, fake_sleep, fake_log};
    assert(radwolf_platform_init(&platform, &ops, &fake) == 0);
    assert(radwolf_platform_now_us(&platform) == 100);
    fake.now = 90;
    assert(radwolf_platform_now_us(&platform) == 100);
    fake.now = 120;
    assert(radwolf_platform_now_us(&platform) == 120);
    radwolf_platform_sleep_us(&platform, 25);
    assert(fake.slept == 25);
    radwolf_platform_log(&platform, "ready");
    assert(strcmp(fake.logged, "ready") == 0);
    fake.now = 1000000;
    assert(radwolf_platform_now_ms(&platform) == 1000);
    assert(radwolf_platform_ticks70(&platform) == 70);
    fake.now = UINT64_C(700000000000);
    assert(radwolf_platform_ticks70(&platform) == 49000000);
    fake.now = (UINT64_C(1) << 32) * 1000;
    assert(radwolf_platform_now_ms(&platform) == 0);
    assert(radwolf_platform_ticks70(&platform) == 300647710);
    assert(radwolf_platform_init(&platform, 0, &fake) == -1);

    static struct {
        uint32_t before;
        uint32_t pixels[RADWOLF_WIDTH * RADWOLF_HEIGHT];
        uint32_t after;
    } guarded;
    guarded.before = guarded.after = 0xDEADBEEF;
    radwolf_video video;
    fake_video display = {&video, 0, 0x123456, 0};
    assert(radwolf_video_init(&video, guarded.pixels,
            RADWOLF_WIDTH * RADWOLF_HEIGHT - 1, fake_present, &display) == -1);
    assert(radwolf_video_present(&video, &src[0][0], 325, pal) == -1);
    assert(radwolf_video_init(&video, guarded.pixels,
            RADWOLF_WIDTH * RADWOLF_HEIGHT, fake_present, &display) == 0);
    assert(radwolf_video_present(&video, &src[0][0], 325, pal) == 0);
    assert(video.presented == 1 && video.rejected == 1);
    pal[1].r = 0xAB;
    display.expected = 0xAB3456;
    assert(radwolf_video_present(&video, &src[0][0], 325, pal) == 0);
    assert(radwolf_video_present(&video, NULL, 325, pal) == -1);
    assert(display.calls == 2);
    display.fail = -1;
    assert(radwolf_video_present(&video, &src[0][0], 325, pal) == -1);
    assert(video.presented == 2 && video.rejected == 5 && !video.busy);
    assert(guarded.before == 0xDEADBEEF && guarded.after == 0xDEADBEEF);
    puts("RAD-Wolf bridge host tests: PASS");
    return 0;
}
