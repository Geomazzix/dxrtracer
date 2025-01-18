#include <gtest/gtest.h>
#include "core/vath/vath.h"

using namespace dxray;
using namespace dxray::vath;

TEST(Matrix3x3, Multiplication)
{
	Matrix3x3 a = Matrix3x3(2, 8, -4, 2, 3, 4, 9, 1, -2);
	Matrix3x3 b = Matrix3x3(4, -8, 4, -2, 5, 2, 7, 3, -2);

	Matrix3x3 c = b * a;
	Matrix3x3 expectedResult = Matrix3x3(-36, 12, 32, 30, 11, 6, 20, -73, 42);
	EXPECT_EQ(c, expectedResult);

	c = a;
	c *= b;
	EXPECT_EQ(c, expectedResult);
}

TEST(Matrix3x3, Determinant)
{
	Matrix3x3 a = Matrix3x3(2, 8, -4, 2, 3, 4, 9, 1, -2);
	float detA = Determinant(a);
	EXPECT_EQ(detA, 400);
}

TEST(Matrix3x3, Inverse)
{
	Matrix3x3 a = Matrix3x3(1, 1, 0, 0, 1, 0, 0, 0, 1);
	Matrix3x3 inverseA = Inverse(a);
	Matrix3x3 identity = inverseA * a;

	EXPECT_EQ(identity, Matrix3x3());
}