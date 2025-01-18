#include <gtest/gtest.h>
#include "core/vath/vath.h"

using namespace dxray;
using namespace dxray::vath;

static float QuatEpsilon = 1e-3f; //Custom epsilon needed for floating quaternions, seem to be lacking precision severely... gets 5~ decimals better when doubling precision.

#define CHECK_VEC3(lhs, rhs, epsilon)		\
	EXPECT_NEAR(lhs[0], rhs[0], epsilon);	\
	EXPECT_NEAR(lhs[1], rhs[1], epsilon);	\
	EXPECT_NEAR(lhs[2], rhs[2], epsilon);

#define CHECK_VEC4(lhs, rhs, epsilon)		\
	EXPECT_NEAR(lhs[0], rhs[0], epsilon);	\
	EXPECT_NEAR(lhs[1], rhs[1], epsilon);	\
	EXPECT_NEAR(lhs[2], rhs[2], epsilon);	\
	EXPECT_NEAR(lhs[3], rhs[3], epsilon);

#define CHECK_MATRIX3X3(lhs, rhs, epsilon)		\
	EXPECT_NEAR(lhs[0][0], rhs[0][0], epsilon);	\
	EXPECT_NEAR(lhs[0][1], rhs[0][1], epsilon);	\
	EXPECT_NEAR(lhs[0][2], rhs[0][2], epsilon);	\
	EXPECT_NEAR(lhs[1][0], rhs[1][0], epsilon);	\
	EXPECT_NEAR(lhs[1][1], rhs[1][1], epsilon);	\
	EXPECT_NEAR(lhs[1][2], rhs[1][2], epsilon);	\
	EXPECT_NEAR(lhs[2][0], rhs[2][0], epsilon);	\
	EXPECT_NEAR(lhs[2][1], rhs[2][1], epsilon);	\
	EXPECT_NEAR(lhs[2][2], rhs[2][2], epsilon);

TEST(Quaternion, EulerCasting)
{
	Vector3 a = Vector3(DegToRad(62.0f), DegToRad(45.0f), DegToRad(-22.0f));
	Quaternion b = ToQuat(a);
	Vector3 c = ToEuler(b);

	CHECK_VEC3(a, c, QuatEpsilon);
}

TEST(Quaternion, Multiplication)
{
	Quaternion a = Quaternion(2.0f, 4.0f, 9.0f, 9.0f);
	Quaternion b = Quaternion(2.0f, 16.0f, 6.0f, 3.0f);

	Quaternion c = a * b;
	Quaternion d = Quaternion(-96.0f, 162.0f, 105.0f, -95.0f);

	CHECK_VEC4(d, c, QuatEpsilon);
}

TEST(Quaternion, VectorMultiplication)
{
	Quaternion a = ToQuat(Vector3(0.0f, 0.0f, DegToRad(45.0f)));
	Vector3 b = Vector3(0.0f, 1.0f, 1.0f);

	Vector3 c = b * Normalize(a);
	Vector3 d = Vector3(-0.707106f, 0.707106f, 1.0f);

	CHECK_VEC3(d, c, QuatEpsilon);
}