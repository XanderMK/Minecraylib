//
// Created by Xander Kleiber on 11/4/24.
//

#include "chunk.hpp"

#include <iostream>

#include "world.hpp"
#include "blocktype.hpp"

Chunk::Chunk(World* world, const Vector3 pos) :
    position(pos), world(world)
{
    this->worldPosition = Vector3{pos.x * CHUNK_WIDTH, pos.y * CHUNK_HEIGHT, pos.z * CHUNK_WIDTH};
}

Chunk::~Chunk()
{
    validMesh = false;
    reuploadMeshFlag = false;
    UnloadMesh(opaqueMesh);
    UnloadMesh(transparentMesh);
}

void Chunk::GenerateChunkMesh()
{
    validMesh = false;

    // Allocate initial memory
    int opaqueTriangleCount = 0, maxOpaqueTriangleCount = 10000, opaqueVertexCount = 0;
    auto opaqueVertices = static_cast<float*>(MemAlloc(maxOpaqueTriangleCount * 3 * 3 * sizeof(float)));
    auto opaqueNormals = static_cast<float*>(MemAlloc(maxOpaqueTriangleCount * 3 * 3 * sizeof(float)));
    auto opaqueTexcoords = static_cast<float*>(MemAlloc(maxOpaqueTriangleCount * 3 * 2 * sizeof(float)));

    int transparentTriangleCount = 0, maxTransparentTriangleCount = 2000, transparentVertexCount = 0;
    auto transparentVertices = static_cast<float*>(MemAlloc(maxTransparentTriangleCount * 3 * 3 * sizeof(float)));
    auto transparentNormals = static_cast<float*>(MemAlloc(maxTransparentTriangleCount * 3 * 3 * sizeof(float)));
    auto transparentTexcoords = static_cast<float*>(MemAlloc(maxTransparentTriangleCount * 3 * 2 * sizeof(float)));

    // Iterate through every block and calculate opaqueMesh
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                AssembleMeshPieceFromBlockModel(x, y, z, this->data[x][y][z],
                                                opaqueTriangleCount, maxOpaqueTriangleCount, opaqueVertexCount, opaqueVertices, opaqueNormals, opaqueTexcoords,
                                                transparentTriangleCount, maxTransparentTriangleCount, transparentVertexCount, transparentVertices, transparentNormals, transparentTexcoords);
            }
        }
    }

    // Push opaque data to opaqueMesh
    Mesh opaqueResult{};
    opaqueResult.triangleCount = opaqueTriangleCount;
    opaqueResult.vertexCount = opaqueVertexCount;
    opaqueResult.vertices = opaqueVertices;
    opaqueResult.normals = opaqueNormals;
    opaqueResult.texcoords = opaqueTexcoords;
    this->opaqueMesh = opaqueResult;

    // Push transparent data to transparentMesh
    Mesh transparentResult{};
    transparentResult.triangleCount = transparentTriangleCount;
    transparentResult.vertexCount = transparentVertexCount;
    transparentResult.vertices = transparentVertices;
    transparentResult.normals = transparentNormals;
    transparentResult.texcoords = transparentTexcoords;
    this->transparentMesh = transparentResult;

    reuploadMeshFlag = true;
}

void Chunk::UploadChunkMesh()
{
    UploadMesh(&this->opaqueMesh, false);
    UploadMesh(&this->transparentMesh, false);

    validMesh = true;
}


