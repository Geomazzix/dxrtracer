#include <gtest/gtest.h>
#include "core/vath/vath.h"

using namespace dxray;
using namespace dxray::vath;

TEST(Matrix2x2, Multiplication)
{
	Matrix2x2 a = Matrix2x2(2, 8, -4, 2);
	Matrix2x2 b = Matrix2x2(4, -8, 4, -2);

	Matrix2x2 c = b * a;
	Matrix2x2 expectedResult = Matrix2x2(40, -32, -8, 28);
	EXPECT_EQ(c, expectedResult);

	c = a;
	c *= b;
	EXPECT_EQ(c, expectedResult);
}

TEST(Matrix2x2, Determinant)
{
	Matrix2x2 a = Matrix2x2(2, 8, -4, 2);
	float detA = Determinant(a);
	EXPECT_EQ(detA, 36);
}

TEST(Matrix2x2, Inverse)
{
	Matrix2x2 a = Matrix2x2(2, 8, -4, 2);
	Matrix2x2 inverseA = Inverse(a);
	Matrix2x2 identity = inverseA * a;

	EXPECT_EQ(identity, Matrix2x2());
}