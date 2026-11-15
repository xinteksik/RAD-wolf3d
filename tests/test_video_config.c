/* GPL-3.0-or-later. Defaults, atomic fallback and bounded config input. */
#include "radwolf_video_config.h"
#include <assert.h>
#include <string.h>

static void defaults(radwolf_video_config config)
{
    assert(config.dither_percent == 0 && config.brightness == 20 && config.exposure == 350);
    assert(config.flash_mode == RADWOLF_FLASH_FRAME);
}

void test_video_config(void)
{
    radwolf_video_config config;
    assert(radwolf_video_config_parse(&config, NULL) == 0);
    defaults(config);
    assert(radwolf_video_config_parse(&config,
        "\xef\xbb\xbf# Test\r\nDITHER = 50 # inline\r\nBRIGHTNESS=18\r\nEXPOSURE=300") == 0);
    assert(config.dither_percent == 50 && config.brightness == 18 && config.exposure == 300);
    assert(radwolf_video_config_parse(&config, "DITHER=100\nBRIGHTNESS=32\nEXPOSURE=512") == 0);
    assert(config.dither_percent == 100 && config.brightness == 32 && config.exposure == 512);
    assert(radwolf_video_config_parse(&config, "BRIGHTNESS=1\nEXPOSURE=1") == 0);
    assert(config.dither_percent == 0 && config.brightness == 1 && config.exposure == 1);
    const char *modes[] = {"OFF", "FRAME", "STATUS", "FULL"};
    for (unsigned i = 0; i < 4; ++i) {
        char line[32];
        snprintf(line, sizeof(line), "FLASH=%s", modes[i]);
        assert(radwolf_video_config_parse(&config, line) == 0 && config.flash_mode == i);
    }
    const char *invalid[] = {
        "DITHER=101", "DITHER=-1", "DITHER=+1", "DITHER=", "DITHER=0.5",
        "BRIGHTNESS=0", "BRIGHTNESS=33", "EXPOSURE=0", "EXPOSURE=513",
        "DITHER=50\nEXPOSURE=bad", "DITHER=50\nDITHER=0", "UNKNOWN=1",
        "DITHER=10oops", "DITHER=9999999999999999999999999999999999999999",
        "FLASH=BAD", "FLASH=", "FLASH=FRAME\nFLASH=OFF", "DITHER=100\nFLASH=BAD"
    };
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        assert(radwolf_video_config_parse(&config, invalid[i]) == -1);
        defaults(config);
    }
    FILE *file = tmpfile();
    assert(file);
    fputs("DITHER=25\n", file);
    rewind(file);
    assert(radwolf_video_config_read(&config, file) == 0 && config.dither_percent == 25);
    fclose(file);
    file = tmpfile();
    assert(file);
    for (unsigned i = 0; i < 600; ++i) fputs("#\n", file);
    rewind(file);
    assert(radwolf_video_config_read(&config, file) == -1);
    defaults(config);
    fclose(file);
    file = tmpfile();
    assert(file);
    assert(fwrite("DITHER=50\0EXPOSURE=512", 1, 22, file) == 22);
    rewind(file);
    assert(radwolf_video_config_read(&config, file) == -1);
    defaults(config);
    fclose(file);
    assert(radwolf_video_config_read(&config, NULL) == 0);
    defaults(config);
}
