// Overworld scene: a small multi-room engine. A room is data — background, a per-column
// walkable band, size, and a list of door triggers. Walking into a door fades to white and
// loads the target room at a spawn point. Currently: room_area1 (flower bed) <-> room_area1_2
// (Flowey's arena). Adding a room = drop in its bg t3x + band header + a RoomDef entry.
#include "game.h"
#include "flowey.h"
#include <math.h>
#include "collision_area1_bands.h"
#include "collision_area1_2_bands.h"

#include "room_area1_bg_t3x.h"
#include "room_area1_2_bg_t3x.h"
#include "chara_lie_t3x.h"
#include "chara_d0_t3x.h"
#include "chara_d1_t3x.h"
#include "chara_d2_t3x.h"
#include "chara_d3_t3x.h"
#include "chara_u0_t3x.h"
#include "chara_u1_t3x.h"
#include "chara_u2_t3x.h"
#include "chara_u3_t3x.h"
#include "chara_l0_t3x.h"
#include "chara_l1_t3x.h"
#include "chara_r0_t3x.h"
#include "chara_r1_t3x.h"

#define K_LEFT  (KEY_DLEFT  | KEY_CPAD_LEFT)
#define K_RIGHT (KEY_DRIGHT | KEY_CPAD_RIGHT)
#define K_UP    (KEY_DUP    | KEY_CPAD_UP)
#define K_DOWN  (KEY_DDOWN  | KEY_CPAD_DOWN)
#define K_ANY_DIR (K_LEFT | K_RIGHT | K_UP | K_DOWN)

enum { DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT };
static const int DIR_FRAMES[4] = { 4, 4, 2, 2 };

#define VIEW_W 320
#define VIEW_H 240
#define CHARA_W 20
#define CHARA_H 30
#define WALK_SPEED  2.0f
#define ANIM_PERIOD 8
#define WAKE_FRAMES 110
#define FADE_IN_FRAMES 50
#define LEAVE_FRAMES 45

#define FEET_OX 0
#define FEET_OY 19
#define FEET_W  20
#define FEET_H  11

typedef enum { ROOM_AREA1, ROOM_AREA1_2, ROOM_COUNT } RoomId;

typedef struct {
    short rect[4];   // door trigger {x,y,w,h}
    RoomId target;   // room to enter
    short sx, sy;    // spawn in target room
    short dir;       // facing on arrival
} Door;

typedef struct {
    const short* top;
    const short* bot;
    short w, h;
    const Door* doors;
    short doorCount;
} RoomDef;

static const Door DOORS_AREA1[] = {
    { { 601, 134, 27, 22 }, ROOM_AREA1_2, 170, 350, DIR_UP }, // up into the archway -> Flowey's arena
};
static const Door DOORS_AREA1_2[] = {
    { { 120, 398, 80, 20 }, ROOM_AREA1, 590, 168, DIR_LEFT }, // bottom -> back onto the corridor floor
    { { 120, 150, 80, 18 }, ROOM_AREA1, 590, 168, DIR_LEFT }, // top (arch) -> Ruins (placeholder)
};

static const RoomDef ROOMS[ROOM_COUNT] = {
    [ROOM_AREA1]   = { AREA1_TOP,   AREA1_BOT,   680, 260, DOORS_AREA1,   1 },
    [ROOM_AREA1_2] = { AREA1_2_TOP, AREA1_2_BOT, 320, 420, DOORS_AREA1_2, 2 },
};

enum { PHASE_WAKE, PHASE_PLAY };

static C2D_SpriteSheet s_charaSheets[13]; // 12 walk frames + lie
static C2D_Image s_walk[4][4];
static C2D_Image s_lie;
static C2D_SpriteSheet s_bgSheet;
static C2D_Image s_bg;

static const RoomDef* s_room;
static RoomId s_roomId;
static float s_x, s_y;
static int   s_dir, s_frame, s_animTimer, s_fadeIn, s_phase, s_wakeTimer, s_leaving;
static bool  s_doorArmed; // becomes true once Frisk steps off all doors (prevents spawn-on-door bounce)
static bool  s_floweySeen; // one-shot: Flowey cutscene plays once on entering the arena
static RoomId s_pendRoom; static short s_pendX, s_pendY, s_pendDir;

static void setNearest(C2D_Image img) { if (img.tex) C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST); }
static C2D_Image loadChara(int idx, const u8* data, u32 size) {
    s_charaSheets[idx] = C2D_SpriteSheetLoadFromMem(data, size);
    C2D_Image img = C2D_SpriteSheetGetImage(s_charaSheets[idx], 0);
    setNearest(img);
    return img;
}

