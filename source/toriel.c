// Toriel's arrival (room_area1_2). She walks down from the arch to the child, introduces
// herself (dialogue from data.win), then turns and leads you back up toward the Ruins.
// Sprites are Toriel's own overworld set (32x58) + her dialogue face.
#include "toriel.h"
#include "game.h"
#include <string.h>
#include <math.h>

#include "toriel_d0_t3x.h"
#include "toriel_d1_t3x.h"
#include "toriel_d2_t3x.h"
#include "toriel_d3_t3x.h"
#include "toriel_u0_t3x.h"
#include "toriel_u1_t3x.h"
#include "toriel_u2_t3x.h"
#include "toriel_u3_t3x.h"
#include "toriel_dt0_t3x.h"
#include "toriel_dt1_t3x.h"
#include "toriel_face0_t3x.h"
#include "toriel_face1_t3x.h"
#include "toriel_face2_t3x.h"

// Arena-space positions (room_area1_2 is 320x420). Centered on the room path.
#define TORIEL_X 160
#define ARCH_Y   150     // top, where she enters/exits
#define STOP_Y   250     // where she stops to talk (above the child)
#define TORIEL_W 32
#define TORIEL_H 58
#define WALK_SPEED 1.6f
#define ANIM_PERIOD 8
#define CHAR_DELAY 3

typedef enum { PH_WALK_IN, PH_TALK, PH_LEAD_OUT, PH_DONE } Phase;

// Her introduction, in order (obj_torielcutscene / textdata_en).
static const char* LINES[] = {
    "* What a terrible creature,&  torturing such a poor,&  innocent youth...",
    "* Ah, do not be afraid,&  my child.",
    "* I am TORIEL,&  caretaker of the RUINS.",
    "* I pass through this place&  every day to see if&  anyone has fallen down.",
    "* You are the first human&  to come here in a&  long time.",
    "* Come!&  I will guide you&  through the catacombs.",
};
#define NUM_LINES ((int)(sizeof(LINES) / sizeof(LINES[0])))

static C2D_SpriteSheet s_sheets[13];
static C2D_Image s_down[4], s_up[4], s_walk[2], s_face[3];

static bool  s_active;
static Phase s_phase;
static float s_ty;
static int   s_line, s_revealed, s_charTimer, s_animTimer, s_frame, s_faceTimer, s_faceFrame;

static void setNearest(C2D_Image img) { if (img.tex) C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST); }
static C2D_Image ld(int i, const u8* d, u32 s) {
    s_sheets[i] = C2D_SpriteSheetLoadFromMem(d, s);
    C2D_Image im = C2D_SpriteSheetGetImage(s_sheets[i], 0);
    setNearest(im);
    return im;
}

void torielLoad(void) {
    s_down[0] = ld(0, toriel_d0_t3x, toriel_d0_t3x_size);
    s_down[1] = ld(1, toriel_d1_t3x, toriel_d1_t3x_size);
    s_down[2] = ld(2, toriel_d2_t3x, toriel_d2_t3x_size);
    s_down[3] = ld(3, toriel_d3_t3x, toriel_d3_t3x_size);
    s_up[0]   = ld(4, toriel_u0_t3x, toriel_u0_t3x_size);
    s_up[1]   = ld(5, toriel_u1_t3x, toriel_u1_t3x_size);
    s_up[2]   = ld(6, toriel_u2_t3x, toriel_u2_t3x_size);
    s_up[3]   = ld(7, toriel_u3_t3x, toriel_u3_t3x_size);
    s_walk[0] = ld(8, toriel_dt0_t3x, toriel_dt0_t3x_size);
    s_walk[1] = ld(9, toriel_dt1_t3x, toriel_dt1_t3x_size);
    s_face[0] = ld(10, toriel_face0_t3x, toriel_face0_t3x_size);
    s_face[1] = ld(11, toriel_face1_t3x, toriel_face1_t3x_size);
    s_face[2] = ld(12, toriel_face2_t3x, toriel_face2_t3x_size);
}
void torielFree(void) { for (int i = 0; i < 13; i++) C2D_SpriteSheetFree(s_sheets[i]); }

bool torielActive(void) { return s_active; }
bool torielDoneLeading(void) { return s_phase == PH_DONE; }

