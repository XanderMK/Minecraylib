//
// Created by Xander Kleiber on 10/28/24.
//

#include "resourceloader.hpp"

ResourceLoader::ResourceLoader() = default;

ResourceLoader::~ResourceLoader()
{
    UnloadAssets();
}

void ResourceLoader::UnloadAssets()
{
    // Clear images
    for (const auto& p : this->Images | std::views::values)
    {
        UnloadImage(p);
    }
    this->Images.clear();

    // Clear textures
    for (const auto& p : this->Textures2D | std::views::values)
    {
        UnloadTexture(p);
    }
    this->Textures2D.clear();

    // Clear models
    for (const auto& p : this->Models | std::views::values)
    {
        UnloadModel(p);
    }
    this->Models.clear();

    // Clear shaders
    for (const auto& p : this->Shaders | std::views::values)
    {
        UnloadShader(p);
    }

    // Clear sounds
    for (const auto& p : this->Sounds | std::views::values)
    {
        UnloadSound(p);
    }
    this->Sounds.clear();

    // Clear music
    for (const auto& p : this->Musics | std::views::values)
    {
        UnloadMusicStream(p);
    }
    this->Musics.clear();
}


Image ResourceLoader::GetImage(const std::string& path)
{
    if (!this->Images.contains(path)) // No image, load it
    {
        this->Images.insert(std::pair(
            path,
            LoadImage((ASSETS_PATH + path).c_str())
        ));
    }
    return this->Images[path];
}

Texture2D ResourceLoader::GetTexture2D(const std::string& path)
{
    if (!this->Textures2D.contains(path)) // No texture, load it
    {
        this->Textures2D.insert(std::pair(
            path,
            LoadTexture((ASSETS_PATH + path).c_str())
        ));
    }
    return this->Textures2D[path];
}

Model ResourceLoader::GetModel(const std::string& path)
{
    if (!this->Models.contains(path)) // No texture, load it
    {
        this->Models.insert(std::pair(
            path,
            LoadModel((ASSETS_PATH + path).c_str())
        ));
    }
    return this->Models[path];
}

Shader ResourceLoader::GetShader(const std::string& path)
{
    if (!this->Shaders.contains(path))
    {
        this->Shaders.insert(std::pair(
            path,
            LoadShader(nullptr, (ASSETS_PATH + path).c_str())
        ));
    }
    return this->Shaders[path];
}

Sound ResourceLoader::GetSound(const std::string& path)
{
    if (!this->Sounds.contains(path)) // No sound, load it
    {
        this->Sounds.insert(std::pair<std::string, Sound>(
            path,
            LoadSound((ASSETS_PATH + path).c_str())
        ));
    }
    return this->Sounds[path];
}

Music ResourceLoader::GetMusic(const std::string& path)
{
    if (!this->Musics.contains(path)) // No music, load it
    {
        this->Musics.insert(std::pair<std::string, Music>(
            path,
            LoadMusicStream((ASSETS_PATH + path).c_str())
        ));
    }
    return this->Musics[path];
}