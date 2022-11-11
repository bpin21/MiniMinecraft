#include "noise.h"
#include <vector>
#include <iostream>
#include <random>
// hyrbid FBM using (1 - abs(Perlin)) as base
float Noise::hybridMultiFractalInv(float x, float z, int seed, float H, float scale)
{
    // H = fractal increment : 1 = smooth, 0 = rough/noise
    // scale = perlin noise scale factor (200-500)
    float lacunarity = 10; // relative distance between frequencies
    float octaves = 8; // number of frequenies used (6-10)
    float offset = .7;
    // basis() = perlin noise or other noise function
    float frequency, result, signal, weight, remainder, pX, pZ;
    std::vector<float> exp = std::vector<float>();
    frequency = 1;
    for (int i = 0; i < octaves; i++)
    {
        exp.push_back(pow(frequency, -H));
        frequency *= lacunarity;
    }
    // calculate first octave
    pX = x / scale * exp.at(0) + seed;
    pZ = z / scale * exp.at(0) + seed;
    result = (1 - glm::abs(perlin(pX, pZ, seed)) + offset) * exp.at(0);
    weight = result;
    // increase frequency
    x *= lacunarity;
    z *= lacunarity;
    // spectral contruction
    for (int i = 1; i < octaves; i++)
    {
        // prevent divergence
        if (weight > 1) { weight = 1; }
        // calculate next frequency and add to result
        pX = x / scale * exp.at(i) + seed;
        pZ = z / scale * exp.at(i) + seed;
        signal = (1 - glm::abs(perlin(pX, pZ, seed)) + offset) * exp.at(i);
        result += weight * signal;
        // update weight and frequency for next iteration
        weight *= signal;
        x *= lacunarity;
        z *= lacunarity;
    }
    /*
    // deal with remainder
    remainder = octaves - (int) octaves;
    if (remainder != 0)
    {
        result += remainder * glm::abs(perlin(x/scale, z/scale, seed)) * exp.at(octaves - 1);
    } */
    //std::cout << result << std::endl;
    return result;
}

// hyrbid FBM using Perlin as base
float Noise::hybridMultiFractal(float x, float z, int seed, float H, float scale)
{
    // H = fractal increment : 1 = smooth, 0 = rough/noise
    // scale = perlin noise scale factor (200-500)
    float lacunarity = 10; // relative distance between frequencies
    float octaves = 8; // number of frequenies used (6-10)
    float offset = .7;
    // basis() = perlin noise or other noise function
    float frequency, result, signal, weight, remainder, pX, pZ;
    std::vector<float> exp = std::vector<float>();
    frequency = 1;
    for (int i = 0; i < octaves; i++)
    {
        exp.push_back(pow(frequency, -H));
        frequency *= lacunarity;
    }
    // calculate first octave
    pX = x / scale * exp.at(0) + seed;
    pZ = z / scale * exp.at(0) + seed;
    result = (1 - glm::abs(perlin(pX, pZ, seed)) + offset) * exp.at(0);
    weight = result;
    // increase frequency
    x *= lacunarity;
    z *= lacunarity;
    // spectral contruction
    for (int i = 1; i < octaves; i++)
    {
        // prevent divergence
        if (weight > 1) { weight = 1; }
        // calculate next frequency and add to result
        pX = x / scale * exp.at(i) + seed;
        pZ = z / scale * exp.at(i) + seed;
        signal = (1 - glm::abs(perlin(pX, pZ, seed)) + offset) * exp.at(i);
        result += weight * signal;
        // update weight and frequency for next iteration
        weight *= signal;
        x *= lacunarity;
        z *= lacunarity;
    }
    /*
    // deal with remainder
    remainder = octaves - (int) octaves;
    if (remainder != 0)
    {
        result += remainder * glm::abs(perlin(x/scale, z/scale, seed)) * exp.at(octaves - 1);
    } */
    //std::cout << result << std::endl;
    return result;
}

// weighted average (quintic) falloff for perlin
float Noise::falloff(glm::vec2 P, glm::vec2 gridP)
{
    // compute falloff
    /*
    float dX = glm::abs(P[0] - gridP[0]);
    float dY = glm::abs(P[1] - gridP[1]);
    float tX = 1.f - 6.f * glm::pow(dX, 5.f) + 15.f * glm::pow(dX, 4.f) - 10.f * glm::pow(dX, 3.f);
    float tY = 1.f - 6.f * glm::pow(dY, 5.f) + 15.f * glm::pow(dY, 4.f) - 10.f * glm::pow(dY, 3.f);
    return tX * tY; */
    // linear
    float t = glm::distance(P, gridP);
    // quintic
    t = t * t * t * (t * (t * 6.f - 15.f) + 10.f);
    // inversely weighted by distance (higher when closer)
    return 1 - t;
}

// 2D perlin noise
float Noise::perlin(glm::vec2 uv, int seed)
{
    float surfSum = 0;
    // calculate surflet for each corner
    for (int i = 0; i <= 1; ++i)
    {
        for (int j = 0; j <= 1; ++j)
        {
            surfSum += surflet(uv, glm::floor(uv) + glm::vec2(i, j), seed);
        }
    }

    return surfSum;
}
float Noise::perlin(float x, float z, int seed)
{
    return perlin(glm::vec2(x, z), seed);
}

float Noise::surflet(glm::vec2 P, glm::vec2 gridP, int seed)
{
    // create random gradient vector for grid point
    glm::vec2 grad = 2.f * random2(gridP, seed) - glm::vec2(1.f);
    // vector from gridP to P
    glm::vec2 diff = P - gridP;
    // calculate height/value of noise field
    float value = glm::dot(diff, grad);
    // scale by falloff function
    //return (1 - glm::length(diff)) * value;
    return falloff(P, gridP) * value;
}

glm::vec2 Noise::random2(int x, int z, int seed)
{
    glm::vec2 p = glm::vec2(x, z);

    return glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(127.1f, 311.7f)),
                      glm::dot(p, glm::vec2(269.5f, 183.3f)))) * 43758.5453f);

    //return glm::vec2(random1(seed), random1(seed));
}

glm::vec2 Noise::random2(glm::vec2 p, int seed)
{
    return random2(p[0], p[1], seed);
}

int Noise::irandom1()
{
    float x;
    //std::default_random_engine rand(reinterpret_cast<uint64_t>(&x));
    //std::uniform_int_distribution<int> distribution(0, 214762);
    //return distribution(rand);
    glm::vec2 p = glm::vec2(reinterpret_cast<uint64_t>(&x), 417.27 * glm::fract(reinterpret_cast<uint64_t>(&x) / 41857.74127));

    return glm::fract(glm::sin(glm::dot(p, glm::vec2(127.1f, 311.7f)))) * 21462;
}

float Noise::random1(int seed)
{
    int max = 214746;
    std::default_random_engine rand(seed);
    std::uniform_int_distribution<int> distribution(0, max);
    return ((float) distribution(rand)) / max;
}







