// Overworld scene: a small multi-room engine. A room is data — background, a per-column
// walkable band, size, and a list of door triggers. Walking into a door fades to white and
// loads the target room at a spawn point. Currently: room_area1 (flower bed) <-> room_area1_2
// (Flowey's arena). Adding a room = drop in its bg t3x + band header + a RoomDef entry.
#include "game.h"
#include "flowey.h"
#include "toriel.h"
#include <math.h>
#include <string.h>
#include "collision_area1_bands.h"
#include "collision_area1_2_bands.h"
#include "collision_ruins1_bands.h"
#include "collision_ruins2_bands.h"
#include "collision_ruins3_bands.h"
#include "collision_ruins4_bands.h"
#include "solids_ruins1.h"
#include "solids_ruins2.h"
#include "solids_ruins3.h"
#include "solids_ruins4.h"
#include "solids_ruins5.h"
#include "solids_ruins6.h"
#include "solids_ruins7.h"
#include "solids_ruins7A.h"
#include "solids_ruins12A.h"
#include "solids_ruins12B.h"
#include "solids_ruins13.h"
#include "solids_ruins14.h"
#include "solids_ruins15A.h"
#include "solids_ruins15B.h"
#include "solids_ruins15C.h"
#include "solids_ruins15D.h"
#include "solids_ruins15E.h"
#include "solids_ruins16.h"
#include "solids_ruins17.h"
#include "solids_ruins18OLD.h"
#include "solids_ruins19.h"
#include "solids_torhouse1.h"
#include "solids_torhouse2.h"
#include "solids_torhouse3.h"
#include "ruins_holes.h"
#include "solids_ruins8.h"
#include "solids_ruins9.h"
#include "solids_ruins10.h"
#include "solids_ruins11.h"
#include "solids_ruins12.h"

// Wide rooms are split into 1024px strips; the rest are a single texture.
#include "room_ruins5_s0_t3x.h"
#include "room_ruins5_s1_t3x.h"
#include "room_ruins6_s0_t3x.h"
#include "room_ruins6_s1_t3x.h"
#include "room_ruins6_s2_t3x.h"
#include "room_ruins7_bg_t3x.h"
#include "room_ruins8_bg_t3x.h"
#include "room_ruins9_bg_t3x.h"
#include "room_ruins10_bg_t3x.h"
#include "room_ruins11_bg_t3x.h"
#include "room_ruins12_bg_t3x.h"

#include "room_area1_bg_t3x.h"
#include "room_area1_2_bg_t3x.h"
#include "room_ruins1_bg_t3x.h"
#include "room_ruins2_bg_t3x.h"
#include "room_ruins3_bg_t3x.h"
#include "room_ruins4_bg_t3x.h"
#include "room_ruins7A_bg_t3x.h"
#include "room_ruins12A_bg_t3x.h"
#include "room_ruins12B_bg_t3x.h"
#include "room_ruins13_bg_t3x.h"
#include "room_ruins14_bg_t3x.h"
#include "room_ruins15A_bg_t3x.h"
#include "room_ruins15B_bg_t3x.h"
#include "room_ruins15C_bg_t3x.h"
#include "room_ruins15D_bg_t3x.h"
#include "room_ruins15E_bg_t3x.h"
#include "room_ruins16_bg_t3x.h"
#include "room_ruins17_bg_t3x.h"
#include "room_ruins18OLD_bg_t3x.h"
#include "room_ruins19_bg_t3x.h"
#include "room_torhouse1_bg_t3x.h"
#include "room_torhouse2_bg_t3x.h"
#include "room_torhouse3_bg_t3x.h"
#include "candydish_full_t3x.h"
#include "candydish_fallen_t3x.h"
#include "candydish_spill_t3x.h"
#include "rock_t3x.h"
#include "spiketile_up_t3x.h"
#include "spiketile_down_t3x.h"
#include "napstablook_t3x.h"
#include "bigweb_t3x.h"
#include "smallweb_t3x.h"
#include "savepoint_t3x.h"
#include "cheesetable_t3x.h"
#include "mousehole_t3x.h"
#include "smallfrog_t3x.h"
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

typedef enum { ROOM_AREA1, ROOM_AREA1_2, ROOM_RUINS1, ROOM_RUINS2, ROOM_RUINS3,
               ROOM_RUINS4, ROOM_RUINS5, ROOM_RUINS6, ROOM_RUINS7, ROOM_RUINS8,
               ROOM_RUINS9, ROOM_RUINS10, ROOM_RUINS11, ROOM_RUINS12,
               ROOM_RUINS7A, ROOM_RUINS12A, ROOM_RUINS12B, ROOM_RUINS13,
               ROOM_RUINS14, ROOM_RUINS15A, ROOM_RUINS15B, ROOM_RUINS15C, ROOM_RUINS15D, ROOM_RUINS15E,
               ROOM_RUINS16, ROOM_RUINS17, ROOM_RUINS18OLD, ROOM_RUINS19,
               ROOM_TORHOUSE1, ROOM_TORHOUSE2, ROOM_TORHOUSE3, ROOM_COUNT } RoomId;

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
    const short (*solids)[5]; // {x,y,w,h,type}: 0=rect, 1=sur,2=sul,3=sdr,4=sdl (triangles)
    short solidCount;
} RoomDef;

