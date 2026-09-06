// Fall-through cracked floors + vents for ruins8 & ruins10 (obj_holedown / obj_holedown2 /
// obj_holeup, decompiled from data.win). This is IN-ROOM vertical movement, NOT a room change:
//   FALL (holedown): on stepping into a crack rect the player turns translucent (alpha 0.5),
//     drops HOLE_FALL_DY px downward over ~81 frames (snd_fall2), a crack (data/cosmetic_hole.t3x
//     = spr_hole, 20x16) appears where you fell, then solidity/alpha restore. No HP loss.
//   VENT (holeup): on stepping onto a vent the player goes invisible and rises HOLE_VENT_DY px.
// Both rooms are tall (ruins8 320x640, ruins10 580x760): an upper floor holds the cracks, the
// lower floor holds the vents that send you back up. In ruins10 the SAFE path is the gap between
// the fall rects below (the "leaves"); the plaques warn "Please don't step on the leaves."
#pragma once

#define HOLE_FALL_DY  405   // obj_holedown: vspeed 5 * 81 frames (30fps). Port @60fps: 2.5px * 162f.
#define HOLE_VENT_DY  410   // obj_holeup:   vspeed 10 * 41 frames (30fps).

// {x, y, w, h} in room coords. Stepping (feet box) into a FALL rect drops you; onto a VENT lifts you.
static const short RUINS8_FALL[][4] = {
    { 150,  96, 20, 80 },   // obj_holedown2 @150,96, spr_event scaled yscale 4 (20x80 strip)
};
static const short RUINS8_VENT[][4] = {
    {  70, 482, 20, 20 }, { 230, 482, 20, 20 },   // two vents on the lower floor
};

// ruins10 fall rects: from spr_holemask (301x160) opaque pixels, two instances @ (238,138) and
// (160,272). Safe leaf-path is the gap that shifts left as you descend (upper gap x298-478,
// lower gap x220-400). Refine against the room art if needed.
static const short RUINS10_FALL[][4] = {
    { 238, 158, 60, 120 }, { 258, 138, 40, 20 }, { 258, 278, 40, 20 }, // inst A left block + nubs
    { 478, 178, 60, 40 },  { 478, 218, 20, 40 },                       // inst A right block
    { 160, 292, 60, 100 }, { 180, 272, 40, 20 }, { 180, 392, 40, 40 }, // inst B left block + nubs
    { 400, 312, 60, 40 },  { 400, 352, 20, 40 },                       // inst B right block
};
static const short RUINS10_VENT[][4] = {
    { 150, 484, 20, 20 },   // obj_holeup @150,484 (lower floor -> back up)
};

#define RUINS8_FALL_COUNT   ((int)(sizeof(RUINS8_FALL)  / sizeof(RUINS8_FALL[0])))
#define RUINS8_VENT_COUNT   ((int)(sizeof(RUINS8_VENT)  / sizeof(RUINS8_VENT[0])))
#define RUINS10_FALL_COUNT  ((int)(sizeof(RUINS10_FALL) / sizeof(RUINS10_FALL[0])))
#define RUINS10_VENT_COUNT  ((int)(sizeof(RUINS10_VENT) / sizeof(RUINS10_VENT[0])))
