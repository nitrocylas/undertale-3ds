// Flowey's introduction — reconstructed 1:1 from the game's own controller (obj_floweybattle1)
// and dialogue (all decompiled from data.win). Overworld: the flower grows and talks. Battle:
// he winks (with a winkstar) on "See that heart?", cycles expressions
// (niceside/nice/sassy/pissed/evil/laugh), spawns friendliness pellets that home in, and on
// the trap Toriel's flame knocks him spinning off-screen while your HP is restored.
#include "flowey.h"
#include "game.h"
#include "battlebox.h"
#include "audio.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "flowey_nice0_t3x.h"
#include "flowey_nice1_t3x.h"
#include "flowey_evil0_t3x.h"
#include "flowey_evil1_t3x.h"
#include "flowey_body0_t3x.h"
#include "flowey_body1_t3x.h"
#include "flowey_wink_t3x.h"
#include "flowey_niceside0_t3x.h"
#include "flowey_niceside1_t3x.h"
#include "flowey_sassy0_t3x.h"
#include "flowey_sassy1_t3x.h"
#include "flowey_pissed0_t3x.h"
#include "flowey_pissed1_t3x.h"
#include "flowey_laugh0_t3x.h"
#include "flowey_laugh1_t3x.h"
#include "flowey_grin0_t3x.h"
#include "flowey_grin1_t3x.h"
#include "flowey_winkstar_t3x.h"
#include "torielflame0_t3x.h"
#include "torielflame1_t3x.h"
#include "torielflame2_t3x.h"
#include "torielflame3_t3x.h"
#include "snd_heartshot_wav.h"

// Overworld flower position (arena space).
#define FLOWEY_CX 160
#define FLOWEY_CY 300
#define FLOWEY_SCALE 1.5f

// Combat view (top-screen pixel space).
#define BOX_X 125
#define BOX_Y 108
#define BOX_W 150
#define BOX_H 100
#define BOX_CX (BOX_X + BOX_W / 2)
#define BOX_CY (BOX_Y + BOX_H / 2)
#define NUM_PELLETS 5
#define FLOWEY_COMBAT_CX (TOP_W / 2)
#define FLOWEY_COMBAT_CY 46          // face center up top
#define PEL_SPAWN_Y 84

// Flowey expressions -> which sprite shows in the battle.
typedef enum { EX_NICE, EX_WINK, EX_NICESIDE, EX_SASSY, EX_PISSED, EX_GRIN, EX_EVIL, EX_LAUGH } Expr;
typedef struct { const char* text; Expr expr; int action; } Beat;

enum { ACT_NONE, ACT_RISE, ACT_TRANSITION, ACT_WINK, ACT_PELLETS_SHOW, ACT_PELLETS_MOVE,
       ACT_PELLETS_LAST, ACT_RING_TRAP, ACT_HIT, ACT_RING, ACT_TORIEL, ACT_END, ACT_PAUSE,
       ACT_HITPATH, ACT_DODGEPATH, ACT_JUMP_HIT }; // post-pellet branch: hit -> msc669, dodge -> msc673

