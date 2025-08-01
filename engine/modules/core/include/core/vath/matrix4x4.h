#pragma once
#include "core/vath/vathTemplate.h"
#include "core/vath/matrix3x3.h"
#include "core/vath/vector3.h"
#include "core/debug.h"

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
	/**
	 * @brief Specialization of a 4x4 matrix, represented by the value type T.
	 * @tparam T Value type of elements.
	 */
	template<typename T>
	class Matrix<4, 4, T>
	{
	public:
		using ColumnType = Vector<4, T>;
		using RowType = Vector<4, T>;

		constexpr Matrix();
		constexpr explicit Matrix(T a_scalar);
		constexpr Matrix(T a_x0, T a_y0, T a_z0, T a_w0, T a_x1, T a_y1, T a_z1, T a_w1, T a_x2, T a_y2, T a_z2, T a_w2, T a_x3, T a_y3, T a_z3, T a_w3);
		constexpr Matrix(T* a_pData);
		constexpr Matrix(const ColumnType& a_col0, const ColumnType& a_col1, const ColumnType& a_col2, const ColumnType& a_col3);
		constexpr Matrix(const Matrix<3, 3, T>& a_innerMatrix);
		~Matrix() = default;

		constexpr Matrix<4, 4, T>(const Matrix<4, 4, T>& a_rhs) = default;
		constexpr Matrix<4, 4, T>& operator=(const Matrix<4, 4, T>& a_rhs) = default;

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

		constexpr Matrix<3, 3, T> GetInner() const;

		constexpr usize GetColumnCount() const;
		constexpr usize GetRowCount() const;

		constexpr Matrix& operator +=(const Matrix& a_rhs);
		constexpr Matrix& operator -=(const Matrix& a_rhs);
		constexpr Matrix& operator *=(const Matrix& a_rhs);
		constexpr Matrix& operator *=(const T a_scalar);

	private:
		ColumnType m_data[4];
	};


	//--- Matrix definitions ---

	using Matrix4x4u8 = Matrix<4, 4, u8>;
	using Matrix4x4u16 = Matrix<4, 4, u16>;
	using Matrix4x4u32 = Matrix<4, 4, u32>;
	using Matrix4x4u64 = Matrix<4, 4, u64>;
	using Matrix4x4i8 = Matrix<4, 4, i8>;
	using Matrix4x4i16 = Matrix<4, 4, i16>;
	using Matrix4x4i32 = Matrix<4, 4, i32>;
	using Matrix4x4i64 = Matrix<4, 4, i64>;
	using Matrix4x4i32 = Matrix<4, 4, i32>;
	using Matrix4x4f = Matrix<4, 4, fp32>;
	using Matrix4x4d = Matrix<4, 4, fp64>;
	using Matrix4x4 = Matrix4x4f;


	//--- Matrix construction/destruction ---

	template<typename T>
	constexpr Matrix<4, 4, T>::Matrix() :
		m_data{ ColumnType(1, 0, 0, 0), ColumnType(0, 1, 0, 0), ColumnType(0, 0, 1, 0), ColumnType(0, 0, 0, 1)}
	{}

	template<typename T>
	constexpr Matrix<4, 4, T>::Matrix(T a_scalar) :
		m_data{ ColumnType(a_scalar), ColumnType(a_scalar), ColumnType(a_scalar), ColumnType(a_scalar)}
	{}

	template<typename T>
	constexpr Matrix<4, 4, T>::Matrix(T a_x0, T a_y0, T a_z0, T a_w0, T a_x1, T a_y1, T a_z1, T a_w1, T a_x2, T a_y2, T a_z2, T a_w2, T a_x3, T a_y3, T a_z3, T a_w3) :
		m_data
		{ 
			ColumnType(a_x0, a_y0, a_z0, a_w0), 
			ColumnType(a_x1, a_y1, a_z1, a_w1), 
			ColumnType(a_x2, a_y2, a_z2, a_w2), 
			ColumnType(a_x3, a_y3, a_z3, a_w3) 
		}
	{}

	template<typename T>
	constexpr Matrix<4, 4, T>::Matrix(T* a_pData)
	{
		memcpy(m_data, a_pData, GetColumnCount() * GetRowCount() * sizeof(T));
	}

	template<typename T>
	constexpr Matrix<4, 4, T>::Matrix(const ColumnType& a_col0, const ColumnType& a_col1, const ColumnType& a_col2, const ColumnType& a_col3) :
		m_data{ ColumnType(a_col0), ColumnType{a_col1}, ColumnType{a_col2}, ColumnType{a_col3} }
	{}

	template<typename T>
	constexpr Matrix<4, 4, T>::Matrix(const Matrix<3, 3, T>& a_innerMatrix) :
		m_data{ ColumnType(a_innerMatrix[0][0], a_innerMatrix[0][1], a_innerMatrix[0][2], 0), 
				ColumnType(a_innerMatrix[1][0], a_innerMatrix[1][1], a_innerMatrix[1][2], 0), 
				ColumnType(a_innerMatrix[2][0], a_innerMatrix[2][1], a_innerMatrix[2][2], 0), 
				ColumnType(0, 0, 0, 1)}
	{}


	//--- Matrix getters/setters ---

	template<typename T>
	constexpr usize Matrix<4, 4, T>::GetColumnCount() const
	{
		return 4;
	}

	template<typename T>
	constexpr usize Matrix<4, 4, T>::GetRowCount() const
	{
		return 4;
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Matrix<4, 4, T>::GetInner() const
	{
		return Matrix<3, 3, T>(
			m_data[0][0], m_data[0][0], m_data[0][0],
			m_data[1][0], m_data[1][1], m_data[1][2],
			m_data[2][0], m_data[2][1], m_data[2][2]
		);
	}


	//--- Matrix class operators ---

	template<typename T>
	constexpr Matrix<4, 4, T>& Matrix<4, 4, T>::operator+=(const Matrix<4, 4, T>& a_rhs)
	{
		m_data[0] += a_rhs[0];
		m_data[1] += a_rhs[1];
		m_data[2] += a_rhs[2];
		m_data[3] += a_rhs[3];
		return *this;
	}

	template<typename T>
	constexpr Matrix<4, 4, T>& Matrix<4, 4, T>::operator-=(const Matrix<4, 4, T>& a_rhs)
	{
		m_data[0] -= a_rhs[0];
		m_data[1] -= a_rhs[1];
		m_data[2] -= a_rhs[2];
		m_data[3] -= a_rhs[3];
		return *this;
	}

	template<typename T>
	constexpr Matrix<4, 4, T>& Matrix<4, 4, T>::operator*=(const Matrix<4, 4, T>& a_rhs)
	{
		return (*this = a_rhs * (*this));
	}

	template<typename T>
	constexpr Matrix<4, 4, T>& Matrix<4, 4, T>::operator*=(const T a_scalar)
	{
		m_data[0] *= a_scalar;
		m_data[1] *= a_scalar;
		m_data[2] *= a_scalar;
		m_data[3] *= a_scalar;
		return *this;
	}


	//--- Matrix comparison operators ---

	template<typename T>
	constexpr bool operator==(const Matrix<4, 4, T>& a_lhs, const Matrix<4, 4, T>& a_rhs)
	{
		return a_lhs[0] == a_rhs[0] && a_lhs[1] == a_rhs[1] && a_lhs[2] == a_rhs[2] && a_lhs[3] == a_rhs[3];
	}

	template<typename T>
	constexpr bool operator!=(const Matrix<4, 4, T>& a_lhs, const Matrix<4, 4, T>& a_rhs)
	{
		return !(a_lhs == a_rhs);
	}


	//--- Matrix global operators ---

	template<typename T>
	constexpr Matrix<4, 4, T> operator+(const Matrix<4, 4, T>& a_lhs, const Matrix<4, 4, T>& a_rhs)
	{
		return Matrix<4, 4, T>(
			a_lhs[0] + a_rhs[0],
			a_lhs[1] + a_rhs[1],
			a_lhs[2] + a_rhs[2],
			a_lhs[3] + a_rhs[3]
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> operator-(const Matrix<4, 4, T>& a_lhs, const Matrix<4, 4, T>& a_rhs)
	{
		return Matrix<4, 4, T>(
			a_lhs[0] - a_rhs[0],
			a_lhs[1] - a_rhs[1],
			a_lhs[2] - a_rhs[2],
			a_lhs[3] - a_rhs[3]
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> operator*(const Matrix<4, 4, T>& a_lhs, const Matrix<4, 4, T>& a_rhs)
	{
		// A = n x 
		// Forumla: AB[i][j] = SUM(

		return Matrix<4, 4, T>(
			//Col0
			a_lhs[0][0] * a_rhs[0][0] + a_lhs[1][0] * a_rhs[0][1] + a_lhs[2][0] * a_rhs[0][2] + a_lhs[3][0] * a_rhs[0][3],
			a_lhs[0][1] * a_rhs[0][0] + a_lhs[1][1] * a_rhs[0][1] + a_lhs[2][1] * a_rhs[0][2] + a_lhs[3][1] * a_rhs[0][3],
			a_lhs[0][2] * a_rhs[0][0] + a_lhs[1][2] * a_rhs[0][1] + a_lhs[2][2] * a_rhs[0][2] + a_lhs[3][2] * a_rhs[0][3],
			a_lhs[0][3] * a_rhs[0][0] + a_lhs[1][3] * a_rhs[0][1] + a_lhs[2][3] * a_rhs[0][2] + a_lhs[3][3] * a_rhs[0][3],

			//Col1
			a_lhs[0][0] * a_rhs[1][0] + a_lhs[1][0] * a_rhs[1][1] + a_lhs[2][0] * a_rhs[1][2] + a_lhs[3][0] * a_rhs[1][3],
			a_lhs[0][1] * a_rhs[1][0] + a_lhs[1][1] * a_rhs[1][1] + a_lhs[2][1] * a_rhs[1][2] + a_lhs[3][1] * a_rhs[1][3],
			a_lhs[0][2] * a_rhs[1][0] + a_lhs[1][2] * a_rhs[1][1] + a_lhs[2][2] * a_rhs[1][2] + a_lhs[3][2] * a_rhs[1][3],
			a_lhs[0][3] * a_rhs[1][0] + a_lhs[1][3] * a_rhs[1][1] + a_lhs[2][3] * a_rhs[1][2] + a_lhs[3][3] * a_rhs[1][3],

			//Col2
			a_lhs[0][0] * a_rhs[2][0] + a_lhs[1][0] * a_rhs[2][1] + a_lhs[2][0] * a_rhs[2][2] + a_lhs[3][0] * a_rhs[2][3],
			a_lhs[0][1] * a_rhs[2][0] + a_lhs[1][1] * a_rhs[2][1] + a_lhs[2][1] * a_rhs[2][2] + a_lhs[3][1] * a_rhs[2][3],
			a_lhs[0][2] * a_rhs[2][0] + a_lhs[1][2] * a_rhs[2][1] + a_lhs[2][2] * a_rhs[2][2] + a_lhs[3][2] * a_rhs[2][3],
			a_lhs[0][3] * a_rhs[2][0] + a_lhs[1][3] * a_rhs[2][1] + a_lhs[2][3] * a_rhs[2][2] + a_lhs[3][3] * a_rhs[2][3],
		
			//Col3
			a_lhs[0][0] * a_rhs[3][0] + a_lhs[1][0] * a_rhs[3][1] + a_lhs[2][0] * a_rhs[3][2] + a_lhs[3][0] * a_rhs[3][3],
			a_lhs[0][1] * a_rhs[3][0] + a_lhs[1][1] * a_rhs[3][1] + a_lhs[2][1] * a_rhs[3][2] + a_lhs[3][1] * a_rhs[3][3],
			a_lhs[0][2] * a_rhs[3][0] + a_lhs[1][2] * a_rhs[3][1] + a_lhs[2][2] * a_rhs[3][2] + a_lhs[3][2] * a_rhs[3][3],
			a_lhs[0][3] * a_rhs[3][0] + a_lhs[1][3] * a_rhs[3][1] + a_lhs[2][3] * a_rhs[3][2] + a_lhs[3][3] * a_rhs[3][3]
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> operator*(const Matrix<4, 4, T>& a_lhs, const T a_scalar)
	{
		return Matrix<4, 4, T>(
			a_lhs[0] * a_scalar,
			a_lhs[1] * a_scalar,
			a_lhs[2] * a_scalar,
			a_lhs[3] * a_scalar
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> operator*(const T a_scalar, const Matrix<4, 4, T>& a_lhs)
	{
		return Matrix<4, 4, T>(
			a_lhs[0] * a_scalar,
			a_lhs[1] * a_scalar,
			a_lhs[2] * a_scalar,
			a_lhs[3] * a_scalar
		);
	}

	template<typename T>
	constexpr Vector<4, T> operator*(const Matrix<4, 4, T>& a_lhs, const Vector<4, T>& a_vector)
	{
		return Vector<4, T>(
			a_lhs[0][0] * a_vector[0] + a_lhs[0][1] * a_vector[1] + a_lhs[0][2] * a_vector[2] + a_lhs[0][3] * a_vector[3],
			a_lhs[1][0] * a_vector[0] + a_lhs[1][1] * a_vector[1] + a_lhs[1][2] * a_vector[2] + a_lhs[1][3] * a_vector[3],
			a_lhs[2][0] * a_vector[0] + a_lhs[2][1] * a_vector[1] + a_lhs[2][2] * a_vector[2] + a_lhs[2][3] * a_vector[3],
			a_lhs[3][0] * a_vector[0] + a_lhs[3][1] * a_vector[1] + a_lhs[3][2] * a_vector[2] + a_lhs[3][3] * a_vector[3]
		);
	}


	//--- Matrix applicable functionality ---

	template<typename T>
	constexpr Matrix<4, 4, T> Transpose(const Matrix<4, 4, T>& a_rhs)
	{
		return Matrix<4, 4, T>(
			a_rhs[0][0], a_rhs[1][0], a_rhs[2][0], a_rhs[3][0],
			a_rhs[0][1], a_rhs[1][1], a_rhs[2][1], a_rhs[3][1],
			a_rhs[0][2], a_rhs[1][2], a_rhs[2][2], a_rhs[3][2],
			a_rhs[0][3], a_rhs[1][3], a_rhs[2][3], a_rhs[3][3]
		);
	}

	template<typename T>
	constexpr T Determinant(const Matrix<4, 4, T>& a_rhs)
	{
		//2x2 determinants.
		const T Coef0 = a_rhs[2][2] * a_rhs[3][3] - a_rhs[3][2] * a_rhs[2][3];
		const T Coef1 = a_rhs[1][2] * a_rhs[3][3] - a_rhs[3][2] * a_rhs[1][3];
		const T Coef2 = a_rhs[1][2] * a_rhs[2][3] - a_rhs[2][2] * a_rhs[1][3];
		const T Coef3 = a_rhs[0][2] * a_rhs[3][3] - a_rhs[3][2] * a_rhs[0][3];
		const T Coef4 = a_rhs[0][2] * a_rhs[2][3] - a_rhs[2][2] * a_rhs[0][3];
		const T Coef5 = a_rhs[0][2] * a_rhs[1][3] - a_rhs[1][2] * a_rhs[0][3];

		//4x4 determinant (calculating 3x3 inline)
		return a_rhs[0][0] * (a_rhs[1][1] * Coef0 - a_rhs[2][1] * Coef1 + a_rhs[3][1] * Coef2)
			- a_rhs[1][0] * (a_rhs[0][1] * Coef0 - a_rhs[2][1] * Coef3 + a_rhs[3][1] * Coef4) 
			+ a_rhs[2][0] * (a_rhs[0][1] * Coef1 - a_rhs[1][1] * Coef3 + a_rhs[3][1] * Coef5)
			- a_rhs[3][0] * (a_rhs[0][1] * Coef2 - a_rhs[1][1] * Coef4 + a_rhs[2][1] * Coef5);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> Inverse(const Matrix<4, 4, T>& a_rhs)
	{
		//#Source: Foundations of game engine development volume 1, chapter 1.7.

		// Take the top 3 row column vectors (3 components).
		const Vector<3, T>& a = a_rhs[0];
		const Vector<3, T>& b = a_rhs[1];
		const Vector<3, T>& c = a_rhs[2];
		const Vector<3, T>& d = a_rhs[3];

		// Take the remaining row.
		const T& x = a_rhs[0][3];
		const T& y = a_rhs[1][3];
		const T& z = a_rhs[2][3];
		const T& w = a_rhs[3][3];

		// Define the following vectors to determine the determinant.
		Vector<3, T> s = Cross(a, b);
		Vector<3, T> t = Cross(c, d);
		Vector<3, T> u = a * y - b * x;
		Vector<3, T> v = c * w - d * z;
		
		const T invDet = static_cast<T>(1) / (Dot(s, v) + Dot(t, u));	
		s *= invDet;
		t *= invDet;
		u *= invDet;
		v *= invDet;

		const Vector<3, T> r0 = Cross(b, v) + t * y;
		const Vector<3, T> r1 = Cross(v, a) - t * x;
		const Vector<3, T> r2 = Cross(d, u) + s * w;
		const Vector<3, T> r3 = Cross(u, c) - s * z;

		return Matrix<4, 4, T>(
			r0.x, r1.x, r2.x, r3.x,
			r0.y, r1.y, r2.y, r3.y,
			r0.z, r1.z, r2.z, r3.z,
			-Dot(b, t), Dot(a, t), -Dot(d, s), Dot(c, s)
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> Translation(const Vector<3, T>& a_translation)
	{
		return Matrix<4, 4, T>(
			1, 0, 0, a_translation[0],
			0, 1, 0, a_translation[1],
			0, 0, 1, a_translation[2],
			0, 0, 0, 1
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> Scale(const Vector<3, T>& a_scale)
	{
		return Matrix<4, 4, T>(
			a_scale[0], 0, 0, 0,
			0, a_scale[1], 0, 0,
			0, 0, a_scale[2], 0,
			0, 0, 0, 1
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> PerspectiveFovRH(T a_fovInRad, T a_aspectRatio, T a_zNear, T a_zFar)
	{
		const T focalLength = std::tan(a_fovInRad / 2);
		const T range = -a_zFar / (a_zFar - a_zNear);

		return Matrix<4, 4, T>(
			static_cast<T>(1) / (a_aspectRatio * focalLength), 0, 0, 0,
			0, static_cast<T>(1) / focalLength, 0, 0,
			0, 0, range, -static_cast<T>(1),
			0, 0, range * a_zNear, 0
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> PerspectiveFovLH(T a_fovInRad, T a_aspectRatio, T a_zNear, T a_zFar)
	{
		const T focalLength = std::tan(a_fovInRad / 2);
		const T range = -a_zFar / (a_zFar - a_zNear);

		return Matrix<4, 4, T>(
			static_cast<T>(1) / (a_aspectRatio * focalLength), 0, 0, 0,
			0, static_cast<T>(1) / focalLength, 0, 0,
			0, 0, range, static_cast<T>(1),
			0, 0, range * a_zNear, 0
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> LookToRH(const Vector<3, T>& a_position, const Vector<3, T>& a_lookTo, const Vector<3, T>& a_worldUp)
	{
		const Vector<3, T> forward = Normalize(a_lookTo);
		const Vector<3, T> right(Normalize(Cross(a_worldUp, forward)));
		const Vector<3, T> up(Cross(forward, right));

		return Matrix<4, 4, T>(
			right[0], right[1], right[2], -vath::Dot(right, a_position),
			up[0], up[1], up[2], -vath::Dot(up, a_position),
			forward[0], forward[1], forward[2], -vath::Dot(forward, a_position),
			static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)
		);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> LookToLH(const Vector<3, T>& a_position, const Vector<3, T>& a_lookTo, const Vector<3, T>& a_worldUp)
	{
		return LookToRH(a_position, -a_lookTo, a_worldUp);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> LookAtRH(const Vector<3, T>& a_position, const Vector<3, T>& a_lookAt, const Vector<3, T>& a_worldUp)
	{
		return LookToRH(a_position, a_lookAt - a_position, a_worldUp);
	}

	template<typename T>
	constexpr Matrix<4, 4, T> LookAtLH(const Vector<3, T>& a_position, const Vector<3, T>& a_lookAt, const Vector<3, T>& a_worldUp)
	{
		return LookToRH(a_position, a_position - a_lookAt, a_worldUp);
	}
}

#pragma warning(pop)