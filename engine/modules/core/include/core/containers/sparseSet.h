#pragma once
#include "core/vath/vathUtility.h"
#include "core/valueTypes.h"
#include "core/containers/array.h"

namespace dxray
{
	template<typename T>
	concept SparseSetKeyType = std::same_as<T, u16> || std::same_as<T, u32> || std::same_as<T, u64>;

	/*!
	 * @brief Resize-able sparse set implementation. To be used with an ECS.
	 * @tparam ElementType: The element type to store in the dense array.
	 * @tparam IndexType: The index type to use as handle to retrieve.
	 */
	template<typename ElementType, SparseSetKeyType KeyType = usize>
	class SparseSet
	{
	public:
		using value_type = ElementType;
		using key_type = KeyType;
		using size_type = KeyType;

		using iterator = Array<value_type>::iterator;
		using const_iterator = Array<value_type>::const_iterator;
		using reverse_iterator = Array<value_type>::reverse_iterator;
		using const_reverse_iterator = Array<value_type>::const_reverse_iterator;
		
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

		inline void SetDenseMapping(const key_type a_keyId, const size_type a_index)
		{
			EnsurePage(PageIndex(a_keyId))[PageOffset(a_keyId)] = a_index;
		}

		inline size_type GetDenseMapping(const key_type a_keyId)
		{
			return EnsurePage(PageIndex(a_keyId))[PageOffset(a_keyId)];
		}

	public:
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

		void Reserve(const size_type a_numElements)
		{
			m_dense.reserve(a_numElements);
		}

		void ShrinkToFit()
		{
			m_sparsePages.shrink_to_fit();
			m_dense.shrink_to_fit();
		}

		template<typename... Args>
		ElementType& Emplace(const key_type a_keyId, Args&&... a_pArgs)
		{
			const size_type Index = EnsurePage(PageIndex(a_keyId))[PageOffset(a_keyId)];
			if (Index != InvalidKey)
			{
				m_dense[Index] = ElementType(std::forward<Args>(a_pArgs)...);
				m_keyToDenseMapping[Index] = a_keyId;
				return m_dense[Index];
			}

			m_sparsePages[PageIndex(a_keyId)][PageOffset(a_keyId)] = m_dense.size();
			m_dense.push_back(ElementType(std::forward<Args>(a_pArgs)...));
			m_keyToDenseMapping.push_back(a_keyId);
			return m_dense.back();
		}

		void Remove(const key_type a_keyId)
		{
			const size_type index = GetDenseMapping(a_keyId);
			if (index == InvalidKey)
			{
				return;
			}

			SetDenseMapping(m_keyToDenseMapping.back(), index);
			SetDenseMapping(a_keyId, InvalidKey);
			
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
			const size_type index = GetDenseMapping(a_keyId);
			DXRAY_ASSERT(index != InvalidKey);
			return m_dense[index];
		}

		[[nodiscard]] bool Contains(const key_type a_keyId) const noexcept
		{
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
			const size_type Index = GetDenseMapping(a_keyId);
			DXRAY_ASSERT(a_keyId != InvalidKey);
			return m_dense.at(Index);
		}

		[[nodiscard]] value_type operator[](const key_type a_keyId) const
		{
			return At(a_keyId);
		}

		// Scoped iterator implementations - cannot confirm to coding standards.

		[[nodiscard]] iterator begin() noexcept
		{
			return m_dense.begin();
		}

		[[nodiscard]] const_iterator begin() const noexcept
		{
			return m_dense.begin();
		}

		[[nodiscard]] const_iterator cbegin() const noexcept
		{
			return m_dense.cbegin();
		}

		[[nodiscard]] reverse_iterator rbegin() noexcept
		{
			return m_dense.rbegin();
		}

		[[nodiscard]] const_reverse_iterator rbegin() const noexcept
		{
			return m_dense.rbegin();
		}
		
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept
		{
			return m_dense.crbegin();
		}

		[[nodiscard]] iterator end() noexcept
		{
			return m_dense.end();
		}

		[[nodiscard]] const_iterator end() const noexcept
		{
			return m_dense.end();
		}

		[[nodiscard]] const_iterator cend() const noexcept
		{
			return m_dense.cend();
		}

		[[nodiscard]] reverse_iterator rend() noexcept
		{
			return m_dense.rend();
		}

		[[nodiscard]] const_reverse_iterator rend() const noexcept
		{
			return m_dense.rend();
		}
		
		[[nodiscard]] const_reverse_iterator crend() const noexcept
		{
			return m_dense.crend();
		}

	private:
		Array<SparsePage> m_sparsePages;
		Array<value_type> m_dense;
		Array<key_type> m_keyToDenseMapping;
	};
}