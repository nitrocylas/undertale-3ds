// Main-menu scene = Undertale's fresh-save flow, on the bottom (touch) screen:
//   INSTRUCTIONS (Begin Game / Settings) -> NAME entry grid -> CONFIRM (No / Yes)
//   -> FADEOUT (name zooms + shakes, screen whites out) -> game.
// Text and easter-egg name responses are the game's own (extracted from data.win).
#include "game.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Input: adapt Undertale's Z/X to the 3DS. A = confirm, B = cancel/backspace.
#define K_LEFT  (KEY_DLEFT  | KEY_CPAD_LEFT)
#define K_RIGHT (KEY_DRIGHT | KEY_CPAD_RIGHT)
#define K_UP    (KEY_DUP    | KEY_CPAD_UP)
#define K_DOWN  (KEY_DDOWN  | KEY_CPAD_DOWN)

typedef enum { M_INSTRUCTIONS, M_NAME, M_CONFIRM, M_FADEOUT } MenuState;

#define NAME_MAX 6
#define GRID_ROWS 8
#define GRID_COLS 7

static const int XMAP[GRID_COLS] = { 60, 92, 124, 156, 188, 220, 252 };
static const int YMAP[GRID_ROWS] = { 75, 89, 103, 117, 135, 149, 163, 177 };
static char s_charmap[GRID_ROWS][GRID_COLS]; // 0 = empty cell

// Menu row (Quit/Backspace/Done) lives at pseudo-row -1.
#define MENU_ROW (-1)
static const int MENU_X[3] = { 60, 120, 220 };
#define MENU_Y 200

// Fade-out (naming==5) timing, in frames @ 60fps.
#define FADE_Q_MAX   120
#define FADE_DONE    170

static MenuState s_state;
static int  s_selRow, s_selCol;
static int  s_instrSel;
static int  s_confSel;
static int  s_allow;
static int  s_restartToIntro;
static int  s_q, s_alerm;         // fade-out animation clocks
static char s_name[NAME_MAX + 1];
static char s_specMsg[128];

// Subtle per-frame jitter (Undertale's naming text shakes). amp in pixels.
static float jitter(float amp) { return (((rand() % 1000) / 1000.0f) - 0.5f) * 2.0f * amp; }

typedef struct { const char* n; int allow; const char* m; } NameCheck;
static const NameCheck NAMECHECK[] = {
    {"aaaaaa", 1, "Not very creative...?"},
    {"asgore", 0, "You cannot."},
    {"toriel", 0, "I think you should#think of your own#name, my child."},
    {"sans",   0, "nope."},
    {"undyne", 0, "Get your OWN name!"},
    {"flowey", 0, "I already CHOSE#that name."},
    {"chara",  1, "The true name."},
    {"alphys", 0, "D-don't do that."},
    {"alphy",  1, "Uh... OK?"},
    {"papyru", 1, "I'LL ALLOW IT!!!!"},
    {"napsta", 1, "...........#(They're powerless to#stop you.)"},
    {"blooky", 1, "...........#(They're powerless to#stop you.)"},
    {"murder", 1, "That's a little on-#the nose, isn't it...?"},
    {"mercy",  1, "That's a little on-#the nose, isn't it...?"},
    {"asriel", 0, "..."},
    {"catty",  1, "Bratty! Bratty!#That's MY name!"},
    {"bratty", 1, "Like, OK I guess."},
    {"mtt",    1, "OOOOH!!! ARE YOU#PROMOTING MY BRAND?"},
    {"metta",  1, "OOOOH!!! ARE YOU#PROMOTING MY BRAND?"},
    {"mett",   1, "OOOOH!!! ARE YOU#PROMOTING MY BRAND?"},
    {"gerson", 1, "Wah ha ha! Why not?"},
    {"shyren", 1, "...?"},
    {"aaron",  1, "Is this name correct? ; )"},
    {"temmie", 1, "hOI!"},
    {"woshua", 1, "Clean name."},
    {"jerry",  1, "Jerry."},
    {"bpants", 1, "You are really scraping the#bottom of the barrel."},
    {"cylas",  1, "hey i made this port hi!!! :D"}, // this port's author
};
#define NUM_NAMECHECK ((int)(sizeof(NAMECHECK) / sizeof(NAMECHECK[0])))

