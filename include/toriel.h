// Toriel's arrival cutscene (after Flowey, in room_area1_2). She walks in, delivers her
// introduction, then leads you out toward the Ruins.
#pragma once
#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>

void torielLoad(void);
void torielFree(void);
void torielStart(void);
bool torielActive(void);
bool torielDoneLeading(void);  // true when she has led you out -> transition to the Ruins
void torielUpdate(u32 kDown, u32 kHeld);
void torielDrawTop(float camX, float camY, int topXOffset);
void torielDrawBottom(void);
