#include "direction.h"

#define blockUV 0.03125f

// Enum value DEFINITIONS
// The initialization occurs in the scope of the class,
// so the private BlockType constructor can be used.

vector<Direction*> Direction::all;

const Direction Direction::XPOS = Direction(0, "x pos", &Direction::XNEG, ivec3(1,0,0),
                                            VertexInfo(vec4(1,0,1,1), vec2(0,0)),
                                            VertexInfo(vec4(1,0,0,1), vec2(blockUV,0)),
                                            VertexInfo(vec4(1,1,0,1), vec2(blockUV,blockUV)),
                                            VertexInfo(vec4(1,1,1,1), vec2(0,blockUV)));
const Direction Direction::XNEG = Direction(1, "x neg", &Direction::XPOS, ivec3(-1,0,0),
                                            VertexInfo(vec4(0,0,0,1), vec2(0,0)),
                                            VertexInfo(vec4(0,0,1,1), vec2(blockUV,0)),
                                            VertexInfo(vec4(0,1,1,1), vec2(blockUV,blockUV)),
                                            VertexInfo(vec4(0,1,0,1), vec2(0,blockUV)));
const Direction Direction::YPOS = Direction(2, "y pos", &Direction::YNEG, ivec3(0,1,0),
                                            VertexInfo(vec4(0,1,1,1), vec2(0,0)),
                                            VertexInfo(vec4(1,1,1,1), vec2(blockUV,0)),
                                            VertexInfo(vec4(1,1,0,1), vec2(blockUV,blockUV)),
                                            VertexInfo(vec4(0,1,0,1), vec2(0,blockUV)));
const Direction Direction::YNEG = Direction(3, "y neg", &Direction::YPOS, ivec3(0,-1,0),
                                            VertexInfo(vec4(0,0,0,1), vec2(0,0)),
                                            VertexInfo(vec4(1,0,0,1), vec2(blockUV,0)),
                                            VertexInfo(vec4(1,0,1,1), vec2(blockUV,blockUV)),
                                            VertexInfo(vec4(0,0,1,1), vec2(0,blockUV)));
const Direction Direction::ZPOS = Direction(4, "z pos", &Direction::ZNEG, ivec3(0,0,1),
                                            VertexInfo(vec4(0,0,1,1), vec2(0,0)),
                                            VertexInfo(vec4(1,0,1,1), vec2(blockUV,0)),
                                            VertexInfo(vec4(1,1,1,1), vec2(blockUV,blockUV)),
                                            VertexInfo(vec4(0,1,1,1), vec2(0,blockUV)));
const Direction Direction::ZNEG = Direction(5, "z neg", &Direction::ZPOS, ivec3(0,0,-1),
                                            VertexInfo(vec4(1,0,0,1), vec2(0,0)),
                                            VertexInfo(vec4(0,0,0,1), vec2(blockUV,0)),
                                            VertexInfo(vec4(0,1,0,1), vec2(blockUV,blockUV)),
                                            VertexInfo(vec4(1,1,0,1), vec2(0,blockUV)));
