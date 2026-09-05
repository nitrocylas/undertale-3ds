// Undertale-3DS entry point: init graphics, then run the active scene each frame.
// Top + bottom screens are drawn every frame; scenes decide what goes on each.
#include "game.h"
#include "audio.h"

static const Scene* sceneFor(SceneId id) {
    switch (id) {
        case SCENE_INTRO: return &SCENE_intro;
        case SCENE_MENU:  return &SCENE_menu;
        case SCENE_GAME:  return &SCENE_game;
        default:          return &SCENE_intro;
    }
}

int main(int argc, char** argv) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    gameCoreInit();
    audioInit();

    const Scene* cur = &SCENE_intro;
    cur->init();

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        if (kDown & KEY_START) break;

        SceneId next = cur->update(kDown, kHeld);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, COL_BLACK);
        C2D_SceneBegin(top);
        cur->drawTop();
        C2D_TargetClear(bot, COL_BLACK);
        C2D_SceneBegin(bot);
        cur->drawBottom();
        C3D_FrameEnd(0);

        if (next != SCENE_NONE) {
            cur->cleanup();
            cur = sceneFor(next);
            cur->init();
        }
    }

    audioExit();
    gameCoreExit();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
