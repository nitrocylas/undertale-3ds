// room_ruins7A collision — Undertale's real solids (room 320x240). {x,y,w,h,type}; type 0=rect.
// The candy-bowl side room: a narrow vertical corridor (walkable x120-200) with the dish at the
// top and the doorway back to ruins7 at the bottom.
static const short RUINS7A_SOLIDS[][5] = {
    { 120, 200, 20, 40, 0 },
    { 180, 200, 20, 40, 0 },
    { 100,  20, 20, 220, 0 }, // left wall column
    { 200,  20, 20, 220, 0 }, // right wall column
    {  80, 200, 20, 20, 0 },
    { 100, 200, 20, 20, 0 },
    { 200, 200, 20, 20, 0 },
    { 220, 200, 20, 20, 0 },
    {  60,  60, 260, 20, 0 }, // top wall
    { 150, 110, 20, 14, 0 },  // the candy pedestal (obj_candydish1 @150,95) — solid base
};
#define RUINS7A_SOLID_COUNT 10