static const Door DOORS_AREA1[] = {
    { { 601, 134, 27, 22 }, ROOM_AREA1_2, 160, 340, DIR_UP }, // up into the archway -> Flowey's arena
};
static const Door DOORS_AREA1_2[] = {
    { { 120, 398, 80, 20 }, ROOM_AREA1,  590, 168, DIR_LEFT }, // bottom -> back onto the corridor floor
    { { 120, 150, 80, 18 }, ROOM_RUINS1, 150, 430, DIR_UP },   // top (arch) -> the Ruins
};
// Door triggers & spawns come from Undertale's own door instances (obj_doorA/B/C/D positions
// in each room). doorA=next room, doorB=previous, doorC=skip-forward, doorD=skip-back. Walls
// now span the doorways (real collision), so triggers reach a bit toward the player: pushing
// against the wall at the door fires the transition, exactly as in the game. Spawns land just
// inside the door you emerge from, facing into the room.
// Trigger rects are kept TIGHT to the real doorway (they reach only ~10px toward the player, so
// the transition fires when you're actually in the opening, not before you get there).
static const Door DOORS_RUINS1[] = {   // 320x480
    { { 135, 448, 50, 30 }, ROOM_AREA1_2, 150, 175, DIR_DOWN }, // bottom door -> back to the arena
    { { 135,  98, 50, 28 }, ROOM_RUINS2,  150, 185, DIR_UP },   // top door -> ruins2
};
static const Door DOORS_RUINS2[] = {   // 320x240
    { { 135, 214, 50, 26 }, ROOM_RUINS1, 150, 108, DIR_DOWN },  // bottom door -> back to ruins1 (top corridor)
    { { 115,  58, 55, 26 }, ROOM_RUINS3, 130, 185, DIR_UP },    // top door -> ruins3
};
static const Door DOORS_RUINS3[] = {   // 740x240
    { { 115, 214, 45, 26 }, ROOM_RUINS2, 140,  90, DIR_DOWN },  // bottom door -> back to ruins2
    { { 712, 135, 28, 50 }, ROOM_RUINS4,  30, 150, DIR_RIGHT }, // right door -> ruins4
};
static const Door DOORS_RUINS4[] = {   // 320x240
    { {   0, 135, 28, 50 }, ROOM_RUINS3, 695, 150, DIR_LEFT },  // left door -> back to ruins3
    { { 140,  56, 40, 28 }, ROOM_RUINS5, 150, 185, DIR_UP },    // top door -> ruins5
};
static const Door DOORS_RUINS5[] = {   // 1200x240
    { { 135, 214, 50, 26 }, ROOM_RUINS4, 150,  95, DIR_DOWN },  // bottom door -> back to ruins4
    { { 1172, 96, 28, 48 }, ROOM_RUINS6,  30, 110, DIR_RIGHT }, // right door -> ruins6
};
static const Door DOORS_RUINS6[] = {   // 2500x240
    { {   0,  96, 28, 46 }, ROOM_RUINS5, 1155, 110, DIR_LEFT }, // left door -> back to ruins5
    { { 2472, 96, 28, 46 }, ROOM_RUINS7,  30, 110, DIR_RIGHT }, // right door -> ruins7
};
static const Door DOORS_RUINS7[] = {   // 320x480: left back, top -> candy room, right (doorC) -> ruins8
    { {   0,  98, 28, 46 }, ROOM_RUINS6, 2450, 110, DIR_LEFT }, // left door -> back to ruins6
    { { 292, 335, 28, 50 }, ROOM_RUINS8,  30, 110, DIR_RIGHT }, // doorC (right) -> ruins8 (main path)
    { { 135,  56, 50, 28 }, ROOM_RUINS7A, 150, 185, DIR_UP },   // top door -> the candy-bowl room
};
static const Door DOORS_RUINS7A[] = {  // 320x240: dead-end candy room; only exit is back to ruins7
    { { 135, 214, 50, 26 }, ROOM_RUINS7, 150,  95, DIR_DOWN },  // bottom door -> back to ruins7
};
static const Door DOORS_RUINS8[] = {   // 320x640
    { {   0,  98, 28, 46 }, ROOM_RUINS7, 275, 350, DIR_LEFT },  // left door -> back to ruins7 (doorC)
    { { 292,  98, 28, 46 }, ROOM_RUINS9,  30, 110, DIR_RIGHT }, // right door -> ruins9
};
static const Door DOORS_RUINS9[] = {   // 480x240
    { {   0,  98, 28, 46 }, ROOM_RUINS8, 275, 110, DIR_LEFT },  // left door -> back to ruins8
    { { 452,  98, 28, 46 }, ROOM_RUINS10, 30, 110, DIR_RIGHT }, // right door -> ruins10
};
static const Door DOORS_RUINS10[] = {  // 580x760
    { {   0,  98, 28, 46 }, ROOM_RUINS9, 435, 110, DIR_LEFT },  // left door -> back to ruins9
    { { 552, 218, 28, 46 }, ROOM_RUINS11, 30, 130, DIR_RIGHT }, // right door -> ruins11
};
static const Door DOORS_RUINS11[] = {  // 560x240
    { {   0, 118, 28, 46 }, ROOM_RUINS10, 535, 230, DIR_LEFT }, // left door -> back to ruins10
    { { 532, 118, 28, 46 }, ROOM_RUINS12A, 25, 125, DIR_RIGHT },// right door -> ruins12A (save room)
};
static const Door DOORS_RUINS12A[] = { // 320x240: the SAVE room between ruins11 and ruins12
    { {   0, 118, 28, 46 }, ROOM_RUINS11, 535, 130, DIR_LEFT },  // left -> back to ruins11
    { { 292, 118, 28, 46 }, ROOM_RUINS12,  25, 125, DIR_RIGHT }, // right -> ruins12 (Napstablook)
};
static const Door DOORS_RUINS12[] = {  // 320x240: Napstablook's hall. Three exits.
    { {   0, 118, 28, 46 }, ROOM_RUINS12A, 275, 125, DIR_LEFT }, // left -> back to ruins12A
    { { 292, 118, 28, 46 }, ROOM_RUINS12B, 25, 125, DIR_RIGHT }, // right -> spider bake-sale
    { { 195,  56, 40, 30 }, ROOM_RUINS13,  150, 255, DIR_UP },   // top -> ruins13 (froggits)
};
static const Door DOORS_RUINS12B[] = { // 320x240: spider bake-sale (dead-end)
    { {   0, 120, 28, 46 }, ROOM_RUINS12, 275, 125, DIR_LEFT },  // only exit -> back to ruins12
};
static const Door DOORS_RUINS13[] = {  // 640x300: froggit room
    { { 135, 262, 50, 30 }, ROOM_RUINS12, 205,  85, DIR_UP },    // bottom -> back to ruins12
    { { 612, 145, 28, 50 }, ROOM_RUINS14,  25, 165, DIR_RIGHT }, // right -> ruins14
};
static const Door DOORS_RUINS14[] = {  // 640x720: "just one switch" fall-floor room
    { {   0,  98, 28, 46 }, ROOM_RUINS13, 595, 165, DIR_LEFT },
    { { 612, 150, 28, 46 }, ROOM_RUINS15A, 25, 165, DIR_RIGHT },
};
static const Door DOORS_RUINS15A[] = { // 400x440: perspective/switch puzzle
    { {   0, 158, 28, 46 }, ROOM_RUINS14, 595, 165, DIR_LEFT },
    { { 235, 412, 45, 28 }, ROOM_RUINS15B, 250, 125, DIR_DOWN },
};
static const Door DOORS_RUINS15B[] = { // 400x440
    { { 235, 100, 45, 28 }, ROOM_RUINS15A, 250, 390, DIR_UP },
    { {   0, 318, 28, 46 }, ROOM_RUINS15C, 355, 325, DIR_LEFT },
};
static const Door DOORS_RUINS15C[] = { // 400x440
    { { 372, 318, 28, 46 }, ROOM_RUINS15B, 25, 325, DIR_RIGHT },
    { { 115, 100, 45, 28 }, ROOM_RUINS15D, 125, 125, DIR_DOWN },
};
static const Door DOORS_RUINS15D[] = { // 400x440
    { { 115, 412, 45, 28 }, ROOM_RUINS15C, 125, 125, DIR_DOWN },
    { { 372, 150, 28, 46 }, ROOM_RUINS16,  25, 285, DIR_RIGHT }, // doorC -> ruins16
};
static const Door DOORS_RUINS15E[] = { // 320x240: secret perspective room
    { { 135,  58, 50, 28 }, ROOM_RUINS15A, 150, 130, DIR_DOWN },
};
static const Door DOORS_RUINS16[] = {  // 600x400: three-way hub
    { {   0, 272, 28, 46 }, ROOM_RUINS15D, 355, 160, DIR_LEFT }, // left (doorD) -> ruins15D
    { { 572, 272, 28, 46 }, ROOM_RUINS17,  25, 125, DIR_RIGHT },
    { { 280,  52, 40, 28 }, ROOM_RUINS19, 150, 495, DIR_UP },    // top -> house front (main path)
};
static const Door DOORS_RUINS17[] = {  // 320x640: MERCY-advice froggit
    { {   0, 112, 28, 46 }, ROOM_RUINS16, 555, 285, DIR_LEFT },
    { { 135,  56, 40, 28 }, ROOM_RUINS18OLD, 150, 190, DIR_UP },
};
static const Door DOORS_RUINS18OLD[] = { // 320x240: Toy Knife dead-end
    { { 135, 214, 50, 26 }, ROOM_RUINS17, 150, 90, DIR_DOWN },
};
static const Door DOORS_RUINS19[] = {  // 320x540: house front + SAVE
    { { 135, 514, 50, 26 }, ROOM_RUINS16, 290, 90, DIR_DOWN },
    { { 135, 152, 50, 28 }, ROOM_TORHOUSE1, 150, 205, DIR_UP }, // up into Toriel's house
};
static const Door DOORS_TORHOUSE1[] = { // 320x240: entrance parlor (stairs->basement not built)
    { { 135, 214, 50, 26 }, ROOM_RUINS19,   150, 190, DIR_DOWN }, // bottom -> back outside
    { {   0, 148, 28, 46 }, ROOM_TORHOUSE2, 272, 158, DIR_LEFT }, // left  -> living room
    { { 292, 146, 28, 46 }, ROOM_TORHOUSE3,  28, 150, DIR_RIGHT },// right -> bedroom hallway
};
static const Door DOORS_TORHOUSE2[] = { // 320x240: living/dining (top->kitchen not built)
    { { 292, 150, 28, 40 }, ROOM_TORHOUSE1, 28, 165, DIR_RIGHT },
};
static const Door DOORS_TORHOUSE3[] = { // 800x240: bedroom hallway + mirror (doors->rooms not built)
    { {   0, 142, 28, 46 }, ROOM_TORHOUSE1, 272, 158, DIR_LEFT },
};

