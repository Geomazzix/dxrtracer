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

	u32 innerIdx = sparseSize;
	for (auto it : sparseSet)
	{
		--innerIdx;
		EXPECT_EQ(it, ids[innerIdx]);
	}

	innerIdx = sparseSize - 1;
	for (SparseSet<KeyType>::iterator it = sparseSet.begin(); it != sparseSet.end(); ++it)
	{
		//printf("it: %llu, i: %i \n", *it, innerIdx);
		EXPECT_EQ(*it, ids[innerIdx]);
		--innerIdx;
	}

	innerIdx = sparseSize - 1;
	for (SparseSet<KeyType>::const_iterator cit = sparseSet.cbegin(); cit != sparseSet.cend(); ++cit)
	{
		//printf("cit: %llu, i: %i \n", *cit, innerIdx);
		EXPECT_EQ(*cit, ids[innerIdx]);
		--innerIdx;
	}

	innerIdx = 0;
	for (SparseSet<KeyType>::reverse_iterator rit = sparseSet.rbegin(); rit != sparseSet.rend(); ++rit)
	{
		//printf("rit: %llu, i: %i \n", *rit, innerIdx);
		EXPECT_EQ(*rit, ids[innerIdx]);
		++innerIdx;
	}

	innerIdx = 0;
	for (SparseSet<KeyType>::const_reverse_iterator crit = sparseSet.rcbegin(); crit != sparseSet.rcend(); crit++)
	{
		//printf("crit: %llu, i: %i \n", *crit, innerIdx);
		EXPECT_EQ(*crit, ids[innerIdx]);
		++innerIdx;
	}

	// Removal - WIP.

	// Problem: iterator gets invalidated. Might be better to store a reference to the container in the iterator so it can refer to it's internal values as opposed to keeping track of a pointer that randomly gets invalidated.
	//const usize sparseSetSize = sparseSet.GetSize();
	//for (auto it : sparseSet)
	//{
	//	sparseSet.Remove(it);
	//	sparseSet.ShrinkCapacityToSize();
	//}
	

	//Cleanup.

	sparseSet.Clear();
	EXPECT_EQ(sparseSet.GetSize(), 0);
	EXPECT_EQ(sparseSet.GetCapacity(), sparseSize);

	sparseSet.ShrinkCapacityToSize();
	EXPECT_EQ(sparseSet.GetCapacity(), 0);
}