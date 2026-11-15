/* GPL-3.0-or-later. Exercise actual Wolf palette-shift policy and counters. */
#include "../upstream/Wolf4SDL/wl_def.h"
#include "radwolf_flash.h"
#include <assert.h>
#include <stdio.h>

SDL_Color gamepal[256];
extern SDL_Color redshifts[6][256];
extern int damagecount, bonuscount;
extern boolean palshifted;
void ClearPaletteShifts(void);
void UpdatePaletteShifts(void);
static unsigned palette_calls;
static SDL_Color *last_palette;

void VL_SetPalette(SDL_Color *palette, bool forceupdate)
{
    (void)forceupdate;
    ++palette_calls;
    last_palette = palette;
}

int main(void)
{
    ClearPaletteShifts();
    tics = 1;
    for (unsigned mode = RADWOLF_FLASH_OFF; mode <= RADWOLF_FLASH_STATUS; ++mode) {
        radwolf_flash_mode = mode;
        damagecount = 20; bonuscount = 18;
        UpdatePaletteShifts();
        assert(damagecount == 19 && bonuscount == 17);
        assert(palette_calls == 0 && !palshifted);
    }
    radwolf_flash_mode = RADWOLF_FLASH_FULL;
    damagecount = 20; bonuscount = 0;
    UpdatePaletteShifts();
    assert(palette_calls == 1 && last_palette == redshifts[2] && palshifted);
    radwolf_flash_mode = RADWOLF_FLASH_FRAME;
    UpdatePaletteShifts();
    assert(palette_calls == 2 && last_palette == gamepal && !palshifted);
    static uint32_t pixels[320 * 200];
    radwolf_flash_prepare_view(8, 4, 304, 152, 40);
    FinishPaletteShifts();
    radwolf_flash_apply(pixels, 320);
    for (unsigned i = 0; i < 320 * 200; ++i) assert(pixels[i] == 0);
    puts("Wolf palette-shift integration passed");
    return 0;
}
