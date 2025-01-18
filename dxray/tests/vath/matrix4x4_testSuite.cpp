#include <gtest/gtest.h>
#include "core/vath/vath.h"

using namespace dxray;
using namespace dxray::vath;

TEST(Matrix4x4, Multiplication)
{
	Matrix4x4 a = Matrix4x4(2, 3, -2, 1, 3, 2, 1, 4, 2, -3, -2, 1, 1, 3, 2, -2);
	Matrix4x4 b = Matrix4x4(2, 3, -2, 1, 3, 2, 1, 4, 2, -3, -2, 1, 1, 3, 2, -2);

	Matrix4x4 c = b * a;
	Matrix4x4 expectedResult = Matrix4x4(10, 21, 5, 10, 18, 22, 2, 4, -8, 9, -1, -14, 13, -3, -7, 19);
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
		1, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	Matrix4x4 inverseA = Inverse(a);
	Matrix4x4 identity = inverseA * a;

	EXPECT_EQ(identity, Matrix4x4());
}