// Battle dialogue is the game's own verbatim text (data.win: SCR_TEXT_291-297 overworld,
// SCR_TEXT_3495-3520 battle). Expressions & pellet timing follow obj_floweybattle1's own
// conversation state machine exactly: msc666 nice -> wink -> msc667 niceside (+pellets) ->
// msc668 nice (wave 1) -> sassy (wave 2) -> pissed / nice (wave 3) -> grin -> evil (+ring)
// -> laugh (ring attacks) -> Toriel.
static const Beat BEATS[] = {
    { NULL, EX_NICE, ACT_RISE },
    { "* Howdy^2!&* I'm FLOWEY.^2&* FLOWEY the FLOWER!", EX_NICE, ACT_NONE }, // 291
    { "* Hmmm...", EX_NICE, ACT_NONE },                                       // 292
    { "* You're new to the&  UNDERGROUND^2, aren'tcha?", EX_NICE, ACT_NONE }, // 293
    { "* Golly^1, you must be&  so confused.", EX_NICE, ACT_NONE },           // 294
    { "* Someone ought to teach&  you how things work&  around here!", EX_NICE, ACT_NONE }, // 295
    { "* I guess little old me&  will have to do.", EX_NICE, ACT_NONE },      // 296
    { "* Ready^2?&* Here we go!", EX_NICE, ACT_NONE },                        // 297
    { NULL, EX_NICE, ACT_TRANSITION }, // heart flash + black -> battle box
    // --- battle (obj_floweybattle1) ---
    // conv 1 / msc 666: the whole SOUL/LOVE speech, one neutral face.
    { "See that heart^1?&That is your SOUL^1,&the very culmination&of your being!", EX_NICE, ACT_NONE }, // 3495
    { "Your SOUL starts off&weak^1, but can grow&strong if you gain&a lot of LV.", EX_NICE, ACT_NONE },// 3496
    { "What's LV stand for^1?&Why^1, LOVE^1, of course!", EX_NICE, ACT_NONE },// 3497
    { "You want some&LOVE, don't you?", EX_NICE, ACT_NONE },                  // 3498
    { "Don't worry,&I'll share some&with you!", EX_NICE, ACT_NONE },          // 3499
    { NULL, EX_WINK, ACT_WINK },                          // conv 1->1.5: a wink + winkstar
    // conv 1.5 / msc 667: niceside; the 5 friendliness pellets appear.
    { "Down here^1, LOVE is&shared through..^1.", EX_NICESIDE, ACT_PELLETS_SHOW },// 3503
    { "Little white..^2.&\"friendliness&pellets.\"", EX_NICESIDE, ACT_NONE }, // 3504
    // conv 2 / msc 668: nice; the pellets attack (wave 1).
    { "Are you ready?", EX_NICE, ACT_NONE },                                  // 3505
    { "Move around^1!&Get as many as&you can^2!", EX_NICE, ACT_PELLETS_MOVE },// 3509  wave 1
    // conv 5 -> sassy, then msc 671; conv 7 -> nice + wave 2.
    { "Hey buddy^1,&you missed them.", EX_SASSY, ACT_NONE },                  // 3524
    { "Let's try again^1,&okay?", EX_SASSY, ACT_PELLETS_MOVE },               // 3525  wave 2
    // conv 9 -> pissed. He barks "...BULLETS!!!" (his mask slips, SCR_TEXT_3528), catches
    // himself, and forces a smile as he corrects it to "friendliness pellets" (268) + wave 3.
    { "Is this a joke^2?&Are you braindead^2?", EX_PISSED, ACT_NONE },        // 672
    { "RUN^2. INTO^2. THE^2.&BULLETS!!!", EX_PISSED, ACT_NONE },              // 3528 — the angry slip
    { "...friendliness&pellets!", EX_NICE, ACT_PELLETS_LAST },                // 268 — smiling correction, wave 3
    // If you were HIT at any point -> msc 669 (grin). If you dodged everything -> msc 673 (evil).
    { "You idiot.", EX_GRIN, ACT_HITPATH },                                   // 3514
    { "In this world^1, it's&kill or BE killed.", EX_GRIN, ACT_NONE },        // 3515
    { "Why would ANYONE pass&up an opportunity&like this!?", EX_GRIN, ACT_JUMP_HIT }, // 3516 -> evil+ring
    { "You know what's&going on here^1,&don't you?", EX_EVIL, ACT_DODGEPATH }, // 3531 (dodge path)
    { "You just wanted to&see me suffer.", EX_EVIL, ACT_NONE },               // 3532
    { NULL, EX_EVIL, ACT_HIT },                           // conv 12->13: evil turn + ring spawns
    { "Die.", EX_EVIL, ACT_NONE },                                           // 3520  ring closing
    { NULL, EX_LAUGH, ACT_RING },                         // conv 14: laugh + ring attacks -> Toriel
    { NULL, EX_EVIL, ACT_TORIEL },                        // Toriel's flame drives him off
    { NULL, EX_NICE, ACT_END },
};
#define BEAT_COUNT ((int)(sizeof(BEATS) / sizeof(BEATS[0])))
#define CHAR_DELAY 6   // 60fps port vs Undertale's 30fps -> double the per-char delay

// Transition timing (frames).
#define TR_SOUND2 30
#define TR_FLASH  58
#define TR_BLACK  74
#define TR_DONE   108

