#include "core.hpp"

Core::Core() : camera((Camera){{ 16, 240, 16 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 60.0f, 0}), world(World(&camera))
{}

Core::~Core() = default;

void Core::Update(float deltaTime)
{
    UpdateCamera(&camera, CAMERA_FREE);
    world.Update();
}

void Core::Render() const
{
    BeginDrawing();
    {
        ClearBackground(SKYBLUE);
        BeginMode3D(camera);
        {
            world.RenderChunks();
        }
        EndMode3D();
        DrawFPS(20, 20);
    }
    EndDrawing();
}
