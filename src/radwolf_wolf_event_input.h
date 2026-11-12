/* RAD-Wolf bring-up: logical action to Wolf4SDL keyboard bridge. */
#ifndef RADWOLF_WOLF_EVENT_INPUT_H
#define RADWOLF_WOLF_EVENT_INPUT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void radwolf_wolf_input_poll(void);
void radwolf_wolf_input_set_actions(uint32_t actions);
void radwolf_wolf_input_reset(void);
#ifdef __cplusplus
}
#endif
#endif
