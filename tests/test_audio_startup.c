/* Integration fixture: actual Wolf SD startup and callbacks, synthetic pages. */
#include "wl_def.h"
#include "baremetal/SDL_mixer.h"
#include "radwolf_audio_config.h"
#include "radwolf_midi.h"
uint64_t soundFirstuSecTick;
uint8_t soundRingBuffer[16384];
static uint8_t lookup[256];
const uint8_t *mahoneyLUT = lookup;
unsigned radwolf_sid2_address;
unsigned radwolf_sid2_model;
unsigned radwolf_sid1_address = 0xd418, radwolf_sid1_model;
unsigned radwolf_sid1_boot_model(void) { return 8580; }
int radwolf_sid1_initialize(unsigned base, unsigned model)
{ radwolf_sid1_address = base+0x18; radwolf_sid1_model = model; return 1; }
const unsigned char *radwolf_sid2_lut;
int radwolf_sid2_initialize(unsigned base, unsigned model)
{ (void)base; (void)model; return 0; }
void disableInterrupts(void) { }
void enableInterrupts(void) { }
uint64_t GetuSec(void) { return 1000000; }
void startSIDSamplePlayer(void) { }
void stopSIDSamplePlayer(void) { }
void radwolf_wolf_delay(uint32_t milliseconds) { (void)milliseconds; }
word ChunksInFile = 2, PMSoundStart = 1;
bool PMSoundInfoPagePadded;
byte *audiosegs[NUMSNDCHUNKS];
char str[80];
int param_samplerate = 44100;
int param_audiobuffer = DEFAULT_AUDIO_BUFFER_SIZE;
static byte empty_page[4];
byte *PM_GetPage(int page) { (void)page; return empty_page; }
byte *PM_GetPageEnd(void) { return empty_page + sizeof(empty_page); }
uint32_t PM_GetPageSize(int page) { (void)page; return 0; }
int32_t CA_CacheAudioChunk(int chunk)
{
    static word music[] = {4, 0x0120, 1};
    audiosegs[chunk] = (byte *)music;
    return sizeof(music);
}
void *safe_malloc(size_t bytes, const char *file, uint32_t line)
{ (void)file; (void)line; void *p = malloc(bytes ? bytes : 1); assert(p); return p; }
void Quit(const char *format, ...)
{ fprintf(stderr, "Unexpected Quit: %s\n", format); abort(); }
void Error(const char *message)
{ fprintf(stderr, "Unexpected Error: %s\n", message); abort(); }
int main(int argc, char **argv)
{
    (void)argv;
    for (unsigned i = 0; i < 256; ++i) lookup[i] = i;
    SD_Startup();
    assert(AdLibPresent && SoundBlasterPresent);
    assert(param_samplerate == 22050);
    assert(SD_SetSoundMode(sdm_AdLib));
    assert(SD_SetMusicMode(smm_AdLib));
    SD_SetDigiDevice(sds_SoundBlaster);
    /* Exercise actual SD_PositionSound -> SD_PlaySound -> PCM mixer path. */
    free(DigiList);
    DigiList = calloc(1, sizeof(*DigiList));
    NumDigi = 1;
    DigiList[0].length = 1;
    empty_page[0] = 192;
    SD_PrepareSound(0);
    static SoundCommon sound = {1, 1};
    audiosegs[STARTADLIBSOUNDS + HALTSND] = (byte *)&sound;
    DigiMap[HALTSND] = 0;
    DigiChannel[0] = -1;
    radwolf_sid2_address = 0xd518;
    radwolf_sid2_lut = lookup;
    int left = 0, right = 0;
    radwolf_spatial_attenuation(0, -TILEGLOBAL, &left, &right, 100);
    assert(left == 0 && right == 15);
    SD_PositionSound(left, right);
    assert(SD_PlaySound(HALTSND) == 3);
    radwolf_audio_pump();
    assert(soundFirstuSecTick == 1000000);
    assert(soundRingBuffer[0] == 192 && radwolf_right_ring[0] == 128);
    radwolf_audio_pump();
    SD_StartMusic(STARTMUSIC + WONDERIN_MUS);
    assert(SD_MusicPlaying());
    assert(radwolf_midi_active() == (argc > 1));
    SD_MusicOff();
    assert(!SD_MusicPlaying() && !radwolf_midi_active());
    SD_StartMusic(STARTMUSIC + WONDERIN_MUS);
    assert(radwolf_midi_active() == (argc > 1));
    SD_StartMusic(STARTMUSIC + CORNER_MUS);
    assert(SD_MusicPlaying());
    assert(radwolf_midi_active() == (argc > 1));
    SD_ContinueMusic(STARTMUSIC + CORNER_MUS, 0);
    assert(radwolf_midi_active() == (argc > 1));
    SD_Shutdown();
    puts("Wolf audio startup integration passed");
    return 0;
}