static const RoomDef ROOMS[ROOM_COUNT] = {
    [ROOM_AREA1]   = { AREA1_TOP,   AREA1_BOT,   680, 260, DOORS_AREA1,   1, NULL, 0 },
    [ROOM_AREA1_2] = { AREA1_2_TOP, AREA1_2_BOT, 320, 420, DOORS_AREA1_2, 2, NULL, 0 },
    [ROOM_RUINS1]  = { RUINS1_TOP,  RUINS1_BOT,  320, 480, DOORS_RUINS1,  2, RUINS1_SOLIDS, RUINS1_SOLID_COUNT },
    [ROOM_RUINS2]  = { RUINS2_TOP,  RUINS2_BOT,  320, 240, DOORS_RUINS2,  2, RUINS2_SOLIDS, RUINS2_SOLID_COUNT },
    [ROOM_RUINS3]  = { RUINS3_TOP,  RUINS3_BOT,  740, 240, DOORS_RUINS3,  2, RUINS3_SOLIDS, RUINS3_SOLID_COUNT },
    [ROOM_RUINS4]  = { RUINS4_TOP,  RUINS4_BOT,  320, 240, DOORS_RUINS4,  2, RUINS4_SOLIDS, RUINS4_SOLID_COUNT },
    [ROOM_RUINS5]  = { NULL, NULL, 1200, 240, DOORS_RUINS5,  2, RUINS5_SOLIDS,  RUINS5_SOLID_COUNT },
    [ROOM_RUINS6]  = { NULL, NULL, 2500, 240, DOORS_RUINS6,  2, RUINS6_SOLIDS,  RUINS6_SOLID_COUNT },
    [ROOM_RUINS7]  = { NULL, NULL,  320, 480, DOORS_RUINS7,  3, RUINS7_SOLIDS,  RUINS7_SOLID_COUNT },
    [ROOM_RUINS8]  = { NULL, NULL,  320, 640, DOORS_RUINS8,  2, RUINS8_SOLIDS,  RUINS8_SOLID_COUNT },
    [ROOM_RUINS9]  = { NULL, NULL,  480, 240, DOORS_RUINS9,  2, RUINS9_SOLIDS,  RUINS9_SOLID_COUNT },
    [ROOM_RUINS10] = { NULL, NULL,  580, 760, DOORS_RUINS10, 2, RUINS10_SOLIDS, RUINS10_SOLID_COUNT },
    [ROOM_RUINS11] = { NULL, NULL,  560, 240, DOORS_RUINS11, 2, RUINS11_SOLIDS, RUINS11_SOLID_COUNT },
    [ROOM_RUINS12] = { NULL, NULL,  320, 240, DOORS_RUINS12, 3, RUINS12_SOLIDS, RUINS12_SOLID_COUNT },
    [ROOM_RUINS7A] = { NULL, NULL,  320, 240, DOORS_RUINS7A, 1, RUINS7A_SOLIDS, RUINS7A_SOLID_COUNT },
    [ROOM_RUINS12A]= { NULL, NULL,  320, 240, DOORS_RUINS12A, 2, RUINS12A_SOLIDS, RUINS12A_SOLID_COUNT },
    [ROOM_RUINS12B]= { NULL, NULL,  320, 240, DOORS_RUINS12B, 1, RUINS12B_SOLIDS, RUINS12B_SOLID_COUNT },
    [ROOM_RUINS13] = { NULL, NULL,  640, 300, DOORS_RUINS13, 2, RUINS13_SOLIDS, RUINS13_SOLID_COUNT },
    [ROOM_RUINS14] = { NULL, NULL,  640, 720, DOORS_RUINS14, 2, RUINS14_SOLIDS, RUINS14_SOLID_COUNT },
    [ROOM_RUINS15A]= { NULL, NULL,  400, 440, DOORS_RUINS15A, 2, RUINS15A_SOLIDS, RUINS15A_SOLID_COUNT },
    [ROOM_RUINS15B]= { NULL, NULL,  400, 440, DOORS_RUINS15B, 2, RUINS15B_SOLIDS, RUINS15B_SOLID_COUNT },
    [ROOM_RUINS15C]= { NULL, NULL,  400, 440, DOORS_RUINS15C, 2, RUINS15C_SOLIDS, RUINS15C_SOLID_COUNT },
    [ROOM_RUINS15D]= { NULL, NULL,  400, 440, DOORS_RUINS15D, 2, RUINS15D_SOLIDS, RUINS15D_SOLID_COUNT },
    [ROOM_RUINS15E]= { NULL, NULL,  320, 240, DOORS_RUINS15E, 1, RUINS15E_SOLIDS, RUINS15E_SOLID_COUNT },
    [ROOM_RUINS16] = { NULL, NULL,  600, 400, DOORS_RUINS16, 3, RUINS16_SOLIDS, RUINS16_SOLID_COUNT },
    [ROOM_RUINS17] = { NULL, NULL,  320, 640, DOORS_RUINS17, 2, RUINS17_SOLIDS, RUINS17_SOLID_COUNT },
    [ROOM_RUINS18OLD]={ NULL, NULL, 320, 240, DOORS_RUINS18OLD, 1, RUINS18OLD_SOLIDS, RUINS18OLD_SOLID_COUNT },
    [ROOM_RUINS19] = { NULL, NULL,  320, 540, DOORS_RUINS19, 2, RUINS19_SOLIDS, RUINS19_SOLID_COUNT },
    [ROOM_TORHOUSE1]={ NULL, NULL,  320, 240, DOORS_TORHOUSE1, 3, TORHOUSE1_SOLIDS, TORHOUSE1_SOLID_COUNT },
    [ROOM_TORHOUSE2]={ NULL, NULL,  320, 240, DOORS_TORHOUSE2, 1, TORHOUSE2_SOLIDS, TORHOUSE2_SOLID_COUNT },
    [ROOM_TORHOUSE3]={ NULL, NULL,  800, 240, DOORS_TORHOUSE3, 1, TORHOUSE3_SOLIDS, TORHOUSE3_SOLID_COUNT },
};

enum { PHASE_WAKE, PHASE_PLAY };

static C2D_SpriteSheet s_charaSheets[13]; // 12 walk frames + lie
static C2D_Image s_walk[4][4];
static C2D_Image s_lie;
// Room background: 1-3 horizontal strips (wide rooms are split at the 1024px texture limit).
#define MAX_STRIPS 3
static C2D_SpriteSheet s_stripSheets[MAX_STRIPS];
static C2D_Image s_strips[MAX_STRIPS];
static int s_stripX[MAX_STRIPS];
static int s_stripCount;

static const RoomDef* s_room;
static RoomId s_roomId;
static float s_x, s_y;
static int   s_dir, s_frame, s_animTimer, s_fadeIn, s_phase, s_wakeTimer, s_leaving;
static bool  s_doorArmed; // becomes true once Frisk steps off all doors (prevents spawn-on-door bounce)
static bool  s_floweySeen; // one-shot: Flowey cutscene plays once on entering the arena
static bool  s_torielSeen; // one-shot: Toriel arrival plays once after Flowey
static bool  s_torielLed;  // one-shot: the follow-into-Ruins transition fires once
static RoomId s_pendRoom; static short s_pendX, s_pendY, s_pendDir;

// Candy-bowl side room (ruins7A): interactable dish + overworld dialogue.
static C2D_SpriteSheet s_candySheet[3];
static C2D_Image s_candyFull, s_candyFallen, s_candySpill; // full dish / tipped pedestal / spilled candy
static C2D_SpriteSheet s_puzzleSheet[3];
static C2D_Image s_rockImg, s_spikeUp, s_spikeDown;
static C2D_SpriteSheet s_propSheet[7];
static C2D_Image s_napsta, s_bigweb, s_smallweb, s_savepoint, s_cheese, s_mousehole, s_frog;
static int  s_napstaStage;   // Napstablook dialogue progression in ruins12
static bool s_napstaGone;    // once he drifts away, the hall is clear
static int  s_fallMode;      // 0 none, 1 falling (down), 2 venting (up)
static int  s_fallFrames;
static bool s_fallArmed;     // must step off a crack/vent before it triggers again
static int  s_candyCount;            // pieces taken (persists across visits); >=4 => bowl empty
#define CANDY_X 150                  // dish top-left in room coords (obj_candydish1 @150,95)
#define CANDY_Y 95
enum { DLG_NONE, DLG_CHOICE, DLG_MSG };
static int   s_dlgStage;
static const char* s_dlgText;
static int   s_dlgRevealed, s_dlgTimer, s_dlgSel; // s_dlgSel: 0=Yes, 1=No
static void  candyChoose(void);      // fwd

