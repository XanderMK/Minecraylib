#pragma once

#include <map>

#include "raylib.h"
#include <ranges>
#include <string>

class ResourceLoader
{
public:
    ResourceLoader();
    ~ResourceLoader();

    void UnloadAssets();

    Image GetImage(const std::string& path);
    Texture2D GetTexture2D(const std::string& path);
    Model GetModel(const std::string& path);
    Shader GetShader(const std::string& path);
    Sound GetSound(const std::string& path);
    Music GetMusic(const std::string& path);
private:
    std::map<std::string, Image> Images;
    std::map<std::string, Texture2D> Textures2D;
    std::map<std::string, Model> Models;
    std::map<std::string, Shader> Shaders;
    std::map<std::string, Sound> Sounds;
    std::map<std::string, Music> Musics;
};