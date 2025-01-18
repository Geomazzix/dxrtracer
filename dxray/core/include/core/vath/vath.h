#pragma once

//#Todo: Add more downcasting options, e.g vector4 to vector3, a few have been implemented, but limited.
//#Todo: Add Simd to vector4 and matrix4 when the time arises, vector3 and matrix3x3 *could* perhaps use them, though with padded registers.

#include "core/vath/vathUtility.h"
#include "core/vath/vector4.h"
#include "core/vath/vector3.h"
#include "core/vath/vector2.h"
#include "core/vath/matrix4x4.h"
#include "core/vath/matrix3x3.h"
#include "core/vath/matrix2x2.h"
#include "core/vath/quaternion.h"

namespace dxray::vath
{
	//Vector2
	using Vector2u8 = Vector<2, u8>;
	using Vector2u16 = Vector<2, u16>;
	using Vector2u32 = Vector<2, u32>;
	using Vector2u64 = Vector<2, u64>;
	using Vector2i8 = Vector<2, i8>;
	using Vector2i16 = Vector<2, i16>;
	using Vector2i32 = Vector<2, i32>;
	using Vector2i64 = Vector<2, i64>;
	using Vector2f = Vector<2, fp32>;
	using Vector2d = Vector<2, fp64>;

	//Vector3
	using Vector3u8 = Vector<3, u8>;
	using Vector3u16 = Vector<3, u16>;
	using Vector3u32 = Vector<3, u32>;
	using Vector3u64 = Vector<3, u64>;
	using Vector3i8 = Vector<3, i8>;
	using Vector3i16 = Vector<3, i16>;
	using Vector3i32 = Vector<3, i32>;
	using Vector3i64 = Vector<3, i64>;
	using Vector3f = Vector<3, fp32>;
	using Vector3d = Vector<3, fp64>;
	
	//Vector4
	using Vector4u8 = Vector<4, u8>;
	using Vector4u16 = Vector<4, u16>;
	using Vector4u32 = Vector<4, u32>;
	using Vector4u64 = Vector<4, u64>;
	using Vector4i8 = Vector<4, i8>;
	using Vector4i16 = Vector<4, i16>;
	using Vector4i32 = Vector<4, i32>;
	using Vector4i64 = Vector<4, i64>;
	using Vector4f = Vector<4, fp32>;
	using Vector4d = Vector<4, fp64>;

	//Matrix2x2
	using Matrix2x2u8 = Matrix<2, 2, u8>;
	using Matrix2x2u16 = Matrix<2, 2, u16>;
	using Matrix2x2u32 = Matrix<2, 2, u32>;
	using Matrix2x2u64 = Matrix<2, 2, u64>;
	using Matrix2x2i8 = Matrix<2, 2, i8>;
	using Matrix2x2i16 = Matrix<2, 2, i16>;
	using Matrix2x2i32 = Matrix<2, 2, i32>;
	using Matrix2x2i64 = Matrix<2, 2, i64>;
	using Matrix2x2i32 = Matrix<2, 2, i32>;
	using Matrix2x2f = Matrix<2, 2, fp32>;
	using Matrix2x2d = Matrix<2, 2, fp64>;

	//Matrix3x3
	using Matrix3x3u8 = Matrix<3, 3, u8>;
	using Matrix3x3u16 = Matrix<3, 3, u16>;
	using Matrix3x3u32 = Matrix<3, 3, u32>;
	using Matrix3x3u64 = Matrix<3, 3, u64>;
	using Matrix3x3i8 = Matrix<3, 3, i8>;
	using Matrix3x3i16 = Matrix<3, 3, i16>;
	using Matrix3x3i32 = Matrix<3, 3, i32>;
	using Matrix3x3i64 = Matrix<3, 3, i64>;
	using Matrix3x3i32 = Matrix<3, 3, i32>;
	using Matrix3x3f = Matrix<3, 3, fp32>;
	using Matrix3x3d = Matrix<3, 3, fp64>;

	//Matrix4x4
	using Matrix4x4u8 = Matrix<4, 4, u8>;
	using Matrix4x4u16 = Matrix<4, 4, u16>;
	using Matrix4x4u32 = Matrix<4, 4, u32>;
	using Matrix4x4u64 = Matrix<4, 4, u64>;
	using Matrix4x4i8 = Matrix<4, 4, i8>;
	using Matrix4x4i16 = Matrix<4, 4, i16>;
	using Matrix4x4i32 = Matrix<4, 4, i32>;
	using Matrix4x4i64 = Matrix<4, 4, i64>;
	using Matrix4x4i32 = Matrix<4, 4, i32>;
	using Matrix4x4f = Matrix<4, 4, fp32>;
	using Matrix4x4d = Matrix<4, 4, fp64>;

	//Quaternion
	using Quaternionf = Quat<fp32>;
	using Quaterniond = Quat<fp64>;

	//Generalize the usage of the math library.
	using Vector2 = Vector2f;
	using Vector3 = Vector3f;
	using Vector4 = Vector4f;
	using Matrix2x2 = Matrix2x2f;
	using Matrix3x3 = Matrix3x3f;
	using Matrix4x4 = Matrix4x4f;
	using Quaternion = Quaternionf;
}