void torielStart(void) {
    s_active = true; s_phase = PH_WALK_IN;
    s_ty = ARCH_Y - TORIEL_H;   // just above the arch, entering
    s_line = 0; s_revealed = 0; s_charTimer = 0; s_animTimer = 0; s_frame = 0;
    s_faceTimer = 0; s_faceFrame = 0;
}

void torielUpdate(u32 kDown, u32 kHeld) {
    (void)kHeld;
    if (!s_active) return;
    // Eyes stay open; blink briefly every couple of seconds (frames: 0 open, 1 half, 2 shut).
    s_faceTimer++;
    if      (s_faceTimer < 140) s_faceFrame = 0;
    else if (s_faceTimer < 146) s_faceFrame = 1;
    else if (s_faceTimer < 152) s_faceFrame = 2;
    else if (s_faceTimer < 158) s_faceFrame = 1;
    else { s_faceFrame = 0; s_faceTimer = 0; }

    switch (s_phase) {
        case PH_WALK_IN:
            s_ty += WALK_SPEED;
            if (++s_animTimer >= ANIM_PERIOD) { s_animTimer = 0; s_frame ^= 1; }
            if (s_ty >= STOP_Y) { s_ty = STOP_Y; s_phase = PH_TALK; s_line = 0; s_revealed = 0; }
            break;

        case PH_TALK: {
            int len = (int)strlen(LINES[s_line]);
            if (s_revealed < len) {
                if (kDown & (KEY_A | KEY_B)) s_revealed = len;
                else if (++s_charTimer >= CHAR_DELAY) { s_charTimer = 0; s_revealed++; }
            } else if (kDown & (KEY_A | KEY_B)) {
                if (s_line + 1 >= NUM_LINES) { s_phase = PH_LEAD_OUT; s_frame = 0; }
                else { s_line++; s_revealed = 0; }
            }
            break;
        }

        case PH_LEAD_OUT:
            s_ty -= WALK_SPEED;
            if (++s_animTimer >= ANIM_PERIOD) { s_animTimer = 0; s_frame ^= 1; }
            if (s_ty <= ARCH_Y - TORIEL_H) s_phase = PH_DONE;
            break;

        case PH_DONE:
            s_active = false;
            break;
    }
}

void torielDrawTop(float camX, float camY, int topXOffset) {
    if (!s_active) return;
    float ox = topXOffset - camX, oy = -camY;
    C2D_Image img;
    switch (s_phase) {
        case PH_WALK_IN:  img = s_walk[s_frame & 1]; break;           // walking down
        case PH_TALK:     img = s_down[0]; break;                     // facing the child (static)
        case PH_LEAD_OUT: img = s_up[s_frame & 1]; break;            // walking up/away
        default:          img = s_down[0]; break;
    }
    C2D_DrawImageAt(img, floorf(ox + TORIEL_X - TORIEL_W / 2), floorf(oy + s_ty),
                    0.25f, NULL, 1.0f, 1.0f);
}

// Dialogue textbox with Toriel's face (32x32, drawn 2x).
#define TB_X 10
#define TB_Y 46
#define TB_W 300
#define TB_H 150
#define TB_BORDER 4
#define TB_FACE_X (TB_X + 20)
#define TB_FACE_Y (TB_Y + 40)
#define TB_TEXT_X (TB_X + 96)
#define TB_TEXT_Y (TB_Y + 28)

void torielDrawBottom(void) {
    if (!s_active || s_phase != PH_TALK) return;
    C2D_DrawRectSolid(TB_X, TB_Y, 0.0f, TB_W, TB_H, COL_WHITE);
    C2D_DrawRectSolid(TB_X + TB_BORDER, TB_Y + TB_BORDER, 0.0f,
                      TB_W - 2 * TB_BORDER, TB_H - 2 * TB_BORDER, COL_BLACK);
    C2D_DrawImageAt(s_face[s_faceFrame], TB_FACE_X, TB_FACE_Y, 0.1f, NULL, 2.0f, 2.0f);

    const char* t = LINES[s_line];
    int n = s_revealed;
    if (t[0] == '*' && t[1] == ' ') { t += 2; n -= 2; if (n < 0) n = 0; }
    char shown[200];
    if (n > 199) n = 199;
    memcpy(shown, t, n);
    for (int i = 0; i < n; i++) if (shown[i] == '&') shown[i] = '\n';
    shown[n] = '\0';
    txtDraw(shown, TB_TEXT_X, TB_TEXT_Y, 0.6f, COL_WHITE);
}
