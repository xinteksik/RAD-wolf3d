/* GPL-3.0-or-later. Flash locality, priority and one-presentation lifetime. */
#include "radwolf_flash.h"
#include <assert.h>
#include <string.h>

void test_flash(void)
{
    static struct {
        uint32_t before;
        uint32_t pixels[320 * 200];
        uint32_t after;
    } frame;
    const uint32_t original = UINT32_C(0x8c717171);
    frame.before = frame.after = UINT32_C(0x12345678);
    for (unsigned i = 0; i < 320 * 200; ++i) frame.pixels[i] = original;
    radwolf_flash_mode = RADWOLF_FLASH_FRAME;
    radwolf_flash_event(0, 3);
    radwolf_flash_prepare_view(8, 4, 304, 152, 40);
    radwolf_flash_apply(frame.pixels, 320);
    unsigned changed = 0;
    for (int y = 0; y < 200; ++y) {
        for (int x = 0; x < 320; ++x) {
            uint32_t pixel = frame.pixels[y * 320 + x];
            if (pixel != original) {
                ++changed;
                assert(pixel == UINT32_C(0x87b8c76f));
                assert(x < 8 || x >= 312 || y < 4 || y >= 156);
                assert(y < 160);
            }
        }
    }
    assert(changed > 0);
    /* Damage wins; no prepared view means menus/maps cannot inherit it. */
    radwolf_flash_event(6, 3);
    radwolf_flash_apply(frame.pixels, 320);
    assert(frame.pixels[2 * 320 + 6] == UINT32_C(0x87b8c76f));
    radwolf_flash_prepare_view(8, 4, 304, 152, 40);
    radwolf_flash_apply(frame.pixels, 320);
    assert(frame.pixels[2 * 320 + 6] == UINT32_C(0x8268372b));
    /* Fullscreen has a bounded inset frame, with the interior untouched. */
    radwolf_flash_prepare_view(0, 0, 320, 200, 0);
    radwolf_flash_apply(frame.pixels, 320);
    assert(frame.pixels[0] == UINT32_C(0x8268372b));
    assert(frame.pixels[100 * 320 + 160] == original);
    for (unsigned i = 0; i < 320 * 200; ++i) frame.pixels[i] = original;
    radwolf_flash_mode = RADWOLF_FLASH_STATUS;
    radwolf_flash_prepare_view(8, 4, 304, 152, 40);
    radwolf_flash_apply(frame.pixels, 320);
    for (unsigned i = 0; i < 320 * 160; ++i) assert(frame.pixels[i] == original);
    assert(frame.pixels[199 * 320] != original);
    for (unsigned mode = RADWOLF_FLASH_OFF; mode <= RADWOLF_FLASH_FULL; mode += 3) {
        for (unsigned i = 0; i < 320 * 200; ++i) frame.pixels[i] = original;
        radwolf_flash_mode = mode;
        radwolf_flash_prepare_view(8, 4, 304, 152, 40);
        radwolf_flash_apply(frame.pixels, 320);
        for (unsigned i = 0; i < 320 * 200; ++i) assert(frame.pixels[i] == original);
    }
    radwolf_flash_mode = RADWOLF_FLASH_FRAME;
    radwolf_flash_prepare_view(-1, 4, 304, 152, 40);
    radwolf_flash_apply(frame.pixels, 320);
    radwolf_flash_prepare_view(8, 4, 304, 152, 40);
    radwolf_flash_event(0, 0);
    radwolf_flash_apply(frame.pixels, 320);
    for (unsigned i = 0; i < 320 * 200; ++i) assert(frame.pixels[i] == original);
    assert(frame.before == UINT32_C(0x12345678) && frame.after == UINT32_C(0x12345678));
}
