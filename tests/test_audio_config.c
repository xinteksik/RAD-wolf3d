#include "radwolf_audio_config.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>
int main(void)
{
    radwolf_audio_config config;
    assert(radwolf_audio_config_parse(&config, NULL) == 0 && !config.stereo);
    assert(config.music_width == 70);
    assert(radwolf_audio_config_parse(&config, "MIDI_STEREO_WIDTH=0") == 0);
    assert(config.music_width == 0);
    assert(radwolf_audio_config_parse(&config,
        "MIDI_STEREO_WIDTH=85\nSTEREO_WIDTH=20") == 0);
    assert(config.music_width == 85 && config.width == 20);
    assert(radwolf_audio_config_parse(&config,
        "# stereo board\r\nSTEREO = ON\r\nSID2_ADDRESS = $D500\nSID2_TYPE=AUTO\n") == 0);
    assert(config.stereo && config.address == 0xd500 && config.model == 0);
    assert(radwolf_audio_config_parse(&config,
        "STEREO=ON\nSID2_ADDRESS=0xDE00\nSID2_TYPE=6581") == 0);
    assert(config.stereo && config.address == 0xde00 && config.model == 6581);
    assert(radwolf_audio_config_parse(&config,
        "STEREO=ON\nSID1_ADDRESS=$D400\nSID1_TYPE=8580\nSID2_ADDRESS=$D500\nSID2_TYPE=6581\nSTEREO_WIDTH=100") == 0);
    assert(config.primary_address == 0xd400 && config.primary_model == 8580);
    assert(config.address == 0xd500 && config.model == 6581 && config.width == 100);
    assert(radwolf_audio_config_parse(&config,
        "STEREO=ON\nSID1_TYPE=ULTISID\nSID2_TYPE=ULTISID") == 0);
    assert(config.primary_model == RADWOLF_SID_ULTISID && config.model == RADWOLF_SID_ULTISID);
    const char *invalid[] = {
        "STEREO=ON\nSID2_ADDRESS=D400", "STEREO=ON\nSID2_ADDRESS=DC00",
        "STEREO=ON\nSID2_ADDRESS=D501", "STEREO=ON\nSID2_ADDRESS=-D500",
        "STEREO=ON\nSID2_ADDRESS=D500oops", "STEREO=ON\nSID2_TYPE=bogus",
        "STEREO=yes", "STEREO=ON\nTYPO=1", "STEREO_WIDTH=101",
        "SID1_ADDRESS=DC00", "STEREO=ON\nSID1_ADDRESS=D500\nSID2_ADDRESS=D500",
        "MIDI_STEREO_WIDTH=101", "MIDI_STEREO_WIDTH=-1", "MIDI_STEREO_WIDTH=abc"
    };
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i) {
        assert(radwolf_audio_config_parse(&config, invalid[i]) == -1);
        assert(!config.stereo);
    }
    int left = 0, right = 0;
    radwolf_spatial_attenuation(0, -65536, &left, &right, 100);
    assert(left == 0 && right == 15);
    left = right = 0;
    radwolf_spatial_attenuation(0, 65536, &left, &right, 100);
    assert(left == 15 && right == 0);
    left = 3; right = 3;
    radwolf_spatial_attenuation(65536, 0, &left, &right, 100);
    assert(left == 3 && right == 3);
    left = 2; right = 7;
    radwolf_spatial_attenuation(0, 65536, &left, &right, 0);
    assert(left == 2 && right == 7);
    left = right = 0;
    radwolf_spatial_attenuation(INT_MIN, INT_MIN, &left, &right, 100);
    assert(left == 0 && right > 0);
    puts("audio config tests passed");
}
