/* GPL-3.0-or-later. Actual Wolf save/load code, synthetic state, no game data. */
#include "../upstream/Wolf4SDL/wl_def.h"
#include "radwolf_map.h"
#include <assert.h>
#include <stdio.h>

gametype gamestate;
LRstruct LevelRatios[LRpack];
tiletype tilemap[MAPSIZE][MAPSIZE];
objtype objlist[MAXACTORS], *player = objlist;
objtype *actorat[MAPSIZE][MAPSIZE];
word mapwidth = 64, mapheight = 64;
byte areaconnect[NUMAREAS][NUMAREAS];
boolean areabyplayer[NUMAREAS];
statobj_t statobjlist[MAXSTATS], *laststatobj = statobjlist;
doorobj_t doorobjlist[MAXDOORS], *lastdoorobj = doorobjlist;
word pwallstate, pwallpos, pwallx, pwally;
byte pwalldir;
tiletype pwalltile;
int lastgamemusicoffset;
statetype s_player, s_grdstand;
word *mapsegs[MAPPLANES];
SDL_Surface *screenBuffer;
static unsigned warnings;
word WindowX, WindowW, PrintY;
int fontnumber, radwolf_map_colors;
int viewsize;
byte fontcolor, backcolor;
boolean Keyboard[sc_Last];
static unsigned map_polls;
static int exit_legend;
static unsigned char map_pixels[200][320];

void VWB_Bar(int x, int y, int width, int height, int color)
{
    assert(x >= 0 && y >= 0 && x + width <= 320 && y + height <= 200);
    for (int row = y; row < y + height; ++row)
        memset(&map_pixels[row][x], color, (size_t)width);
}
void US_CPrint(const char *text)
{
    if (strstr(text, "BLUE: EXIT")) exit_legend = 1;
}
void IN_ProcessEvents(void)
{
    /* Press and release F1 to close the real modal map loop. */
    Keyboard[sc_F1] = ++map_polls == 1;
}
void radwolf_wolf_delay(uint32_t milliseconds) { (void)milliseconds; }
void DrawPlayScreen(void) {}

/* Recreate only level/actor allocation; loading and checksums are real. */
void SetupGameLevel(void)
{
    radwolf_map_history_reset(&radwolf_current_map_history, 64, 64,
                              (unsigned)(gamestate.mapon + 10 * gamestate.episode));
    lastdoorobj = doorobjlist + 2;
}
void InitActorList(void)
{
    memset(objlist, 0, sizeof(objlist));
    player = objlist;
}
objtype *GetNewActor(void)
{
    player->next = &objlist[1];
    objlist[1].prev = player;
    return &objlist[1];
}
void Thrust(int angle, int32_t speed) { (void)angle; (void)speed; }
void VWB_DrawPic(int x, int y, int pic) { (void)x; (void)y; (void)pic; }
void VH_UpdateScreen(SDL_Surface *surface) { (void)surface; }
void Message(const char *message) { (void)message; ++warnings; }
void IN_ClearKeysDown(void) { memset(Keyboard, 0, sizeof(Keyboard)); }
void IN_Ack(void) {}

static void check_loaded(void)
{
    assert(!warnings);
    assert(gamestate.health == 73 && gamestate.ammo == 42);
    assert(gamestate.score == 12345 && gamestate.lives == 3);
    assert(gamestate.episode == 1 && gamestate.mapon == 2);
    assert(player->tilex == 7 && player->tiley == 9);
    assert(player->state == &s_player && player->angle == 135);
    assert(objlist[1].state == &s_grdstand && objlist[1].hitpoints == 17);
    assert(tilemap[4][5] == 3 && actorat[4][5] == &objlist[1]);
    assert(lastgamemusicoffset == 234);
    assert(gamestate.keys == 0);
    assert(laststatobj == statobjlist + 5);
    assert(statobjlist[0].itemnumber == bo_key1 && statobjlist[0].shapenum >= 0);
    assert(statobjlist[1].itemnumber == bo_key2 && statobjlist[1].shapenum >= 0);
    assert(statobjlist[2].shapenum == -1);
    assert(doorobjlist[0].lock == dr_lock1 && doorobjlist[1].lock == dr_lock2);
}

