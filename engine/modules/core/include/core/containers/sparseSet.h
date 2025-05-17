#pragma once
#include "core/vath/vathUtility.h"
#include "core/valueTypes.h"
#include "core/containers/array.h"

namespace dxray
{
	/*!
	 * @brief Acceptable key types for the sparse set.
	 */
	template<typename ValueType>
	concept SparseSetKeyType = std::same_as<ValueType, u16> || std::same_as<ValueType, u32> || std::same_as<ValueType, u64>;

	/*!
	* @brief The iterator used to traverse the sparse set. Expects an STL compatible container.
	* Internally this traverses the dense array in the sparse set in reverse order,
	* thereby allowing insertions/removals to occur without the invalidation of this iterator.
	*/
	template<typename Container>
	class SparseSetIterator final
	{
	public:
		using iterator_concept [[maybe_unused]] = std::contiguous_iterator_tag;
		using difference_type = typename Container::difference_type;
		using value_type = typename Container::value_type;
		using pointer = typename Container::const_pointer;
		using reference = typename Container::const_reference;

		SparseSetIterator() noexcept :
			m_container(nullptr),
			m_offset(0)
		{ }

		explicit SparseSetIterator(const Container& a_container, const value_type a_offset = 0) noexcept :
			m_container(&a_container),
			m_offset(a_offset)
		{ }

		~SparseSetIterator() = default;

		[[nodiscard]] constexpr reference operator[](difference_type a_idx) const noexcept
		{
			return (*m_container)[m_offset];
		}

		[[nodiscard]] constexpr reference operator*() const noexcept
		{
			return operator[](m_offset);
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept
		{
			return &operator[](m_offset);
		}

		constexpr SparseSetIterator& operator++() noexcept
		{
			--m_offset;
			return *this;
		}

		constexpr SparseSetIterator operator++(int) noexcept
		{
			SparseSetIterator tmp = *this;
			++(*this);
			return tmp;
		}

		constexpr SparseSetIterator& operator+=(int a_idx) noexcept
		{
			m_offset -= a_idx;
			return *this;
		}

		[[nodiscard]] constexpr SparseSetIterator operator+(const difference_type a_rhs) const noexcept
		{
			return SparseSetIterator(*m_container, m_offset - a_rhs);
		}

		[[nodiscard]] constexpr friend SparseSetIterator operator+(const difference_type a_value, const SparseSetIterator& a_rhs) noexcept
		{
			return SparseSetIterator(m_container, a_rhs - a_value);
		}

		constexpr SparseSetIterator& operator--() noexcept
		{
			++m_offset;
			return *this;
		}

		constexpr SparseSetIterator operator--(int) noexcept
		{
			SparseSetIterator tmp = *this;
			--(*this);
			return tmp;
		}

		constexpr SparseSetIterator& operator-=(int a_idx) noexcept
		{
			m_offset += a_idx;
			return *this;
		}

		[[nodiscard]] constexpr difference_type operator-(const SparseSetIterator& a_rhs) const noexcept
		{
			return static_cast<difference_type>(m_offset - a_rhs.m_offset);
		}

		[[nodiscard]] constexpr SparseSetIterator operator-(const difference_type a_value) const noexcept
		{
			return SparseSetIterator(*m_container, m_offset + a_value);
		}

		[[nodiscard]] constexpr friend SparseSetIterator operator-(const difference_type a_value, const SparseSetIterator& a_rhs) noexcept
		{
			return SparseSetIterator(a_rhs - a_value);
		}

		[[nodiscard]] constexpr bool operator==(const SparseSetIterator& a_rhs) const noexcept
		{
			return m_offset == a_rhs.m_offset;
		}

		[[nodiscard]] constexpr auto operator<=>(const SparseSetIterator&) const = default;

	private:
		const Container* m_container;
		value_type m_offset;
	};


	/*!
	 * @brief Resize-able sparse set implementation.
	 * @tparam KeyType: The handle type used to store map the values indices to.
	 * @tparam PageSize: The page size of the sparse array, default: 4096.
	 * 
	 * Differs from usual sparse set, optimizations taken from: ECS back and forth - Part 9 - Sparse sets and EnTT
	 * - Uses an invalidations value for faster lookups.
	 * - Uses a reverse iterator to allow insertions/removals to be made without invaliding the iterators.
	 * - Uses pagination to minimize allocations/frees.
	 */
	template<SparseSetKeyType KeyType, usize PageSize = 4096>
	class SparseSet
	{
	public:
		using key_type = KeyType;
		using size_type = usize;

	private:
		static constexpr const size_type InvalidKey = std::numeric_limits<KeyType>::max();
		using SparsePage = std::unique_ptr<key_type[]>;
		using DenseArray = Array<key_type>;
		using SparseArray = Array<SparsePage>;

	public:
		using iterator = SparseSetIterator<DenseArray>;
		using const_iterator = iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static_assert(std::contiguous_iterator<SparseSetIterator<DenseArray>>);

	private:
		[[nodiscard]] constexpr inline size_type PageIndex(const key_type a_keyId) const noexcept
		{
			return a_keyId / PageSize;
		}

		[[nodiscard]] constexpr inline size_type PageOffset(const key_type a_keyId) const noexcept
		{
			return a_keyId & (PageSize - 1);
		}

