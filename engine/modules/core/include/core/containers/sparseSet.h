#pragma once
#include "core/vath/vathUtility.h"
#include "core/valueTypes.h"
#include "core/containers/array.h"

// #Todo: Revise the whole class:
// - Strip all the value_type ops and move em into the sparseArray class.
// - Revise iterators, MIGHT need a custom one for the sparse set.
// - Revise the current paged sparse implementation - i.e. since we're not dealing with value_types anymore
// we can now get away with different types of tricks that do not require us having to deal with strong types.

namespace dxray
{
	template<typename T>
	concept SparseSetKeyType = std::same_as<T, u16> || std::same_as<T, u32> || std::same_as<T, u64>;

	/*!
	 * @brief Resize-able sparse set implementation. To be used with an ECS.
	 * @tparam ElementType: The element type to store in the dense array.
	 * @tparam IndexType: The index type to use as handle to retrieve.
	 */
	template<SparseSetKeyType KeyType>
	class SparseSet
	{
	public:
		using key_type = KeyType;
		using size_type = usize;
		using difference_type = u64;

		using iterator = Array<key_type>::iterator;
		using const_iterator = Array<key_type>::const_iterator;
		using reverse_iterator = Array<key_type>::reverse_iterator;
		using const_reverse_iterator = Array<key_type>::const_reverse_iterator;

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
		SparseSet() :
			m_sparsePages(),
			m_dense(),
			m_keyToDenseMapping()
		{
		}

		explicit SparseSet(const size_type a_initialCapacity = PageSize) :
			m_sparsePages(),
			m_dense(),
			m_keyToDenseMapping()
		{
			// Reserve 1 page, or more if needed to match the capacity of the dense capacity.
			m_sparsePages.reserve(static_cast<size_type>(static_cast<fp32>(a_initialCapacity) / PageSize));
			m_dense.reserve(a_initialCapacity);
			m_keyToDenseMapping.reserve(a_initialCapacity);
		}

		virtual ~SparseSet() = default;

		SparseSet(const SparseSet& a_rhs) = delete;

		SparseSet& operator=(const SparseSet& a_rhs) = delete;

		SparseSet(SparseSet&& a_rhs) = default;

		SparseSet& operator=(SparseSet&& a_rhs) = default;

		void Reserve(const size_type a_numElements)
		{
			m_keyToDenseMapping.reserve(a_numElements);
			m_dense.reserve(a_numElements);
		}

		void ShrinkToFit()
		{
			m_sparsePages.shrink_to_fit();
			m_keyToDenseMapping.shrink_to_fit();
			m_dense.shrink_to_fit();
		}

		template<typename... Args>
		ElementType& Emplace(const key_type a_keyId, Args&&... a_pArgs)
		{
			if (const size_type Index = EnsurePage(PageIndex(a_keyId))[PageOffset(a_keyId)] != InvalidKey)
			{
				m_keyToDenseMapping[Index] = a_keyId;
				m_dense[Index] = ElementType(std::forward<Args>(a_pArgs)...);
				return m_dense[Index];
			}

			m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)] = m_dense.size();
			m_keyToDenseMapping.push_back(a_keyId);
			m_dense.push_back(ElementType(std::forward<Args>(a_pArgs)...));
			return m_dense.back();
		}

		void Remove(const key_type a_keyId)
		{
			DXRAY_ASSERT(Contains(a_keyId));

			const size_type index = m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)];
			const key_type backKeyId = m_keyToDenseMapping.back();
			m_sparsePages[PageIndex(backKeyId)][PageOffset(backKeyId)] = index;
			m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)] = InvalidKey;

			std::swap(m_dense.back(), m_dense[index]);
			std::swap(m_keyToDenseMapping.back(), m_keyToDenseMapping[index]);

			m_dense.pop_back();
			m_keyToDenseMapping.pop_back();
		}

		void Clear() noexcept
		{
			m_dense.clear();
			m_sparsePages.clear();
			m_keyToDenseMapping.clear();
		}

		[[nodiscard]] value_type& Get(const key_type a_keyId) noexcept
		{
			DXRAY_ASSERT(PageIndex(a_keyId) < m_sparsePages.size());
			DXRAY_ASSERT(PageOffset(a_keyId) < PageSize);
			return m_dense[m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)]];
		}

		[[nodiscard]] bool Contains(const key_type a_keyId) const noexcept
		{
			DXRAY_ASSERT(PageIndex(a_keyId) < m_sparsePages.size());
			DXRAY_ASSERT(PageOffset(a_keyId) < PageSize);
			return m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)] != InvalidKey;
		}

		[[nodiscard]] bool IsEmpty() const noexcept
		{
			return m_dense.empty();
		}

		[[nodiscard]] size_type GetCapacity() const
		{
			return m_dense.capacity();
		}

		[[nodiscard]] size_type GetSize() const noexcept
		{
			return m_dense.size();
		}

		[[nodiscard]] const value_type* const GetData() const noexcept
		{
			return m_dense.data();
		}

		[[nodiscard]] value_type At(const key_type a_keyId) const
		{
			DXRAY_ASSERT(PageIndex(a_keyId) < m_sparsePages.size());
			DXRAY_ASSERT(PageOffset(a_keyId) < PageSize);
			const size_type Index = m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)];
			DXRAY_ASSERT(a_keyId != InvalidKey);
			return m_dense.at(Index);
		}

		[[nodiscard]] value_type operator[](const key_type a_keyId) const
		{
			return m_dense[m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)]];
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
		Array<key_type> m_keyToDenseMapping;
	};


	template<typename ValueType, SparseSetKeyType KeyType = usize>
	class SparseArray : public SparseSet<KeyType>
	{
	public:
		using value_type = ElementType;

		using iterator = Array<value_type>::iterator;
		using const_iterator = Array<value_type>::const_iterator;
		using reverse_iterator = Array<value_type>::reverse_iterator;
		using const_reverse_iterator = Array<value_type>::const_reverse_iterator;

	public:
		SparseArray() :
			m_elements()
		{ }

		explicit SparseArray(const size_type a_initialCapacity = PageSize) :
			m_elements(),
		{
			m_elements.reserve(a_initialCapacity);
		}

		virtual ~SparseArray() = default;

		SparseArray(const SparseArray& a_rhs) = delete;

		SparseArray& operator=(const SparseArray& a_rhs) = delete;

		SparseArray(SparseArray&& a_rhs) = default;

		SparseArray& operator=(SparseArray&& a_rhs) = default;

		value_type& Emplace()
		{
			// #Todo: Implementation.
		}

		void Remove()
		{
			// #Todo: Implementation.
		}

		[[nodiscard]] iterator begin() noexcept
		{
			return m_elements.begin();
		}

		[[nodiscard]] const_iterator begin() const noexcept
		{
			return m_elements.begin();
		}

		[[nodiscard]] iterator end() noexcept
		{
			return m_elements.end();
		}

		[[nodiscard]] const_iterator end() const noexcept
		{
			return m_elements.end();
		}

		[[nodiscard]] reverse_iterator rbegin() noexcept
		{
			return m_elements.rbegin();
		}

		[[nodiscard]] const_reverse_iterator rbegin() const noexcept
		{
			return m_elements.rbegin();
		}

		[[nodiscard]] reverse_iterator rend() noexcept
		{
			return m_elements.rend();
		}

		[[nodiscard]] const_reverse_iterator rend() const noexcept
		{
			return m_elements.rend();
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
		Array<value_type> m_elements;
	};
}