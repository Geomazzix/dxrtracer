#pragma once
#include "core/vath/vathUtility.h"
#include "core/valueTypes.h"
#include "core/containers/array.h"

// #Todo_container_sparseSet: Implement all methods.
// #Todo_container_sparseSet: Is EnTT the way forward...? The current implementation suggests so, though it looks overly complex looking ahead into the ECS they created.
// #Todo_container_sparseSet: Potentially implement a custom allocator.
// #Todo_container_sparseSet: Potentially implement a custom storage container - this should ensure generic elements can use the container.

// #Note_container_sparseSet: Are the key types and size types correct? i.e. can the current implementation actually work?

namespace dxray
{
	template<typename T>
	concept SparseSetKeyType = std::same_as<T, u16> || std::same_as<T, u32> || std::same_as<T, u64>;

	template<SparseSetKeyType KeyType = usize>
	class SparseSet
	{
	public:
		using key_type = KeyType;
		using size_type = usize;
		
	private:
		static constexpr const size_type InvalidKey = std::numeric_limits<KeyType>::max();
		static constexpr const size_type PageSize = 4096;
		using SparsePage = FixedArray<size_type, PageSize>;

		[[nodiscard]] inline size_type PageIndex(const key_type a_keyId) const noexcept
		{
			return a_keyId / PageSize;
		}

		[[nodiscard]] inline size_type PageOffset(const key_type a_keyId) const noexcept
		{
			return a_keyId & (PageSize - 1);
		}

		[[nodiscard]] inline SparsePage EnsurePage(const size_type a_pageIndex)
		{
			if (a_pageIndex >= m_sparsePages.size())
			{
				m_sparsePages.resize(a_pageIndex + 1);
				m_sparsePages[a_pageIndex].fill(InvalidKey);
			}

			return m_sparsePages[a_pageIndex];
		}

	public:
		using iterator = Array<size_type>::iterator;
		using const_iterator = iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::const_reverse_iterator<const_iterator>;

		SparseSet() :
			m_sparsePages(),
			m_dense()
		{ }

		explicit SparseSet(const size_type a_initialCapacity = PageSize) :
			m_sparsePages(),
			m_dense()
		{
			m_sparsePages.reserve(static_cast<size_type>(static_cast<fp32>(a_initialCapacity) / PageSize));
			m_dense.reserve(a_initialCapacity);
		}

		virtual ~SparseSet() = default;

		SparseSet(const SparseSet& a_rhs) = delete;

		SparseSet& operator=(const SparseSet& a_rhs) = delete;

		SparseSet(SparseSet&& a_rhs) = default;

		SparseSet&& operator=(SparseSet&& a_rhs) = default;

		void Reserve(const size_type a_capacity)
		{
			m_dense.reserve(a_capacity);
		}

		void Emplace(const key_type a_keyId)
		{
			// #Todo: Implement.
		}

		void Insert(iterator a_first, iterator a_end)
		{
			// #Todo: Implement.
		}

		void Remove(const key_type a_keyId)
		{
			// #Todo: Implement.
		}

		void Remove(iterator a_first, iterator a_end)
		{
			// #Todo: Implement.
		}

		void Swap(const key_type a_lhs, const key_type a_rhs)
		{
			// #Todo: Implement.
		}

		void Clear()
		{
			// #Todo: Implement.
		}

		[[nodiscard]] const size_type GetCapacity() const noexcept
		{
			return m_dense.capacity();
		}

		[[nodiscard]] const size_type GetSize() const noexcept
		{
			return m_dense.size();
		}

		[[nodiscard]] const bool IsEmpty() const noexcept
		{
			return m_dense.empty();
		}

		[[nodiscard]] const bool Contains() const noexcept
		{
			// #Todo: Implement.
		}

		[[nodiscard]] iterator Find(const key_type a_keyId)
		{
			// #Todo: Implement.
		}

		[[nodiscard]] size_type MapToIndex(const key_type a_keyId)
		{
			// #Todo: Implement.
		}

		[[nodiscard]] key_type At(const size_type a_index) const
		{
			// #Todo: Implement.
		}

		[[nodiscard]] key_type operator[](const size_type a_index) const
		{
			// #Todo: Implement.
		}

		[[nodiscard]] iterator begin() noexcept
		{
			return m_dense.begin();
		}

		[[nodiscard]] const_iterator begin() const noexcept
		{
			return m_dense.begin();
		}

		[[nodiscard]] iterator end() noexcept
		{
			return m_dense.end();
		}

		[[nodiscard]] const_iterator end() const noexcept
		{
			return m_dense.end();
		}

		[[nodiscard]] reverse_iterator rbegin() noexcept
		{
			return m_dense.rbegin();
		}

		[[nodiscard]] const_reverse_iterator rbegin() const noexcept
		{
			return m_dense.rbegin();
		}

		[[nodiscard]] reverse_iterator rend() noexcept
		{
			return m_dense.rend();
		}

		[[nodiscard]] const_reverse_iterator rend() const noexcept
		{
			return m_dense.rend();
		}

		[[nodiscard]] const_iterator cbegin() const noexcept
		{
			return begin();
		}

		[[nodiscard]] const_iterator cend() const noexcept
		{
			return end();
		}

		[[nodiscard]] const_reverse_iterator rcbegin() const noexcept
		{
			return rbegin();
		}

		[[nodiscard]] const_reverse_iterator rcend() const noexcept
		{
			return rend();
		}

	private:
		Array<SparsePage> m_sparsePages;
		Array<key_type> m_dense;
	};
}