static C2D_SpriteSheet s_sheets[24];
static C2D_Image s_nice[2], s_evil[2], s_body[2], s_niceside[2], s_sassy[2], s_pissed[2],
                 s_laugh[2], s_grin[2], s_wink, s_winkstar, s_flame[4];

static bool  s_active, s_combat, s_finished;
static int   s_beat, s_revealed, s_charTimer, s_actTimer, s_faceTimer, s_faceFrame, s_riseFrame;
static int   s_pauseTimer;     // honors the game's ^N in-line pause codes
#define PAUSE_UNIT 11          // frames per ^N level (^1 short, ^2 longer) — the dramatic beats
static int   s_animClock;
static bool  s_pelletsShown, s_converge, s_soulControllable, s_transSound2;
static int   s_pelIdx[NUM_PELLETS];
static float s_pelAngle[NUM_PELLETS];
static float s_pelSpeed[NUM_PELLETS], s_pelDir[NUM_PELLETS]; // accelerating dive (obj_friendlypellet)
static float s_pelRestX[NUM_PELLETS], s_pelRestY[NUM_PELLETS]; // where each pellet fans out to
static bool  s_pelAttack;                                    // the wave is diving at the SOUL

// The radial "friendliness pellets": a spinning emitter spirals pellets out (one every few
// frames), which then slowly close in — matching obj_radialfakegen / obj_fakepellet.
#define RING_COUNT 18
static int   s_ringIdx[RING_COUNT];
static int   s_ringSpawned, s_ringEmit;
static bool  s_ringActive, s_ringHoming;
// Toriel finale.
static int   s_toriel;         // countdown
static float s_flameX, s_floweySpin, s_floweyFly, s_floweyFlyX, s_floweyVY;
static int   s_torielFlash;    // white impact flash frames
static bool  s_gotHit;         // did any pellet wave hit you? -> picks Flowey's post-pellet line
static float s_ringCX, s_ringCY;   // center the closing ring on the SOUL

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
    s_wink    = ld(6, flowey_wink_t3x, flowey_wink_t3x_size);
    s_niceside[0] = ld(7, flowey_niceside0_t3x, flowey_niceside0_t3x_size);
    s_niceside[1] = ld(8, flowey_niceside1_t3x, flowey_niceside1_t3x_size);
    s_sassy[0] = ld(9, flowey_sassy0_t3x, flowey_sassy0_t3x_size);
    s_sassy[1] = ld(10, flowey_sassy1_t3x, flowey_sassy1_t3x_size);
    s_pissed[0] = ld(11, flowey_pissed0_t3x, flowey_pissed0_t3x_size);
    s_pissed[1] = ld(12, flowey_pissed1_t3x, flowey_pissed1_t3x_size);
    s_laugh[0] = ld(13, flowey_laugh0_t3x, flowey_laugh0_t3x_size);
    s_laugh[1] = ld(14, flowey_laugh1_t3x, flowey_laugh1_t3x_size);
    s_winkstar = ld(15, flowey_winkstar_t3x, flowey_winkstar_t3x_size);
    s_flame[0] = ld(16, torielflame0_t3x, torielflame0_t3x_size);
    s_flame[1] = ld(17, torielflame1_t3x, torielflame1_t3x_size);
    s_flame[2] = ld(18, torielflame2_t3x, torielflame2_t3x_size);
    s_flame[3] = ld(19, torielflame3_t3x, torielflame3_t3x_size);
    s_grin[0] = ld(20, flowey_grin0_t3x, flowey_grin0_t3x_size);
    s_grin[1] = ld(21, flowey_grin1_t3x, flowey_grin1_t3x_size);
    battleLoad();
}
void floweyFree(void) { for (int i = 0; i < 22; i++) C2D_SpriteSheetFree(s_sheets[i]); battleFree(); }

bool floweyActive(void) { return s_active; }
bool floweyInCombat(void) { return s_active && s_combat; }
bool floweyFinished(void) { return s_finished; }

// Expression -> big battle face (2 talk frames).
static C2D_Image exprFace(Expr e, int frame) {
    switch (e) {
        case EX_WINK:     return s_wink;
        case EX_NICESIDE: return s_niceside[frame];
        case EX_SASSY:    return s_sassy[frame];
        case EX_PISSED:   return s_pissed[frame];
        case EX_GRIN:     return s_grin[frame];
        case EX_EVIL:     return s_evil[frame];
        case EX_LAUGH:    return s_laugh[frame];
        default:          return s_nice[frame];
    }
}

