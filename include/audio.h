// Minimal music player: decodes an embedded OGG (stb_vorbis) and streams it on ndsp
// channel 0. One track at a time (enough for area BGM). Silently no-ops if the DSP is
// unavailable (e.g. real hardware without dumped DSP firmware).
#pragma once
#include <stdbool.h>
#include <stddef.h>

void audioInit(void);
void audioExit(void);
void audioPlayOgg(const void* data, int size, bool loop);
void audioStop(void);