// Readable wall signs / plaques (obj_readable_*): interact to show the game's own text.
typedef struct { RoomId room; short x, y; const char* text; } Sign;
static const Sign SIGNS[] = {
    { ROOM_RUINS2,   80,  60, "* Only the fearless may proceed.&* Brave ones, foolish ones.&* Both walk not the middle road." },
    { ROOM_RUINS3,  100,  60, "* Stay on the path." },
    { ROOM_RUINS3,  130, 120, "* \"Press A to read signs!\"" },
    { ROOM_RUINS5,  580, 140, "* The western room is the&  eastern room's blueprint." },
    { ROOM_RUINS5,  600, 140, "* The western room is the&  eastern room's blueprint." },
    { ROOM_RUINS9,  160,  60, "* Three out of four grey rocks&  recommend you push them." },
    { ROOM_RUINS10, 460, 560, "* Please don't step on the&  leaves." },
    { ROOM_RUINS10, 460, 160, "* Didn't you read the sign&  downstairs?" },
    { ROOM_RUINS12B, 150, 120, "* Spider Bake Sale&* All proceeds go to&  real spiders." },
    { ROOM_RUINS14,  100,  80, "* There is just one switch." },
    { ROOM_RUINS15A,  80, 100, "* The far door is not an exit.&* It simply marks a rotation&  in perspective." },
    { ROOM_RUINS15B,  80, 240, "* If you can read this,&  press the blue switch." },
    { ROOM_RUINS15C, 300, 240, "* If you can read this,&  press the red switch." },
    { ROOM_RUINS15D, 180, 100, "* If you can read this,&  press the green switch." },
    // Toriel's house
    { ROOM_TORHOUSE1, 252, 52, "* These books are worn...&* They must have been read&  many times." },
    { ROOM_TORHOUSE1,  40, 58, "* Inside is an old calendar&  from the beginning of&  201X." },
    { ROOM_TORHOUSE2, 278, 56, "* The ends of the tools have&  been filed down to&  make them safer." },
    { ROOM_TORHOUSE2, 226, 61, "* It's a history book.&* Trapped behind the barrier&  and fearful of further&  human attacks, we retreated." },
    { ROOM_TORHOUSE3, 568, 125, "* \"Room under renovations.\"" },
    { ROOM_TORHOUSE3, 701, 131, "* You have seen this type&  of plant before but&  do not know its name.&* Oh! It is a \"water sausage.\"" },
    { ROOM_TORHOUSE3, 537, 131, "* Inside the drawer are&  flower seeds and some&  broken crayons." },
    { ROOM_TORHOUSE3, 640, 125, "* It's you!" }, // the mirror
};
#define SIGN_COUNT ((int)(sizeof(SIGNS) / sizeof(SIGNS[0])))

// Push-rock -> button -> spikes puzzles (ruins9 & ruins11). A rock is a 20x20 solid you shove
// onto a button (a target x); with a rock on its button the room's spikes retract. The goofyrock
// in ruins11 refuses to be pushed — you have to TALK it into moving.
#define ROCK_SZ 20
typedef struct { RoomId room; short xstart, y, buttonX; bool talkable; } RockDef;
static const RockDef ROCKS[] = {
    { ROOM_RUINS9,  180, 135, 240, false },
    { ROOM_RUINS11, 180, 135, 280, false },
    { ROOM_RUINS11, 200,  95, 280, false },
    { ROOM_RUINS11, 220, 175, 280, true  }, // goofyrock — talkable
};
#define ROCK_COUNT ((int)(sizeof(ROCKS) / sizeof(ROCKS[0])))
#define GOOFY_IDX  3
#define GOOFY_DONE 5   // talk stages 0..4, then solved
static short s_rockX[ROCK_COUNT];
static bool  s_rockOn[ROCK_COUNT];
static int   s_goofyStage;

typedef struct { RoomId room; short x, y; } SpikeTile;
static const SpikeTile SPIKES[] = {
    { ROOM_RUINS9, 300,  80 }, { ROOM_RUINS9, 300, 100 }, { ROOM_RUINS9, 300, 120 },
    { ROOM_RUINS9, 300, 140 }, { ROOM_RUINS9, 300, 160 }, { ROOM_RUINS9, 300, 180 },
    { ROOM_RUINS11, 380, 120 }, { ROOM_RUINS11, 400, 120 }, { ROOM_RUINS11, 380, 140 },
    { ROOM_RUINS11, 400, 140 }, { ROOM_RUINS11, 380, 160 }, { ROOM_RUINS11, 400, 160 },
};
#define SPIKE_COUNT ((int)(sizeof(SPIKES) / sizeof(SPIKES[0])))

static bool spikesRetracted(RoomId r) {
    if (r == ROOM_RUINS9)  return s_rockOn[0];             // ruins9 rock on its button
    if (r == ROOM_RUINS11) return s_goofyStage >= GOOFY_DONE;
    return false;
}

static void setNearest(C2D_Image img) { if (img.tex) C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST); }
static C2D_Image loadChara(int idx, const u8* data, u32 size) {
    s_charaSheets[idx] = C2D_SpriteSheetLoadFromMem(data, size);
    C2D_Image img = C2D_SpriteSheetGetImage(s_charaSheets[idx], 0);
    setNearest(img);
    return img;
}

static void addStrip(const u8* data, u32 size, int xoff) {
    int i = s_stripCount++;
    s_stripSheets[i] = C2D_SpriteSheetLoadFromMem(data, size);
    s_strips[i] = C2D_SpriteSheetGetImage(s_stripSheets[i], 0);
    setNearest(s_strips[i]);
    s_stripX[i] = xoff;
}