// The 5 friendliness pellets hover in a fan above the box (obj_friendlypellet fans out at
// direction i*30 then stops), bobbing gently until Flowey sends them at you.
static void spawnPellets(void) {
    s_pelletsShown = true; s_converge = false; s_pelAttack = false;
    battleClearBullets();
    for (int i = 0; i < NUM_PELLETS; i++) {
        s_pelRestX[i] = FLOWEY_COMBAT_CX + (i - 2) * 30.0f;        // fanned arc above the box
        s_pelRestY[i] = PEL_SPAWN_Y + fabsf((float)(i - 2)) * 4.0f;
        s_pelIdx[i] = battleAddBullet(s_pelRestX[i], s_pelRestY[i], 5.0f);
        s_pelAngle[i] = (float)i * 1.2f;
        s_pelSpeed[i] = 0.0f; s_pelDir[i] = 0.0f;
    }
}
// Launch the wave: each pellet aims at the SOUL's CURRENT position and accelerates in — the real
// obj_friendlypellet gives itself a tiny speed toward the heart then negative friction speeds it up.
static void pelletsAttack(void) {
    s_pelAttack = true;
    float sx, sy; battleSoulPos(&sx, &sy);
    for (int i = 0; i < NUM_PELLETS; i++) {
        Bullet* bl = battleBullet(s_pelIdx[i]);
        if (!bl) continue;
        s_pelDir[i] = atan2f(sy - bl->y, sx - bl->x) + ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
        s_pelSpeed[i] = 0.35f;
    }
}
// Begin the radial: an emitter that spirals pellets out AROUND the SOUL, which then close
// in slowly. The heart is locked in place (as in the real fight).
static void initRing(void) {
    s_ringActive = true; s_ringSpawned = 0; s_ringEmit = 0; s_ringHoming = false;
    for (int i = 0; i < RING_COUNT; i++) s_ringIdx[i] = -1;
    s_soulControllable = false;
    s_pelletsShown = false; s_converge = false;
    for (int i = 0; i < NUM_PELLETS; i++) s_pelIdx[i] = -1;
    battleClearBullets();
    battleSoulPos(&s_ringCX, &s_ringCY); // the ring circles YOU
}

// Per-frame: spiral pellets outward from a spinning emitter, then close them in very slowly.
// Returns true when a pellet reaches the SOUL (Toriel's cue).
static bool tickRing(void) {
    if (s_ringSpawned < RING_COUNT) {
        if (s_ringEmit++ % 3 == 0) {        // one pellet every 3 frames (gradual)
            float ang = s_ringSpawned * 0.9f;         // spin
            float rad = 8.0f + s_ringSpawned * 2.2f;  // grow outward -> spiral around the SOUL
            s_ringIdx[s_ringSpawned] = battleAddBullet(s_ringCX + cosf(ang) * rad,
                                                       s_ringCY + sinf(ang) * rad, 4.0f);
            s_ringSpawned++;
        }
        return false;
    }
    s_ringHoming = true;
    bool reached = false;
    for (int i = 0; i < RING_COUNT; i++) {
        Bullet* bl = battleBullet(s_ringIdx[i]);
        if (!bl || !bl->active) continue;
        bl->x += (s_ringCX - bl->x) * 0.007f;   // slow close-in over the whole monologue
        bl->y += (s_ringCY - bl->y) * 0.007f;
        float dx = bl->x - s_ringCX, dy = bl->y - s_ringCY;
        if (dx * dx + dy * dy < 10 * 10) reached = true;
    }
    return reached;
}
// The wave is dodged once every pellet has left the box (all gone inactive) without hitting you.
static bool pelletsDodged(void) {
    if (!s_pelAttack) return false;
    for (int i = 0; i < NUM_PELLETS; i++) {
        Bullet* bl = battleBullet(s_pelIdx[i]);
        if (bl && bl->active) return false; // one still in play
    }
    return true;
}
static int findAction(int act) {
    for (int i = 0; i < BEAT_COUNT; i++) if (BEATS[i].action == act) return i;
    return BEAT_COUNT - 1;
}
static int hitBeatIndex(void) { return findAction(ACT_HIT); }
static int torielBeatIndex(void) {
    for (int i = 0; i < BEAT_COUNT; i++) if (BEATS[i].action == ACT_TORIEL) return i;
    return BEAT_COUNT - 1;
}

