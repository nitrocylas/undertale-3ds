// Reusable Undertale battle box: a white-bordered arena on the top screen, the red SOUL
// (heart) that the player moves inside it, a small bullet list, HP, i-frames and screen
// shake. Every battle (Flowey's intro, Toriel, ...) drives this. Bullets are generic; each
// battle supplies their motion by updating positions each frame.
#pragma once
#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>

#define MAX_BULLETS 24
#define SOUL_SIZE 16

typedef struct { float x, y, r; bool active; } Bullet; // r = collision radius

void  battleLoad(void);      // load the heart sprite (once)
void  battleFree(void);

// Configure the arena (top-screen pixel coords) and place the SOUL at its center.
void  battleSetBox(float x, float y, float w, float h);
void  battleResetSoul(void);
void  battleSetHP(int hp, int maxhp);
int   battleHP(void);

// Per-frame: move the SOUL (clamped to the box) and tick i-frames / shake.
void  battleUpdateSoul(u32 kHeld, bool soulControllable);

// Bullets: the battle owns their motion; these are helpers.
int   battleAddBullet(float x, float y, float r);   // returns index or -1
Bullet* battleBullet(int i);
void  battleClearBullets(void);
// Check SOUL vs all active bullets; applies damage + i-frames + shake on hit. Returns true if hit.
bool  battleCheckHits(int damage);

void  battleSoulPos(float* x, float* y); // SOUL center (for homing bullets)

// Draw the box, SOUL and bullets. shakeXY is applied by the box internally.
void  battleDrawBox(void);
void  battleDrawSoul(void);
void  battleDrawBullets(void);
void  battleDrawHP(float x, float y); // simple HP bar/label
float battleShakeX(void);
float battleShakeY(void);
