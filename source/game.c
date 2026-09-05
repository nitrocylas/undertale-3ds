// Game core: shared text buffer, draw helpers, and the scene registry / main loop dispatch.
#include "game.h"
#include <string.h>

char g_playerName[16] = "";

static C2D_TextBuf s_textBuf;

void gameCoreInit(void) { s_textBuf = C2D_TextBufNew(4096); }
void gameCoreExit(void) { C2D_TextBufDelete(s_textBuf); }

void txtDraw(const char* s, float x, float y, float scale, u32 color) {
    C2D_TextBufClear(s_textBuf);
    C2D_Text t;
    C2D_TextParse(&t, s_textBuf, s);
    C2D_TextOptimize(&t);
    C2D_DrawText(&t, C2D_WithColor, x, y, 0.0f, scale, scale, color);
}

void txtDrawCentered(const char* s, float cx, float y, float scale, u32 color) {
    C2D_TextBufClear(s_textBuf);
    C2D_Text t;
    C2D_TextParse(&t, s_textBuf, s);
    C2D_TextOptimize(&t);
    C2D_DrawText(&t, C2D_WithColor | C2D_AlignCenter, cx, y, 0.0f, scale, scale, color);
}
