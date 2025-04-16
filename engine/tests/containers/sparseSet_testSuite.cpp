#include <gtest/gtest.h>
#include "core/containers/sparseSet.h"

using namespace dxray;

TEST(SparseSet, StressTest)
{
	struct SparseItem
	{
		u32 a = 141241233;
		fp32 b = 1231.1212f;
		u8 c = 12;

		SparseItem() = default;
		SparseItem(u32 _a, fp32 _b, u8 _c) :
			a(_a),
			b(_b),
			c(_c)
		{
		}
	};

	using KeyType = usize;
	const u32 sparseSize = 2048;
	std::vector<KeyType> ids;
	ids.resize(sparseSize);

	for (u32 i = 0; i < sparseSize; i++)
	{
		ids[i] = i;
	}

	SparseSet<KeyType> sparseSet(sparseSize);
	for (u32 i = 0; i < sparseSize; i++)
	{
		sparseSet.Emplace(i, SparseItem());
	}

	if (sparseSet.Contains(128))
	{
		SparseItem i = sparseSet.Get(128);
	}

	sparseSet.Remove(ids[2046]);
	sparseSet.Remove(ids.back());

	for (auto& it : sparseSet)
	{
		it.b = 2.0f;
	}

	const usize sparseSetSize = sparseSet.GetSize();
	for (i32 i = 0; i < sparseSetSize; i++)
	{
		EXPECT_EQ(sparseSet[i].b, 2.0f);
		sparseSet.Remove(i);
	}

	sparseSet.Clear();
	EXPECT_EQ(sparseSet.GetSize(), 0);
}