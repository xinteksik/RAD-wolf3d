/* GPL-3.0-or-later. Edition selection uses complete, compatible data sets. */
#include "radwolf_game_edition.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {
    int wl1, wl6;
    const char *missing;
    const char *wrong_size;
    uint32_t replacement_size;
} fixture;
static int asset_size(void *user, const char *name, uint32_t *size)
{
    fixture *data = user;
    int shareware = strstr(name, ".WL1") != NULL;
    if (!(shareware ? data->wl1 : data->wl6)) return -1;
    if (data->missing && strcmp(name, data->missing) == 0) return -1;
    *size = 42;
    if (strncmp(name, "AUDIOHED.", 9) == 0) *size = 1156;
    if (strncmp(name, "VGADICT.", 8) == 0) *size = 1024;
    if (strncmp(name, "VGAHEAD.", 8) == 0) *size = shareware ? 471 : 450;
    if (data->wrong_size && strcmp(name, data->wrong_size) == 0)
        *size = data->replacement_size;
    return 0;
}
int main(void)
{
    fixture data = {0, 0, NULL, NULL, 0};
    assert(radwolf_game_detect(NULL, NULL) == RADWOLF_EDITION_MISSING);
    assert(radwolf_game_detect(asset_size, &data) == RADWOLF_EDITION_MISSING);
    data.wl1 = 1;
    assert(radwolf_game_detect(asset_size, &data) == RADWOLF_EDITION_WL1);
    data.wl6 = 1;
    assert(radwolf_game_detect(asset_size, &data) == RADWOLF_EDITION_WL6);
    data.missing = "AUDIOT.WL6";
    assert(radwolf_game_detect(asset_size, &data) == RADWOLF_EDITION_WL1);
    data.wl1 = 0;
    assert(radwolf_game_detect(asset_size, &data) == RADWOLF_EDITION_MISSING);
    data.missing = NULL;
    data.wrong_size = "VGAHEAD.WL6";
    data.replacement_size = 471;
    assert(radwolf_game_detect(asset_size, &data) == RADWOLF_EDITION_MISSING);
    data.wrong_size = "VSWAP.WL6";
    data.replacement_size = 0;
    assert(radwolf_game_detect(asset_size, &data) == RADWOLF_EDITION_MISSING);
    puts("Game edition detection tests passed");
    return 0;
}
