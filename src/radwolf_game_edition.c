/* GPL-3.0-or-later. WL6 has priority when both complete editions are present. */
#include "radwolf_game_edition.h"
#include <stdio.h>

static int complete(radwolf_asset_size_fn asset_size, void *user,
                    const char *extension, uint32_t graphics_header_size)
{
    static const char *const bases[] = {
        "AUDIOHED", "AUDIOT", "GAMEMAPS", "MAPHEAD", "VGADICT", "VGAGRAPH",
        "VGAHEAD", "VSWAP"
    };
    for (unsigned i = 0; i < sizeof(bases) / sizeof(bases[0]); ++i) {
        char name[32];
        uint32_t size = 0;
        snprintf(name, sizeof(name), "%s.%s", bases[i], extension);
        if (asset_size(user, name, &size) != 0 || size == 0) return 0;
        if (i == 0 && size != 1156) return 0;
        if (i == 4 && size != 1024) return 0;
        if (i == 6 && size != graphics_header_size) return 0;
    }
    return 1;
}

radwolf_game_edition radwolf_game_detect(radwolf_asset_size_fn asset_size,
                                       void *user)
{
    if (asset_size == NULL) return RADWOLF_EDITION_MISSING;
    if (complete(asset_size, user, "WL6", 450)) return RADWOLF_EDITION_WL6;
    if (complete(asset_size, user, "WL1", 471)) return RADWOLF_EDITION_WL1;
    return RADWOLF_EDITION_MISSING;
}
