#include <gtest/gtest.h>
#include "core/containers/sparseSet.h"

using namespace dxray;

TEST(SparseSet, StressTest)
{
	//Test setup.

	using KeyType = usize;
	const u32 sparseSize = 2048;

	std::vector<KeyType> ids;
	ids.resize(sparseSize);
	for (u32 i = 0; i < sparseSize; i++)
	{
		ids[i] = i;
	}


	//Creation/initialization.

	SparseSet<KeyType> sparseSet;
	sparseSet.Reserve(sparseSize);
	for (u32 i = 0; i < sparseSize; i++)
	{
		sparseSet.Emplace(i);
	}

	for (u32 i = 0; i < sparseSize; i++)
	{
		EXPECT_EQ(sparseSet[i], ids[i]);
	}


	// Iterators.

	u32 i = sparseSize;
	for (auto it : sparseSet)
	{
		--i;
		EXPECT_EQ(it, ids[i]);
	}

	u32 j = sparseSize;
	for (const auto& it : sparseSet)
	{
		--j;
		EXPECT_EQ(it, ids[j]);
	}


	// Removal.

	const usize sparseSetSize = sparseSet.GetSize();
	for (i32 i = 0; i < sparseSetSize; i++)
	{
		sparseSet.Remove(i);
	}
	

	//Cleanup.

	sparseSet.Clear();
	EXPECT_EQ(sparseSet.GetSize(), 0);
	EXPECT_EQ(sparseSet.GetCapacity(), sparseSize);

	sparseSet.ShrinkCapacityToSize();
	EXPECT_EQ(sparseSet.GetCapacity(), 0);
}