static void loadRoom(RoomId id, float sx, float sy, int dir, bool wake) {
    if (s_bgSheet) C2D_SpriteSheetFree(s_bgSheet);
    switch (id) {
        case ROOM_AREA1:   s_bgSheet = C2D_SpriteSheetLoadFromMem(room_area1_bg_t3x,   room_area1_bg_t3x_size);   break;
        case ROOM_AREA1_2: s_bgSheet = C2D_SpriteSheetLoadFromMem(room_area1_2_bg_t3x, room_area1_2_bg_t3x_size); break;
        default: break;
    }
    s_bg = C2D_SpriteSheetGetImage(s_bgSheet, 0);
    setNearest(s_bg);
    s_roomId = id; s_room = &ROOMS[id];
    s_x = sx; s_y = sy; s_dir = dir; s_frame = 0; s_animTimer = 0;
    s_phase = wake ? PHASE_WAKE : PHASE_PLAY; s_wakeTimer = WAKE_FRAMES;
    s_fadeIn = FADE_IN_FRAMES; s_leaving = 0; s_doorArmed = false;
}

static bool allowedAt(float fx, float fy) {
    int x0 = (int)floorf(fx) + FEET_OX;
    int by = (int)floorf(fy) + FEET_OY;
    for (int x = x0; x < x0 + FEET_W; x++) {
        if (x < 0 || x >= s_room->w) return false;
        if (by < s_room->top[x] || by + FEET_H - 1 > s_room->bot[x]) return false;
    }
    return true;
}
// A door fires on Frisk's CENTER entering it, so you walk into the opening (not clip its edge).
static bool doorHit(const short* r) {
    float cxp = s_x + CHARA_W / 2.0f;
    float cyp = s_y + FEET_OY + FEET_H / 2.0f;
    return cxp >= r[0] && cxp < r[0] + r[2] && cyp >= r[1] && cyp < r[1] + r[3];
}

static void owInit(void) {
    s_walk[DIR_DOWN][0]  = loadChara(0,  chara_d0_t3x, chara_d0_t3x_size);
    s_walk[DIR_DOWN][1]  = loadChara(1,  chara_d1_t3x, chara_d1_t3x_size);
    s_walk[DIR_DOWN][2]  = loadChara(2,  chara_d2_t3x, chara_d2_t3x_size);
    s_walk[DIR_DOWN][3]  = loadChara(3,  chara_d3_t3x, chara_d3_t3x_size);
    s_walk[DIR_UP][0]    = loadChara(4,  chara_u0_t3x, chara_u0_t3x_size);
    s_walk[DIR_UP][1]    = loadChara(5,  chara_u1_t3x, chara_u1_t3x_size);
    s_walk[DIR_UP][2]    = loadChara(6,  chara_u2_t3x, chara_u2_t3x_size);
    s_walk[DIR_UP][3]    = loadChara(7,  chara_u3_t3x, chara_u3_t3x_size);
    s_walk[DIR_LEFT][0]  = loadChara(8,  chara_l0_t3x, chara_l0_t3x_size);
    s_walk[DIR_LEFT][1]  = loadChara(9,  chara_l1_t3x, chara_l1_t3x_size);
    s_walk[DIR_RIGHT][0] = loadChara(10, chara_r0_t3x, chara_r0_t3x_size);
    s_walk[DIR_RIGHT][1] = loadChara(11, chara_r1_t3x, chara_r1_t3x_size);
    s_lie = loadChara(12, chara_lie_t3x, chara_lie_t3x_size);
    s_bgSheet = NULL;
    floweyLoad();
    s_floweySeen = false;
#ifdef DEBUG_BOOT_FLOWEY
    loadRoom(ROOM_AREA1_2, 170, 360, DIR_UP, false); // debug: straight into Flowey's arena
    s_floweySeen = true;
    floweyStart();
#else
    loadRoom(ROOM_AREA1, 140, 120, DIR_DOWN, true); // fresh start: wake on the flowers
#endif
}

static void owCleanup(void) {
    for (int i = 0; i < 13; i++) C2D_SpriteSheetFree(s_charaSheets[i]);
    if (s_bgSheet) C2D_SpriteSheetFree(s_bgSheet);
    floweyFree();
}

