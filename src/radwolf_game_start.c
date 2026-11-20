/* GPL-3.0-or-later. Runtime dispatch between isolated WL1 and WL6 engines. */
#include "radwolf_game_edition.h"
#include "radwolf_stdio_file.h"
#include <stdio.h>
#include <stdlib.h>
extern void radwolf_boot_trace(const char *stage);
extern void radwolf_wl6_main(void);
extern void radwolf_wl1_radwolf_wolf_main(void);
extern void radwolf_wl6_input_event(uint32_t event);
extern void radwolf_wl1_radwolf_wolf_input_event(uint32_t event);
static radwolf_game_edition active_edition;

static int asset_size(void *user, const char *name, uint32_t *size)
{
    radwolf_file file;
    (void)user;
    if (radwolf_stdio_open(&file, name) != 0) return -1;
    int result = radwolf_file_size(&file, size);
    radwolf_file_close(&file);
    return result;
}

void radwolf_wolf_input_event(uint32_t event)
{
    if (active_edition == RADWOLF_EDITION_WL1)
        radwolf_wl1_radwolf_wolf_input_event(event);
    else if (active_edition == RADWOLF_EDITION_WL6)
        radwolf_wl6_input_event(event);
}

void radwolf_wolf_main(void)
{
    active_edition = radwolf_game_detect(asset_size, NULL);
    switch (active_edition) {
        case RADWOLF_EDITION_WL1:
            radwolf_boot_trace("edition: WL1 shareware 1.4");
            radwolf_wl1_radwolf_wolf_main();
            break;
        case RADWOLF_EDITION_WL6:
            radwolf_boot_trace("edition: WL6 full GT/ID/Activision 1.4");
            radwolf_wl6_main();
            break;
        default:
            radwolf_boot_trace("edition: no complete supported WL1/WL6 data set in RADWOLF");
            fprintf(stderr, "RAD-Wolf: supply all eight WL1 shareware 1.4 or "
                    "WL6 GT/ID/Activision 1.4 files in RADWOLF.\n");
            exit(1);
    }
}
