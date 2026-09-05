// Flowey's introduction cutscene. Dialogue text is the game's own (from data.win).
// Two phases: (1) overworld — the flower grows from the green patch and talks; (2) combat —
// the battle box appears (the reusable battlebox module), your SOUL is placed inside, and
// the "friendliness pellets" orbit then converge on you (Flowey's trap: HP 20 -> 1), before
// he turns evil and Toriel's flame drives him off.
#include "flowey.h"
#include "game.h"
#include "battlebox.h"
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "flowey_nice0_t3x.h"
#include "flowey_nice1_t3x.h"
#include "flowey_evil0_t3x.h"
#include "flowey_evil1_t3x.h"
#include "flowey_body0_t3x.h"
#include "flowey_body1_t3x.h"

// Overworld flower position (room_area1_2 arena space).
#define FLOWEY_CX 160
#define FLOWEY_CY 300
#define FLOWEY_SCALE 1.5f

// Combat view (top-screen pixel space).
#define BOX_X 125
#define BOX_Y 104
#define BOX_W 150
#define BOX_H 104
#define BOX_CX (BOX_X + BOX_W / 2)
#define BOX_CY (BOX_Y + BOX_H / 2)
#define PEL_RADIUS 30.0f
#define NUM_PELLETS 5

typedef enum { FACE_NICE, FACE_EVIL } Face;
typedef struct { const char* text; Face face; int action; } Beat;

enum { ACT_NONE, ACT_RISE, ACT_SOUL, ACT_PELLETS_SHOW, ACT_PELLETS_MOVE, ACT_HIT,
       ACT_LAUGH, ACT_RING, ACT_TORIEL, ACT_END };

static const Beat BEATS[] = {
    { NULL, FACE_NICE, ACT_RISE },
    { "* Howdy!&* I'm FLOWEY.&* FLOWEY the FLOWER!", FACE_NICE, ACT_NONE },
    { "* Hmmm...", FACE_NICE, ACT_NONE },
    { "* You're new to the&  UNDERGROUND,&  aren'tcha?", FACE_NICE, ACT_NONE },
    { "* Golly, you must be&  so confused.", FACE_NICE, ACT_NONE },
    { "* Someone ought to&  teach you how things&  work around here!", FACE_NICE, ACT_NONE },
    { "* I guess little old&  me will have to do.", FACE_NICE, ACT_NONE },
    { "* Ready?&* Here we go!", FACE_NICE, ACT_SOUL },
    { "* See that heart?", FACE_NICE, ACT_NONE },
    { "* That is your SOUL,&  the culmination of&  your being!", FACE_NICE, ACT_NONE },
    { "* Your SOUL starts&  weak, but grows&  strong with LV.", FACE_NICE, ACT_NONE },
    { "* What's LV stand&  for? Why, LOVE,&  of course!", FACE_NICE, ACT_NONE },
    { "* You want some&  LOVE, don't you?", FACE_NICE, ACT_NONE },
    { "* Don't worry!&  I'll share some&  with you.", FACE_NICE, ACT_NONE },
    { "* Down here, LOVE is&  shared through...", FACE_NICE, ACT_PELLETS_SHOW },
    { "* ...little white&  \"friendliness&  pellets.\"", FACE_NICE, ACT_NONE },
    { "* Move around!&  Get as many as&  you can!", FACE_NICE, ACT_PELLETS_MOVE },
    { NULL, FACE_EVIL, ACT_HIT },
    { "* You IDIOT.", FACE_EVIL, ACT_NONE },
    { "* In this world,&  it's KILL&  or BE killed.", FACE_EVIL, ACT_NONE },
    { "* Why would ANYONE&  pass up an&  opportunity like&  this!?", FACE_EVIL, ACT_LAUGH },
    { "* DIE.", FACE_EVIL, ACT_RING },
    { NULL, FACE_NICE, ACT_TORIEL },
    { NULL, FACE_NICE, ACT_END },
};
#define BEAT_COUNT ((int)(sizeof(BEATS) / sizeof(BEATS[0])))
#define CHAR_DELAY 3

static C2D_SpriteSheet s_sheets[6];
static C2D_Image s_nice[2], s_evil[2], s_body[2];

static bool  s_active;
static int   s_beat, s_revealed, s_charTimer, s_actTimer, s_faceTimer, s_faceFrame, s_riseFrame;
static bool  s_combat;        // battle box visible
static bool  s_pelletsShown, s_converge, s_soulControllable;
static int   s_pelIdx[NUM_PELLETS];
static float s_pelAngle[NUM_PELLETS];
static int   s_toriel;

static void setNearest(C2D_Image img) { if (img.tex) C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST); }
static C2D_Image ld(int i, const u8* d, u32 s) {
    s_sheets[i] = C2D_SpriteSheetLoadFromMem(d, s);
    C2D_Image im = C2D_SpriteSheetGetImage(s_sheets[i], 0);
    setNearest(im);
    return im;
}

