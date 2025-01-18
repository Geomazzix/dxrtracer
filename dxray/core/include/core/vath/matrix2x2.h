#pragma once
#include "core/vath/vathTemplate.h"
#include "core/debug.h"

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
	/// <summary>
	/// Specialization of a 2x2 matrix, represented by the value type T.
	/// </summary>
	/// <typeparam name="T">Value type of elements.</typeparam>
	template<typename T>
	class Matrix<2, 2, T> final
	{
	public:
		typedef Vector<2, T> ColumnType;
		typedef Vector<2, T> RowType;

		Matrix();
		explicit Matrix(T a_scalar);
		Matrix(T a_x0, T a_y0, T a_x1, T a_y1);
		Matrix(T* a_pData);
		Matrix(const ColumnType& a_col0, const ColumnType& a_col1);
		~Matrix() = default;

		constexpr Matrix<2, 2, T>(const Matrix<2, 2, T>& a_rhs) = default;
		constexpr Matrix<2, 2, T>& operator=(const Matrix<2, 2, T>& a_rhs) = default;

		ColumnType& operator[](usize i)
		{
			DXRAY_ASSERT(i < GetColumnCount() * GetRowCount());
			return m_data[i];
		}

		const ColumnType& operator[](usize i) const
		{
			DXRAY_ASSERT(i < GetColumnCount() * GetRowCount());
			return m_data[i];
		}

		usize GetColumnCount() const;
		usize GetRowCount() const;

		constexpr Matrix& operator +=(const Matrix& a_rhs);
		constexpr Matrix& operator -=(const Matrix& a_rhs);
		constexpr Matrix& operator *=(const Matrix& a_rhs);
		constexpr Matrix& operator *=(float a_scalar);
	
	private:
		ColumnType m_data[2];
	};


	//--- Matrix construction/destruction ---

	template<typename T>
	Matrix<2, 2, T>::Matrix() :
		m_data{ ColumnType(1, 0),ColumnType(0, 1) }
	{}

	template<typename T>
	Matrix<2, 2, T>::Matrix(T a_scalar) :
		m_data{ ColumnType(a_scalar), ColumnType(a_scalar) }
	{}
	
	template<typename T>
	Matrix<2, 2, T>::Matrix(T a_x0, T a_y0, T a_x1, T a_y1) :
		m_data{ ColumnType(a_x0, a_y0), ColumnType(a_x1, a_y1) }
	{}

	template<typename T>
	Matrix<2, 2, T>::Matrix(T* a_pData)
	{
		memcpy(m_data, a_pData, GetColumnCount() * GetRowCount() * sizeof(T));
	}

	template<typename T>
	Matrix<2, 2, T>::Matrix(const ColumnType& a_col0, const ColumnType& a_col1) :
		m_data{ ColumnType(a_col0), ColumnType{a_col1} }
	{}


	//--- Matrix getters/setters ---

	template<typename T>
	usize Matrix<2, 2, T>::GetColumnCount() const
	{
		return 2;
	}

	template<typename T>
	usize Matrix<2, 2, T>::GetRowCount() const
	{
		return 2;
	}


	//--- Matrix class operators ---

	template<typename T>
	constexpr Matrix<2, 2, T>& Matrix<2, 2, T>::operator+=(const Matrix<2, 2, T>& a_rhs)
	{
		m_data[0] += a_rhs[0];
		m_data[1] += a_rhs[1];
		return *this;
	}

	template<typename T>
	constexpr Matrix<2, 2, T>& Matrix<2, 2, T>::operator-=(const Matrix<2, 2, T>& a_rhs)
	{
		m_data[0] -= a_rhs[0];
		m_data[1] -= a_rhs[1];
		return *this;
	}

	template<typename T>
	constexpr Matrix<2, 2, T>& Matrix<2, 2, T>::operator*=(const Matrix<2, 2, T>& a_rhs)
	{
		return (*this = a_rhs * (*this));
	}

	template<typename T>
	constexpr Matrix<2, 2, T>& Matrix<2, 2, T>::operator*=(float a_scalar)
	{
		m_data[0] *= a_scalar;
		m_data[1] *= a_scalar;
		return *this;
	}


	//--- Matrix comparison operators ---

	template<typename T>
	constexpr bool operator==(const Matrix<2, 2, T>& a_lhs, const Matrix<2, 2, T>& a_rhs)
	{
		return a_lhs[0] == a_rhs[0] && a_lhs[1] == a_rhs[1];
	}

	template<typename T>
	constexpr bool operator!=(const Matrix<2, 2, T>& a_lhs, const Matrix<2, 2, T>& a_rhs)
	{
		return !(a_lhs == a_rhs);
	}


	//--- Matrix global operators ---

	template<typename T>
	constexpr Matrix<2, 2, T> operator+(const Matrix<2, 2, T>& a_lhs, const Matrix<2, 2, T>& a_rhs)
	{
		return Matrix<2, 2, T>(
			a_lhs[0] + a_rhs[0],
			a_lhs[1] + a_rhs[1]
		);
	}

	template<typename T>
	constexpr Matrix<2, 2, T> operator-(const Matrix<2, 2, T>& a_lhs, const Matrix<2, 2, T>& a_rhs)
	{
		return Matrix<2, 2, T>(
			a_lhs[0] - a_rhs[0],
			a_lhs[1] - a_rhs[1]
		);
	}
	
	template<typename T>
	constexpr Matrix<2, 2, T> operator*(const Matrix<2, 2, T>& a_lhs, const Matrix<2, 2, T>& a_rhs)
	{
		return Matrix<2, 2, T>(
			//Col0
			a_lhs[0][0] * a_rhs[0][0] + a_lhs[1][0] * a_rhs[0][1],
			a_lhs[0][1] * a_rhs[0][0] + a_lhs[1][1] * a_rhs[0][1],
			
			//Col1
			a_lhs[0][0] * a_rhs[1][0] + a_lhs[1][0] * a_rhs[1][1],
			a_lhs[0][1] * a_rhs[1][0] + a_lhs[1][1] * a_rhs[1][1]
		);
	}
	
	template<typename T>
	constexpr Matrix<2, 2, T> operator*(const Matrix<2, 2, T>& a_lhs, float a_scalar)
	{
		return Matrix<2, 2, T>(
			a_lhs[0] * a_scalar,
			a_lhs[1] * a_scalar
		);
	}

	template<typename T>
	constexpr Vector<2, T> operator*(const Matrix<2, 2, T>& a_lhs, const Vector<2, T>& a_vector)
	{
		return Vector<2, T>(
			a_lhs[0][0] * a_vector[0] + a_lhs[0][1] * a_vector[1],
			a_lhs[1][0] * a_vector[0] + a_lhs[1][1] * a_vector[1]
		);
	}
		

	//--- Matrix applicable functionality ---

	template<typename T>
	constexpr Matrix<2, 2, T> Transpose(const Matrix<2, 2, T>& a_rhs) 
	{
		return Matrix<2, 2, T>(
			a_rhs[0][0], a_rhs[1][0],
			a_rhs[0][1], a_rhs[1][1]
		);
	}
	
	template<typename T>
	constexpr T Determinant(const Matrix<2, 2, T>& a_rhs)
	{
		return a_rhs[0][0] * a_rhs[1][1] - a_rhs[0][1] * a_rhs[1][0];
	}
	
	template<typename T>
	constexpr Matrix<2, 2, T> Inverse(const Matrix<2, 2, T>& a_rhs)
	{
		//#Source: Foundations of game engine development volume 1, chapter 1.7.
		Matrix<2, 2, T> adjointMatrix = Matrix<2, 2, T>(
			a_rhs[1][1],
			-a_rhs[0][1],
			-a_rhs[1][0],
			a_rhs[0][0]
		);

		T invDet = static_cast<T>(1) / Determinant(a_rhs);
		return adjointMatrix * invDet;
	}

	template<typename T>
	constexpr Matrix<2, 2, T> Rotation(T a_angle)
	{
		const T c = std::cos(a_angle);
		const T s = std::sin(a_angle);

		return Matrix<2, 2, T>(
			c, -s,
			s, c
		);
	}

}

#pragma warning(pop)