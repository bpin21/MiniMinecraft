#include "chunk.h"

#include <iostream>

using namespace std;
using namespace glm;

Chunk::Chunk(OpenGLContext* mp_context) : Drawable(mp_context), m_blocks(), m_neighbors{{Direction::XPOS, nullptr}, {Direction::XNEG, nullptr}, {Direction::ZPOS, nullptr}, {Direction::ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, BlockType::EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    if (y > 255){
        return BlockType::EMPTY;
    }

    if (x > 15) {
        if (this->m_neighbors.at(Direction::XPOS) == nullptr){
            return BlockType::EMPTY;
        }

        return this->m_neighbors.at(Direction::XPOS)->getBlockAt(x - 16, y, z);
    }

    if (z > 15){
        if (this->m_neighbors.at(Direction::ZPOS) == nullptr){
            return BlockType::EMPTY;
        }

        return this->m_neighbors.at(Direction::ZPOS)->getBlockAt(x, y, z - 16);
    }

    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    if (y < 0){
        return BlockType::EMPTY;
    }

    if (x < 0) {
        if (this->m_neighbors.at(Direction::XNEG) == nullptr){
            return BlockType::EMPTY;
        }

        return this->m_neighbors.at(Direction::XNEG)->getBlockAt(x + 16, y, z);
    }

    if (z < 0){
        if (this->m_neighbors.at(Direction::ZNEG) == nullptr){
            return BlockType::EMPTY;
        }

        return this->m_neighbors.at(Direction::ZNEG)->getBlockAt(x, y, z + 16);
    }

    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Use to get block with an ivec
BlockType Chunk::getBlockAt(ivec3 pos) const {
    return getBlockAt(pos.x, pos.y, pos.z);
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

//const static Direction all_directions[] = { XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG };

//const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
//    {XPOS, XNEG},
//    {XNEG, XPOS},
//    {YPOS, YNEG},
//    {YNEG, YPOS},
//    {ZPOS, ZNEG},
//    {ZNEG, ZPOS}
//};

//const static std::unordered_map<Direction, ivec3, EnumHash> directionVec {
//    {XPOS, ivec3(1,  0,  0)},
//    {XNEG, ivec3(-1, 0,  0)},
//    {YPOS, ivec3(0,  1,  0)},
//    {YNEG, ivec3(0,  -1, 0)},
//    {ZPOS, ivec3(0,  0,  1)},
//    {ZNEG, ivec3(0,  0,  -1)}
//};


void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[*dir.opposite] = this;
    }
}

void addToVector(vector<float> &vec, vec2 v){
    vec.push_back(v.x);
    vec.push_back(v.y);
}

void addToVector(vector<float> &vec, vec3 v){
    vec.push_back(v.x);
    vec.push_back(v.y);
    vec.push_back(v.z);
}

void addToVector(vector<float> &vec, vec4 v){
    vec.push_back(v.x);
    vec.push_back(v.y);
    vec.push_back(v.z);
    vec.push_back(v.w);
}

void addToVector(vector<float> &vec, ivec3 v){
    vec.push_back(v.x);
    vec.push_back(v.y);
    vec.push_back(v.z);
}


void Chunk::createVBOdata()
{
    vector<float> buffer;
    vector<GLuint> idx;

    int elem_count = 0;

    for (int x = 0; x < 16; x++){
        for (int y = 0; y < 256; y++){
            for (int z = 0; z < 16; z++){
                ivec3 pos(x,y,z);
                BlockType b = getBlockAt(pos);
                if(b.isOpaque()){
                    for (auto d : Direction::all){
                        if(!getBlockAt(pos + d->vector).isOpaque()){
                            //generate idx
                            int initial = elem_count;

                            int count = 0;
                            for (auto v : d->vertices) {
                                addToVector(buffer, v.pos + vec4(pos, 0));
                                addToVector(buffer, vec4(d->vector, 1));
                                addToVector(buffer, vec4(b.getColor(),0));
                                count++;
                                elem_count++;
                            }


                            for (int i = initial; i < initial + count - 2; i++){
                                idx.push_back(initial);
                                idx.push_back(i + 1);
                                idx.push_back(i + 2);
                            }
                        }
                    }
                }
            }
        }
    }

    this->m_count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateInterleaved();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleaved);
    mp_context->glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

}
