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
	Matrix4x4f InverseView;
	Matrix4x4f InverseProjection;

	Vector4f CameraPosition;		// 4th param being focal length.
	Vector4f SkyColour;
	Vector4f SunDirection;
};