void Chunk::AssembleMeshPieceFromBlockModel(const int x, const int y, const int z, const unsigned int blockType,
                                            int& opaqueTriangleCount, int& maxOpaqueTriangleCount, int& opaqueVertexCount, float* &opaqueVertices, float* &opaqueNormals, float* &opaqueTexcoords,
                                            int& transparentTriangleCount, int& maxTransparentTriangleCount, int& transparentVertexCount, float* &transparentVertices, float* &transparentNormals, float* &transparentTexcoords) const
{
    if (BlockType::Types[blockType].isTransparent)
    {
        // Reallocate memory if opaqueMesh is larger than buffer
        while (transparentTriangleCount + BlockType::Types[blockType].model.triangleCount > maxTransparentTriangleCount)
        {
            maxTransparentTriangleCount *= 2;
            transparentVertices = static_cast<float*>(MemRealloc(transparentVertices, maxTransparentTriangleCount * 3 * 3 * sizeof(float)));
            transparentNormals = static_cast<float*>(MemRealloc(transparentNormals, maxTransparentTriangleCount * 3 * 3 * sizeof(float)));
            transparentTexcoords = static_cast<float*>(MemRealloc(transparentTexcoords, maxTransparentTriangleCount * 3 * 2 * sizeof(float)));

            if (transparentVertices == nullptr || transparentNormals == nullptr || transparentTexcoords == nullptr)
            {
                std::cout << "Failed to reallocate memory!!!!!!!!!!!!" << std::endl;
                return;
            }
        }

        const unsigned int faceCount = BlockType::Types[blockType].model.faces.size();
        for (int i = 0; i < faceCount; i++)
        {
            // Check for occlusion
            auto [globalX, globalY, globalZ] = LocalToGlobalPos(Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            const int worldCoordX = static_cast<int>(globalX);
            const int worldCoordY = static_cast<int>(globalY);
            const int worldCoordZ = static_cast<int>(globalZ);
            if
            (
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Right)       && !world->IsBlockAtCoordsTransparent(worldCoordX+1, worldCoordY, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Left)        && !world->IsBlockAtCoordsTransparent(worldCoordX-1, worldCoordY, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Up)          && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY+1, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Down)        && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY-1, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Forward)     && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY, worldCoordZ+1)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Backward)    && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY, worldCoordZ-1))
            )
            {
                continue;
            }

            for (auto & vertex : BlockType::Types[blockType].model.faces[i].vertices)
            {
                transparentVertices[transparentVertexCount*3] = vertex[0] + x;
                transparentVertices[transparentVertexCount*3 + 1] = vertex[1] + y;
                transparentVertices[transparentVertexCount*3 + 2] = vertex[2] + z;

                transparentNormals[transparentVertexCount*3] = vertex[3];
                transparentNormals[transparentVertexCount*3 + 1] = vertex[4];
                transparentNormals[transparentVertexCount*3 + 2] = vertex[5];

                const auto [texX, texY] = BlockType::TransformTexcoordsToBlockmap(Vector2{vertex[6], vertex[7]}, i, blockType);
                transparentTexcoords[transparentVertexCount*2] = texX;
                transparentTexcoords[transparentVertexCount*2 + 1] = texY;

                transparentVertexCount++;
            }
            transparentTriangleCount += BlockType::Types[blockType].model.faces[i].triangleCount;
        }
    }
    else
    {
        // Reallocate memory if opaqueMesh is larger than buffer
        while (opaqueTriangleCount + BlockType::Types[blockType].model.triangleCount > maxOpaqueTriangleCount)
        {
            maxOpaqueTriangleCount *= 2;
            opaqueVertices = static_cast<float*>(MemRealloc(opaqueVertices, maxOpaqueTriangleCount * 3 * 3 * sizeof(float)));
            opaqueNormals = static_cast<float*>(MemRealloc(opaqueNormals, maxOpaqueTriangleCount * 3 * 3 * sizeof(float)));
            opaqueTexcoords = static_cast<float*>(MemRealloc(opaqueTexcoords, maxOpaqueTriangleCount * 3 * 2 * sizeof(float)));

            if (opaqueVertices == nullptr || opaqueNormals == nullptr || opaqueTexcoords == nullptr)
            {
                std::cout << "Failed to reallocate memory!!!!!!!!!!!!" << std::endl;
                return;
            }
        }

        const unsigned int faceCount = BlockType::Types[blockType].model.faces.size();
        for (int i = 0; i < faceCount; i++)
        {
            // Check for occlusion
            auto [globalX, globalY, globalZ] = LocalToGlobalPos(Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            const int worldCoordX = static_cast<int>(globalX);
            const int worldCoordY = static_cast<int>(globalY);
            const int worldCoordZ = static_cast<int>(globalZ);
            if
            (
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Right)       && !world->IsBlockAtCoordsTransparent(worldCoordX+1, worldCoordY, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Left)        && !world->IsBlockAtCoordsTransparent(worldCoordX-1, worldCoordY, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Up)          && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY+1, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Down)        && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY-1, worldCoordZ)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Forward)     && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY, worldCoordZ+1)) ||
                (BlockType::Types[blockType].model.faces[i].occlusionNeighbors & static_cast<int>(BlockModel::Direction::Backward)    && !world->IsBlockAtCoordsTransparent(worldCoordX, worldCoordY, worldCoordZ-1))
            )
            {
                continue;
            }

            for (auto & vertex : BlockType::Types[blockType].model.faces[i].vertices)
            {
                opaqueVertices[opaqueVertexCount*3] = vertex[0] + x;
                opaqueVertices[opaqueVertexCount*3 + 1] = vertex[1] + y;
                opaqueVertices[opaqueVertexCount*3 + 2] = vertex[2] + z;

                opaqueNormals[opaqueVertexCount*3] = vertex[3];
                opaqueNormals[opaqueVertexCount*3 + 1] = vertex[4];
                opaqueNormals[opaqueVertexCount*3 + 2] = vertex[5];

                const auto [texX, texY] = BlockType::TransformTexcoordsToBlockmap(Vector2{vertex[6], vertex[7]}, i, blockType);
                opaqueTexcoords[opaqueVertexCount*2] = texX;
                opaqueTexcoords[opaqueVertexCount*2 + 1] = texY;

                opaqueVertexCount++;
            }
            opaqueTriangleCount += BlockType::Types[blockType].model.faces[i].triangleCount;
        }
    }
}

Vector3 Chunk::LocalToGlobalPos(Vector3 in) const
{
    in.x += (this->position.x * CHUNK_WIDTH);
    in.y += (this->position.y * CHUNK_HEIGHT);
    in.z += (this->position.z * CHUNK_WIDTH);

    return in;
}