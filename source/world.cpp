#include "world.hpp"

#include "blocktype.hpp"
#include "raymath.h"

World::World(Camera *player) : music(loader.GetMusic("boss.mp3")), playerPos(&player->position)
{
    SetMusicVolume(music, 0.10f);
    PlayMusicStream(music);

    auto [x, y, z] = GetChunkPositionAt(*playerPos);
    minChunkPos = Vector3{x + static_cast<float>(-renderDistance)+1, y + static_cast<float>(-renderDistance)+1, z + static_cast<float>(-renderDistance)+1};
    maxChunkPos = Vector3{x + static_cast<float>(renderDistance), y + static_cast<float>(renderDistance), z + static_cast<float>(renderDistance)};

    for (int i = 0; i < renderDistance * 2; i++)
    {
        chunks.push_back(std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<Chunk>>>>>());
    }

    GenerateChunks();

    const Texture2D tex = loader.GetTexture2D("textures/blockmap.png");
    //GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    opaqueChunkMat = transparentChunkMat = LoadMaterialDefault();
    SetMaterialTexture(&opaqueChunkMat, MATERIAL_MAP_ALBEDO, tex);
    SetMaterialTexture(&transparentChunkMat, MATERIAL_MAP_ALBEDO, tex);

    opaqueChunkMat.shader = loader.GetShader("shaders/opaque.fs");
    transparentChunkMat.shader = loader.GetShader("shaders/opaque.fs");
}

World::~World() = default;

void World::Update()
{
    UpdateMusicStream(music);

    static Vector3 playerLastChunk = GetChunkPositionAt(*playerPos);
    if (const Vector3 playerCurrentChunk = GetChunkPositionAt(*playerPos); playerLastChunk != playerCurrentChunk)
    {
        std::cout << "Regenerating chunks" << std::endl;

        // Remove newly inactive chunks (right now just erasing all chunks)
        chunks.clear();

        // Insert new chunk groups
        for (int i = 0; i < renderDistance * 2; i++)
        {
            chunks.push_back(std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<Chunk>>>>>());
        }

        // Reset position values for generation
        playerLastChunk = playerCurrentChunk;
        minChunkPos = Vector3{playerLastChunk.x + static_cast<float>(-renderDistance)+1, playerLastChunk.y + static_cast<float>(-renderDistance)+1, playerLastChunk.z + static_cast<float>(-renderDistance)+1};
        maxChunkPos = Vector3{playerLastChunk.x + static_cast<float>(renderDistance), playerLastChunk.y + static_cast<float>(renderDistance), playerLastChunk.z + static_cast<float>(renderDistance)};

        // Generate new chunks
        GenerateChunks();
    }
}

void World::GenerateChunks()
{
    // Generate chunks
    std::vector<std::thread> chunkGenThreads;
    for (int x = minChunkPos.x; x <= maxChunkPos.x; x++)
    {
        auto chunkGenThread = std::thread([x, this]()
        {
            for (int y = minChunkPos.y; y <= maxChunkPos.y; y++)
            {
                auto row = std::make_shared<std::vector<std::shared_ptr<Chunk>>>();
                for (int z = minChunkPos.z; z <= maxChunkPos.z; z++)
                {
                    const Vector3 chunkPos {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
                    auto newChunk = GenerateChunk(chunkPos);
                    row->push_back(newChunk);
                }

                chunks[x-minChunkPos.x]->push_back(row);
            }
        });
        chunkGenThreads.push_back(std::move(chunkGenThread));
    }

    // Make sure all chunks have generated
    for (auto & thread : chunkGenThreads)
    {
        thread.join();
    }

    // Generate chunk meshes
    for (const auto &x : chunks)
    {
        for (const auto &y : *x)
        {
            for (const auto &z : *y)
            {
                z->GenerateChunkMesh();
            }
        }
    }
}

void World::RenderChunks() const
{
    // Sort chunks based on distance
    // TODO: Cache result and only update when player crosses chunk boundary
    std::vector<std::shared_ptr<Chunk>> sortedChunks;

    for (const auto &x : chunks)
    {
        for (const auto &y : *x)
        {
            for (const auto &z : *y)
            {
                if (z != nullptr)
                    sortedChunks.push_back(z);
            }
        }
    }

    Vector3 pos = *playerPos;
    std::ranges::sort(sortedChunks, [pos](std::shared_ptr<Chunk> a, std::shared_ptr<Chunk> b)
    {
        return Vector3DistanceSqr(pos, a->worldPosition) < Vector3DistanceSqr(pos, b->worldPosition);
    });

    // Render opaques front to back
    for (const auto &chunk : sortedChunks)
    {
        if (chunk->reuploadMeshFlag)
            chunk->UploadChunkMesh();
        if (chunk->validMesh)
        {
            auto [x, y, z] = chunk->worldPosition;
            DrawMesh(chunk->opaqueMesh, opaqueChunkMat, MatrixTranslate(x, y, z));
        }
    }

    // Render transparents back to front
    for (int i = sortedChunks.size() - 1; i >= 0; i--)
    {
        if (sortedChunks[i]->validMesh)
        {
            auto [x, y, z] = sortedChunks[i]->worldPosition;
            DrawMesh(sortedChunks[i]->transparentMesh, transparentChunkMat, MatrixTranslate(x, y, z));
        }
    }
}

unsigned char World::GetBlockAt(const int x, const int y, const int z) const
{
    const Vector3 chunkIndex = GetChunkPositionAt(Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});

    // If chunk doesn't exist, return air
    const std::shared_ptr chunk = GetChunkAt(chunkIndex);
    if (chunk == nullptr)
        return 0;

    const unsigned int blockIndexX = x % CHUNK_WIDTH;
    const unsigned int blockIndexY = y % CHUNK_HEIGHT;
    const unsigned int blockIndexZ = z % CHUNK_WIDTH;

    return chunk->data[blockIndexX][blockIndexY][blockIndexZ];
}

