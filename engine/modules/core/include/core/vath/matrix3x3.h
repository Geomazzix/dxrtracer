#pragma once
#include "core/vath/vector3.h"
#include "core/valueTypes.h"
#include "core/debug.h"

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
	/// <summary>
	/// Specialization of a 3x3 matrix, represented by the value type T.
	/// </summary>
	/// <typeparam name="T">Value type of elements.</typeparam>
	template<typename T>
	class Matrix<3, 3, T> final
	{
	public:
		using ColumnType = Vector<3, T>;
		using RowType = Vector<3, T>;

		constexpr Matrix();
		constexpr explicit Matrix(T a_scalar);
		constexpr Matrix(T a_x0, T a_y0, T a_z0, T a_x1, T a_y1, T a_z1, T a_x2, T a_y2, T a_z2);
		constexpr Matrix(T* a_pData);
		constexpr Matrix(const ColumnType& a_col0, const ColumnType& a_col1, const ColumnType& a_col2);
		constexpr Matrix(const Matrix<2, 2, T>& a_innerMatrix);
		~Matrix() = default;

		constexpr Matrix<3, 3, T>(const Matrix<3, 3, T>& a_rhs) = default;
		constexpr Matrix<3, 3, T>& operator=(const Matrix<3, 3, T>& a_rhs) = default;

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

		constexpr usize GetColumnCount() const;
		constexpr usize GetRowCount() const;

		constexpr Matrix& operator +=(const Matrix& a_rhs);
		constexpr Matrix& operator -=(const Matrix& a_rhs);
		constexpr Matrix& operator *=(const Matrix& a_rhs);
		constexpr Matrix& operator *=(const T a_scalar);

	private:
		ColumnType m_data[3];
	};

	//--- Matrix construction/destruction ---

	template<typename T>
	constexpr Matrix<3, 3, T>::Matrix() :
		m_data{ ColumnType(1, 0, 0), ColumnType(0, 1, 0), ColumnType(0, 0, 1)}
	{}

	template<typename T>
	constexpr Matrix<3, 3, T>::Matrix(T a_scalar) :
		m_data{ ColumnType(a_scalar), ColumnType(a_scalar), ColumnType(a_scalar) }
	{}

	template<typename T>
	constexpr Matrix<3, 3, T>::Matrix(T a_x0, T a_y0, T a_z0, T a_x1, T a_y1, T a_z1, T a_x2, T a_y2, T a_z2) :
		m_data{ ColumnType(a_x0, a_y0, a_z0), ColumnType(a_x1, a_y1, a_z1), ColumnType(a_x2, a_y2, a_z2) }
	{}

	template<typename T>
	constexpr Matrix<3, 3, T>::Matrix(T* a_pData)
	{
		memcpy(m_data, a_pData, GetColumnCount() * GetRowCount() * sizeof(T));
	}

	template<typename T>
	constexpr Matrix<3, 3, T>::Matrix(const ColumnType& a_col0, const ColumnType& a_col1, const ColumnType& a_col2) :
		m_data{ ColumnType(a_col0), ColumnType{a_col1}, ColumnType{a_col2} }
	{}

	template<typename T>
	constexpr Matrix<3, 3, T>::Matrix(const Matrix<2, 2, T>& a_innerMatrix) :
		m_data{ ColumnType(a_innerMatrix[0][0], a_innerMatrix[0][1], 0), ColumnType(a_innerMatrix[1][0], a_innerMatrix[1][1], 0), ColumnType(0, 0, 1) }
	{}


	//--- Matrix getters/setters ---

	template<typename T>
	constexpr usize Matrix<3, 3, T>::GetColumnCount() const
	{
		return 3;
	}

	template<typename T>
	constexpr usize Matrix<3, 3, T>::GetRowCount() const
	{
		return 3;
	}


	//--- Matrix class operators ---

	template<typename T>
	constexpr Matrix<3, 3, T>& Matrix<3, 3, T>::operator+=(const Matrix<3, 3, T>& a_rhs)
	{
		m_data[0] += a_rhs[0];
		m_data[1] += a_rhs[1];
		m_data[2] += a_rhs[2];
		return *this;
	}

	template<typename T>
	constexpr Matrix<3, 3, T>& Matrix<3, 3, T>::operator-=(const Matrix<3, 3, T>& a_rhs)
	{
		m_data[0] -= a_rhs[0];
		m_data[1] -= a_rhs[1];
		m_data[2] -= a_rhs[2];
		return *this;
	}

	template<typename T>
	constexpr Matrix<3, 3, T>& Matrix<3, 3, T>::operator*=(const Matrix<3, 3, T>& a_rhs)
	{
		return (*this = a_rhs * (*this));
	}

	template<typename T>
	constexpr Matrix<3, 3, T>& Matrix<3, 3, T>::operator*=(const T a_scalar)
	{
		m_data[0] *= a_scalar;
		m_data[1] *= a_scalar;
		m_data[2] *= a_scalar;
		return *this;
	}


	//--- Matrix comparison operators ---

	template<typename T>
	constexpr bool operator==(const Matrix<3, 3, T>& a_lhs, const Matrix<3, 3, T>& a_rhs)
	{
		return a_lhs[0] == a_rhs[0] && a_lhs[1] == a_rhs[1] && a_lhs[2] == a_rhs[2];
	}

	template<typename T>
	constexpr bool operator!=(const Matrix<3, 3, T>& a_lhs, const Matrix<3, 3, T>& a_rhs)
	{
		return !(a_lhs == a_rhs);
	}


	//--- Matrix global operators ---

	template<typename T>
	constexpr Matrix<3, 3, T> operator+(const Matrix<3, 3, T>& a_lhs, const Matrix<3, 3, T>& a_rhs)
	{
		return Matrix<3, 3, T>(
			a_lhs[0] + a_rhs[0],
			a_lhs[1] + a_rhs[1],
			a_lhs[2] + a_rhs[2]
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> operator-(const Matrix<3, 3, T>& a_lhs, const Matrix<3, 3, T>& a_rhs)
	{
		return Matrix<3, 3, T>(
			a_lhs[0] - a_rhs[0],
			a_lhs[1] - a_rhs[1],
			a_lhs[2] - a_rhs[2]
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> operator*(const Matrix<3, 3, T>& a_lhs, const Matrix<3, 3, T>& a_rhs)
	{
		return Matrix<3, 3, T>(
			//Col0
			a_lhs[0][0] * a_rhs[0][0] + a_lhs[1][0] * a_rhs[0][1] + a_lhs[2][0] * a_rhs[0][2],
			a_lhs[0][1] * a_rhs[0][0] + a_lhs[1][1] * a_rhs[0][1] + a_lhs[2][1] * a_rhs[0][2],
			a_lhs[0][2] * a_rhs[0][0] + a_lhs[1][2] * a_rhs[0][1] + a_lhs[2][2] * a_rhs[0][2],

			//Col1
			a_lhs[0][0] * a_rhs[1][0] + a_lhs[1][0] * a_rhs[1][1] + a_lhs[2][0] * a_rhs[1][2],
			a_lhs[0][1] * a_rhs[1][0] + a_lhs[1][1] * a_rhs[1][1] + a_lhs[2][1] * a_rhs[1][2],
			a_lhs[0][2] * a_rhs[1][0] + a_lhs[1][2] * a_rhs[1][1] + a_lhs[2][2] * a_rhs[1][2],
			
			//Col2
			a_lhs[0][0] * a_rhs[2][0] + a_lhs[1][0] * a_rhs[2][1] + a_lhs[2][0] * a_rhs[2][2],
			a_lhs[0][1] * a_rhs[2][0] + a_lhs[1][1] * a_rhs[2][1] + a_lhs[2][1] * a_rhs[2][2],
			a_lhs[0][2] * a_rhs[2][0] + a_lhs[1][2] * a_rhs[2][1] + a_lhs[2][2] * a_rhs[2][2]
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> operator*(const Matrix<3, 3, T>& a_lhs, const T a_scalar)
	{
		return Matrix<3, 3, T>(
			a_lhs[0] * a_scalar,
			a_lhs[1] * a_scalar,
			a_lhs[2] * a_scalar
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> operator*(const T a_scalar, const Matrix<3, 3, T>& a_lhs)
	{
		return Matrix<3, 3, T>(
			a_lhs[0] * a_scalar,
			a_lhs[1] * a_scalar,
			a_lhs[2] * a_scalar
		);
	}

	template<typename T>
	constexpr Vector<3, T> operator*(const Matrix<3, 3, T>& a_lhs, const Vector<3, T>& a_vector)
	{
		return Vector<3, T>(
			a_lhs[0][0] * a_vector[0] + a_lhs[0][1] * a_vector[1] + a_lhs[0][2] * a_vector[2],
			a_lhs[1][0] * a_vector[0] + a_lhs[1][1] * a_vector[1] + a_lhs[1][2] * a_vector[2],
			a_lhs[2][0] * a_vector[0] + a_lhs[2][1] * a_vector[1] + a_lhs[2][2] * a_vector[2]
		);
	}


	//--- Matrix applicable functionality ---

	template<typename T>
	constexpr Matrix<3, 3, T> Transpose(const Matrix<3, 3, T>& a_rhs)
	{
		return Matrix<3, 3, T>(
			a_rhs[0][0], a_rhs[1][0], a_rhs[2][0],
			a_rhs[0][1], a_rhs[1][1], a_rhs[2][1],
			a_rhs[0][2], a_rhs[1][2], a_rhs[2][2]
		);
	}

	template<typename T>
	constexpr T Determinant(const Matrix<3, 3, T>& a_rhs)
	{
		return a_rhs[0][0] * (a_rhs[1][1] * a_rhs[2][2] - a_rhs[2][1] * a_rhs[1][2])
			- a_rhs[1][0] * (a_rhs[0][1] * a_rhs[2][2] - a_rhs[2][1] * a_rhs[0][2])
			+ a_rhs[2][0] * (a_rhs[0][1] * a_rhs[1][2] - a_rhs[1][1] * a_rhs[0][2]);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Inverse(const Matrix<3, 3, T>& a_rhs)
	{
		//#Source: Foundations of game engine development volume 1, chapter 1.7.
		//Using the triple dot product the determinant can be replaced with a dot product, saving computation.
		const Vector<3, T> c0 = Cross(a_rhs[1], a_rhs[2]);
		const Vector<3, T> c1 = Cross(a_rhs[2], a_rhs[0]);
		const Vector<3, T> c2 = Cross(a_rhs[0], a_rhs[1]);

		const T invDet = static_cast<T>(1) / Dot(c2, a_rhs[2]);

		return Matrix<3, 3, T>(
			c0[0] * invDet, c1[0] * invDet, c2[0] * invDet,
			c0[1] * invDet, c1[1] * invDet, c2[1] * invDet,
			c0[2] * invDet, c1[2] * invDet, c2[2] * invDet
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Translation(const Vector<2, T>& a_translation)
	{
		return Matrix<3, 3, T>(
			1, 0, 0,
			0, 1, 0,
			a_translation[0], a_translation[1], 1
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Scale(const Vector<2, T>& a_scale)
	{
		return Matrix<3, 3, T>(
			a_scale[0], 0, 0,
			0, a_scale[1], 0,
			0, 0, 1
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Rotation(T a_angle, const Vector<3, T>& a_axis)
	{
		const T cosA = std::cos(a_angle);
		const T sinA = std::sin(a_angle);

		const Vector<3, T> axis(a_axis);
		const Vector<3, T> temp(axis * (static_cast<T>(1) - cosA));

		return Matrix<3, 3, T>(
			cosA + temp[0] * axis[0], temp[0] * axis[1] + sinA * axis[2], temp[0] * axis[2] - sinA * axis[1],
			temp[1] * axis[0] - sinA * axis[2], cosA + temp[1] * axis[1], temp[1] * axis[2] + sinA * axis[0],
			temp[2] * axis[0] + sinA * axis[1], temp[2] * axis[1] - sinA * axis[0], cosA + temp[2] * axis[2]
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> RotateX(T a_angle, const Vector<3, T>& a_axis)
	{
		const T c = std::cos(a_angle);
		const T s = std::sin(a_angle);

		return Matrix<3, 3, T>(
			1, 0, 0,
			0, c, -s,
			0, s, c
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> RotateY(T a_angle, const Vector<3, T>& a_axis)
	{
		const T c = std::cos(a_angle);
		const T s = std::sin(a_angle);

		return Matrix<3, 3, T>(
			c, 0, s,
			0, 1, 0,
			-s, 0, c
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> RotateZ(T a_angle, const Vector<3, T>& a_axis)
	{
		const T c = std::cos(a_angle);
		const T s = std::sin(a_angle);

		return Matrix<3, 3, T>(
			c, -s, 0,
			s, c, 0,
			0, 0, 1
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Reflect(const Vector<3, T>& a_vector)
	{
		const Vector<3, T> axis = a_vector * -2.0;
		const Vector<3, T> temp = Vector<3, T>(axis[0] * a_vector[1], axis[0] * a_vector[2], axis[1] * a_vector[2]);

		return Matrix<3, 3, T>(
			axis[0] * a_vector[0] + 1.0, temp[0], temp[1],
			temp[0], axis[1] * a_vector[1] + 1.0, temp[2],
			temp[1], temp[2], axis[2] * a_vector[2] + 1.0
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Skew(T a_angle, const Vector<3, T>& a_direction, const Vector<3, T>& a_orthogonalDirection)
	{
		const Vector<3, T> direction = a_direction * Tan<T>(a_angle);
		return Matrix<3, 3, T>(
			direction[0] * a_orthogonalDirection[0] + 1.0, direction[0] * a_orthogonalDirection[1], direction[0] * a_orthogonalDirection[2],
			direction[1] * a_orthogonalDirection[0], direction[1] * a_orthogonalDirection[1] + 1.0, direction[1] * a_orthogonalDirection[2],
			direction[2] * a_orthogonalDirection[0], direction[2] * a_orthogonalDirection[1], direction[2] * a_orthogonalDirection[2] + 1.0
		);
	}

	template<typename T>
	constexpr Vector<3, T> ToEuler(const Matrix<3, 3, T>& a_rotationMat)
	{
		Vector<3, T> euler;
		
		//Retrieves the euler angle in yaw-pitch-roll order.
		euler[1] = std::asin(a_rotationMat[0][2]);

		//Check for gimbals lock.
		if (Abs<T>(Abs<T>(a_rotationMat[0][2]) - static_cast<T>(1.0)) < 1e-3)
		{
			euler[2] = std::atan2(a_rotationMat[2][1], a_rotationMat[1][1]);
			euler[0] = static_cast<T>(0.0);
		}
		else
		{
			euler[2] = std::atan2(-a_rotationMat[1][2], a_rotationMat[2][2]);
			euler[0] = std::atan2(-a_rotationMat[0][1], a_rotationMat[0][0]);
		}

		return euler;
	}

}

#pragma warning(pop)