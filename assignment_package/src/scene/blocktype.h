#pragma once
#ifndef BLOCKTYPE_H
#define BLOCKTYPE_H


#include "glm_includes.h"

using namespace glm;

//based on https://stackoverflow.com/questions/1965249/how-to-write-a-java-enum-like-class-with-multiple-data-fields-in-c

class BlockType {
  public:
    // Enum value DECLARATIONS - they are defined later
    static const BlockType EMPTY;
    static const BlockType GRASS;
    static const BlockType DIRT;
    static const BlockType STONE;
    static const BlockType WATER;
    static const BlockType SNOW;

    BlockType() : index(0), name("undefined")
    {}

  private:
    int index;
    std::string name;
    bool opaque = true;
    vec3 color = vec3(1.f, 0.f, 1.f); //purple default color

  private:
    BlockType(int index, std::string name, bool opaque, vec3 color) : index(index), name(name), opaque(opaque), color(color)
    {}
//    BlockType(int index, bool opaque, vec3 color);

  public:

    static constexpr int length() {return 5;}
    constexpr operator int() const { return index; }

    bool isOpaque(){
        return opaque;
    }

    vec3 getColor(){
        return color;
    }

    std::string getName(){
        return name;
    }

//    bool isOpaque();
//    vec3 getColor();
};

//// Enum value DEFINITIONS
//// The initialization occurs in the scope of the class,
//// so the private BlockType constructor can be used.
//const BlockType BlockType::EMPTY = BlockType(0, false, vec3(1,1,1));
//const BlockType BlockType::GRASS = BlockType(1, true,  vec3(95.f, 159.f, 53.f) / 255.f);
//const BlockType BlockType::DIRT  = BlockType(2, true,  vec3(121.f, 85.f, 58.f) / 255.f);
//const BlockType BlockType::STONE = BlockType(3, true,  vec3(0.5f));
//const BlockType BlockType::WATER = BlockType(4, true,  vec3(0.f, 0.f, 0.75f));

#endif // BLOCKTYPE_H


