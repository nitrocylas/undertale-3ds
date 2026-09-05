// Intro storyboard scene. Slide art on the top screen, narration typed on the bottom.
// Text/codes are Undertale's own: ^N pause, & newline, \E advance slide, % end msg.
#include "game.h"
#include "audio.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "mus_story_ogg.h"

#include "introimg0_t3x.h"
#include "introimg1_t3x.h"
#include "introimg2_t3x.h"
#include "introimg3_t3x.h"
#include "introimg4_t3x.h"
#include "introimg5_t3x.h"
#include "introimg6_t3x.h"
#include "introimg7_t3x.h"
#include "introimg8_t3x.h"
#include "introimg9_t3x.h"
#include "introimg10_t3x.h"
#include "introlast_t3x.h"

#define NUM_SLIDES 12
#define SLIDE_INTROLAST 11
#define INTROLAST_H 350
#define INTROLAST_PAN_MAX (INTROLAST_H - SCR_H)

#define CHAR_DELAY    4
#define PAUSE_UNIT    7
#define MSG_END_DELAY 55
#define PAN_SPEED     1

typedef struct { const char* text; int slide; } IntroMessage;

static const IntroMessage MESSAGES[] = {
    {"Long ago^1, two races&ruled over Earth^1:&HUMANS and MONSTERS^6. \\E1 ^1 %", 0},
    {"One day^1, war broke&out between the two&races^6. \\E0 ^1 %", 1},
    {"After a long battle^1,&the humans were&victorious^6. \\E1 ^1 %", 2},
    {"They sealed the monsters&underground with a magic&spell^6. \\E0 ^1 %", 3},
    {"Many years later^2.^2.^5.  \\E1 ^1 %", 4},
    {"      MT. EBOTT&         201X^9 \\E0 %", 5},
    {"Legends say that those&who climb the mountain&never return^5.^3 \\E1 %", 6},
    {" \\E1 %", 7},
    {" ^9 ^5 \\E0 %", 8},
    {" ^9 ^5 ^2 \\E1 %", 9},
    {" ^9 ^5 ^2 \\E2 %", 10},
    {" ^9 ^9 ^9 ^9 ^9 ^9 \\E2 %", SLIDE_INTROLAST},
};
#define NUM_MESSAGES ((int)(sizeof(MESSAGES) / sizeof(MESSAGES[0])))

typedef struct { char plain[256]; u16 pauseBefore[256]; int len; } ParsedMsg;

static ParsedMsg      s_parsed[NUM_MESSAGES];
static C2D_SpriteSheet s_sheets[NUM_SLIDES];
static C2D_Image       s_slides[NUM_SLIDES];

static int   s_curMsg, s_revealed, s_frameTimer, s_pauseTimer, s_endTimer;
static float s_pan;

static void parseMessage(const char* src, ParsedMsg* out) {
    int o = 0, pending = 0;
    while (*src && o < 255) {
        char c = *src;
        if (c == '^' && isdigit((unsigned char)src[1])) {
            pending += (src[1] - '0') * PAUSE_UNIT; src += 2;
        } else if (c == '\\' && src[1] == 'E') {
            src += 2; if (isdigit((unsigned char)*src)) src++;
        } else if (c == '%') {
            break;
        } else {
            out->plain[o] = (c == '&') ? '\n' : c;
            out->pauseBefore[o] = pending; pending = 0; o++; src++;
        }
    }
    out->plain[o] = '\0';
    out->len = o;
}

static void startMessage(int idx) {
    s_curMsg = idx; s_revealed = 0; s_frameTimer = 0; s_endTimer = -1;
    s_pauseTimer = s_parsed[idx].pauseBefore[0];
    if (MESSAGES[idx].slide != SLIDE_INTROLAST) s_pan = 0.0f;
}