static void loadRoom(RoomId id, float sx, float sy, int dir, bool wake) {
    for (int i = 0; i < s_stripCount; i++) C2D_SpriteSheetFree(s_stripSheets[i]);
    s_stripCount = 0;
    switch (id) {
        case ROOM_AREA1:   addStrip(room_area1_bg_t3x,   room_area1_bg_t3x_size,   0); break;
        case ROOM_AREA1_2: addStrip(room_area1_2_bg_t3x, room_area1_2_bg_t3x_size, 0); break;
        case ROOM_RUINS1:  addStrip(room_ruins1_bg_t3x,  room_ruins1_bg_t3x_size,  0); break;
        case ROOM_RUINS2:  addStrip(room_ruins2_bg_t3x,  room_ruins2_bg_t3x_size,  0); break;
        case ROOM_RUINS3:  addStrip(room_ruins3_bg_t3x,  room_ruins3_bg_t3x_size,  0); break;
        case ROOM_RUINS4:  addStrip(room_ruins4_bg_t3x,  room_ruins4_bg_t3x_size,  0); break;
        case ROOM_RUINS5:  addStrip(room_ruins5_s0_t3x, room_ruins5_s0_t3x_size, 0);
                           addStrip(room_ruins5_s1_t3x, room_ruins5_s1_t3x_size, 1024); break;
        case ROOM_RUINS6:  addStrip(room_ruins6_s0_t3x, room_ruins6_s0_t3x_size, 0);
                           addStrip(room_ruins6_s1_t3x, room_ruins6_s1_t3x_size, 1024);
                           addStrip(room_ruins6_s2_t3x, room_ruins6_s2_t3x_size, 2048); break;
        case ROOM_RUINS7:  addStrip(room_ruins7_bg_t3x,  room_ruins7_bg_t3x_size,  0); break;
        case ROOM_RUINS8:  addStrip(room_ruins8_bg_t3x,  room_ruins8_bg_t3x_size,  0); break;
        case ROOM_RUINS9:  addStrip(room_ruins9_bg_t3x,  room_ruins9_bg_t3x_size,  0); break;
        case ROOM_RUINS10: addStrip(room_ruins10_bg_t3x, room_ruins10_bg_t3x_size, 0); break;
        case ROOM_RUINS11: addStrip(room_ruins11_bg_t3x, room_ruins11_bg_t3x_size, 0); break;
        case ROOM_RUINS12: addStrip(room_ruins12_bg_t3x, room_ruins12_bg_t3x_size, 0); break;
        case ROOM_RUINS7A: addStrip(room_ruins7A_bg_t3x, room_ruins7A_bg_t3x_size, 0); break;
        case ROOM_RUINS12A: addStrip(room_ruins12A_bg_t3x, room_ruins12A_bg_t3x_size, 0); break;
        case ROOM_RUINS12B: addStrip(room_ruins12B_bg_t3x, room_ruins12B_bg_t3x_size, 0); break;
        case ROOM_RUINS13: addStrip(room_ruins13_bg_t3x, room_ruins13_bg_t3x_size, 0); break;
        case ROOM_RUINS14: addStrip(room_ruins14_bg_t3x, room_ruins14_bg_t3x_size, 0); break;
        case ROOM_RUINS15A: addStrip(room_ruins15A_bg_t3x, room_ruins15A_bg_t3x_size, 0); break;
        case ROOM_RUINS15B: addStrip(room_ruins15B_bg_t3x, room_ruins15B_bg_t3x_size, 0); break;
        case ROOM_RUINS15C: addStrip(room_ruins15C_bg_t3x, room_ruins15C_bg_t3x_size, 0); break;
        case ROOM_RUINS15D: addStrip(room_ruins15D_bg_t3x, room_ruins15D_bg_t3x_size, 0); break;
        case ROOM_RUINS15E: addStrip(room_ruins15E_bg_t3x, room_ruins15E_bg_t3x_size, 0); break;
        case ROOM_RUINS16: addStrip(room_ruins16_bg_t3x, room_ruins16_bg_t3x_size, 0); break;
        case ROOM_RUINS17: addStrip(room_ruins17_bg_t3x, room_ruins17_bg_t3x_size, 0); break;
        case ROOM_RUINS18OLD: addStrip(room_ruins18OLD_bg_t3x, room_ruins18OLD_bg_t3x_size, 0); break;
        case ROOM_RUINS19: addStrip(room_ruins19_bg_t3x, room_ruins19_bg_t3x_size, 0); break;
        case ROOM_TORHOUSE1: addStrip(room_torhouse1_bg_t3x, room_torhouse1_bg_t3x_size, 0); break;
        case ROOM_TORHOUSE2: addStrip(room_torhouse2_bg_t3x, room_torhouse2_bg_t3x_size, 0); break;
        case ROOM_TORHOUSE3: addStrip(room_torhouse3_bg_t3x, room_torhouse3_bg_t3x_size, 0); break;
        default: break;
    }
    s_roomId = id; s_room = &ROOMS[id];
    s_x = sx; s_y = sy; s_dir = dir; s_frame = 0; s_animTimer = 0;
    s_phase = wake ? PHASE_WAKE : PHASE_PLAY; s_wakeTimer = WAKE_FRAMES;
    s_fadeIn = FADE_IN_FRAMES; s_leaving = 0; s_doorArmed = false;
    s_fallMode = 0; s_fallArmed = false;
}

static bool napstaBlocking(void); // fwd (used in allowedAt, defined with the interact helpers)
static bool boxesOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
// Does an arbitrary box hit a wall (or leave the room)? Slopes are treated as full cells here —
// rocks are never pushed onto the diagonal stairs, so the approximation is safe.
static bool boxHitsWall(int x, int y, int w, int h) {
    if (x < 0 || y < 0 || x + w > s_room->w || y + h > s_room->h) return true;
    for (int i = 0; i < s_room->solidCount; i++) {
        const short* s = s_room->solids[i];
        if (boxesOverlap(x, y, w, h, s[0], s[1], s[2], s[3])) return true;
    }
    return false;
}
// Current-room rock whose 20x20 box overlaps the feet box at (px,py); -1 if none (skip 'except').
static int rockOverlap(float px, float py, int except) {
    int bx = (int)floorf(px) + FEET_OX, by = (int)floorf(py) + FEET_OY;
    for (int i = 0; i < ROCK_COUNT; i++) {
        if (ROCKS[i].room != s_roomId || i == except) continue;
        if (boxesOverlap(bx, by, FEET_W, FEET_H, s_rockX[i], ROCKS[i].y, ROCK_SZ, ROCK_SZ)) return i;
    }
    return -1;
}
// Is a rock's candidate box (at x) clear of the OTHER rocks in the room?
static bool rockBoxClear(float rx, int self) {
    int x = (int)floorf(rx);
    for (int i = 0; i < ROCK_COUNT; i++) {
        if (i == self || ROCKS[i].room != s_roomId) continue;
        if (boxesOverlap(x, ROCKS[self].y, ROCK_SZ, ROCK_SZ, s_rockX[i], ROCKS[i].y, ROCK_SZ, ROCK_SZ)) return false;
    }
    return true;
}

