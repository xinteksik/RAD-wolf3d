/* GPL-3.0-or-later. RWF1LE stores original RGB and explicit surface tags. */
#include "radwolf_capture.h"

static int capture_pending;
static unsigned next_capture;

int radwolf_capture_write(FILE *file, const uint32_t *pixels)
{
    static const unsigned char header[16] = {
        'R','W','F','1','L','E',0,0, 0x40,1,0xc8,0, 0,0xe8,3,0
    };
    if (!file || !pixels || fwrite(header, 1, 16, file) != 16) return -1;
    for (unsigned y = 0; y < 200; ++y) {
        unsigned char row[320 * 4];
        for (unsigned x = 0; x < 320; ++x) {
            uint32_t pixel = pixels[y * 320 + x];
            for (unsigned channel = 0; channel < 4; ++channel)
                row[x * 4 + channel] = (unsigned char)(pixel >> (channel * 8));
        }
        if (fwrite(row, 1, sizeof(row), file) != sizeof(row)) return -1;
    }
    return 0;
}

void radwolf_capture_request(void) { capture_pending = 1; }

void radwolf_capture_frame(const uint32_t *pixels)
{
    if (!capture_pending) return;
    capture_pending = 0;
    for (; next_capture < 10000; ++next_capture) {
        char path[48];
        snprintf(path, sizeof(path), "SD:RADWOLF/CAP%04u.RWF", next_capture);
        FILE *existing = fopen(path, "rb");
        if (existing) { fclose(existing); continue; }
        FILE *output = fopen(path, "wbx");
        if (!output) return;
        int result = radwolf_capture_write(output, pixels);
        if (fclose(output)) result = -1;
        printf("Video capture %s: %s\n", path, result ? "FAILED" : "OK");
        ++next_capture;
        return;
    }
}
