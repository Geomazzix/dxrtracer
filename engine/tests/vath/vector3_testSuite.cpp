#include <gtest/gtest.h>
#include "core/vath/vath.h"

using namespace dxray;
using namespace dxray::vath;

TEST(Vector3, Construction)
{
	Vector3 vec3 = { 0, 1, 0.52f };
	Vector3 vec3_1(1.0f);
	Vector3 vec3_2;

	fp32 data[3] = { 1, 2, 3 };
	Vector3 data_vec3(data);

	EXPECT_EQ(vec3[0], 0);
	EXPECT_EQ(vec3[1], 1);
	EXPECT_EQ(vec3[2], 0.52f);

	EXPECT_EQ(vec3_1[0], 1);
	EXPECT_EQ(vec3_1[1], 1);
	EXPECT_EQ(vec3_1[2], 1);

	EXPECT_EQ(vec3_2[0], 0);
	EXPECT_EQ(vec3_2[1], 0);
	EXPECT_EQ(vec3_2[2], 0);

	EXPECT_EQ(data_vec3[0], 1);
	EXPECT_EQ(data_vec3[1], 2);
	EXPECT_EQ(data_vec3[2], 3);
}

TEST(Vector3, Addition)
{
	Vector3 a = { 0, -2, 3 };
	Vector3 b = { 1, -5, 7 };
	Vector3 c = a + b;

	EXPECT_EQ(c, Vector3(1, -7, 10));
}

TEST(Vector3, Subtraction)
{
	Vector3 a = { 0, -2, 3 };
	Vector3 b = { 1, -5, 7 };
	Vector3 c = a - b;

	EXPECT_EQ(c, Vector3(-1, 3, -4));
}

TEST(Vector3, Multiplication)
{
	Vector3 a = { 0, -2, 3 };
	fp32 scalar = 0.74f;
	Vector3 c = a * scalar;

	EXPECT_EQ(c, Vector3(0, -1.48f, 2.22f));
}

TEST(Vector3, Division)
{
	Vector3 a = { 0, -2, 3 };
	fp32 scalar = 2;
	Vector3 c = a / scalar;

	EXPECT_EQ(c, Vector3(0, -1, 1.5f));
}

TEST(Vector3, Cross)
{
	Vector3 i = { 1, 0, 0 };
	Vector3 j = { 0, 1, 0 };
	Vector3 k = { 0, 0, 1 };

	Vector3 v1 = Cross(i, j);
	Vector3 v2 = Cross(j, k);
	Vector3 v3 = Cross(k, i);

	EXPECT_EQ(v1, k);
	EXPECT_EQ(v2, i);
	EXPECT_EQ(v3, j);
}