#pragma once

#include <vector>
#include <string>

#include "blockmodel.hpp"
#include "raymath.h"

namespace BlockType
{
    struct Type
    {
        std::string name;
        BlockModel::Model model;
        std::vector<unsigned int> textureIndices; // Texture indices per face in model
        bool isTransparent = false;
    };

    static const std::vector<Type> Types
    {
        {{"Air"}, {BlockModel::None}, {}, true},
        {{"Grass Block"}, {BlockModel::FullBlock}, {1, 1, 1, 1, 0, 2}},
        {{"Dirt Block"}, {BlockModel::FullBlock}, {2, 2, 2, 2, 2, 2}},
        {{"Short Grass"}, {BlockModel::Decal}, {3, 3, 3, 3}, true},
        {{"Stone Block"}, {BlockModel::FullBlock}, {4, 4, 4, 4, 4, 4}},
        {{"Sand Block"}, {BlockModel::FullBlock}, {5, 5, 5, 5, 5, 5}},
        {{"Glass Block"}, {BlockModel::FullBlock}, {6, 6, 6, 6, 6, 6}, true},
        {{"Sugar Cane"}, {BlockModel::Decal}, {7, 7, 7, 7}, true},
        {{"Gravel Block"}, {BlockModel::FullBlock}, {8, 8, 8, 8, 8, 8}},
        {{"Wood Log Block"}, {BlockModel::FullBlock}, {9, 9, 9, 9, 10, 10}},
        {{"Flower"}, {BlockModel::Decal}, {11, 11, 11, 11}, true},
        {{"Isaac Block"}, {BlockModel::FullBlock}, {12, 12, 12, 12, 12, 12}},
        {{"Bedrock Block"}, {BlockModel::FullBlock}, {13, 13, 13, 13, 13, 13}},
        {{"Diamond Ore Block"}, {BlockModel::FullBlock}, {14, 14, 14, 14, 14, 14}},
    };

    static constexpr unsigned int blockmapWidth = 4, blockmapHeight = 4;

    static Vector2 TransformTexcoordsToBlockmap(Vector2 texcoords, unsigned int faceIndex, unsigned int blockType)
    {
        // Get position of texture based on index
        unsigned int index = BlockType::Types[blockType].textureIndices[faceIndex];

        // Normalize texcoords to size of blockmap
        texcoords.x /= blockmapWidth;
        texcoords.y /= blockmapHeight;

        // Find texture at index
        texcoords.x += (index % blockmapWidth) / static_cast<float>(blockmapWidth);
        texcoords.y += (index / blockmapWidth) / static_cast<float>(blockmapHeight);

        // Just in case, clamp values between 0 and 1
        return Vector2Clamp(texcoords, Vector2{0, 0}, Vector2{1, 1});
    }
}


