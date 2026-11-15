/* GPL-3.0-or-later. Atomic, bounded VIDEO.CFG parsing. */
#include "radwolf_video_config.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *trim(char *text)
{
    while (isspace((unsigned char)*text)) ++text;
    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) *--end = 0;
    return text;
}

int radwolf_video_config_parse(radwolf_video_config *config, const char *text)
{
    if (!config) return -1;
    *config = (radwolf_video_config){0, 20, 350, RADWOLF_FLASH_FRAME};
    if (!text) return 0;
    if (!strncmp(text, "\xef\xbb\xbf", 3)) text += 3;
    radwolf_video_config parsed = *config;
    unsigned seen = 0;
    while (*text) {
        char line[128];
        size_t length = strcspn(text, "\n");
        if (length >= sizeof(line)) return -1;
        memcpy(line, text, length);
        line[length] = 0;
        text += length;
        if (*text) ++text;
        char *comment = strchr(line, '#');
        if (comment) *comment = 0;
        char *key = trim(line);
        if (!*key) continue;
        char *value = strchr(key, '=');
        if (!value) return -1;
        *value++ = 0;
        key = trim(key);
        value = trim(value);
        unsigned minimum, maximum, bit, *target;
        if (!strcmp(key, "FLASH")) {
            if (seen & 8) return -1;
            if (!strcmp(value, "OFF")) parsed.flash_mode = RADWOLF_FLASH_OFF;
            else if (!strcmp(value, "FRAME")) parsed.flash_mode = RADWOLF_FLASH_FRAME;
            else if (!strcmp(value, "STATUS")) parsed.flash_mode = RADWOLF_FLASH_STATUS;
            else if (!strcmp(value, "FULL")) parsed.flash_mode = RADWOLF_FLASH_FULL;
            else return -1;
            seen |= 8;
            continue;
        }
        if (!strcmp(key, "DITHER")) {
            target = &parsed.dither_percent; minimum = 0; maximum = 100; bit = 1;
        } else if (!strcmp(key, "BRIGHTNESS")) {
            target = &parsed.brightness; minimum = 1; maximum = 32; bit = 2;
        } else if (!strcmp(key, "EXPOSURE")) {
            target = &parsed.exposure; minimum = 1; maximum = 512; bit = 4;
        } else return -1;
        if ((seen & bit) || !isdigit((unsigned char)*value)) return -1;
        char *end;
        unsigned long number = strtoul(value, &end, 10);
        if (*end || number < minimum || number > maximum) return -1;
        *target = (unsigned)number;
        seen |= bit;
    }
    *config = parsed;
    return 0;
}

int radwolf_video_config_read(radwolf_video_config *config, FILE *file)
{
    if (radwolf_video_config_parse(config, NULL)) return -1;
    if (!file) return 0;
    char text[1024];
    size_t length = fread(text, 1, sizeof(text) - 1, file);
    int extra = fgetc(file);
    if (extra != EOF || ferror(file) || memchr(text, 0, length)) return -1;
    text[length] = 0;
    return radwolf_video_config_parse(config, text);
}
