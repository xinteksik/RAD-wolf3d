/* GPL-3.0-or-later. Capture format and explicit keyboard event checks. */
#include "radwolf_capture.h"
#include "radwolf_rad_input.h"
#include <assert.h>
#include <string.h>
void test_capture(void)
{
    static uint32_t pixels[320 * 200];
    pixels[0] = UINT32_C(0x8c717171);
    pixels[320 * 200 - 1] = UINT32_C(0x00123456);
    FILE *file = tmpfile();
    assert(file);
    assert(radwolf_capture_write(file, pixels) == 0);
    assert(ftell(file) == 256016);
    rewind(file);
    unsigned char bytes[20];
    assert(fread(bytes, 1, 20, file) == 20);
    assert(memcmp(bytes, "RWF1LE\0\0", 8) == 0);
    assert(bytes[8] == 0x40 && bytes[9] == 1 && bytes[10] == 200);
    assert(bytes[16] == 0x71 && bytes[17] == 0x71 && bytes[18] == 0x71 && bytes[19] == 0x8c);
    assert(fseek(file, -4, SEEK_END) == 0);
    assert(fread(bytes, 1, 4, file) == 4);
    assert(bytes[0] == 0x56 && bytes[1] == 0x34 && bytes[2] == 0x12 && bytes[3] == 0);
    assert(radwolf_capture_write(NULL, pixels) == -1);
    assert(radwolf_capture_write(file, NULL) == -1);
    fclose(file);
    radwolf_rad_input input = {{0}, 0};
    radwolf_rad_input_event(&input, 'P' | 256u);
    assert(radwolf_rad_input_actions(&input) == RW_CAPTURE);
    radwolf_rad_input_event(&input, 'P');
    assert(radwolf_rad_input_actions(&input) == 0);
}