static bool allowedAt(float fx, float fy) {
    int bx = (int)floorf(fx) + FEET_OX;
    int by = (int)floorf(fy) + FEET_OY;
    // Stay within the room.
    if (bx < 0 || by < 0 || bx + FEET_W > s_room->w || by + FEET_H > s_room->h) return false;

    // Raised spikes are solid; once the room's puzzle is solved they retract and are walkable.
    if (!spikesRetracted(s_roomId)) {
        for (int i = 0; i < SPIKE_COUNT; i++) {
            if (SPIKES[i].room != s_roomId) continue;
            if (boxesOverlap(bx, by, FEET_W, FEET_H, SPIKES[i].x, SPIKES[i].y, 20, 20)) return false;
        }
    }
    // Napstablook blocks the ruins12 hall until you talk him aside.
    if (napstaBlocking() && boxesOverlap(bx, by, FEET_W, FEET_H, 150, 95, 25, 55)) return false;

    // Rooms with extracted solids use Undertale's real collision: rectangles (walls/blocks)
    // and 20x20 triangular slopes (sur/sul/sdr/sdl) for the diagonal stairs & corners.
    if (s_room->solidCount > 0) {
        for (int i = 0; i < s_room->solidCount; i++) {
            const short* s = s_room->solids[i];
            // Overlap of the feet box with this solid's cell.
            int ox0 = bx > s[0] ? bx : s[0];
            int ox1 = (bx + FEET_W) < (s[0] + s[2]) ? (bx + FEET_W) : (s[0] + s[2]);
            int oy0 = by > s[1] ? by : s[1];
            int oy1 = (by + FEET_H) < (s[1] + s[3]) ? (by + FEET_H) : (s[1] + s[3]);
            if (ox0 >= ox1 || oy0 >= oy1) continue; // no overlap with the cell
            if (s[4] == 0) return false;            // full rectangle
            // Slope: does the overlap region contain a solid point? (local coords vs diagonal)
            int lx0 = ox0 - s[0], lx1 = ox1 - s[0], ly0 = oy0 - s[1], ly1 = oy1 - s[1];
            bool hit;
            switch (s[4]) {
                case 1: hit = (lx1 >= ly0); break;        // sur: solid where lx >= ly
                case 2: hit = (lx0 + ly0 <= 20); break;   // sul: solid where lx+ly <= 20
                case 3: hit = (lx1 + ly1 >= 20); break;   // sdr: solid where lx+ly >= 20
                case 4: hit = (lx0 <= ly1); break;        // sdl: solid where lx <= ly
                default: hit = true; break;
            }
            if (hit) return false;
        }
        return true;
    }
    for (int x = bx; x < bx + FEET_W; x++) {
        if (by < s_room->top[x] || by + FEET_H - 1 > s_room->bot[x]) return false;
    }
    return true;
}
// A door fires on Frisk's FULL sprite box (20x30) overlapping it — like Undertale's bbox-vs-
// door check. This matters at top doors: the feet stop at the wall, but the head reaches the
// doorway above it, so the door still triggers.
static bool doorHit(const short* r) {
    return s_x < r[0] + r[2] && s_x + CHARA_W > r[0] &&
           s_y < r[1] + r[3] && s_y + CHARA_H > r[1];
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
    s_candySheet[0] = C2D_SpriteSheetLoadFromMem(candydish_full_t3x, candydish_full_t3x_size);
    s_candySheet[1] = C2D_SpriteSheetLoadFromMem(candydish_fallen_t3x, candydish_fallen_t3x_size);
    s_candySheet[2] = C2D_SpriteSheetLoadFromMem(candydish_spill_t3x, candydish_spill_t3x_size);
    s_candyFull   = C2D_SpriteSheetGetImage(s_candySheet[0], 0); setNearest(s_candyFull);
    s_candyFallen = C2D_SpriteSheetGetImage(s_candySheet[1], 0); setNearest(s_candyFallen);
    s_candySpill  = C2D_SpriteSheetGetImage(s_candySheet[2], 0); setNearest(s_candySpill);
    s_candyCount = 0; s_dlgStage = DLG_NONE;
    s_puzzleSheet[0] = C2D_SpriteSheetLoadFromMem(rock_t3x, rock_t3x_size);
    s_puzzleSheet[1] = C2D_SpriteSheetLoadFromMem(spiketile_up_t3x, spiketile_up_t3x_size);
    s_puzzleSheet[2] = C2D_SpriteSheetLoadFromMem(spiketile_down_t3x, spiketile_down_t3x_size);
    s_rockImg   = C2D_SpriteSheetGetImage(s_puzzleSheet[0], 0); setNearest(s_rockImg);
    s_spikeUp   = C2D_SpriteSheetGetImage(s_puzzleSheet[1], 0); setNearest(s_spikeUp);
    s_spikeDown = C2D_SpriteSheetGetImage(s_puzzleSheet[2], 0); setNearest(s_spikeDown);
    s_goofyStage = 0;
    for (int i = 0; i < ROCK_COUNT; i++) { s_rockX[i] = ROCKS[i].xstart; s_rockOn[i] = false; }
    s_propSheet[0] = C2D_SpriteSheetLoadFromMem(napstablook_t3x, napstablook_t3x_size);
    s_propSheet[1] = C2D_SpriteSheetLoadFromMem(bigweb_t3x, bigweb_t3x_size);
    s_propSheet[2] = C2D_SpriteSheetLoadFromMem(smallweb_t3x, smallweb_t3x_size);
    s_propSheet[3] = C2D_SpriteSheetLoadFromMem(savepoint_t3x, savepoint_t3x_size);
    s_propSheet[4] = C2D_SpriteSheetLoadFromMem(cheesetable_t3x, cheesetable_t3x_size);
    s_propSheet[5] = C2D_SpriteSheetLoadFromMem(mousehole_t3x, mousehole_t3x_size);
    s_propSheet[6] = C2D_SpriteSheetLoadFromMem(smallfrog_t3x, smallfrog_t3x_size);
    s_napsta    = C2D_SpriteSheetGetImage(s_propSheet[0], 0); setNearest(s_napsta);
    s_bigweb    = C2D_SpriteSheetGetImage(s_propSheet[1], 0); setNearest(s_bigweb);
    s_smallweb  = C2D_SpriteSheetGetImage(s_propSheet[2], 0); setNearest(s_smallweb);
    s_savepoint = C2D_SpriteSheetGetImage(s_propSheet[3], 0); setNearest(s_savepoint);
    s_cheese    = C2D_SpriteSheetGetImage(s_propSheet[4], 0); setNearest(s_cheese);
    s_mousehole = C2D_SpriteSheetGetImage(s_propSheet[5], 0); setNearest(s_mousehole);
    s_frog      = C2D_SpriteSheetGetImage(s_propSheet[6], 0); setNearest(s_frog);
    s_napstaStage = 0; s_napstaGone = false;
    s_stripCount = 0;
    floweyLoad();
    torielLoad();
    s_floweySeen = false;
    s_torielSeen = false;
    s_torielLed = false;
#ifdef DEBUG_BOOT_RUINS
    loadRoom(ROOM_RUINS11, 30, 130, DIR_RIGHT, false); // debug: ruins11 -> 12A -> 12 -> 12B/13 chain
    s_floweySeen = true; s_torielSeen = true; s_torielLed = true;
#elif defined(DEBUG_BOOT_TORIEL)
    loadRoom(ROOM_AREA1_2, 160, 340, DIR_UP, false); // debug: straight into Toriel's arrival
    s_floweySeen = true;
    torielStart();
#elif defined(DEBUG_BOOT_FLOWEY)
    loadRoom(ROOM_AREA1_2, 160, 340, DIR_UP, false); // debug: straight into Flowey's arena
    s_floweySeen = true;
    floweyStart();
#else
    loadRoom(ROOM_AREA1, 140, 120, DIR_DOWN, true); // fresh start: wake on the flowers
#endif
}

static void owCleanup(void) {
    for (int i = 0; i < 13; i++) C2D_SpriteSheetFree(s_charaSheets[i]);
    for (int i = 0; i < s_stripCount; i++) C2D_SpriteSheetFree(s_stripSheets[i]);
    for (int i = 0; i < 3; i++) C2D_SpriteSheetFree(s_candySheet[i]);
    for (int i = 0; i < 3; i++) C2D_SpriteSheetFree(s_puzzleSheet[i]);
    for (int i = 0; i < 7; i++) C2D_SpriteSheetFree(s_propSheet[i]);
    floweyFree();
    torielFree();
}

// The candy bowl (obj_candydish1; dialoguer msc 508/509, SCR_TEXT_1603-1639). Verbatim from the
// game: take a piece each time (1st..3rd give graduated guilt-trip lines), the 4th spills and
// topples the pedestal; afterward it just says "Look at what you've done."
static void candyStart(void) {
    s_dlgRevealed = 0; s_dlgTimer = 0; s_dlgSel = 0;
    if (s_candyCount >= 4) {                 // bowl already emptied (flag[34] > 3) -> SCR_TEXT_1606
        s_dlgStage = DLG_MSG;
        s_dlgText = "* Look at what you've done.";
    } else {
        s_dlgStage = DLG_CHOICE;
        s_dlgText = (s_candyCount == 0)
            ? "* It says 'take one.'&* Take a piece of candy?"  // SCR_TEXT_1604 (first time)
            : "* 'Take one.'&* Take a candy?";                  // SCR_TEXT_1603 (thereafter)
    }
}
static void candyChoose(void) {
    s_dlgRevealed = 0; s_dlgTimer = 0;
    if (s_dlgSel == 1) { s_dlgText = "* You decided not to take some."; s_dlgStage = DLG_MSG; return; } // 1639
    s_candyCount++;
    switch (s_candyCount) {
        case 1:  s_dlgText = "* You took a piece of candy."; break;                     // 1624
        case 2:  s_dlgText = "* You took more candy.&* How disgusting.."; break;        // 1625
        case 3:  s_dlgText = "* You take another piece.&* You feel like the&  scum of the earth..."; break; // 1626
        default: s_dlgText = "* You took too much too fast.&* The candy spills onto&  the floor."; break;   // 1628
    }
    s_dlgStage = DLG_MSG;
}
static void showMsg(const char* t) { s_dlgStage = DLG_MSG; s_dlgText = t; s_dlgRevealed = 0; s_dlgTimer = 0; }

static bool nearRock(int i) {
    return s_x + CHARA_W > s_rockX[i] - 8 && s_x < s_rockX[i] + ROCK_SZ + 8 &&
           s_y + CHARA_H > ROCKS[i].y - 8 && s_y < ROCKS[i].y + ROCK_SZ + 8;
}
// The stubborn "goofyrock" in ruins11: it won't be shoved — talk it into moving (obj_goofyrock,
// dialoguer msc 505). After the last line it settles onto its button and the spikes drop.
static void goofyTalk(void) {
    static const char* lines[GOOFY_DONE] = {
        "* WHOA there, pardner!&* Who said you could&  push me around?",
        "* HMM?&* So you're ASKIN' me&  to move over?&* Okay, just for you.",
        "* HMM?&* You want me to move&  some more?",
        "* HMM?&* That was the wrong&  direction?&* Okay, think I got it.",
        "* HMM?&* You wanted me to&  STAY there?&* Aren't things easier&  when you just ask?",
    };
    if (s_goofyStage < GOOFY_DONE) {
        showMsg(lines[s_goofyStage]);
        s_goofyStage++;
        if (s_goofyStage >= GOOFY_DONE) { s_rockX[GOOFY_IDX] = ROCKS[GOOFY_IDX].buttonX; s_rockOn[GOOFY_IDX] = true; }
    } else {
        showMsg("* (It's just a rock now.)");
    }
}

