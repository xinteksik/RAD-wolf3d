/* RAD-Wolf bring-up: GPL-3.0-or-later. SDL-free engine clock boundary. */
#ifndef RADWOLF_WOLF_CLOCK_H
#define RADWOLF_WOLF_CLOCK_H
#include <stdint.h>
uint32_t radwolf_wolf_milliseconds(void);
uint32_t radwolf_wolf_ticks70(void);
void radwolf_wolf_delay(uint32_t milliseconds);
#endif