static void buildCharmap(void) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            int index = i * GRID_COLS + j;
            s_charmap[i][j]     = (index < 26) ? (char)('A' + index) : 0;
            s_charmap[i + 4][j] = (index < 26) ? (char)('a' + index) : 0;
        }
    }
}

static void drawMsg(const char* s, float x, float y, float scale, u32 color) {
    char buf[160];
    int o = 0;
    for (const char* p = s; *p && o < 159; p++) buf[o++] = (*p == '#') ? '\n' : *p;
    buf[o] = '\0';
    txtDraw(buf, x, y, scale, color);
}

// Returns 1 if the name triggers a restart (gaster).
static int checkName(void) {
    char low[NAME_MAX + 1];
    int n = 0;
    for (const char* p = s_name; *p; p++) low[n++] = (char)tolower((unsigned char)*p);
    low[n] = '\0';
    if (strcmp(low, "gaster") == 0) return 1;
    for (int i = 0; i < NUM_NAMECHECK; i++) {
        if (strcmp(low, NAMECHECK[i].n) == 0) {
            s_allow = NAMECHECK[i].allow;
            strncpy(s_specMsg, NAMECHECK[i].m, sizeof(s_specMsg) - 1);
            return 0;
        }
    }
    s_allow = 1;
    strcpy(s_specMsg, "Is this name correct?");
    return 0;
}

static void menuInit(void) {
    buildCharmap();
    s_state = M_INSTRUCTIONS;
    s_selRow = 0; s_selCol = 0;
    s_instrSel = 0; s_confSel = 0; s_allow = 1; s_restartToIntro = 0;
    s_q = 0; s_alerm = 0;
    s_name[0] = '\0';
}

static void menuCleanup(void) {}

static int menuColForGrid(int gridCol) { return gridCol <= 1 ? 0 : (gridCol <= 4 ? 1 : 2); }
static int gridColForMenu(int menuCol) { return menuCol == 0 ? 0 : (menuCol == 1 ? 3 : 6); }
static bool cellEmpty(int r, int c) { return r >= 0 && s_charmap[r][c] == 0; }

static void navName(u32 kDown) {
    if (kDown & K_RIGHT) {
        do {
            if (s_selRow == MENU_ROW) { s_selCol = (s_selCol + 1) % 3; break; }
            s_selCol++;
            if (s_selCol >= GRID_COLS) {
                if (s_selRow == GRID_ROWS - 1) { s_selCol = GRID_COLS - 1; break; }
                s_selCol = 0; s_selRow++;
            }
        } while (cellEmpty(s_selRow, s_selCol));
    }
    if (kDown & K_LEFT) {
        do {
            if (s_selRow == MENU_ROW) { s_selCol = (s_selCol + 2) % 3; break; }
            s_selCol--;
            if (s_selCol < 0) {
                if (s_selRow == 0) { s_selCol = 0; break; }
                s_selCol = GRID_COLS - 1; s_selRow--;
            }
        } while (cellEmpty(s_selRow, s_selCol));
    }
    if (kDown & K_DOWN) {
        if (s_selRow == MENU_ROW) { s_selRow = 0; s_selCol = menuColForGrid(gridColForMenu(s_selCol)); }
        else {
            s_selRow++;
            if (s_selRow >= GRID_ROWS) { s_selRow = MENU_ROW; s_selCol = menuColForGrid(s_selCol); }
        }
        while (cellEmpty(s_selRow, s_selCol)) s_selRow++;
        if (s_selRow >= GRID_ROWS) { s_selRow = MENU_ROW; s_selCol = menuColForGrid(GRID_COLS - 1); }
    }
    if (kDown & K_UP) {
        if (s_selRow == MENU_ROW) { s_selRow = GRID_ROWS - 1; s_selCol = menuColForGrid(gridColForMenu(s_selCol)); }
        else {
            s_selRow--;
            if (s_selRow < 0) { s_selRow = MENU_ROW; s_selCol = menuColForGrid(s_selCol); }
        }
        while (cellEmpty(s_selRow, s_selCol)) s_selRow--;
        if (s_selRow < MENU_ROW) { s_selRow = MENU_ROW; s_selCol = menuColForGrid(0); }
    }
}

