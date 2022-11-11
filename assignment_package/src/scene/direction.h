#pragma once

#include "glm_includes.h"

#include <array>
#include <vector>

using namespace std;
using namespace glm;

struct VertexInfo {
    vec4 pos;
    vec2 uv;

    VertexInfo(vec4 pos, vec2 uv) : pos(pos), uv(uv)
    {}
};


//based on https://stackoverflow.com/questions/1965249/how-to-write-a-java-enum-like-class-with-multiple-data-fields-in-c

class Direction {
  public:
    // Enum value DECLARATIONS - they are defined later
    // The six cardinal directions in 3D space
    static const Direction XPOS;
    static const Direction XNEG;
    static const Direction YPOS;
    static const Direction YNEG;
    static const Direction ZPOS;
    static const Direction ZNEG;

    static vector<Direction*> all;

  public:
    const int index;
    const string name;
    const Direction* opposite;
    const ivec3 vector;
    const array<VertexInfo, 4> vertices;

    static constexpr int length() {return 6;}
    constexpr operator int() const { return index; }

  private:
    Direction(int index, string name, const Direction* opposite, ivec3 vector, VertexInfo a, VertexInfo b, VertexInfo c, VertexInfo d) : index(index), name(name), opposite(opposite), vector(vector), vertices {a,b,c,d}
    {all.push_back(this);}

};


