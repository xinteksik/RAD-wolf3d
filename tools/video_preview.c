/* GPL-3.0-or-later. Host entry point using the firmware's real VIC-II packer.
 * Only the three-channel NEON arithmetic has a scalar host equivalent.
 * The hardware compilation keeps the original NEON path and constants. */
#define RADWOLF_VIDEO_HOST 1
#include "../src/radwolf_rad_video.c"
#include <stdio.h>
#include <stdlib.h>

void blitScreenDOOM(uint8_t *image, uint32_t *events, uint8_t *count, uint8_t *mouse)
{ (void)image; (void)events; (void)count; (void)mouse; }

int main(int argc, char **argv)
{
    if (argc != 8) {
        fprintf(stderr, "Usage: video_preview frame.rwf output.koa brightness dither%% exposure keep-tags frames\n");
        return 2;
    }
    unsigned char header[16];
    FILE *input = fopen(argv[1], "rb");
    if (!input || fread(header, 1, 16, input) != 16 ||
        memcmp(header, "RWF1LE\0\0\x40\x01\xc8\0\0\xe8\x03\0", 16) != 0) {
        fprintf(stderr, "Expected RWF1LE 320x200 little-endian frame\n");
        return 1;
    }
    dg_Create();
    for (unsigned i = 0; i < 320u * 200u; ++i) {
        unsigned char pixel[4];
        if (fread(pixel, 1, 4, input) != 4) return 1;
        DG_ScreenBuffer[i] = (uint32_t)pixel[0] | ((uint32_t)pixel[1] << 8) |
                            ((uint32_t)pixel[2] << 16) | ((uint32_t)pixel[3] << 24);
        if (!atoi(argv[6])) DG_ScreenBuffer[i] &= 0x00ffffffu;
        else if (DG_ScreenBuffer[i] & UINT32_C(0x80000000))
            DG_ScreenBuffer[i] = radwolf_flat_tag(DG_ScreenBuffer[i] & UINT32_C(0x00ffffff));
    }
    if (fgetc(input) != EOF || fclose(input)) return 1;
    brightnessScale = (int16_t)atoi(argv[3]);
    dither_percent = atoi(argv[4]);
    exposure = atoi(argv[5]);
    int frames = atoi(argv[7]);
    if (brightnessScale < 1 || brightnessScale > 32 || exposure < 1 || exposure > 512 ||
        dither_percent < 0 || dither_percent > 100 || frames < 1 || frames > 100)
        return 2;
    precomputeColorQuantization();
    for (int i = 0; i < frames; ++i) doImageConversion();
    FILE *output = fopen(argv[2], "wb");
    if (!output) return 1;
    unsigned char address[2] = {0, 0x60};
    int ok = fwrite(address, 1, 2, output) == 2 &&
             fwrite(koalaData, 1, 10001, output) == 10001;
    if (fclose(output)) ok = 0;
    return ok ? 0 : 1;
}