static void beginBeat(int idx) {
    s_beat = idx; s_revealed = 0; s_charTimer = 0; s_actTimer = 0; s_pauseTimer = 0;
    if (idx >= BEAT_COUNT) { s_active = false; return; }
    switch (BEATS[idx].action) {
        case ACT_TRANSITION: s_transSound2 = false; break;
        case ACT_PELLETS_SHOW: spawnPellets(); break;
        case ACT_PELLETS_MOVE:
        case ACT_PELLETS_LAST:
            if (!s_pelletsShown) spawnPellets(); // waves 2 & 3 re-form the fan; wave 1 reuses the shown pellets
            s_soulControllable = true;
            pelletsAttack();
            break;
        case ACT_TORIEL:
            s_flameX = TOP_W + 20; s_floweySpin = 0; s_floweyFly = 0; s_floweyFlyX = 0; s_floweyVY = 0; s_torielFlash = 0;
            battleClearBullets(); s_pelletsShown = false; s_soulControllable = false;
            s_ringActive = false; s_ringHoming = false;
            battleSetHP(20, 20); // Toriel's flame drives the ring off and restores your HP to full
            break;
        default: break;
    }
}

#ifdef DEBUG_FLOWEY_RING
// Debug: jump straight into the radial ("friendliness pellets!") to inspect the spiral.
void floweyStart(void) {
    s_active = true; s_combat = true;
    s_faceFrame = 0; s_faceTimer = 0; s_animClock = 0;
    s_pelletsShown = false; s_converge = false; s_soulControllable = false;
    s_ringActive = false; s_ringHoming = false; s_ringSpawned = 0; s_toriel = 0;
    battleSetBox(BOX_X, BOX_Y, BOX_W, BOX_H);
    battleSetHP(20, 20);
    for (int i = 0; i < BEAT_COUNT; i++)
        if (BEATS[i].action == ACT_RING_TRAP) { beginBeat(i); s_revealed = 999; return; }
    beginBeat(0);
}
#else
void floweyStart(void) {
    s_active = true; s_combat = false; s_finished = false;
    s_faceFrame = 0; s_faceTimer = 0; s_riseFrame = 0; s_animClock = 0;
    s_pelletsShown = false; s_converge = false; s_soulControllable = false;
    s_ringActive = false; s_ringHoming = false; s_ringSpawned = 0;
    s_toriel = 0; s_gotHit = false;
    beginBeat(0);
}
#endif

