/* RAD-Wolf bring-up: poll the C64 matrix through the known RAD-Doom scanner. */
#include "radwolf_rad_input.h"

extern void readKeyDoom(unsigned int *events, unsigned char *count);
extern "C" void disableInterrupts();
extern "C" void enableInterrupts();
extern "C" uint64_t GetuSec(void);
extern "C" void radwolf_audio_pump(void);
extern "C" void radwolf_boot_trace(const char *);

static radwolf_rad_input input = {};

/* Both the blitter and explicit polls advance the scanner's previous matrix. */
extern "C" void radwolf_wolf_capture_keys(const unsigned int *events,
                                         unsigned char count)
{
    for (unsigned int i = 0; i < count; ++i)
        radwolf_rad_input_event(&input, events[i]);
}

extern "C" uint32_t radwolf_wolf_milliseconds(void)
{
    return static_cast<uint32_t>(GetuSec() / 1000u);
}

extern "C" uint32_t radwolf_wolf_ticks70(void)
{
    uint64_t us = GetuSec();
    return static_cast<uint32_t>((us / 1000000u) * 70u +
                                 ((us % 1000000u) * 70u) / 1000000u);
}

extern "C" void radwolf_wolf_delay(uint32_t milliseconds)
{
    uint64_t deadline = GetuSec() + static_cast<uint64_t>(milliseconds) * 1000u;
    while (GetuSec() < deadline) {
        radwolf_audio_pump();
        asm volatile("yield" ::: "memory");
    }
}

extern "C" unsigned int radwolf_wolf_poll_actions(void)
{
    static bool traced = false;
    if (!traced) radwolf_boot_trace("input: first poll begin");
    radwolf_audio_pump();
    if (!traced) radwolf_boot_trace("input: audio complete");
    unsigned int events[64];
    unsigned char count = 0;
    disableInterrupts();
    readKeyDoom(events, &count);
    enableInterrupts();
    if (!traced) radwolf_boot_trace("input: scan complete");
    traced = true;
    return radwolf_rad_input_actions(&input);
}
