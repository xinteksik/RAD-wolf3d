/* RAD-Wolf bring-up: GPL-3.0-or-later. Original platform boundary. */
#include "radwolf_platform.h"

int radwolf_platform_init(radwolf_platform *platform,
                          const radwolf_platform_ops *ops, void *user)
{
    if (platform == 0 || ops == 0 || ops->now_us == 0 ||
        ops->sleep_us == 0 || ops->log == 0) {
        return -1;
    }
    platform->ops = *ops;
    platform->user = user;
    platform->last_us = 0;
    return 0;
}

uint64_t radwolf_platform_now_us(radwolf_platform *platform)
{
    uint64_t current = platform->ops.now_us(platform->user);
    if (current < platform->last_us) {
        return platform->last_us;
    }
    platform->last_us = current;
    return current;
}

void radwolf_platform_sleep_us(radwolf_platform *platform,
                               uint64_t duration_us)
{
    platform->ops.sleep_us(platform->user, duration_us);
}

uint32_t radwolf_platform_now_ms(radwolf_platform *platform)
{
    return (uint32_t)(radwolf_platform_now_us(platform) / 1000);
}

uint32_t radwolf_platform_ticks70(radwolf_platform *platform)
{
    uint64_t us = radwolf_platform_now_us(platform);
    return (uint32_t)((us / 1000000) * 70 + (us % 1000000) * 70 / 1000000);
}

void radwolf_platform_log(radwolf_platform *platform, const char *message)
{
    platform->ops.log(platform->user, message != 0 ? message : "");
}