static SceneId updateInstructions(u32 kDown) {
    if (kDown & K_DOWN) s_instrSel = 1;
    if (kDown & K_UP)   s_instrSel = 0;
    if (kDown & KEY_A) {
        if (s_instrSel == 0) { s_state = M_NAME; s_selRow = 0; s_selCol = 0; }
    }
    return SCENE_NONE;
}

static SceneId updateName(u32 kDown) {
    navName(kDown);
    if (kDown & KEY_A) {
        if (s_selRow == MENU_ROW) {
            if (s_selCol == 0) { s_state = M_INSTRUCTIONS; s_instrSel = 0; }
            else if (s_selCol == 1) { int L = strlen(s_name); if (L > 0) s_name[L - 1] = '\0'; }
            else if (s_selCol == 2 && strlen(s_name) > 0) {
                if (checkName()) return SCENE_INTRO;   // "gaster" restarts the game
                s_confSel = 0; s_state = M_CONFIRM;
            }
        } else {
            char ch = s_charmap[s_selRow][s_selCol];
            int L = strlen(s_name);
            if (ch && L < NAME_MAX) { s_name[L] = ch; s_name[L + 1] = '\0'; }
        }
    }
    if (kDown & KEY_B) { int L = strlen(s_name); if (L > 0) s_name[L - 1] = '\0'; }
    return SCENE_NONE;
}

static SceneId updateConfirm(u32 kDown) {
    if (s_allow && (kDown & (K_LEFT | K_RIGHT))) s_confSel = !s_confSel;
    if (kDown & KEY_A) {
        if (!s_allow) { s_state = M_NAME; return SCENE_NONE; }
        if (s_confSel == 1 && strlen(s_name) > 0) {
            strncpy(g_playerName, s_name, sizeof(g_playerName) - 1);
            s_state = M_FADEOUT; s_q = 0; s_alerm = 0;   // name zoom + white fade
            return SCENE_NONE;
        }
        s_state = M_NAME;
    }
    if (kDown & KEY_B) s_state = M_NAME;
    return SCENE_NONE;
}

static SceneId updateFadeout(void) {
    if (s_q < FADE_Q_MAX) s_q++;
    s_alerm++;
    if (s_alerm > FADE_DONE) return SCENE_GAME;   // wake up in the Ruins (next slice)
    return SCENE_NONE;
}

static SceneId menuUpdate(u32 kDown, u32 kHeld) {
    (void)kHeld;
    switch (s_state) {
        case M_INSTRUCTIONS: return updateInstructions(kDown);
        case M_NAME:         return updateName(kDown);
        case M_CONFIRM:      return updateConfirm(kDown);
        case M_FADEOUT:      return updateFadeout();
    }
    return SCENE_NONE;
}

static u32 whiteAlpha(void) {
    int a = s_alerm * 2;
    if (a > 255) a = 255;
    return C2D_Color32(0xFF, 0xFF, 0xFF, (u8)a);
}

static void menuDrawTop(void) {
    if (s_state == M_FADEOUT) {
        C2D_DrawRectSolid(0, 0, 0.5f, TOP_W, SCR_H, whiteAlpha());
        return;
    }
    txtDrawCentered("UNDERTALE", TOP_W / 2.0f, 96.0f, 1.2f, COL_WHITE);
}

