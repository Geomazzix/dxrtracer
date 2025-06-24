#include <gtest/gtest.h>
#include <core/vath/vath.h>
#include <core/debug.h>

using namespace dxray;
using namespace dxray::vath;

TEST(Matrix4x4, Multiplication)
{
	Matrix4x4 a = Matrix4x4(2, 3, -2, 1, 3, 2, 1, 4, 2, -3, -2, 1, 1, 3, 2, -2);
	Matrix4x4 b = a;

	Matrix4x4 c = b * a;
	Matrix4x4 expectedResult = Matrix4x4(
		10, 21, 5, 10, 
		18, 22, 2, 4, 
		-8, 9, -1, -14, 
		13, -3, -7, 19
	);
	EXPECT_EQ(c, expectedResult);

	c = a;
	c *= b;
	EXPECT_EQ(c, expectedResult);
}

TEST(Matrix4x4, Determinant)
{
	Matrix4x4 a = Matrix4x4(
		2, 3, 1, 3,
		-2, 1, 1, 8,
		2, 1, 1, 2,
		7, 8, 1, 2
	);
	float detA = Determinant(a);
	EXPECT_EQ(detA, 32);
}

TEST(Matrix4x4, Inverse)
{
	Matrix4x4 a = Matrix4x4(
		1, 1, -2, 0,
		2, 1, 1, 0,
		2, -3, 1, 0,
		0, 0, 0, 1
	);

	Matrix4x4 hardInverseA = Matrix4x4(
		0.2f, 0.25f, 0.15f, 0.0f,
		0.0f, 0.25f, -0.25f, 0.0f,
		-0.4f, 0.25f, -0.05f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix4x4 c = hardInverseA * a;
	EXPECT_EQ(c, Matrix4x4());

	Matrix4x4 inverseA = Inverse(a);
	Matrix4x4 expectedIdentity = inverseA * a;

	EXPECT_EQ(expectedIdentity, Matrix4x4());
}