int main(void)
{
    gamestate.health = 73; gamestate.ammo = 42;
    gamestate.score = 12345; gamestate.lives = 3;
    gamestate.episode = 1; gamestate.mapon = 2;
    player->obclass = playerobj; player->active = ac_yes;
    player->tilex = 7; player->tiley = 9;
    player->state = &s_player; player->angle = 135;
    player->next = &objlist[1];
    objlist[1].obclass = guardobj; objlist[1].active = ac_yes;
    objlist[1].state = &s_grdstand; objlist[1].hitpoints = 17;
    objlist[1].prev = player;
    tilemap[4][5] = 3; actorat[4][5] = &objlist[1];
    lastgamemusicoffset = 234;
    radwolf_map_history_reset(&radwolf_current_map_history, 64, 64, 12);
    radwolf_map_history_mark(&radwolf_current_map_history, 3, 8);

    for (unsigned i = 0; i < 5; ++i) {
        statobjlist[i].tilex = (byte)(20 + i * 2);
        statobjlist[i].tiley = 20;
        statobjlist[i].shapenum = 1;
        statobjlist[i].flags = FL_BONUS;
    }
    statobjlist[0].itemnumber = bo_key1;
    statobjlist[1].itemnumber = bo_key2;
    statobjlist[2].itemnumber = bo_key1;
    statobjlist[2].shapenum = -1; /* Already collected. */
    statobjlist[3].itemnumber = bo_key1;
    statobjlist[3].flags = 0; /* Not an actual bonus/key. */
    statobjlist[4].itemnumber = bo_firstaid;
    laststatobj = statobjlist + 5;
    for (unsigned i = 0; i < 2; ++i) {
        doorobjlist[i].tilex = (byte)(32 + i * 2);
        doorobjlist[i].tiley = 20;
        doorobjlist[i].lock = (byte)(dr_lock1 + i);
        doorobjlist[i].action = dr_closed;
        tilemap[32 + i * 2][20] = (tiletype)(BIT_DOOR | i);
    }
    lastdoorobj = doorobjlist + 2;
    FILE *save = tmpfile();
    assert(save && SaveTheGame(save, 0, 0));
    long full_size = ftell(save);
    assert(full_size > (long)RADWOLF_MAP_HISTORY_RECORD_BYTES);
    radwolf_map_history_reset(&radwolf_current_map_history, 64, 64, 0);
    memset(&gamestate, 0, sizeof(gamestate));
    rewind(save);
    assert(LoadTheGame(save, 0, 0));
    check_loaded();
    assert(radwolf_map_history_contains(&radwolf_current_map_history, 3, 8));
    assert(radwolf_map_history_contains(&radwolf_current_map_history, 7, 9));

    /* A legacy save is exactly the game payload without the optional trailer. */
    FILE *legacy = tmpfile();
    assert(legacy);
    rewind(save);
    long legacy_size = full_size - (long)RADWOLF_MAP_HISTORY_RECORD_BYTES;
    for (long i = 0; i < legacy_size; ++i) {
        int c = fgetc(save);
        assert(c != EOF && fputc(c, legacy) != EOF);
    }
    rewind(legacy);
    assert(LoadTheGame(legacy, 0, 0));
    check_loaded();
    assert(!radwolf_map_history_contains(&radwolf_current_map_history, 3, 8));
    assert(radwolf_map_history_contains(&radwolf_current_map_history, 7, 9));

    /* A damaged optional bitmap must not damage the actual game state. */
    assert(fseek(save, legacy_size + 100, SEEK_SET) == 0);
    int c = fgetc(save);
    assert(c != EOF && fseek(save, -1, SEEK_CUR) == 0);
    assert(fputc(c ^ 1, save) != EOF);
    rewind(save);
    assert(LoadTheGame(save, 0, 0));
    check_loaded();
    assert(!radwolf_map_history_contains(&radwolf_current_map_history, 3, 8));
    assert(radwolf_map_history_contains(&radwolf_current_map_history, 7, 9));
    /* The actual engine adapter must identify the elevator switch, leave
     * normal doors/walls alone and never decorate the live tilemap. */
    tilemap[30][30] = ELEVATORTILE;
    tilemap[31][30] = ELEVATORTILE + 1;
    tilemap[29][30] = BIT_DOOR;
    tilemap[28][30] = 0;
    radwolf_map_history_mark(&radwolf_current_map_history, 28, 30);
    tilemap[5][9] = 1;
    radwolf_map_history_mark(&radwolf_current_map_history, 9, 9);
    tiletype live_tiles[MAPSIZE][MAPSIZE];
    memcpy(live_tiles, tilemap, sizeof(live_tiles));
    fontnumber = 3; fontcolor = 2; backcolor = 4;
    WindowX = 12; WindowW = 123; PrintY = 45;
    viewsize = 21;
    radwolf_wolf_show_map();
    assert(exit_legend && map_polls == 2 && !Keyboard[sc_F1]);
    assert(map_pixels[36 + 60][96 + 60] == RADWOLF_MAP_EXIT_COLOR);
    assert(map_pixels[36 + 60][96 + 62] == 7);
    assert(map_pixels[36 + 60][96 + 58] == 14);
    assert(map_pixels[36 + 60][96 + 56] == RADWOLF_MAP_VISITED_COLOR);
    assert(!memcmp(live_tiles, tilemap, sizeof(live_tiles)));
    assert(fontnumber == 3 && fontcolor == 2 && backcolor == 4);
    assert(WindowX == 12 && WindowW == 123 && PrintY == 45);
    assert(map_pixels[36 + 40][96 + 40] == RADWOLF_MAP_GOLD_KEY_COLOR);
    assert(map_pixels[36 + 40][96 + 44] == RADWOLF_MAP_SILVER_KEY_COLOR);
    assert(map_pixels[36 + 40][96 + 48] == 0);
    assert(map_pixels[36 + 40][96 + 52] == 0);
    assert(map_pixels[36 + 40][96 + 56] == 0);
    assert(map_pixels[36 + 40][96 + 64] == RADWOLF_MAP_GOLD_LOCK_COLOR);
    assert(map_pixels[36 + 40][96 + 68] == RADWOLF_MAP_SILVER_LOCK_COLOR);
    assert(map_pixels[36 + 18][96 + 10] == 7); /* No black player halo. */
    assert(map_pixels[36 + 18][96 + 18] == RADWOLF_MAP_VISITED_COLOR);
    statobjlist[0].shapenum = -1;
    gamestate.keys = 1; /* Gold key acquired: no gold lock warning. */
    map_polls = 0;
    radwolf_wolf_show_map();
    assert(map_pixels[36 + 40][96 + 40] == 0);
    assert(map_pixels[36 + 40][96 + 64] == 14);
    assert(map_pixels[36 + 40][96 + 68] == RADWOLF_MAP_SILVER_LOCK_COLOR);
    FILE *picked = tmpfile();
    assert(picked && SaveTheGame(picked, 0, 0));
    statobjlist[0].shapenum = 1;
    gamestate.keys = 0;
    rewind(picked);
    assert(LoadTheGame(picked, 0, 0));
    assert(statobjlist[0].shapenum == -1 && gamestate.keys == 1);
    map_polls = 0;
    radwolf_wolf_show_map();
    assert(map_pixels[36 + 40][96 + 40] == 0);
    assert(map_pixels[36 + 40][96 + 64] == 14);
    assert(map_pixels[36 + 40][96 + 44] == RADWOLF_MAP_SILVER_KEY_COLOR);
    doorobjlist[1].action = dr_open;
    tilemap[34][20] = 0;
    map_polls = 0;
    radwolf_wolf_show_map();
    assert(map_pixels[36 + 40][96 + 68] == 0);
    fclose(picked);
    puts("Actual map adapter tests passed (exit, keys, locks, pickup/save/load, no player halo)");
    fclose(legacy);
    fclose(save);
    puts("Actual Wolf save/load history tests passed (new, legacy, corrupt trailer)");
    return 0;
}