// Napstablook blocks the hall in ruins12 (obj_napstablook1). Interact to nudge him through his
// sleep-"z"s and sad rambling; at the end he drifts aside and the way opens. (No battle yet.)
#define NAPSTA_DONE 7
static void napstaTalk(void) {
    static const char* lines[NAPSTA_DONE] = {
        "* zzzzzzzzzzzzzzz...&* zzzzzzzzzzzzzz...",
        "* zzzzzzzzzz...&* (are they gone yet)&* zzzzzzzzzzzzzzz...",
        "* (This ghost keeps saying&  'z' out loud repeatedly,&  pretending to sleep.)",
        "* i usually come to the&  RUINS because there's&  nobody around...",
        "* but today i met&  somebody nice...",
        "* oh, i'm rambling again",
        "* i'll get out of your way",
    };
    if (s_napstaStage < NAPSTA_DONE) {
        showMsg(lines[s_napstaStage]);
        s_napstaStage++;
        if (s_napstaStage >= NAPSTA_DONE) s_napstaGone = true; // drifts aside; hall clears
    }
}
static bool napstaBlocking(void) { return s_roomId == ROOM_RUINS12 && !s_napstaGone; }

// Try to read a sign in the current room: the player's box must overlap the sign's zone.
static bool tryReadSign(void) {
    for (int i = 0; i < SIGN_COUNT; i++) {
        const Sign* g = &SIGNS[i];
        if (g->room != s_roomId) continue;
        if (s_x + CHARA_W > g->x - 14 && s_x < g->x + 34 &&
            s_y + CHARA_H > g->y && s_y < g->y + 55) { showMsg(g->text); return true; }
    }
    return false;
}

static SceneId owUpdate(u32 kDown, u32 kHeld) {
    if (s_fadeIn > 0) s_fadeIn--;

    // Falling through a cracked floor / rising up a vent (ruins8 & ruins10): owns input.
    if (s_fallMode != 0) {
        s_y += (s_fallMode == 1) ? (HOLE_FALL_DY / 162.0f) : -(HOLE_VENT_DY / 82.0f);
        if (s_y < 0) s_y = 0;
        if (s_y + CHARA_H > s_room->h) s_y = (float)(s_room->h - CHARA_H);
        if (--s_fallFrames <= 0) { s_fallMode = 0; s_fallArmed = false; }
        return SCENE_NONE;
    }

    // Overworld dialogue (the candy bowl) owns input while open.
    if (s_dlgStage != DLG_NONE) {
        int len = (int)strlen(s_dlgText);
        if (s_dlgRevealed < len) {
            if (kDown & (KEY_A | KEY_B)) s_dlgRevealed = len;
            else if (++s_dlgTimer >= 2) { s_dlgTimer = 0; s_dlgRevealed++; }
            return SCENE_NONE;
        }
        if (s_dlgStage == DLG_CHOICE) {
            if (kDown & K_LEFT)  s_dlgSel = 0;
            if (kDown & K_RIGHT) s_dlgSel = 1;
            if (kDown & KEY_A)     candyChoose();
        } else if (kDown & (KEY_A | KEY_B)) {
            s_dlgStage = DLG_NONE;
        }
        return SCENE_NONE;
    }

    // Cutscenes own input while active (no menu-exit or walking).
    if (floweyActive()) { floweyUpdate(kDown, kHeld); return SCENE_NONE; }
    if (torielActive()) { torielUpdate(kDown, kHeld); return SCENE_NONE; }

    // After Flowey, Toriel arrives once. When she has led you out, head to the Ruins.
    if (s_roomId == ROOM_AREA1_2 && floweyFinished() && !s_torielSeen) {
        s_torielSeen = true; torielStart(); return SCENE_NONE;
    }
    if (s_torielSeen && !s_torielLed && torielDoneLeading() && s_leaving == 0) {
        s_torielLed = true;
        s_pendRoom = ROOM_RUINS1; s_pendX = 150; s_pendY = 420; s_pendDir = DIR_UP; // follow Toriel into the Ruins
        s_leaving = LEAVE_FRAMES;
    }

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

    if (dx != 0.0f) {
        float nx = s_x + dx;
        int ri = rockOverlap(nx, s_y, -1);
        if (ri >= 0) {
            // Walking into a rock: shove it horizontally if it's pushable, unlocked, and its new
            // cell is clear. Snapping onto the button locks it and drops the room's spikes.
            if (!ROCKS[ri].talkable && !s_rockOn[ri]) {
                float rnx = s_rockX[ri] + dx;
                bool dirOK = (dx > 0) || (rnx >= ROCKS[ri].xstart); // never pull it left of its start
                if (dirOK && !boxHitsWall((int)floorf(rnx), ROCKS[ri].y, ROCK_SZ, ROCK_SZ) && rockBoxClear(rnx, ri)) {
                    s_rockX[ri] = (short)floorf(rnx + 0.5f);
                    s_x = nx;
                    if (dx > 0 && s_rockX[ri] >= ROCKS[ri].buttonX) {
                        s_rockX[ri] = ROCKS[ri].buttonX; s_rockOn[ri] = true; s_x -= 6;
                    }
                }
            }
        } else if (allowedAt(nx, s_y)) {
            s_x = nx;
        }
    }
    if (dy != 0.0f) {
        float ny = s_y + dy;
        if (rockOverlap(s_x, ny, -1) < 0 && allowedAt(s_x, ny)) s_y = ny;
    }

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

    // Interact (A): the candy bowl in the candy room, otherwise a readable sign if we're on one.
    if (kDown & KEY_A) {
        bool npFront = s_x + CHARA_W > 135 && s_x < 190 && s_y + CHARA_H > 95 && s_y < 175;
        bool nearSmallWeb = s_x + CHARA_W > 110 && s_x < 150 && s_y + CHARA_H > 80 && s_y < 130;
        bool nearBigWeb   = s_x + CHARA_W > 162 && s_x < 222 && s_y + CHARA_H > 62 && s_y < 122;
        if (s_roomId == ROOM_RUINS7A && s_dir == DIR_UP && s_y < 150) candyStart();
        else if (s_roomId == ROOM_RUINS11 && nearRock(GOOFY_IDX)) goofyTalk();
        else if (napstaBlocking() && npFront) napstaTalk();
        else if (s_roomId == ROOM_RUINS12B && nearSmallWeb)
            showMsg("* (A spider donut sits in&  the web. 7G.)&* (You have no G to leave.)");
        else if (s_roomId == ROOM_RUINS12B && nearBigWeb)
            showMsg("* (Spider cider fills a&  large web. 18G.)&* (You have no G to leave.)");
        else tryReadSign();
    }

    // Cracked floors drop you to the lower floor; vents lift you back up (ruins8 & ruins10).
    if (s_roomId == ROOM_RUINS8 || s_roomId == ROOM_RUINS10) {
        const short (*fall)[4]; const short (*vent)[4]; int fc, vc;
        if (s_roomId == ROOM_RUINS8) { fall = RUINS8_FALL;  fc = RUINS8_FALL_COUNT;  vent = RUINS8_VENT;  vc = RUINS8_VENT_COUNT; }
        else                         { fall = RUINS10_FALL; fc = RUINS10_FALL_COUNT; vent = RUINS10_VENT; vc = RUINS10_VENT_COUNT; }
        int fbx = (int)floorf(s_x) + FEET_OX, fby = (int)floorf(s_y) + FEET_OY;
        bool onTrig = false;
        for (int i = 0; i < fc; i++) if (boxesOverlap(fbx, fby, FEET_W, FEET_H, fall[i][0], fall[i][1], fall[i][2], fall[i][3])) {
            onTrig = true; if (s_fallArmed) { s_fallMode = 1; s_fallFrames = 162; } break;
        }
        if (s_fallMode == 0) for (int i = 0; i < vc; i++) if (boxesOverlap(fbx, fby, FEET_W, FEET_H, vent[i][0], vent[i][1], vent[i][2], vent[i][3])) {
            onTrig = true; if (s_fallArmed) { s_fallMode = 2; s_fallFrames = 82; } break;
        }
        if (!onTrig) s_fallArmed = true;
    }
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
        for (int i = 0; i < s_stripCount; i++)
            C2D_DrawImageAt(s_strips[i], floorf(TOP_X_OFFSET + s_stripX[i] - cx), floorf(-cy), 0.0f, NULL, 1.0f, 1.0f);
        if (s_roomId == ROOM_RUINS7A) { // the candy bowl; after 3 pieces the pedestal topples.
            // Y-sorted against Frisk: standing below the pedestal (the normal spot) puts Frisk in
            // front of it; only when Frisk is above its base does the pedestal draw over him.
            float dx = floorf(TOP_X_OFFSET + CANDY_X - cx), dy = floorf(CANDY_Y - cy);
            float dishBase = CANDY_Y + 24;                       // the pedestal's foot (room coords)
            float d = (s_y + CHARA_H >= dishBase) ? 0.08f : 0.12f;
            if (s_candyCount >= 4) {
                C2D_DrawImageAt(s_candyFallen, dx, dy, d,          NULL, 1.0f, 1.0f); // tipped pedestal
                C2D_DrawImageAt(s_candySpill,  dx, dy, d + 0.005f, NULL, 1.0f, 1.0f); // scattered candy
            } else {
                C2D_DrawImageAt(s_candyFull,   dx, dy, d,          NULL, 1.0f, 1.0f);
            }
        }
        // Puzzle spikes (up or retracted) and push-rocks.
        bool spDown = spikesRetracted(s_roomId);
        for (int i = 0; i < SPIKE_COUNT; i++) if (SPIKES[i].room == s_roomId)
            C2D_DrawImageAt(spDown ? s_spikeDown : s_spikeUp,
                            floorf(TOP_X_OFFSET + SPIKES[i].x - cx), floorf(SPIKES[i].y - cy), 0.085f, NULL, 1.0f, 1.0f);
        for (int i = 0; i < ROCK_COUNT; i++) if (ROCKS[i].room == s_roomId)
            C2D_DrawImageAt(s_rockImg,
                            floorf(TOP_X_OFFSET + s_rockX[i] - cx), floorf(ROCKS[i].y - cy), 0.08f, NULL, 1.0f, 1.0f);

        // Room props (decorative + interactable) for the ruins12 cluster.
        #define PROP(img,px,py,d) C2D_DrawImageAt((img), floorf(TOP_X_OFFSET + (px) - cx), floorf((py) - cy), (d), NULL, 1.0f, 1.0f)
        if (s_roomId == ROOM_RUINS12A) {
            PROP(s_mousehole, 110, 108, 0.08f);
            PROP(s_savepoint, 110, 160, 0.08f);
            PROP(s_cheese,    180, 160, 0.08f);
        } else if (napstaBlocking()) { // Napstablook in the ruins12 hall
            PROP(s_napsta, 140, 120, 0.09f);
        } else if (s_roomId == ROOM_RUINS12B) {
            PROP(s_bigweb,   172, 72, 0.08f);
            PROP(s_smallweb, 120, 90, 0.08f);
        } else if (s_roomId == ROOM_RUINS13) {
            PROP(s_frog, 220, 140, 0.08f);
            PROP(s_frog, 340, 140, 0.08f);
            PROP(s_frog, 460, 140, 0.08f);
        } else if (s_roomId == ROOM_RUINS17) {
            PROP(s_frog, 80, 80, 0.08f);        // the MERCY-advice froggit
        } else if (s_roomId == ROOM_RUINS19) {
            PROP(s_savepoint, 80, 200, 0.08f);  // SAVE point at the house front
        }
        #undef PROP
        if (s_phase == PHASE_WAKE)
            C2D_DrawImageAt(s_lie, floorf(TOP_X_OFFSET + s_x - 5 - cx), floorf(s_y - cy), 0.1f, NULL, 1.0f, 1.0f);
        else
            C2D_DrawImageAt(s_walk[s_dir][s_frame % DIR_FRAMES[s_dir]], floorf(TOP_X_OFFSET + s_x - cx), floorf(s_y - cy), 0.1f, NULL, 1.0f, 1.0f);
    }

    floweyDrawTop(cx, cy, TOP_X_OFFSET);
    torielDrawTop(cx, cy, TOP_X_OFFSET);

