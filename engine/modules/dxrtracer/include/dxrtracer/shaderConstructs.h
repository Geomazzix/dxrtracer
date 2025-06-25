#pragma once

#ifdef HLSL
#include "[source:core]/vath/vathHlsl.h"
#else
#include <core/vath/vathHlsl.h>
#endif

/* Represents the GPU scene data. */
struct SceneConstantBuffer
{
	Matrix4x4f View;
	Matrix4x4f Projection;

	Vector4f SkyColour;
	Vector4f SunDirection;
};