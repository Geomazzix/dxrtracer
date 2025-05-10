#include <gtest/gtest.h>
#include "core/containers/array.h"
#include "core/containers/sparseSet.h"

using namespace dxray;

using KeyType = usize;
const u32 SparseSize = 2048;

TEST(SparseSet, Construction)
{
	Array<KeyType> ids;
	ids.resize(SparseSize);
	for (u32 i = 0; i < SparseSize; i++)
	{
		ids[i] = i;
	}

	SparseSet<KeyType> sparseSet;
	sparseSet.Reserve(SparseSize);
	for (u32 i = 0; i < SparseSize; i++)
	{
		sparseSet.Emplace(i);
	}

	for (u32 i = 0; i < SparseSize; i++)
	{
		EXPECT_EQ(sparseSet[i], ids[i]);
	}
}

TEST(SparseSet, Iterators)
{
	Array<KeyType> ids;
	ids.resize(SparseSize);
	for (u32 i = 0; i < SparseSize; i++)
	{
		ids[i] = i;
	}

	SparseSet<KeyType> sparseSet;
	sparseSet.Reserve(SparseSize);
	for (u32 i = 0; i < SparseSize; i++)
	{
		sparseSet.Emplace(i);
	}

	u32 innerIdx = SparseSize - 1;
	for (SparseSet<KeyType>::iterator it = sparseSet.begin(); it != sparseSet.end(); ++it)
	{
		EXPECT_EQ(*it, ids[innerIdx]);
		--innerIdx;
	}

	innerIdx = SparseSize - 1;
	for (SparseSet<KeyType>::const_iterator cit = sparseSet.cbegin(); cit != sparseSet.cend(); ++cit)
	{
		EXPECT_EQ(*cit, ids[innerIdx]);
		--innerIdx;
	}

	innerIdx = 0;
	for (SparseSet<KeyType>::reverse_iterator rit = sparseSet.rbegin(); rit != sparseSet.rend(); ++rit)
	{
		EXPECT_EQ(*rit, ids[innerIdx]);
		++innerIdx;
	}

	innerIdx = 0;
	for (SparseSet<KeyType>::const_reverse_iterator crit = sparseSet.rcbegin(); crit != sparseSet.rcend(); crit++)
	{
		EXPECT_EQ(*crit, ids[innerIdx]);
		++innerIdx;
	}
}

TEST(SparseSet, Resizing)
{
	Array<KeyType> ids;
	ids.resize(SparseSize);
	for (u32 i = 0; i < SparseSize; i++)
	{
		ids[i] = i;
	}

	SparseSet<KeyType> sparseSet;
	sparseSet.Reserve(SparseSize);
	for (u32 i = 0; i < SparseSize; i++)
	{
		sparseSet.Emplace(i);
	}

	const usize sparseSetSize = sparseSet.GetSize();
	for (auto it : sparseSet)
	{
		sparseSet.Remove(it);
		sparseSet.ShrinkCapacityToSize();
	}

	sparseSet.Emplace(9412);
	sparseSet.Emplace(124014);
	sparseSet.Emplace(121482);
	EXPECT_EQ(sparseSet.GetSize(), 3);
	
	usize max = umax;
	sparseSet.Emplace(94792371);
	sparseSet.Emplace(178243);
	EXPECT_EQ(sparseSet.GetSize(), 5);}

TEST(SparseSet, Destruction)
{
	SparseSet<KeyType> sparseSet;
	sparseSet.Reserve(SparseSize);
	for (u32 i = 0; i < SparseSize; i++)
	{
		sparseSet.Emplace(i);
	}

	sparseSet.Clear();
	EXPECT_EQ(sparseSet.GetSize(), 0);
	EXPECT_EQ(sparseSet.GetCapacity(), SparseSize);
	EXPECT_NE(sparseSet.GetCapacity(), sparseSet.GetSize());

	sparseSet.ShrinkCapacityToSize();
	EXPECT_EQ(sparseSet.GetCapacity(), 0);
}

// #Todo_SparseSet: Sorting
//TEST(SparseSet, Sorting)
//{
//
//}