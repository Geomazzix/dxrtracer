#pragma once

// Source Hash functions for GPU rendering - https://jcgt.org/published/0009/03/02/.

uint4 GenerateSeed(uint2 a_pixelCoords, uint frameNumber)
{
    return uint4(a_pixelCoords.xy, frameNumber, 0);
}

float MapUintToNormFloat(in const uint a_unsigned)
{
    // Turn the sign bits and exponent bits to 0 -> results in [1, 2] - 1 = [0, 1].
    return asfloat(0x3f800000 | (a_unsigned >> 9)) - 1.0f;
}

uint4 RandomPcg4d(in uint4 a_v4Seed)
{
    a_v4Seed = a_v4Seed * 1664525u + 1013904223u;
    a_v4Seed.x += a_v4Seed.y * a_v4Seed.w;
    a_v4Seed.y += a_v4Seed.z * a_v4Seed.x;
    a_v4Seed.z += a_v4Seed.x * a_v4Seed.y;
    a_v4Seed.w += a_v4Seed.y * a_v4Seed.z;
    a_v4Seed ^= (a_v4Seed >> 16u);
    a_v4Seed.x += a_v4Seed.y * a_v4Seed.w;
    a_v4Seed.y += a_v4Seed.z * a_v4Seed.x;
    a_v4Seed.z += a_v4Seed.x * a_v4Seed.y;
    a_v4Seed.w += a_v4Seed.y * a_v4Seed.z;
    return a_v4Seed;
}

float Randf(inout uint4 a_rngState)
{
    a_rngState.w++;
    return MapUintToNormFloat(RandomPcg4d(a_rngState).x);
}