Vector3 World::GetChunkPositionAt(const Vector3 in)
{
    const float chunkIndexX = std::floor(in.x / static_cast<float>(CHUNK_WIDTH));
    const float chunkIndexY = std::floor(in.y / static_cast<float>(CHUNK_HEIGHT));
    const float chunkIndexZ = std::floor(in.z / static_cast<float>(CHUNK_WIDTH));

    return Vector3{chunkIndexX, chunkIndexY, chunkIndexZ};
}

std::shared_ptr<Chunk> World::GetChunkAt(Vector3 chunkPos) const
{
    chunkPos.x -= minChunkPos.x;
    chunkPos.y -= minChunkPos.y;
    chunkPos.z -= minChunkPos.z;

    auto [x, y, z] = maxChunkPos - minChunkPos;

    if (chunkPos.x < 0 || chunkPos.y < 0 || chunkPos.z < 0 ||
        chunkPos.x > x || chunkPos.y > y || chunkPos.z > z)
        return nullptr;

    return (*(*chunks[chunkPos.x])[chunkPos.y])[chunkPos.z];
}


bool World::IsBlockAtCoordsTransparent(const int x, const int y, const int z) const
{
    const unsigned char block = GetBlockAt(x, y, z);
    return BlockType::Types[block].isTransparent;
}

std::shared_ptr<Chunk> World::GenerateChunk(const Vector3 chunkPos)
{
    auto newChunk = std::make_shared<Chunk>(this, chunkPos);

    // First pass; depth and basic block placing
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                Vector3 chunkGlobalPos = newChunk->LocalToGlobalPos(Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
                const int noise = (perlin.octave2D_01((chunkGlobalPos.x * 0.0075), (chunkGlobalPos.z * 0.0075), 4) * 64) + 200;

                if (chunkGlobalPos.y > noise)
                {
                    newChunk->data[x][y][z] = 0;
                }
                else if (chunkGlobalPos.y > noise - 1)
                {
                    newChunk->data[x][y][z] = 1;
                }
                else if (chunkGlobalPos.y > noise - 5)
                {
                    newChunk->data[x][y][z] = 2;
                }
                else
                {
                    newChunk->data[x][y][z] = 4;
                }
            }
        }
    }

    // Second pass; cave generation
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                Vector3 chunkGlobalPos = newChunk->LocalToGlobalPos(Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
                const float noise = perlin.octave3D((chunkGlobalPos.x * 0.025), (chunkGlobalPos.y * 0.025), (chunkGlobalPos.z * 0.025), 4) * (1.85 - (0.005 * chunkGlobalPos.y));

                if (noise > 0.85f)
                    newChunk->data[x][y][z] = 0;
            }
        }
    }

    // Third pass; ore generation
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                if (newChunk->data[x][y][z] == 4)
                {
                    Vector3 chunkGlobalPos = newChunk->LocalToGlobalPos(Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});

                    // Dirt
                    float noise = perlin.noise3D((chunkGlobalPos.x * 0.075), (chunkGlobalPos.y * 0.075), (chunkGlobalPos.z * 0.075)) * (0.2 + (0.005 * chunkGlobalPos.y));
                    if (noise > 0.6f)
                        newChunk->data[x][y][z] = 2;

                    // Gravel
                    noise = perlin.noise3D((1000 + chunkGlobalPos.x * 0.075), (2500 + chunkGlobalPos.y * 0.075), (4215 + chunkGlobalPos.z * 0.075)) * (1.5 - (0.0025 * chunkGlobalPos.y));
                    if (noise > 0.65f)
                        newChunk->data[x][y][z] = 8;

                    // Isaac ore
                    noise = perlin.noise3D((3000 + chunkGlobalPos.x * 0.25), (-2000 + chunkGlobalPos.y * 0.25), (-5201 + chunkGlobalPos.z * 0.25)) * (1.5 - (0.0025 * chunkGlobalPos.y));
                    if (noise > 0.85f)
                        newChunk->data[x][y][z] = 11;

                    // Diamond ore
                    noise = perlin.noise3D((-150 + chunkGlobalPos.x * 0.25), (-8400 + chunkGlobalPos.y * 0.25), (-10000 + chunkGlobalPos.z * 0.25)) * (1.1 - (0.0025 * chunkGlobalPos.y));
                    if (noise > 0.725f)
                        newChunk->data[x][y][z] = 13;
                }
            }
        }
    }

    // Fourth pass; decals
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                Vector3 chunkGlobalPos = newChunk->LocalToGlobalPos(Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
                const int noise = (perlin.octave2D_01((chunkGlobalPos.x * 0.0075), (chunkGlobalPos.z * 0.0075), 4) * 64) + 200;
                if (static_cast<int>(chunkGlobalPos.y) - 1 == noise)
                {
                    int randomVal = GetRandomValue(0, 100);
                    if (randomVal < 15)
                        newChunk->data[x][y][z] = 3;
                    else if (randomVal < 17)
                        newChunk->data[x][y][z] = 10;
                }
            }
        }
    }

    return newChunk;
}
