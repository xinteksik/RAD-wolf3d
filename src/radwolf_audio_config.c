/* GPL-3.0-or-later. Strict opt-in parsing: malformed configuration stays mono. */
#include "radwolf_audio_config.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
unsigned radwolf_stereo_width = 100;

void radwolf_spatial_attenuation(int forward, int side, int *left, int *right,
                                 unsigned width)
{
    if (!left || !right || !width) return;
    if (width > 100) width = 100;
    uint64_t f = forward < 0 ? (uint64_t)-(int64_t)forward : (uint64_t)forward;
    uint64_t s = side < 0 ? (uint64_t)-(int64_t)side : (uint64_t)side;
    if (!f && !s) return;
    int distance = *left < *right ? *left : *right;
    if (distance < 0) distance = 0;
    if (distance > 15) distance = 15;
    int quieter = distance + (int)((15 - distance) * s * width / ((f + s) * 100));
    *left = side > 0 ? quieter : distance;
    *right = side < 0 ? quieter : distance;
}
static char *trim(char *s)
{
    while (isspace((unsigned char)*s)) ++s;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) *--end = 0;
    return s;
}
int radwolf_audio_config_parse(radwolf_audio_config *config, const char *text)
{
    if (!config) return -1;
    *config = (radwolf_audio_config){0, 0xd500, 0, 100, 0xd400, 0, 70};
    if (!text) return 0;
    radwolf_audio_config parsed = *config;
    while (*text) {
        char line[128];
        size_t n = strcspn(text, "\n");
        if (n >= sizeof(line)) return -1;
        memcpy(line, text, n); line[n] = 0;
        text += n; if (*text) ++text;
        char *comment = strchr(line, '#'); if (comment) *comment = 0;
        char *key = trim(line);
        if (!*key) continue;
        char *value = strchr(key, '='); if (!value) return -1;
        *value++ = 0; key = trim(key); value = trim(value);
        if (!strcmp(key, "STEREO")) {
            if (!strcmp(value, "ON")) parsed.stereo = 1;
            else if (!strcmp(value, "OFF")) parsed.stereo = 0;
            else return -1;
        } else if (!strcmp(key, "SID1_ADDRESS") || !strcmp(key, "SID2_ADDRESS")) {
            if (*value == '$') ++value;
            if (!isxdigit((unsigned char)*value)) return -1;
            char *end;
            unsigned long base = strtoul(value, &end, 16);
            if (*end || base > 0xffff || (base & 31)
                || !((base >= 0xd400 && base <= 0xd7e0)
                     || (base >= 0xde00 && base <= 0xdfe0))) return -1;
            if (key[3] == '1') parsed.primary_address = (unsigned)base;
            else parsed.address = (unsigned)base;
        } else if (!strcmp(key, "STEREO_WIDTH") || !strcmp(key, "MIDI_STEREO_WIDTH")) {
            if (!isdigit((unsigned char)*value)) return -1;
            char *end;
            unsigned long width = strtoul(value, &end, 10);
            if (*end || width > 100) return -1;
            if (!strcmp(key, "MIDI_STEREO_WIDTH")) parsed.music_width = (unsigned)width;
            else parsed.width = (unsigned)width;
        } else if (!strcmp(key, "SID1_TYPE") || !strcmp(key, "SID2_TYPE")) {
            unsigned *model = key[3] == '1' ? &parsed.primary_model : &parsed.model;
            if (!strcmp(value, "AUTO")) *model = 0;
            else if (!strcmp(value, "6581")) *model = 6581;
            else if (!strcmp(value, "8580")) *model = 8580;
            else if (!strcmp(value, "VOLUME")) *model = 4;
            else if (!strcmp(value, "ULTISID")) *model = RADWOLF_SID_ULTISID;
            else return -1;
        } else return -1;
    }
    if (parsed.stereo && parsed.address == parsed.primary_address) return -1;
    *config = parsed;
    return 0;
}
