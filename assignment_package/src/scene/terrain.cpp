#include "terrain.h"
#include "cube.h"
#include "noise.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_seed(Noise::irandom1()), mp_context(context)
{}

Terrain::~Terrain() {
    //TODO should we be destroying VBO data for each chunk
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return BlockType::EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}

ivec2 Terrain::getTerrainCornerAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 64.f));
    int zFloor = static_cast<int>(glm::floor(z / 64.f));
    return ivec2(64 * xFloor, 64 * zFloor);
}



const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, Direction::ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, Direction::ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, Direction::XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, Direction::XNEG);
    }
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {

    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {

            Chunk* c = getChunkAt(x, z).get();

            //c->createVBOdata();

            shaderProgram->setModelMatrix(translate(mat4(), vec3(x,0,z)));
            shaderProgram->drawInterleaved(*c);
        }
    }


}

void Terrain::generateTerrain(int x_start, int z_start){

    if(m_generatedTerrain.count(toKey(x_start, z_start)) > 0) {
        return;
    }

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = x_start; x < x_start + 64; x += 16) {
        for(int z = z_start; z < z_start + 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }

    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(x_start, z_start));

    // generate terrain
    for (int x = x_start; x < x_start + 64; x++)
    {
        for (int z = z_start; z < z_start + 64; z++)
        {
            setColumnAt(x, z);
        }
    }

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = x_start; x < x_start + 64; x += 16) {
        for(int z = z_start; z < z_start + 64; z += 16) {
            Chunk* c = getChunkAt(x, z).get();
            c->createVBOdata();
        }
    }

}


void Terrain::CreateTestScene()
{
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));


    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            if((x + z) % 2 == 0) {
                setBlockAt(x, 128, z, BlockType::STONE);
            }
            else {
                setBlockAt(x, 128, z, BlockType::DIRT);
            }
        }
    }


    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x) {
        setBlockAt(x, 129, 0, BlockType::GRASS);
        setBlockAt(x, 130, 0, BlockType::GRASS);
        setBlockAt(x, 129, 63, BlockType::GRASS);
        setBlockAt(0, 130, x, BlockType::GRASS);
    }

    // Add a central column
    for(int y = 129; y < 140; ++y) {
        setBlockAt(32, y, 32, BlockType::GRASS);
    }

    setBlockAt(32, 139, 32, BlockType::STONE);

    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {

            Chunk* c = getChunkAt(x, z).get();

            c->createVBOdata();

        }
    }


}


// get the height of the given x-z coords - GRASSLAND
int Terrain::heightMapGrassland(int x, int z)
{
    // use perturbed perlin noise to create grasslands
    float min = 1.4;
    float max = 2.6;
    float h = Noise::hybridMultiFractalInv(x, z, m_seed, .6, 333);
    //std::cout << h << std::endl;;
    h = glm::clamp(h, min, max);
    return 66 * glm::smoothstep(min, max, h) + 111;
}

// get the height of the given x-z coords - MOUNTAIN
int Terrain::heightMapMountains(int x, int z)
{
    // use perlin noise based FBM to create mountains
    float min = .9;
    float max = 1.7;
    float h = Noise::hybridMultiFractal(x, z, m_seed, .5, 200);
    h = glm::clamp(h, min, max);
    return 80 * glm::smoothstep(min, max, h) + 100;
}

// get the "height" of the biome map
float Terrain::heightMapBiome(int x, int z)
{
    // use perlin-based noise map to LERP b/w biomes
    float min = 1.5;
    float max = 1.8;
    float biomeScale = 2345;
    float biome = Noise::hybridMultiFractal(x, z, m_seed, .9, biomeScale);
    //std::cout << biome;// << std::endl;
    biome = glm::clamp(biome, min, max);
    biome = glm::smoothstep(min, max, biome);
    //std::cout << ", " << biome << std::endl;
    return biome;
}

// populate all terrain for given x-z cooreds (y column)
void Terrain::setColumnAt(int x, int z)
{
    float biome = heightMapBiome(x, z);
    // get the heights of each biome
    int heightGrassland = heightMapGrassland(x, z);
    int heightMountains = heightMapMountains(x, z);

    // LERP between each biome's height map
    int h = heightGrassland * (1 - biome) + heightMountains * biome;

    // call biome specific column function based on larger value
    if (biome > .5)
    {
        setColumnMountains(x, z, h);
    } else {
        setColumnGrassland(x, z, h);
    }
}

// (helper) populate all terrain for given x-z coords (y column) : GRASSLAND BIOME
void Terrain::setColumnGrassland(int x, int z, int h)
{
    int currentBlock = h;
    // if height is lower than 138 : water
    if (currentBlock < 138)
    {
        for (int y = 138; y > currentBlock; --y)
        {
            setBlockAt(x, y, z, BlockType::WATER);
        }
    }
    // top layer : grass
    if (currentBlock > 128)
    {
        setBlockAt(x, currentBlock, z, BlockType::GRASS);
        currentBlock--;
    }
    // above 128 : dirt
    for (; currentBlock >= 128; --currentBlock)
    {
        setBlockAt(x, currentBlock, z, BlockType::DIRT);
    }
    // below 128 : stone
    for (; currentBlock >= 0; --currentBlock)
    {
        setBlockAt(x, currentBlock, z, BlockType::STONE);
    }

}

// (helper) populate all terrain for given x-z coords (y column) : MOUNTAIN BIOME
void Terrain::setColumnMountains(int x, int z, int h)
{
    int currentBlock = h;
    // if height is lower than 138 : water

    if (currentBlock < 138)
    {
        for (int y = 138; y > currentBlock; --y)
        {
            setBlockAt(x, y, z, BlockType::WATER);
        }
    }

    // top layer (above 200) : snow
    if (currentBlock > 200)
    {
        setBlockAt(x, currentBlock, z, BlockType::SNOW);
        currentBlock--;
    }
    // below 200 : stone
    for (; currentBlock >= 0; --currentBlock)
    {
        setBlockAt(x, currentBlock, z, BlockType::STONE);
    }
}
