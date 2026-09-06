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

// --- One-shot SFX on channel 1 (parses a PCM16 WAV) ---
#define SFX_CHANNEL 1
#define SFX_SLOTS   4
static ndspWaveBuf s_sfxBuf[SFX_SLOTS];
static short*      s_sfxMem[SFX_SLOTS];
static int         s_sfxNext;

static u32 rd32(const u8* p) { return p[0] | (p[1]<<8) | (p[2]<<16) | ((u32)p[3]<<24); }
static u16 rd16(const u8* p) { return (u16)(p[0] | (p[1]<<8)); }

void audioPlayWav(const void* data, int size) {
    if (!s_inited) return;
    const u8* b = (const u8*)data;
    if (size < 44 || rd32(b) != 0x46464952 /*RIFF*/) return;
    // Walk chunks to find fmt + data.
    int channels = 1, rate = 44100, bits = 16;
    const u8 *pcm = NULL; u32 pcmLen = 0;
    int off = 12;
    while (off + 8 <= size) {
        u32 id = rd32(b + off), sz = rd32(b + off + 4);
        const u8* body = b + off + 8;
        if (id == 0x20746d66 /*"fmt "*/ && off + 8 + 16 <= size) {
            channels = rd16(body + 2); rate = rd32(body + 4); bits = rd16(body + 14);
        } else if (id == 0x61746164 /*"data"*/) {
            pcm = body; pcmLen = sz; break;
        }
        off += 8 + sz + (sz & 1);
    }
    if (!pcm || bits != 16 || pcmLen == 0) return;

    int slot = s_sfxNext; s_sfxNext = (s_sfxNext + 1) % SFX_SLOTS;
    if (s_sfxMem[slot]) { linearFree(s_sfxMem[slot]); s_sfxMem[slot] = NULL; }
    s_sfxMem[slot] = (short*)linearAlloc(pcmLen);
    if (!s_sfxMem[slot]) return;
    memcpy(s_sfxMem[slot], pcm, pcmLen);

    ndspChnReset(SFX_CHANNEL);
    ndspChnSetInterp(SFX_CHANNEL, NDSP_INTERP_LINEAR);
    ndspChnSetRate(SFX_CHANNEL, (float)rate);
    ndspChnSetFormat(SFX_CHANNEL, channels == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

    memset(&s_sfxBuf[slot], 0, sizeof(s_sfxBuf[slot]));
    s_sfxBuf[slot].data_vaddr = s_sfxMem[slot];
    s_sfxBuf[slot].nsamples   = (int)(pcmLen / (2 * channels));
    DSP_FlushDataCache(s_sfxMem[slot], pcmLen);
    ndspChnWaveBufAdd(SFX_CHANNEL, &s_sfxBuf[slot]);
}

void audioExit(void) {
    if (!s_inited) return;
    audioStop();
    for (int i = 0; i < SFX_SLOTS; i++) if (s_sfxMem[i]) { linearFree(s_sfxMem[i]); s_sfxMem[i] = NULL; }
    ndspExit();
}