static SceneId owUpdate(u32 kDown, u32 kHeld) {
    if (s_fadeIn > 0) s_fadeIn--;

    // Flowey cutscene owns input while active (no menu-exit or walking).
    if (floweyActive()) { floweyUpdate(kDown, kHeld); return SCENE_NONE; }

    if (kDown & KEY_B) return SCENE_MENU;

    if (s_leaving > 0) {
        if (--s_leaving == 0) {
            loadRoom(s_pendRoom, s_pendX, s_pendY, s_pendDir, false);
            // Start Flowey's intro the moment you arrive (blocks movement through the fade-in).
            if (s_pendRoom == ROOM_AREA1_2 && !s_floweySeen) { s_floweySeen = true; floweyStart(); }
        }
        return SCENE_NONE;
    }
    if (s_phase == PHASE_WAKE) {
        if (--s_wakeTimer <= 0 || (kDown & (KEY_A | K_ANY_DIR))) s_phase = PHASE_PLAY;
        return SCENE_NONE;
    }

    float dx = 0.0f, dy = 0.0f;
    if (kHeld & K_LEFT)  dx -= WALK_SPEED;
    if (kHeld & K_RIGHT) dx += WALK_SPEED;
    if (kHeld & K_UP)    dy -= WALK_SPEED;
    if (kHeld & K_DOWN)  dy += WALK_SPEED;

    int oldDir = s_dir;
    if      (dx < 0) s_dir = DIR_LEFT;
    else if (dx > 0) s_dir = DIR_RIGHT;
    else if (dy < 0) s_dir = DIR_UP;
    else if (dy > 0) s_dir = DIR_DOWN;
    if (s_dir != oldDir) { s_frame = 0; s_animTimer = 0; }

    if (dx != 0.0f) { s_x += dx; if (!allowedAt(s_x, s_y)) s_x -= dx; }
    if (dy != 0.0f) { s_y += dy; if (!allowedAt(s_x, s_y)) s_y -= dy; }

    if (dx != 0.0f || dy != 0.0f) {
        if (++s_animTimer >= ANIM_PERIOD) { s_animTimer = 0; s_frame = (s_frame + 1) % DIR_FRAMES[s_dir]; }
    } else { s_frame = 0; s_animTimer = 0; }

    bool onAnyDoor = false;
    for (int i = 0; i < s_room->doorCount; i++) {
        if (!doorHit(s_room->doors[i].rect)) continue;
        onAnyDoor = true;
        if (s_doorArmed) {
            const Door* d = &s_room->doors[i];
            s_pendRoom = d->target; s_pendX = d->sx; s_pendY = d->sy; s_pendDir = d->dir;
            s_leaving = LEAVE_FRAMES;
            break;
        }
    }
    if (!onAnyDoor) s_doorArmed = true; // stepped off doors -> future overlaps trigger
    return SCENE_NONE;
}

static float camAxis(float pos, float chara, int room, int view) {
    float c = pos + chara / 2.0f - view / 2.0f;
    if (c < 0) c = 0;
    if (c > room - view) c = room - view;
    if (room < view) c = (room - view) / 2.0f; // center small rooms
    return floorf(c);
}

static void owDrawTop(void) {
    float cx = camAxis(s_x, CHARA_W, s_room->w, VIEW_W);
    float cy = camAxis(s_y, CHARA_H, s_room->h, VIEW_H);

    // During Flowey's combat the battle box owns the whole top screen — don't draw the
    // overworld/Frisk underneath (they'd show through the box).
    if (!floweyInCombat()) {
        C2D_DrawImageAt(s_bg, floorf(TOP_X_OFFSET - cx), floorf(-cy), 0.0f, NULL, 1.0f, 1.0f);
        if (s_phase == PHASE_WAKE)
            C2D_DrawImageAt(s_lie, floorf(TOP_X_OFFSET + s_x - 5 - cx), floorf(s_y - cy), 0.1f, NULL, 1.0f, 1.0f);
        else
            C2D_DrawImageAt(s_walk[s_dir][s_frame % DIR_FRAMES[s_dir]], floorf(TOP_X_OFFSET + s_x - cx), floorf(s_y - cy), 0.1f, NULL, 1.0f, 1.0f);
    }

    floweyDrawTop(cx, cy, TOP_X_OFFSET);

    u8 wa = 0;
    if (s_fadeIn > 0) wa = (u8)(255 * s_fadeIn / FADE_IN_FRAMES);
    if (s_leaving > 0) { int v = (LEAVE_FRAMES - s_leaving) * 255 / LEAVE_FRAMES; if (v > wa) wa = (u8)v; }
    if (wa > 0) C2D_DrawRectSolid(0, 0, 0.5f, TOP_W, SCR_H, C2D_Color32(0xFF, 0xFF, 0xFF, wa));
}

static void owDrawBottom(void) {
    if (floweyActive()) { floweyDrawBottom(); return; }
    txtDrawCentered(s_roomId == ROOM_AREA1_2 ? "* ..." : "* The RUINS", BOT_W / 2.0f, 100.0f, 0.6f, COL_WHITE);
    txtDraw("D-pad: walk    [B] menu", 20.0f, 214.0f, 0.5f, COL_GRAY);
}

const Scene SCENE_game = { owInit, owUpdate, owDrawTop, owDrawBottom, owCleanup };
