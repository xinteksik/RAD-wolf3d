/* GPL-3.0-or-later. Portable full-level overview, independent of debug keys. */
#ifndef RADWOLF_MAP_H
#define RADWOLF_MAP_H
#include <stdint.h>
typedef void (*radwolf_map_box_fn)(void *, int, int, int, int, unsigned);
int radwolf_map_draw(const uint16_t *tiles, unsigned stride, unsigned width,
                     unsigned height, unsigned door_bit, int player_x,
                     int player_y, int angle, radwolf_map_box_fn box, void *user);
void radwolf_wolf_show_map(void);
#endif