void floweyUpdate(u32 kDown, u32 kHeld) {
    if (!s_active) return;
    const Beat* b = &BEATS[s_beat];
    s_animClock++;
    if (++s_faceTimer >= 12) { s_faceTimer = 0; s_faceFrame ^= 1; }

    if (s_combat) battleUpdateSoul(kHeld, s_soulControllable);

    // Friendliness-pellet motion: bob in place until the wave launches, then accelerate at the
    // SOUL along the aimed direction. A pellet that leaves the box has "missed" (goes inactive).
    if (s_pelletsShown) {
        for (int i = 0; i < NUM_PELLETS; i++) {
            Bullet* bl = battleBullet(s_pelIdx[i]);
            if (!bl || !bl->active) continue;
            if (s_pelAttack) {
                s_pelSpeed[i] += 0.02f;                  // negative friction -> accelerating (60fps)
                bl->x += cosf(s_pelDir[i]) * s_pelSpeed[i];
                bl->y += sinf(s_pelDir[i]) * s_pelSpeed[i];
                // A "miss" is only leaving the SIDES or the BOTTOM — never the top (they start there).
                if (bl->x < BOX_X - 30 || bl->x > BOX_X + BOX_W + 30 || bl->y > BOX_Y + BOX_H + 20)
                    bl->active = false;
            } else {
                s_pelAngle[i] += 0.08f;
                bl->y = s_pelRestY[i] + sinf(s_pelAngle[i]) * 1.5f; // bob gently around the rest spot
            }
        }
    }

    // The radial ring persists across the evil monologue; when it reaches you -> Toriel.
    if (s_ringActive && BEATS[s_beat].action != ACT_TORIEL) {
        if (tickRing()) { beginBeat(torielBeatIndex()); return; }
    }

    // Action beats (no text).
    if (b->text == NULL) {
        s_actTimer++;
        switch (b->action) {
            case ACT_RISE:
                // 12-frame beat + ~30-frame unfurl, then he starts talking (~0.7s, matches the game).
                if (s_actTimer > 44) beginBeat(s_beat + 1);
                break;
            case ACT_TRANSITION:
                if (s_actTimer == 1) audioPlayWav(snd_heartshot_wav, snd_heartshot_wav_size);
                if (s_actTimer >= TR_SOUND2 && !s_transSound2) {
                    s_transSound2 = true; audioPlayWav(snd_heartshot_wav, snd_heartshot_wav_size);
                }
                if (s_actTimer == TR_BLACK) {
                    s_combat = true;
                    battleSetBox(BOX_X, BOX_Y, BOX_W, BOX_H);
                    battleSetHP(20, 20);
                    s_soulControllable = true;
                }
                if (s_actTimer > TR_DONE) beginBeat(s_beat + 1);
                break;
            case ACT_WINK:
                // conv 1->1.5: a quick wink with the star, then on to the pellets.
                if (s_actTimer > 45) beginBeat(s_beat + 1);
                break;
            case ACT_HIT:
                // conv 12->13: the evil turn — the radial ring of pellets spawns and begins
                // to close in around the locked SOUL.
                if (s_actTimer == 1) initRing();
                if (s_actTimer > 35) beginBeat(s_beat + 1);
                break;
            case ACT_RING:
                // conv 14/15: the laughing face while the ring closes; Toriel's flame cuts in
                // when a pellet reaches you (tickRing above) or after the set delay.
                if (s_actTimer > 150) beginBeat(torielBeatIndex());
                break;
            case ACT_TORIEL: {
                // Real obj_floweybattle1 finale: flame sweeps in from Flowey's right, snaps the
                // ring away with a flash, then Flowey reacts (pissed ~40f, shocked ~40f) BEFORE
                // he's knocked spinning up-and-off — that pause is what keeps it from feeling abrupt.
                int t = s_actTimer;
                float target = FLOWEY_COMBAT_CX + 52;
                if (t < 24) s_flameX = (TOP_W + 20) + (target - (TOP_W + 20)) * (t / 24.0f);
                else        s_flameX = target;
                if (t == 24) s_torielFlash = 10;                 // impact
                if (s_torielFlash > 0) s_torielFlash--;
                if (t == 115) s_floweyVY = -9.0f;                // launch up-and-left (dir ~155)...
                if (t >= 115) {                                  // ...then gravity arcs him down & off
                    s_floweySpin += 12;
                    s_floweyFlyX -= 8.0f;                        // to the left
                    s_floweyVY += 0.6f;                          // gravity
                    s_floweyFly += s_floweyVY;
                }
                if (t > 180) beginBeat(s_beat + 1);
                break;
            }
            case ACT_END:
                s_active = false; s_combat = false; s_finished = true;
                break;
            default:
                if (s_actTimer > 30) beginBeat(s_beat + 1);
        }
        return;
    }

    // Dialogue typewriter — types one char per CHAR_DELAY frames, and holds at a ^N code for
    // N*PAUSE_UNIT frames (the game's built-in dramatic pauses, e.g. "RUN^2. INTO^2. THE.").
    int len = (int)strlen(b->text);
    if (s_revealed < len) {
        if (kDown & (KEY_A | KEY_B)) { s_revealed = len; s_pauseTimer = 0; }
        else if (s_pauseTimer > 0) s_pauseTimer--;
        else if (++s_charTimer >= CHAR_DELAY) {
            s_charTimer = 0;
            if (b->text[s_revealed] == '^' && s_revealed + 1 < len) {
                s_pauseTimer = (b->text[s_revealed + 1] - '0') * PAUSE_UNIT;
                s_revealed += 2;             // skip the code itself
            } else {
                s_revealed++;
            }
        }
        return;
    }

    // Dramatic pause: hold on the barked line, then auto-advance to the smiling reveal.
    if (b->action == ACT_PAUSE) {
        s_actTimer++;
        if (s_actTimer > 55) beginBeat(s_beat + 1);
        return;
    }

    // Live dodge: SOUL moves, pellets dive. Getting hit ANY wave -> the "You idiot" (msc669) path.
    if (b->action == ACT_PELLETS_MOVE) {
        if (battleCheckHits(19)) {
            battleClearBullets(); s_pelletsShown = false; s_soulControllable = false;
            s_gotHit = true; beginBeat(findAction(ACT_HITPATH));
        } else if (pelletsDodged()) {
            battleClearBullets(); s_pelletsShown = false; s_soulControllable = false;
            beginBeat(s_beat + 1);
        }
        s_actTimer++;
        return;
    }
    // The FINAL wave (after "RUN. INTO. THE. friendliness pellets!"): whether you dodge or
    // get hit, Flowey is done playing -> the evil turn + the ring.
    // Final wave: hit -> "You idiot" (msc669); a clean dodge of all three -> "You know what's
    // going on here, don't you?" (msc673).
    if (b->action == ACT_PELLETS_LAST) {
        if (battleCheckHits(19)) {
            battleClearBullets(); s_pelletsShown = false; s_soulControllable = false;
            s_gotHit = true; beginBeat(findAction(ACT_HITPATH));
        } else if (pelletsDodged()) {
            battleClearBullets(); s_pelletsShown = false; s_soulControllable = false;
            beginBeat(findAction(ACT_DODGEPATH));
        }
        s_actTimer++;
        return;
    }
    // "friendliness pellets!": start the radial spiral, hold as it forms, then continue the
    // evil monologue while it slowly closes in (Toriel triggers when it reaches you).
    if (b->action == ACT_RING_TRAP) {
        // "friendliness pellets!" — a beat, then into the evil turn (the ring spawns there).
        s_actTimer++;
        if (s_actTimer > 45) beginBeat(hitBeatIndex());
        return;
    }

    if (kDown & (KEY_A | KEY_B)) beginBeat(b->action == ACT_JUMP_HIT ? hitBeatIndex() : s_beat + 1);
}

