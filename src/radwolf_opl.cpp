/* GPL-2.0-or-later: Wolf register API backed by its DOSBox OPL core. */
/* dbopl.cpp declares these classes under pack(1); match its class ABI. */
#pragma pack(push, 1)
#include "../upstream/Wolf4SDL/dosbox/dbopl.h"
#pragma pack(pop)
extern "C" void radwolf_boot_trace(const char *);
#include "radwolf_midi.h"
static DBOPL::Chip chip;
extern "C" int YM3812Init(int count, int clock, int rate)
{
    (void)count; (void)clock;
    radwolf_boot_trace("OPL: setup begin");
    chip.Setup(rate);
    radwolf_boot_trace("OPL: setup complete");
    return 0;
}
extern "C" int YM3812Write(int which, int reg, int value)
{
    (void)which;
    chip.WriteReg(reg, value);
    return 0;
}
extern "C" void YM3812UpdateOne(int which, int16_t *samples, int count)
{
    (void)which;
    Bit32s buffer[512];
    while (count > 0) {
        int n = count > 512 ? 512 : count;
        chip.GenerateBlock2(n, buffer);
        for (int i = 0; i < n; ++i) {
            int value = buffer[i] * 4;
            if (value > 32767) value = 32767;
            if (value < -32768) value = -32768;
            *samples++ = value;
            *samples++ = value;
        }
        radwolf_midi_mix(samples - n * 2, (unsigned)n);
        count -= n;
    }
}