		[[nodiscard]] constexpr inline SparsePage& EnsurePage(const size_type a_pageIndex)
		{
			if (a_pageIndex >= m_pagedSparse.size())
			{
				m_pagedSparse.resize(a_pageIndex + 1);
			}

			if (!m_pagedSparse[a_pageIndex])
			{
				m_pagedSparse[a_pageIndex].reset(new key_type[PageSize]);
				for (usize i = 0; i < PageSize; ++i)
				{
					m_pagedSparse[a_pageIndex][i] = InvalidKey;
				}
			}

			return m_pagedSparse[a_pageIndex];
		}

	public:
		SparseSet() = default;
		virtual ~SparseSet() = default;
		SparseSet(const SparseSet& a_rhs) = delete;
		SparseSet& operator=(const SparseSet& a_rhs) = delete;
		SparseSet(SparseSet&& a_rhs) = default;
		SparseSet& operator=(SparseSet&& a_rhs) = default;

		constexpr void Reserve(const size_type a_numElements)
		{
			m_dense.reserve(a_numElements);
		}

		constexpr void ShrinkCapacityToSize()
		{
			m_pagedSparse.shrink_to_fit();
			m_dense.shrink_to_fit();
		}

		constexpr void Insert(const key_type a_keyId)
		{
			DXRAY_ASSERT(!Contains(a_keyId));

			// Add the keyId to the back of the dense array, as removal does the opposite.
			EnsurePage(PageIndex(a_keyId))[PageOffset(a_keyId)] = m_dense.size();
			m_dense.push_back(a_keyId);
		}

		template<typename Iterator = iterator>
		constexpr void Insert(const Iterator a_beginIt, const Iterator a_endIt)
		{
			for (auto it = a_beginIt; it != a_endIt; it++)
			{
				Insert(*it);
			}
		}

		constexpr void Erase(const key_type a_keyId)
		{
			DXRAY_ASSERT(Contains(a_keyId));

			// Swap the dense last element with the requested one.
			size_type& index = m_pagedSparse[PageIndex(a_keyId)][PageOffset(a_keyId)];
			const key_type backKeyId = m_dense.back();
			std::swap(m_dense.back(), m_dense[index]);
			
			// Update the sparse value to ensure it still points to the just swapped index and invalidate the index that's now located at the back.
			m_pagedSparse[PageIndex(backKeyId)][PageOffset(backKeyId)] = index;
			index = InvalidKey;
			m_dense.pop_back();
		}

		template<typename Iterator = iterator>
		constexpr void Erase(Iterator a_beginIt, Iterator a_endIt)
		{
			for (auto it = a_beginIt; it != a_endIt; it++)
			{
				Erase(*it);
			}
		}

		constexpr void Clear() noexcept
		{
			m_pagedSparse.clear();
			m_dense.clear();
		}

		[[nodiscard]] constexpr bool Contains(const key_type a_keyId) const noexcept
		{
			const size_type pageIdx = PageIndex(a_keyId);
			return pageIdx < m_pagedSparse.size() 
				&& m_pagedSparse[pageIdx] 
				&& m_pagedSparse[PageIndex(a_keyId)][PageOffset(a_keyId)] != InvalidKey;
		}

		[[nodiscard]] constexpr bool IsEmpty() const noexcept
		{
			return m_dense.empty();
		}

		[[nodiscard]] constexpr size_type GetCapacity() const
		{
			return m_dense.capacity();
		}

		[[nodiscard]] constexpr size_type GetSize() const noexcept
		{
			return m_dense.size();
		}

		[[nodiscard]] constexpr const size_type* const GetData() const noexcept
		{
			return m_dense.data();
		}

		[[nodiscard]] constexpr size_type At(const key_type a_keyId) const
		{
			DXRAY_ASSERT(PageIndex(a_keyId) < m_pagedSparse.size());
			DXRAY_ASSERT(PageOffset(a_keyId) < PageSize);

			const size_type Index = m_pagedSparse[PageIndex(a_keyId)][PageOffset(a_keyId)];
			DXRAY_ASSERT(a_keyId != InvalidKey);
			
			return m_dense.at(Index);
		}

		[[nodiscard]] constexpr size_type operator[](const key_type a_keyId) const
		{
			return m_dense[m_pagedSparse[PageIndex(a_keyId)][PageOffset(a_keyId)]];
		}

		[[nodiscard]] constexpr iterator begin() noexcept
		{
			return iterator(m_dense, m_dense.size() - 1);
		}
		
		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return const_iterator(m_dense, m_dense.size() - 1);
		}
		
		[[nodiscard]] constexpr iterator end() noexcept
		{
			return iterator(m_dense, -1);
		}
		
		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return const_iterator(m_dense, -1);
		}
		
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return begin();
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return end();
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept
		{
			return std::make_reverse_iterator(end());
		}
		
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
		{
			return std::make_reverse_iterator(end());
		}
		
		[[nodiscard]] constexpr reverse_iterator rend() noexcept
		{
			return std::make_reverse_iterator(begin());
		}
		
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
		{
			return std::make_reverse_iterator(cbegin());
		}
	
		[[nodiscard]] constexpr const_reverse_iterator rcbegin() const noexcept
		{
			return std::make_reverse_iterator(cend());
		}
		
		[[nodiscard]] constexpr const_reverse_iterator rcend() const noexcept
		{
			return std::make_reverse_iterator(cbegin());
		}
		
	private:
		SparseArray m_pagedSparse;
		DenseArray m_dense;
	};
}