// ---- Drawing ----

static float easeOutBack(float p) {
    const float c1 = 1.70158f, c3 = c1 + 1.0f;
    float q = p - 1.0f;
    return 1.0f + c3 * q * q * q + c1 * q * q;
}

static void drawFloweyWorld(float ox, float oy) {
    const float baseY = FLOWEY_CY + (25 * FLOWEY_SCALE) / 2; // his base sits on the flowerbed
    float xs = FLOWEY_SCALE, ys = FLOWEY_SCALE;
    if (BEATS[s_beat].action == ACT_RISE) {
        if (s_actTimer <= 12) return;                    // a brief beat, then he springs up
        float p = (s_actTimer - 12) / 30.0f;             // unfurl over ~30 frames (~0.5s), like riseanim
        if (p > 1.0f) p = 1.0f;
        ys = FLOWEY_SCALE * easeOutBack(p);              // grow upward, anchored at the ground
        if (ys < 0.01f) ys = 0.01f;
    }
    float bw = 25 * xs, bh = 25 * ys;
    C2D_DrawImageAt(s_body[s_faceFrame], ox + FLOWEY_CX - bw / 2, oy + baseY - bh, 0.2f, NULL, xs, ys);
}

static void drawTransition(void) {
    float cx = TOP_W / 2.0f, cy = SCR_H / 2.0f;
    if ((s_actTimer / 4) % 2 == 0) {
        float sz = 16 + (s_actTimer * 0.2f);
        C2D_DrawRectSolid(cx - sz / 2, cy - sz / 2, 0.5f, sz, sz, C2D_Color32(0xFF, 0, 0, 0xFF));
    }
    if (s_actTimer >= TR_FLASH) {
        int f = s_actTimer - TR_FLASH;
        u8 a = (u8)(f < 8 ? f * 32 : 255);
        C2D_DrawRectSolid(0, 0, 0.6f, TOP_W, SCR_H, C2D_Color32(0xFF, 0xFF, 0xFF, a));
    }
}