static void drawInstructions(void) {
    txtDraw(" --- Instruction ---", 60.0f, 16.0f, 0.6f, COL_LTGRAY);
    txtDraw("[A] - Confirm",         60.0f, 44.0f, 0.55f, COL_WHITE);
    txtDraw("[B] - Cancel",          60.0f, 62.0f, 0.55f, COL_WHITE);
    txtDraw("[X] - Menu (In-game)",  60.0f, 80.0f, 0.55f, COL_WHITE);
    txtDraw("When HP is 0, you lose.", 60.0f, 108.0f, 0.55f, COL_WHITE);
    txtDraw("Begin Game", 60.0f, 150.0f, 0.6f, s_instrSel == 0 ? COL_YELLOW : COL_WHITE);
    txtDraw("Settings",   60.0f, 172.0f, 0.6f, s_instrSel == 1 ? COL_YELLOW : COL_WHITE);
}

static void drawName(void) {
    txtDrawCentered("Name the fallen human.", 160.0f, 18.0f, 0.6f, COL_WHITE);
    txtDraw(s_name, 140.0f, 44.0f, 0.7f, COL_WHITE);

    char cell[2] = { 0, 0 };
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (!s_charmap[r][c]) continue;
            cell[0] = s_charmap[r][c];
            bool sel = (s_selRow == r && s_selCol == c);
            txtDraw(cell, XMAP[c] + jitter(0.8f), YMAP[r] + jitter(0.8f), 0.5f, sel ? COL_YELLOW : COL_WHITE);
        }
    }
    txtDraw("Quit",      MENU_X[0], MENU_Y, 0.55f, (s_selRow == MENU_ROW && s_selCol == 0) ? COL_YELLOW : COL_WHITE);
    txtDraw("Backspace", MENU_X[1], MENU_Y, 0.55f, (s_selRow == MENU_ROW && s_selCol == 1) ? COL_YELLOW : COL_WHITE);
    txtDraw("Done",      MENU_X[2], MENU_Y, 0.55f, (s_selRow == MENU_ROW && s_selCol == 2) ? COL_YELLOW : COL_WHITE);
}

static void drawConfirm(void) {
    drawMsg(s_specMsg, 90.0f, 30.0f, 0.6f, COL_WHITE);
    txtDrawCentered(s_name, 160.0f + jitter(1.0f), 90.0f + jitter(1.0f), 0.9f, COL_WHITE);
    if (s_allow) {
        txtDrawCentered("No",  80.0f,  200.0f, 0.6f, s_confSel == 0 ? COL_YELLOW : COL_WHITE);
        txtDrawCentered("Yes", 240.0f, 200.0f, 0.6f, s_confSel == 1 ? COL_YELLOW : COL_WHITE);
    } else {
        txtDrawCentered("Go back", 80.0f, 200.0f, 0.6f, COL_YELLOW);
    }
}

static void drawFadeout(void) {
    // The name grows and shakes, then the whole screen whites out (naming==5).
    float scale = 0.9f + (s_q / 50.0f);
    float amp = s_q / 40.0f;
    txtDrawCentered(s_name, 160.0f + jitter(amp), (s_q / 2.0f) + 70.0f + jitter(amp), scale, COL_WHITE);
    C2D_DrawRectSolid(0, 0, 0.5f, BOT_W, SCR_H, whiteAlpha());
}

static void menuDrawBottom(void) {
    switch (s_state) {
        case M_INSTRUCTIONS: drawInstructions(); break;
        case M_NAME:         drawName();         break;
        case M_CONFIRM:      drawConfirm();      break;
        case M_FADEOUT:      drawFadeout();      break;
    }
}

const Scene SCENE_menu = {
    menuInit, menuUpdate, menuDrawTop, menuDrawBottom, menuCleanup
};