void floweyLoad(void) {
    s_nice[0] = ld(0, flowey_nice0_t3x, flowey_nice0_t3x_size);
    s_nice[1] = ld(1, flowey_nice1_t3x, flowey_nice1_t3x_size);
    s_evil[0] = ld(2, flowey_evil0_t3x, flowey_evil0_t3x_size);
    s_evil[1] = ld(3, flowey_evil1_t3x, flowey_evil1_t3x_size);
    s_body[0] = ld(4, flowey_body0_t3x, flowey_body0_t3x_size);
    s_body[1] = ld(5, flowey_body1_t3x, flowey_body1_t3x_size);
    battleLoad();
}
void floweyFree(void) { for (int i = 0; i < 6; i++) C2D_SpriteSheetFree(s_sheets[i]); battleFree(); }

bool floweyActive(void) { return s_active; }
bool floweyInCombat(void) { return s_active && s_combat; }

static void spawnPelletRing(void) {
    s_pelletsShown = true; s_converge = false;
    battleClearBullets();
    for (int i = 0; i < NUM_PELLETS; i++) {
        s_pelAngle[i] = (float)i / NUM_PELLETS * 2 * M_PI;
        float x = BOX_CX + cosf(s_pelAngle[i]) * PEL_RADIUS;
        float y = BOX_CY + sinf(s_pelAngle[i]) * PEL_RADIUS;
        s_pelIdx[i] = battleAddBullet(x, y, 4.0f);
    }
}

static void beginBeat(int idx) {
    s_beat = idx; s_revealed = 0; s_charTimer = 0; s_actTimer = 0;
    if (idx >= BEAT_COUNT) { s_active = false; return; }
    switch (BEATS[idx].action) {
        case ACT_SOUL:
            s_combat = true;
            battleSetBox(BOX_X, BOX_Y, BOX_W, BOX_H);
            battleSetHP(20, 20);
            break;
        case ACT_PELLETS_SHOW: spawnPelletRing(); break;
        case ACT_PELLETS_MOVE: s_soulControllable = true; break;
        case ACT_TORIEL: s_toriel = 90; battleClearBullets(); s_pelletsShown = false; break;
        default: break;
    }
}

void floweyStart(void) {
    s_active = true; s_combat = false;
    s_faceFrame = 0; s_faceTimer = 0; s_riseFrame = 0;
    s_pelletsShown = false; s_converge = false; s_soulControllable = false;
    s_toriel = 0;
    beginBeat(0);
}

void floweyUpdate(u32 kDown, u32 kHeld) {
    if (!s_active) return;
    const Beat* b = &BEATS[s_beat];
    if (++s_faceTimer >= 12) { s_faceTimer = 0; s_faceFrame ^= 1; }

    // Battle box upkeep (SOUL movement + i-frames) whenever combat is up.
    if (s_combat) battleUpdateSoul(kHeld, s_soulControllable);

    // Pellet motion: orbit, or converge on the SOUL (the trap).
    if (s_pelletsShown) {
        float sxp, syp; battleSoulPos(&sxp, &syp);
        for (int i = 0; i < NUM_PELLETS; i++) {
            Bullet* bl = battleBullet(s_pelIdx[i]);
            if (!bl || !bl->active) continue;
            if (s_converge) {
                bl->x += (sxp - bl->x) * 0.14f;
                bl->y += (syp - bl->y) * 0.14f;
            } else {
                s_pelAngle[i] += 0.04f;
                bl->x = BOX_CX + cosf(s_pelAngle[i]) * PEL_RADIUS;
                bl->y = BOX_CY + sinf(s_pelAngle[i]) * PEL_RADIUS;
            }
        }
    }

    // Action beats (no text): run then auto-advance.
    if (b->text == NULL) {
        s_actTimer++;
        switch (b->action) {
            case ACT_RISE:
                if (s_actTimer > 45 && s_actTimer % 4 == 0 && s_riseFrame < 8) s_riseFrame++;
                if (s_riseFrame >= 8 && s_actTimer > 100) beginBeat(s_beat + 1);
                break;
            case ACT_HIT:
                s_converge = true;
                if (battleCheckHits(19) || s_actTimer > 150) {
                    battleClearBullets(); s_pelletsShown = false; s_soulControllable = false;
                    beginBeat(s_beat + 1);
                }
                break;
            case ACT_TORIEL:
                if (s_toriel > 0) s_toriel--;
                if (s_actTimer > 90) beginBeat(s_beat + 1);
                break;
            case ACT_END:
                s_active = false; s_combat = false;
                break;
            default:
                if (s_actTimer > 30) beginBeat(s_beat + 1);
        }
        return;
    }

    // Dialogue beat: typewriter; A/B reveals or advances.
    int len = (int)strlen(b->text);
    if (s_revealed < len) {
        if (kDown & (KEY_A | KEY_B)) s_revealed = len;
        else if (++s_charTimer >= CHAR_DELAY) { s_charTimer = 0; s_revealed++; }
    } else if (kDown & (KEY_A | KEY_B)) {
        // Leaving the "Move around!" beat springs the trap.
        if (b->action == ACT_PELLETS_MOVE) s_converge = true;
        if (b->action == ACT_RING) spawnPelletRing(); // reform a ring for the "DIE" beat
        beginBeat(s_beat + 1);
    }
}

