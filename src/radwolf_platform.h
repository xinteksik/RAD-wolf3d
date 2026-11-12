/* RAD-Wolf bring-up: GPL-3.0-or-later. Original platform boundary. */
#ifndef RADWOLF_PLATFORM_H
#define RADWOLF_PLATFORM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t (*radwolf_now_us_fn)(void *user);
typedef void (*radwolf_sleep_us_fn)(void *user, uint64_t duration_us);
typedef void (*radwolf_log_fn)(void *user, const char *message);

typedef struct radwolf_platform_ops {
    radwolf_now_us_fn now_us;
    radwolf_sleep_us_fn sleep_us;
    radwolf_log_fn log;
} radwolf_platform_ops;

typedef struct radwolf_platform {
    radwolf_platform_ops ops;
    void *user;
    uint64_t last_us;
} radwolf_platform;

/* All callbacks are required. Returns -1 for an incomplete backend. */
int radwolf_platform_init(radwolf_platform *platform,
                          const radwolf_platform_ops *ops, void *user);

/* Clamp a faulty backend clock so game time never runs backwards. */
uint64_t radwolf_platform_now_us(radwolf_platform *platform);
uint32_t radwolf_platform_now_ms(radwolf_platform *platform);
/* 70 Hz Wolf ticks, without overflowing intermediate 32-bit milliseconds. */
uint32_t radwolf_platform_ticks70(radwolf_platform *platform);
void radwolf_platform_sleep_us(radwolf_platform *platform,
                               uint64_t duration_us);
void radwolf_platform_log(radwolf_platform *platform, const char *message);

#ifdef __cplusplus
}
#endif
#endif
