#pragma once
#include "glm_includes.h"

class Noise
{
public:
    // fbm noise
    static float hybridMultiFractalInv(float x, float z, int seed, float H, float scale);
    static float hybridMultiFractal(float x, float z, int seed, float H, float scale);
    static glm::vec2 random2(int, int, int seed);
    static glm::vec2 random2(glm::vec2, int seed);

    // perlin noise
    static float falloff(glm::vec2, glm::vec2);
    static float perlin(glm::vec2, int);
    static float perlin(float, float, int);
    static float surflet(glm::vec2, glm::vec2, int);

    static int irandom1();
    static float random1(int);
};