// ---- Drawing ----

// easeOutBack: overshoots slightly past 1 then settles — a springy "pop".
static float easeOutBack(float p) {
    const float c1 = 1.70158f, c3 = c1 + 1.0f;
    float q = p - 1.0f;
    return 1.0f + c3 * q * q * q + c1 * q * q;
}

static void drawFloweyWorld(float ox, float oy) {
    // Overworld: the colored flower POPS out of the green patch (bottom-anchored so its base
    // stays on the patch while it springs up).
    float scale = FLOWEY_SCALE;
    if (BEATS[s_beat].action == ACT_RISE) {
        if (s_actTimer <= 45) return; // still fading in
        float p = s_riseFrame / 8.0f;
        scale = FLOWEY_SCALE * easeOutBack(p);
        if (scale < 0.01f) scale = 0.01f;
    }
    float bw = 25 * scale, bh = 25 * scale;
    const float baseY = FLOWEY_CY + (25 * FLOWEY_SCALE) / 2; // fixed ground line on the patch
    C2D_DrawImageAt(s_body[s_faceFrame], ox + FLOWEY_CX - bw / 2,
                    oy + baseY - bh, 0.2f, NULL, scale, scale);
}

static void drawFloweyCombat(void) {
    // Combat view: black field, Flowey looms at the top (nice flower / evil face), box below.
    C2D_DrawRectSolid(0, 0, 0.0f, TOP_W, SCR_H, COL_BLACK);
    const Beat* b = &BEATS[s_beat];
    if (b->face == FACE_EVIL) {
        C2D_Image f = s_evil[s_faceFrame];
        C2D_DrawImageAt(f, TOP_W / 2 - 42, 8, 0.2f, NULL, 2.0f, 2.0f); // 42x44 *2
    } else {
        C2D_Image f = s_body[s_faceFrame];
        C2D_DrawImageAt(f, TOP_W / 2 - 25, 20, 0.2f, NULL, 2.0f, 2.0f); // 25x25 *2
    }
    battleDrawBox();
    battleDrawBullets();
    battleDrawSoul();
    battleDrawHP(BOX_X, BOX_Y + BOX_H + 8);
}

void floweyDrawTop(float camX, float camY, int topXOffset) {
    if (!s_active) return;
    if (!s_combat) { drawFloweyWorld(topXOffset - camX, -camY); return; }
    drawFloweyCombat();
    if (s_toriel > 0)
        C2D_DrawRectSolid(0, 0, 0.5f, TOP_W, SCR_H, C2D_Color32(0xFF, 0xFF, 0xFF, (u8)(s_toriel * 2)));
}

// Undertale-style textbox with the speaker's face.
#define TB_X 10
#define TB_Y 46
#define TB_W 300
#define TB_H 150
#define TB_BORDER 4
#define TB_FACE_SCALE 2.0f
#define TB_FACE_W (42 * TB_FACE_SCALE)
#define TB_FACE_H (44 * TB_FACE_SCALE)
#define TB_FACE_X (TB_X + 16)
#define TB_FACE_Y (TB_Y + (TB_H - (int)TB_FACE_H) / 2)
#define TB_TEXT_X (TB_FACE_X + (int)TB_FACE_W + 14)
#define TB_TEXT_Y (TB_Y + 28)

static void drawTextbox(C2D_Image* face, const char* text) {
    C2D_DrawRectSolid(TB_X, TB_Y, 0.0f, TB_W, TB_H, COL_WHITE);
    C2D_DrawRectSolid(TB_X + TB_BORDER, TB_Y + TB_BORDER, 0.0f,
                      TB_W - 2 * TB_BORDER, TB_H - 2 * TB_BORDER, COL_BLACK);
    if (face) C2D_DrawImageAt(*face, TB_FACE_X, TB_FACE_Y, 0.1f, NULL, TB_FACE_SCALE, TB_FACE_SCALE);
    if (text) {
        if (text[0] == '*' && text[1] == ' ') text += 2;
        txtDraw(text, TB_TEXT_X, TB_TEXT_Y, 0.6f, COL_WHITE);
    }
}

void floweyDrawBottom(void) {
    if (!s_active) return;
    const Beat* b = &BEATS[s_beat];
    if (b->text == NULL) {
        if (b->action == ACT_TORIEL) drawTextbox(NULL, "* ... (a warm\n  light appears.)");
        return;
    }
    C2D_Image face = (b->face == FACE_EVIL) ? s_evil[s_faceFrame] : s_nice[s_faceFrame];
    char shown[220];
    int n = s_revealed < 219 ? s_revealed : 219;
    memcpy(shown, b->text, n);
    for (int i = 0; i < n; i++) if (shown[i] == '&') shown[i] = '\n';
    shown[n] = '\0';
    drawTextbox(&face, shown);
}
