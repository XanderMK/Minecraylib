#include "raylib.h"
#include "core.hpp"
#include "rlgl.h"

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetTraceLogLevel(LOG_ERROR);

    // Initialize graphics
    InitWindow(screenWidth, screenHeight, "yippee!!!!!!🪳🪳🪳🪳🪳");
    SetTargetFPS(60);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

    // Initialize audio
    InitAudioDevice();

    // Misc init
    DisableCursor();
    SetExitKey(KEY_NULL);

    Core core;
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        core.Update(GetFrameTime());
        core.Render();
    }

    CloseAudioDevice();

    CloseWindow();

    return 0;
}