#ifdef DEBUG_SHOW_SOLIDS
    for (int i = 0; i < s_room->solidCount; i++) {
        const short* s = s_room->solids[i];
        C2D_DrawRectSolid(floorf(TOP_X_OFFSET + s[0] - cx), floorf(s[1] - cy), 0.2f,
                          s[2], s[3], C2D_Color32(0xFF, 0x00, 0x00, 0x80));
    }
#endif

    u8 wa = 0;
    if (s_fadeIn > 0) wa = (u8)(255 * s_fadeIn / FADE_IN_FRAMES);
    if (s_leaving > 0) { int v = (LEAVE_FRAMES - s_leaving) * 255 / LEAVE_FRAMES; if (v > wa) wa = (u8)v; }
    if (wa > 0) C2D_DrawRectSolid(0, 0, 0.5f, TOP_W, SCR_H, C2D_Color32(0xFF, 0xFF, 0xFF, wa));
}

static const char* roomName(RoomId id) {
    switch (id) {
        case ROOM_AREA1: return "area1"; case ROOM_AREA1_2: return "area1_2";
        case ROOM_RUINS1: return "RUINS 1"; case ROOM_RUINS2: return "RUINS 2";
        case ROOM_RUINS3: return "RUINS 3"; case ROOM_RUINS4: return "RUINS 4";
        case ROOM_RUINS5: return "RUINS 5"; case ROOM_RUINS6: return "RUINS 6";
        case ROOM_RUINS7: return "RUINS 7"; case ROOM_RUINS8: return "RUINS 8";
        case ROOM_RUINS9: return "RUINS 9"; case ROOM_RUINS10: return "RUINS 10";
        case ROOM_RUINS11: return "RUINS 11"; case ROOM_RUINS12: return "RUINS 12";
        case ROOM_RUINS7A: return "RUINS 7A";
        case ROOM_RUINS12A: return "RUINS 12A"; case ROOM_RUINS12B: return "SPIDER BAKE SALE";
        case ROOM_RUINS13: return "RUINS 13";
        case ROOM_RUINS14: return "RUINS 14";
        case ROOM_RUINS15A: return "RUINS 15A"; case ROOM_RUINS15B: return "RUINS 15B";
        case ROOM_RUINS15C: return "RUINS 15C"; case ROOM_RUINS15D: return "RUINS 15D";
        case ROOM_RUINS15E: return "RUINS 15E"; case ROOM_RUINS16: return "RUINS 16";
        case ROOM_RUINS17: return "RUINS 17"; case ROOM_RUINS18OLD: return "RUINS 18";
        case ROOM_RUINS19: return "HOME (front)";
        case ROOM_TORHOUSE1: return "HOME"; case ROOM_TORHOUSE2: return "HOME";
        case ROOM_TORHOUSE3: return "HOME";
        default: return "?";
    }
}

static void owDrawBottom(void) {
    if (floweyActive()) { floweyDrawBottom(); return; }
    if (torielActive()) { torielDrawBottom(); return; }

    // Overworld dialogue box (candy bowl), Undertale-style: white border, black fill.
    if (s_dlgStage != DLG_NONE) {
        C2D_DrawRectSolid(8, 40, 0.0f, BOT_W - 16, 150, COL_WHITE);
        C2D_DrawRectSolid(12, 44, 0.0f, BOT_W - 24, 142, COL_BLACK);
        char shown[180]; int n = s_dlgRevealed < 179 ? s_dlgRevealed : 179;
        const char* t = s_dlgText; int j = 0;
        for (int i = 0; i < n && t[i]; i++) shown[j++] = (t[i] == '&') ? '\n' : t[i];
        shown[j] = '\0';
        txtDraw(shown, 26.0f, 64.0f, 0.6f, COL_WHITE);
        if (s_dlgStage == DLG_CHOICE && s_dlgRevealed >= (int)strlen(s_dlgText))
            txtDraw(s_dlgSel == 0 ? "> Yes     No" : "  Yes   > No", 70.0f, 150.0f, 0.6f, COL_WHITE);
        return;
    }

    txtDrawCentered(roomName(s_roomId), BOT_W / 2.0f, 100.0f, 0.7f, COL_WHITE);
    txtDraw("D-pad: walk    [A] check    [B] menu", 20.0f, 214.0f, 0.5f, COL_GRAY);
}

const Scene SCENE_game = { owInit, owUpdate, owDrawTop, owDrawBottom, owCleanup };
