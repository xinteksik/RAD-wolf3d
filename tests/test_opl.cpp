/* Verify the firmware OPL bridge generates interleaved, bounded stereo. */
#include <cassert>
#include <cstdint>
#include <cstdio>
extern "C" int YM3812Init(int, int, int);
extern "C" int YM3812Write(int, int, int);
extern "C" void YM3812UpdateOne(int, int16_t *, int);
extern "C" void radwolf_boot_trace(const char *) { }
int main()
{
    assert(YM3812Init(1, 3579545, 22050) == 0);
    const int registers[][2] = {
        {1, 0x20}, {0x20, 1}, {0x23, 1}, {0x40, 0x3f}, {0x43, 0},
        {0x60, 0xf0}, {0x63, 0xf0}, {0x80, 0x77}, {0x83, 0x77},
        {0xc0, 0}, {0xa0, 0x98}, {0xb0, 0x31}
    };
    for (const auto &reg : registers) YM3812Write(0, reg[0], reg[1]);
    int16_t samples[2 * 1000 + 2] = {};
    samples[2000] = 12345; samples[2001] = -12345;
    YM3812UpdateOne(0, samples, 1000);
    unsigned energy = 0;
    for (unsigned i = 0; i < 1000; ++i) {
        assert(samples[2*i] == samples[2*i+1]);
        energy += samples[2*i] < 0 ? -samples[2*i] : samples[2*i];
    }
    assert(energy > 1000);
    assert(samples[2000] == 12345 && samples[2001] == -12345);
    std::puts("OPL bridge tests passed");
}