static void introInit(void) {
    s_sheets[0]  = C2D_SpriteSheetLoadFromMem(introimg0_t3x,  introimg0_t3x_size);
    s_sheets[1]  = C2D_SpriteSheetLoadFromMem(introimg1_t3x,  introimg1_t3x_size);
    s_sheets[2]  = C2D_SpriteSheetLoadFromMem(introimg2_t3x,  introimg2_t3x_size);
    s_sheets[3]  = C2D_SpriteSheetLoadFromMem(introimg3_t3x,  introimg3_t3x_size);
    s_sheets[4]  = C2D_SpriteSheetLoadFromMem(introimg4_t3x,  introimg4_t3x_size);
    s_sheets[5]  = C2D_SpriteSheetLoadFromMem(introimg5_t3x,  introimg5_t3x_size);
    s_sheets[6]  = C2D_SpriteSheetLoadFromMem(introimg6_t3x,  introimg6_t3x_size);
    s_sheets[7]  = C2D_SpriteSheetLoadFromMem(introimg7_t3x,  introimg7_t3x_size);
    s_sheets[8]  = C2D_SpriteSheetLoadFromMem(introimg8_t3x,  introimg8_t3x_size);
    s_sheets[9]  = C2D_SpriteSheetLoadFromMem(introimg9_t3x,  introimg9_t3x_size);
    s_sheets[10] = C2D_SpriteSheetLoadFromMem(introimg10_t3x, introimg10_t3x_size);
    s_sheets[11] = C2D_SpriteSheetLoadFromMem(introlast_t3x,  introlast_t3x_size);
    for (int i = 0; i < NUM_SLIDES; i++) s_slides[i] = C2D_SpriteSheetGetImage(s_sheets[i], 0);
    for (int i = 0; i < NUM_MESSAGES; i++) parseMessage(MESSAGES[i].text, &s_parsed[i]);
    s_pan = 0.0f;
    startMessage(0);
    audioPlayOgg(mus_story_ogg, mus_story_ogg_size, true); // intro theme, looping
}

static void introCleanup(void) {
    audioStop();
    for (int i = 0; i < NUM_SLIDES; i++) C2D_SpriteSheetFree(s_sheets[i]);
}

static SceneId introUpdate(u32 kDown, u32 kHeld) {
    (void)kHeld;
    if (kDown & (KEY_A | KEY_B)) return SCENE_MENU; // skip to menu

    ParsedMsg* pm = &s_parsed[s_curMsg];
    if (s_endTimer < 0) {
        if (pm->len == 0) {
            s_endTimer = MSG_END_DELAY;
        } else if (s_pauseTimer > 0) {
            s_pauseTimer--;
        } else if (++s_frameTimer >= CHAR_DELAY) {
            s_frameTimer = 0;
            if (s_revealed < pm->len) {
                s_revealed++;
                if (s_revealed < pm->len) s_pauseTimer = pm->pauseBefore[s_revealed];
            } else {
                s_endTimer = MSG_END_DELAY;
            }
        }
    } else if (--s_endTimer <= 0) {
        if (s_curMsg + 1 >= NUM_MESSAGES) return SCENE_MENU; // intro complete
        startMessage(s_curMsg + 1);
    }

    if (MESSAGES[s_curMsg].slide == SLIDE_INTROLAST && s_pan < INTROLAST_PAN_MAX)
        s_pan += PAN_SPEED;
    return SCENE_NONE;
}

static void introDrawTop(void) {
    float y = (MESSAGES[s_curMsg].slide == SLIDE_INTROLAST) ? -s_pan : 0.0f;
    C2D_DrawImageAt(s_slides[MESSAGES[s_curMsg].slide], TOP_X_OFFSET, y, 0.0f, NULL, 1.0f, 1.0f);
}

static void introDrawBottom(void) {
    ParsedMsg* pm = &s_parsed[s_curMsg];
    char shown[257];
    int n = s_revealed < 256 ? s_revealed : 256;
    memcpy(shown, pm->plain, n);
    shown[n] = '\0';
    txtDraw(shown, 20.0f, 36.0f, 0.72f, COL_WHITE);
    txtDraw("[A] skip", 20.0f, 214.0f, 0.5f, COL_GRAY);
}

const Scene SCENE_intro = {
    introInit, introUpdate, introDrawTop, introDrawBottom, introCleanup
};
