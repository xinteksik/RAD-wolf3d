/* GPL-3.0-or-later. Optional second SID configuration. */
#ifndef RADWOLF_AUDIO_CONFIG_H
#define RADWOLF_AUDIO_CONFIG_H
#define RADWOLF_SID_ULTISID 64u
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    unsigned stereo;
    unsigned address;
    unsigned model; /* 0=AUTO, 6581, 8580, 4=VOLUME, 64=ULTISID */
    unsigned width; /* 0=original Wolf tables, 100=full lateral separation */
    unsigned primary_address;
    unsigned primary_model;
} radwolf_audio_config;
int radwolf_audio_config_parse(radwolf_audio_config *config, const char *text);
extern unsigned radwolf_sid2_address; /* volume register, zero disables output */
extern unsigned radwolf_sid1_address;
extern unsigned radwolf_sid1_model;
extern unsigned radwolf_sid2_model;
extern unsigned radwolf_stereo_width;
void radwolf_spatial_attenuation(int forward, int side, int *left, int *right,
                                 unsigned width);
extern const unsigned char *radwolf_sid2_lut;
extern unsigned char radwolf_right_ring[16384];
int radwolf_sid2_initialize(unsigned base, unsigned model);
int radwolf_sid1_initialize(unsigned base, unsigned model);
unsigned radwolf_sid1_boot_model(void);
#ifdef __cplusplus
}
#endif
#endif
