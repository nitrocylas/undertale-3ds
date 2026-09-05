// Shared game core: screen constants, the scene interface, and small draw helpers.
// One scene = one screen of the game (intro, menu, ...). main.c drives the active scene.
#pragma once
#include <3ds.h>
#include <citro2d.h>

// Screen geometry. Undertale renders at 320x240 native; the top screen is 400 wide,
// so game art is centered with 40px side bars (see TOP_X_OFFSET).
#define TOP_W  400
#define BOT_W  320
#define SCR_H  240
#define GAME_W 320
#define TOP_X_OFFSET ((TOP_W - GAME_W) / 2)

// Undertale palette bits we reuse.
#define COL_BLACK  C2D_Color32(0x00, 0x00, 0x00, 0xFF)
#define COL_WHITE  C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define COL_YELLOW C2D_Color32(0xFF, 0xFF, 0x00, 0xFF)
#define COL_GRAY   C2D_Color32(0x80, 0x80, 0x80, 0xFF)
#define COL_LTGRAY C2D_Color32(0xC0, 0xC0, 0xC0, 0xFF)

typedef enum {
    SCENE_NONE = 0, // update() returns this to stay in the current scene
    SCENE_INTRO,
    SCENE_MENU,
    SCENE_GAME,
} SceneId;

// A scene is a small vtable. update() returns the next scene (or SCENE_NONE to stay).
typedef struct {
    void    (*init)(void);
    SceneId (*update)(u32 kDown, u32 kHeld);
    void    (*drawTop)(void);
    void    (*drawBottom)(void);
    void    (*cleanup)(void);
} Scene;

// Scene instances (defined in their own files).
extern const Scene SCENE_intro;
extern const Scene SCENE_menu;
extern const Scene SCENE_game;

// Shared state set by the menu, read by the game scene.
extern char g_playerName[16];

// Text helpers (system font for now; real fnt_main is a later slice).
// Draw at top-left origin, or horizontally centered on cx.
void txtDraw(const char* s, float x, float y, float scale, u32 color);
void txtDrawCentered(const char* s, float cx, float y, float scale, u32 color);

// Core lifecycle (defined in game.c, called by main.c).
void gameCoreInit(void);
void gameCoreExit(void);
