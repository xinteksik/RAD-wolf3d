/* GPL-3.0-or-later. Local C interface to the GPL DOSBox OPL bridge. */
#ifndef RADWOLF_OPL_API_H
#define RADWOLF_OPL_API_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int YM3812Init(int count, int clock, int rate);
int YM3812Write(int which, int reg, int value);
void YM3812UpdateOne(int which, int16_t *samples, int count);
#ifdef __cplusplus
}
#endif
#endif
