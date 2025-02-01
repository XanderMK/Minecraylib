#pragma once

#include <array>
#include <sstream>

#include "blockmodel.hpp"
#include "global.hpp"

class World;

class Chunk {
    public:
        Chunk(World* world, Vector3 pos);
        ~Chunk();

        void GenerateChunkMesh();
        void UploadChunkMesh();
        [[nodiscard]] Vector3 LocalToGlobalPos(Vector3 in) const;

        Vector3 position, worldPosition;
        std::array<std::array<std::array<int, CHUNK_WIDTH>, CHUNK_HEIGHT>, CHUNK_WIDTH> data {};
        Mesh opaqueMesh {};
        Mesh transparentMesh {};

        bool validMesh = false;
        bool reuploadMeshFlag = false;
    private:
        World* world;

        void AssembleMeshPieceFromBlockModel(int x, int y, int z, unsigned int blockType,
                                             int& opaqueTriangleCount, int& maxOpaqueTriangleCount, int& opaqueVertexCount, float* &opaqueVertices, float* &opaqueNormals, float* &opaqueTexcoords,
                                             int& transparentTriangleCount, int& maxTransparentTriangleCount, int& transparentVertexCount, float* &transparentVertices, float* &transparentNormals, float* &transparentTexcoords) const;
};
