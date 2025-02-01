#pragma once

#include <thread>
#include <mutex>
#include <iostream>
#include <queue>

#include "chunk.hpp"
#include "resourceloader.hpp"
#include "PerlinNoise.hpp"
#include "raymath.h"

class World {
    public:
        explicit World(Camera *player);
        ~World();

        void Update();

        void GenerateChunks();
        void RenderChunks() const;

        [[nodiscard]] unsigned char GetBlockAt(int x, int y, int z) const;
        [[nodiscard]] static Vector3 GetChunkPositionAt(Vector3 in);
        [[nodiscard]] std::shared_ptr<Chunk> GetChunkAt(Vector3 chunkPos) const;
        [[nodiscard]] bool IsBlockAtCoordsTransparent(int x, int y, int z) const;

    private:
        ResourceLoader loader;

        Music music;

        std::vector<std::shared_ptr<
            std::vector<std::shared_ptr<
                std::vector<std::shared_ptr<Chunk>>
            >>
        >> chunks;
        Vector3 minChunkPos{}, maxChunkPos{};

        std::vector<std::thread> threads;
        std::queue<std::function<>> taskQueue{};
        std::mutex taskQueueMutex{};
        void CheckThreads();

        Material opaqueChunkMat {};
        Material transparentChunkMat {};

        const siv::PerlinNoise::seed_type seed = GetRandomValue(0, 99999999);
	    const siv::PerlinNoise perlin{ seed };

        Vector3 *playerPos;
        const int renderDistance = 4;

	    std::shared_ptr<Chunk> GenerateChunk(Vector3 chunkPos);
};
