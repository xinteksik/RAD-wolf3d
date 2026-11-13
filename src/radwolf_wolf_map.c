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
void radwolf_wolf_show_map(void)
{
    int saved_font = fontnumber;
    byte saved_color = fontcolor, saved_background = backcolor;
    word saved_x = WindowX, saved_width = WindowW, saved_y = PrintY;
    for (unsigned x = 0; x < mapwidth && x < 64; ++x)
        for (unsigned y = 0; y < mapheight && y < 64; ++y)
            map_tiles[x * 64 + y] = tilemap[x][y];
    VWB_Bar(0, 0, 320, 200, 0);
    fontnumber = 0;
    SETFONTCOLOR(15, 0);
    WindowX = 0; WindowW = 320; PrintY = 10;
    US_CPrint("LEVEL MAP");
    PrintY = 184;
    US_CPrint("RED: YOU   F1 / BACK: CLOSE");
    radwolf_map_draw(map_tiles, 64, mapwidth, mapheight, BIT_DOOR,
                     player->tilex, player->tiley, player->angle, map_box, NULL);
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