static void drawFloweyCombat(void) {
    C2D_DrawRectSolid(0, 0, 0.0f, TOP_W, SCR_H, COL_BLACK);
    const Beat* b = &BEATS[s_beat];

    // Flowey's big face (2x). During the finale he holds a reaction (laugh -> pissed) then spins away.
    if (b->action != ACT_TORIEL || s_floweyFly < SCR_H) {
        C2D_Image face;
        if (b->action == ACT_TORIEL) face = (s_actTimer < 24) ? s_laugh[s_faceFrame] : s_pissed[s_faceFrame];
        else                         face = exprFace(b->expr, s_faceFrame);
        float fx = FLOWEY_COMBAT_CX + s_floweyFlyX, fy = FLOWEY_COMBAT_CY + s_floweyFly;
        // Plain DrawImageAt when upright (DrawImageAtRotated renders nothing at angle 0); rotated
        // only once he's actually spinning off in the finale. 42x44 sprite at 2x -> center offset.
        if (s_floweySpin != 0.0f)
            C2D_DrawImageAtRotated(face, fx, fy, 0.2f, s_floweySpin * (float)M_PI / 180.0f, NULL, 2.0f, 2.0f);
        else
            C2D_DrawImageAt(face, fx - 42.0f, fy - 44.0f, 0.2f, NULL, 2.0f, 2.0f);
        // Winkstar during the "See that heart?" wink.
        if (b->expr == EX_WINK) {
            float ss = 0.6f + 0.4f * fabsf(sinf(s_animClock / 6.0f));
            C2D_DrawImageAtRotated(s_winkstar, fx + 40, fy - 18, 0.3f, s_animClock * 0.1f, NULL, ss, ss);
        }
    }

    battleDrawBox();
    battleDrawBullets();
    battleDrawSoul();
    battleDrawHP(BOX_X, BOX_Y + BOX_H + 8);

    // Toriel's flame sweeps in from the right and lingers on Flowey through his reaction.
    if (b->action == ACT_TORIEL) {
        if (s_actTimer < 120)
            C2D_DrawImageAtRotated(s_flame[(s_animClock / 6) % 4], s_flameX, FLOWEY_COMBAT_CY + 10,
                                   0.5f, 0, NULL, 2.0f, 2.0f);
        if (s_torielFlash > 0) { // white pop on impact
            u8 a = (u8)(s_torielFlash * 22);
            C2D_DrawRectSolid(0, 0, 0.6f, TOP_W, SCR_H, C2D_Color32(0xFF, 0xFF, 0xFF, a));
        }
    }
}

void floweyDrawTop(float camX, float camY, int topXOffset) {
    if (!s_active) return;
    if (!s_combat) {
        drawFloweyWorld(topXOffset - camX, -camY);
        if (BEATS[s_beat].action == ACT_TRANSITION) drawTransition();
        return;
    }
    drawFloweyCombat();
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

void floweyDrawBottom(void) {
    if (!s_active) return;
    const Beat* b = &BEATS[s_beat];
    C2D_DrawRectSolid(TB_X, TB_Y, 0.0f, TB_W, TB_H, COL_WHITE);
    C2D_DrawRectSolid(TB_X + TB_BORDER, TB_Y + TB_BORDER, 0.0f,
                      TB_W - 2 * TB_BORDER, TB_H - 2 * TB_BORDER, COL_BLACK);
    if (b->action == ACT_TORIEL) return; // no narration during the flame — Toriel speaks after, in her own scene
    if (b->text == NULL) return;

    C2D_Image face = exprFace(b->expr, s_faceFrame);
    C2D_DrawImageAt(face, TB_FACE_X, TB_FACE_Y, 0.1f, NULL, TB_FACE_SCALE, TB_FACE_SCALE);
    char shown[220];
    const char* t = b->text;
    int start = (t[0] == '*' && t[1] == ' ') ? 2 : 0;
    int j = 0;
    for (int i = start; i < s_revealed && t[i] && j < 219; i++) {
        if (t[i] == '^' && t[i + 1]) { i++; continue; } // hide the ^N pause codes
        shown[j++] = (t[i] == '&') ? '\n' : t[i];
    }
    shown[j] = '\0';
    txtDraw(shown, TB_TEXT_X, TB_TEXT_Y, 0.6f, COL_WHITE);
}
