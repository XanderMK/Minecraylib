#pragma once

#include <world.hpp>

#include "raylib.h"

class Core
{
    public:
        Core();
        ~Core();

        void Update(float deltaTime);
        void Render() const;
    private:
        Camera3D camera;

        World world;
};