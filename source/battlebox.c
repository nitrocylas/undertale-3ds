// Battle box implementation. See battlebox.h. Screen-space (top screen) UI.
#include "battlebox.h"
#include "game.h"
#include <math.h>
#include <stdio.h>

#include "heart_t3x.h"
#include "pellet_t3x.h"

#define SOUL_SPEED 2.0f
#define IFRAMES    30      // invincibility after a hit (matches global.invc = 30)

static C2D_SpriteSheet s_heartSheet, s_pelletSheet;
static C2D_Image s_heart, s_pellet;
static float s_spin; // shared pellet spin angle

static float s_bx, s_by, s_bw, s_bh; // box rect
static float s_sx, s_sy;             // SOUL top-left
static int   s_hp, s_maxhp;
static int   s_inv;                  // i-frame countdown
static int   s_shake;                // shake countdown
static Bullet s_bullets[MAX_BULLETS];

static void setNearest(C2D_Image img) { if (img.tex) C3D_TexSetFilter(img.tex, GPU_NEAREST, GPU_NEAREST); }

void battleLoad(void) {
    s_heartSheet = C2D_SpriteSheetLoadFromMem(heart_t3x, heart_t3x_size);
    s_heart = C2D_SpriteSheetGetImage(s_heartSheet, 0);
    setNearest(s_heart);
    s_pelletSheet = C2D_SpriteSheetLoadFromMem(pellet_t3x, pellet_t3x_size);
    s_pellet = C2D_SpriteSheetGetImage(s_pelletSheet, 0);
    setNearest(s_pellet);
}
void battleFree(void) {
    if (s_heartSheet) C2D_SpriteSheetFree(s_heartSheet);
    if (s_pelletSheet) C2D_SpriteSheetFree(s_pelletSheet);
}

void battleSetBox(float x, float y, float w, float h) {
    s_bx = x; s_by = y; s_bw = w; s_bh = h;
    battleResetSoul();
}
void battleResetSoul(void) {
    s_sx = s_bx + s_bw / 2 - SOUL_SIZE / 2;
    s_sy = s_by + s_bh / 2 - SOUL_SIZE / 2;
    s_inv = 0; s_shake = 0;
}
void battleSetHP(int hp, int maxhp) { s_hp = hp; s_maxhp = maxhp; }
int  battleHP(void) { return s_hp; }

void battleUpdateSoul(u32 kHeld, bool soulControllable) {
    if (soulControllable) {
        if (kHeld & (KEY_DLEFT | KEY_CPAD_LEFT))   s_sx -= SOUL_SPEED;
        if (kHeld & (KEY_DRIGHT | KEY_CPAD_RIGHT)) s_sx += SOUL_SPEED;
        if (kHeld & (KEY_DUP | KEY_CPAD_UP))       s_sy -= SOUL_SPEED;
        if (kHeld & (KEY_DDOWN | KEY_CPAD_DOWN))   s_sy += SOUL_SPEED;
    }
    // Clamp inside the box interior (2px inset for the border).
    float min_x = s_bx + 3, max_x = s_bx + s_bw - SOUL_SIZE - 3;
    float min_y = s_by + 3, max_y = s_by + s_bh - SOUL_SIZE - 3;
    if (s_sx < min_x) s_sx = min_x;
    if (s_sx > max_x) s_sx = max_x;
    if (s_sy < min_y) s_sy = min_y;
    if (s_sy > max_y) s_sy = max_y;
    if (s_inv > 0) s_inv--;
    if (s_shake > 0) s_shake--;
}

void battleSoulPos(float* x, float* y) { *x = s_sx + SOUL_SIZE / 2; *y = s_sy + SOUL_SIZE / 2; }

int battleAddBullet(float x, float y, float r) {
    for (int i = 0; i < MAX_BULLETS; i++)
        if (!s_bullets[i].active) { s_bullets[i] = (Bullet){ x, y, r, true }; return i; }
    return -1;
}
Bullet* battleBullet(int i) { return (i >= 0 && i < MAX_BULLETS) ? &s_bullets[i] : NULL; }
void battleClearBullets(void) { for (int i = 0; i < MAX_BULLETS; i++) s_bullets[i].active = false; }

bool battleCheckHits(int damage) {
    if (s_inv > 0) return false;
    float scx = s_sx + SOUL_SIZE / 2, scy = s_sy + SOUL_SIZE / 2;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!s_bullets[i].active) continue;
        float dx = s_bullets[i].x - scx, dy = s_bullets[i].y - scy;
        float rr = s_bullets[i].r + SOUL_SIZE / 2 - 2;
        if (dx * dx + dy * dy <= rr * rr) {
            s_hp -= damage; if (s_hp < 0) s_hp = 0;
            s_inv = IFRAMES; s_shake = 8;
            return true;
        }
    }
    return false;
}

float battleShakeX(void) { return s_shake > 0 ? ((s_shake % 2) ? 2.0f : -2.0f) : 0.0f; }
float battleShakeY(void) { return s_shake > 0 ? ((s_shake % 2) ? -1.0f : 1.0f) : 0.0f; }

void battleDrawBox(void) {
    float sx = battleShakeX(), sy = battleShakeY();
    float x = s_bx + sx, y = s_by + sy;
    // White border, black interior (4px border).
    C2D_DrawRectSolid(x, y, 0.0f, s_bw, s_bh, COL_WHITE);
    C2D_DrawRectSolid(x + 4, y + 4, 0.0f, s_bw - 8, s_bh - 8, COL_BLACK);
}

void battleDrawSoul(void) {
    // Blink during i-frames.
    if (s_inv > 0 && (s_inv / 3) % 2) return;
    float sx = battleShakeX(), sy = battleShakeY();
    C2D_DrawImageAt(s_heart, floorf(s_sx + sx), floorf(s_sy + sy), 0.4f, NULL, 1.0f, 1.0f);
}

void battleDrawBullets(void) {
    float sx = battleShakeX(), sy = battleShakeY();
    s_spin += 0.18f; // friendliness pellets spin
    // The 12x12 pellet sprite, drawn centered and rotated about its middle.
    const float half = 6.0f;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!s_bullets[i].active) continue;
        float scale = (s_bullets[i].r * 2.0f) / 12.0f; // fit sprite to the bullet radius
        C2D_DrawImageAtRotated(s_pellet, s_bullets[i].x + sx, s_bullets[i].y + sy,
                               0.4f, s_spin, NULL, scale, scale);
        (void)half;
    }
}

void battleDrawHP(float x, float y) {
    char buf[32];
    snprintf(buf, sizeof(buf), "HP %d/%d", s_hp, s_maxhp);
    // yellow bar
    float bw = 60.0f;
    C2D_DrawRectSolid(x, y, 0.0f, bw, 8, C2D_Color32(0x80, 0x00, 0x00, 0xFF));
    float frac = s_maxhp > 0 ? (float)s_hp / s_maxhp : 0;
    C2D_DrawRectSolid(x, y, 0.0f, bw * frac, 8, COL_YELLOW);
    txtDraw(buf, x + bw + 8, y - 3, 0.5f, COL_WHITE);
}
