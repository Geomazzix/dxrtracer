#pragma once

#ifdef HLSL

typedef float fp32;
typedef double fp64;
typedef uint u32;
typedef int i32;

typedef float2 Vector2f;
typedef float3 Vector3f;
typedef float4 Vector4f;
typedef float2x2 Matrix2x2f;
typedef float3x3 Matrix3x3f;
typedef float4x4 Matrix4x4f;

#else

#include <core/valueTypes.h>
#include <core/vath/vath.h>
// Downside of this framework needing this bridge. With a lot of effort and extra time this can be abstracted away, this is outside of the scope of the goals of this project though.
using namespace dxray::vath;

#endif

