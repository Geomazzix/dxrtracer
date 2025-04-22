#pragma once
#include "core/containers/array.h"
#include "core/containers/sparseSet.h"

namespace dxray
{
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
		SparseArray() = default;
		virtual ~SparseArray() = default;
		SparseArray(const SparseArray& a_rhs) = delete;
		SparseArray& operator=(const SparseArray& a_rhs) = delete;
		SparseArray(SparseArray&& a_rhs) = default;
		SparseArray& operator=(SparseArray&& a_rhs) = default;

		template<typename... Args>
		ElementType& Emplace(const key_type a_keyId, Args&&... a_pArgs)
		{
			//if (const size_type Index = EnsurePage(PageIndex(a_keyId))[PageOffset(a_keyId)] != InvalidKey)
			//{
			//	m_keyToDenseMapping[Index] = a_keyId;
			//	m_dense[Index] = ElementType(std::forward<Args>(a_pArgs)...);
			//	return m_dense[Index];
			//}
			//
			//m_pagedSparse[PageIndex(a_keyId)][PageOffset(a_keyId)] = m_dense.size();
			//m_keyToDenseMapping.push_back(a_keyId);
			//m_dense.push_back(ElementType(std::forward<Args>(a_pArgs)...));
			//return m_dense.back();
			return m_elements.back();
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