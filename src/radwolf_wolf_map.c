/* GPL-3.0-or-later. Pause the game loop while keeping cooperative audio alive. */
#include "../upstream/Wolf4SDL/wl_def.h"
#include "radwolf_map.h"
#include "radwolf_surface.h"
static uint16_t map_tiles[64 * 64];
static void map_box(void *user, int x, int y, int width, int height, unsigned color)
{
    (void)user;
    VWB_Bar(x, y, width, height, (int)color);
}
void radwolf_wolf_map_record_position(void)
{
    if (player)
        radwolf_map_history_mark(&radwolf_current_map_history, player->tilex, player->tiley);
}
void radwolf_wolf_show_map(void)
{
    radwolf_wolf_map_record_position();
    int saved_font = fontnumber;
    byte saved_color = fontcolor, saved_background = backcolor;
    word saved_x = WindowX, saved_width = WindowW, saved_y = PrintY;
    for (unsigned x = 0; x < mapwidth && x < 64; ++x)
        for (unsigned y = 0; y < mapheight && y < 64; ++y) {
            uint16_t tile = tilemap[x][y];
            /* Match Cmd_Use's exact elevator-switch test. Decoration belongs
             * only to this display copy, never to the live collision map. */
            if (tile == ELEVATORTILE) tile |= RADWOLF_MAP_EXIT_FLAG;
            map_tiles[x * 64 + y] = tile;
        }
    /* Static bonuses are the authoritative live/saved state: picked-up
     * keys have shapenum -1 and must disappear even after loading a save. */
    for (statobj_t *item = statobjlist; item < laststatobj; ++item) {
        if (item->shapenum < 0 || !(item->flags & FL_BONUS) ||
            item->tilex >= mapwidth || item->tiley >= mapheight ||
            item->tilex >= 64 || item->tiley >= 64)
            continue;
        uint16_t flag = item->itemnumber == bo_key1 ? RADWOLF_MAP_GOLD_KEY_FLAG :
                        item->itemnumber == bo_key2 ? RADWOLF_MAP_SILVER_KEY_FLAG : 0;
        map_tiles[item->tilex * 64 + item->tiley] |= flag;
    }
    /* Warn about missing keys only. Owned keys and open doors need no
     * special lock color; use the same key bits as OperateDoor. */
    for (doorobj_t *door = doorobjlist; door < lastdoorobj; ++door) {
        if (door->tilex >= mapwidth || door->tiley >= mapheight ||
            door->tilex >= 64 || door->tiley >= 64 || door->action == dr_open)
            continue;
        uint16_t flag = 0;
        if (door->lock == dr_lock1 && !(gamestate.keys & 1))
            flag = RADWOLF_MAP_GOLD_LOCK_FLAG;
        else if (door->lock == dr_lock2 && !(gamestate.keys & 2))
            flag = RADWOLF_MAP_SILVER_LOCK_FLAG;
        map_tiles[door->tilex * 64 + door->tiley] |= flag;
    }
    VWB_Bar(0, 0, 320, 200, 0);
    fontnumber = 0;
    SETFONTCOLOR(15, 0);
    WindowX = 0; WindowW = 320; PrintY = 10;
    US_CPrint("LEVEL MAP");
    PrintY = 24;
    US_CPrint("BLUE: EXIT  KEYS: YELLOW / WHITE");
    PrintY = 174;
    US_CPrint("GREEN: VISITED   RED: YOU");
    PrintY = 188;
    US_CPrint("LOCKS: ORANGE / WHITE  F1 / BACK");
    radwolf_map_draw(map_tiles, 64, mapwidth, mapheight, BIT_DOOR,
                     &radwolf_current_map_history, player->tilex, player->tiley,
                     player->angle, map_box, NULL);
    radwolf_map_colors = 1;
    VW_UpdateScreen();
    radwolf_map_colors = 0;
    /* Do not treat the opening F1 press as a close request. */
    while (Keyboard[sc_F1]) { IN_ProcessEvents(); SDL_Delay(10); }
    do { IN_ProcessEvents(); SDL_Delay(10); }
    while (!Keyboard[sc_F1] && !Keyboard[sc_Escape]);
    while (Keyboard[sc_F1] || Keyboard[sc_Escape]) { IN_ProcessEvents(); SDL_Delay(10); }
    IN_ClearKeysDown();
    fontnumber = saved_font;
    fontcolor = saved_color; backcolor = saved_background;
    WindowX = saved_x; WindowW = saved_width; PrintY = saved_y;
    if (viewsize != 21) DrawPlayScreen();
}
