// Flowey's overworld introduction cutscene (room_area1_2). Self-contained state machine:
// rise -> dialogue -> SOUL appears -> friendliness pellets -> "Die" -> Toriel's rescue.
// The overworld scene hands control here while active.
#pragma once
#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>

void floweyLoad(void);              // load sprites (once)
void floweyFree(void);
void floweyStart(void);             // begin the cutscene
bool floweyActive(void);
bool floweyInCombat(void);          // true once the battle box is up (overworld should hide)
bool floweyFinished(void);          // true after the scene fully completes (Toriel's cue)
void floweyUpdate(u32 kDown, u32 kHeld); // advance; clears active when finished
void floweyDrawTop(float camX, float camY, int topXOffset); // Flowey + SOUL on the arena
void floweyDrawBottom(void);        // dialogue box + talking face
