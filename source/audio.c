#include "audio.h"
#include <3ds.h>
#include <string.h>
#include <stdlib.h>

// From stb_vorbis.c (declared here to avoid pulling the whole header).
extern int stb_vorbis_decode_memory(const unsigned char* mem, int len,
                                    int* channels, int* sample_rate, short** output);

#define BGM_CHANNEL 0

static ndspWaveBuf s_wavebuf;
static short*      s_linbuf;   // linear-heap PCM, owned here, kept alive while playing
static bool        s_inited;

void audioInit(void) {
    // ndspInit fails on hardware without dumped DSP firmware; treat audio as optional.
    // ndspInit needs the 3DS DSP firmware (dspfirm.cdc). Present on CFW consoles; must be
    // provided to emulators. If it's missing, we run silently rather than crash.
    s_inited = (ndspInit() == 0);
    if (s_inited) ndspSetOutputMode(NDSP_OUTPUT_STEREO);
}

void audioStop(void) {
    if (!s_inited) return;
    ndspChnWaveBufClear(BGM_CHANNEL);
    if (s_linbuf) { linearFree(s_linbuf); s_linbuf = NULL; }
}

void audioPlayOgg(const void* data, int size, bool loop) {
    if (!s_inited) return;
    audioStop();

    int channels = 0, rate = 0;
    short* pcm = NULL;
    int samples = stb_vorbis_decode_memory((const unsigned char*)data, size, &channels, &rate, &pcm);
    if (samples <= 0 || pcm == NULL) { if (pcm) free(pcm); return; }

    size_t bytes = (size_t)samples * channels * sizeof(short);
    s_linbuf = (short*)linearAlloc(bytes);
    if (!s_linbuf) { free(pcm); return; }
    memcpy(s_linbuf, pcm, bytes);
    free(pcm);

    ndspChnReset(BGM_CHANNEL);
    ndspChnSetInterp(BGM_CHANNEL, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(BGM_CHANNEL, (float)rate);
    ndspChnSetFormat(BGM_CHANNEL,
        channels == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

    memset(&s_wavebuf, 0, sizeof(s_wavebuf));
    s_wavebuf.data_vaddr = s_linbuf;
    s_wavebuf.nsamples   = samples; // per channel
    s_wavebuf.looping    = loop;

    DSP_FlushDataCache(s_linbuf, bytes);
    ndspChnWaveBufAdd(BGM_CHANNEL, &s_wavebuf);
}

void audioExit(void) {
    if (!s_inited) return;
    audioStop();
    ndspExit();
}
