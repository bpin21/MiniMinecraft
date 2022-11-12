#include "blocktype.h"

//BlockType::BlockType(int index, bool opaque, vec3 color) : index(index), opaque(opaque), color(color)
//{}

//bool BlockType::isOpaque(){
//    return opaque;
//}

//vec3 BlockType::getColor(){
//    return color;
//}

// Enum value DEFINITIONS
// The initialization occurs in the scope of the class,
// so the private BlockType constructor can be used.
const BlockType BlockType::EMPTY = BlockType(0, "empty", false, vec3(1,1,1));
const BlockType BlockType::GRASS = BlockType(1, "grass", true,  vec3(95.f, 159.f, 53.f) / 255.f);
const BlockType BlockType::DIRT  = BlockType(2, "dirt", true,  vec3(121.f, 85.f, 58.f) / 255.f);
const BlockType BlockType::STONE = BlockType(3, "stone", true,  vec3(0.5f));
const BlockType BlockType::WATER = BlockType(4, "water", true,  vec3(0.f, 0.f, 0.75f));
const BlockType BlockType::SNOW  = BlockType(5, "snow", true, vec3(1,1,1));
