/* GPL-3.0-or-later. Select the engine matching a complete data set. */
#ifndef RADWOLF_GAME_EDITION_H
#define RADWOLF_GAME_EDITION_H
#include <stdint.h>
typedef enum {
    RADWOLF_EDITION_MISSING = 0,
    RADWOLF_EDITION_WL1 = 1,
    RADWOLF_EDITION_WL6 = 6
} radwolf_game_edition;
typedef int (*radwolf_asset_size_fn)(void *user, const char *name, uint32_t *size);
radwolf_game_edition radwolf_game_detect(radwolf_asset_size_fn asset_size,
                                       void *user);
#endif
