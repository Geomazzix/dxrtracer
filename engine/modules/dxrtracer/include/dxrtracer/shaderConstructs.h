#pragma once

#ifdef HLSL
#include "[source:core]/vath/vathHlsl.h"
#else
#include <core/vath/vathHlsl.h>
#endif

/**
 * @brief The trace distance clamps - these apply to all traces.
 */
static const fp32 MinTraceDistance = 0.01f;
static const fp32 MaxTraceDistance = 1000.0f;

/**
 * @brief Contains global scene data, set every frame.
 */
struct SceneConstantBuffer
{
	Matrix4x4f View;
	Matrix4x4f Projection;

	Vector4f SkyColour;
	Vector4f SunDirection;

	u32 FrameIndex;
	u32 SuperSampleSize;
};

/**
 * @brief Used for the initial trace in the ray generation shader.
 * Note: Alignment is explicitly done in a bad way as shaders require booleans to take up a whole byte.
 */
struct HitInfo
{
	Vector3f Colour;
	bool AllowReflection; // #Todo: optimize this - can be turned into bitwise flags.
	fp32 Distance;
	bool Missed;
};