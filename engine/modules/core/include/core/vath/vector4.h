#pragma once
#include "core/vath/vathTemplate.h"
#include "core/debug.h"

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
	/**
	 * @brief Vector 4 specialization, represented by value type T.
	 * @tparam T Value type of elements.
	 */
	template<typename T>
	class Vector<4, T> final
	{
	public:
		using Type = Vector<4, T>;
		using ValueType = T;

		constexpr Vector();
		constexpr explicit Vector(T a_scalar);
		constexpr Vector(T a_x, T a_y, T a_z, T a_w);
		constexpr Vector(T* a_pData);
		constexpr Vector(const Vector<3, T>& a_rhs);
		~Vector<4, T>() = default;

		constexpr Vector<4, T>(const Vector<4, T>& a_rhs) = default;
		constexpr Vector<4, T>& operator=(const Vector<4, T>& a_rhs) = default;

		T& operator[](usize i);
		const T& operator[](usize i) const;
		constexpr usize GetLength() const;

		constexpr Vector<4, T>& operator +=(T a_scalar);
		constexpr Vector<4, T>& operator -=(T a_scalar);
		constexpr Vector<4, T>& operator *=(T a_scalar);
		constexpr Vector<4, T>& operator /=(T a_scalar);

		union
		{
			T Data[4];
			struct { T x, y, z, w; };
			struct { T r, g, b, a; };
		};
	};


	//--- Vector definitions ---

	using Vector4u8 = Vector<4, u8>;
	using Vector4u16 = Vector<4, u16>;
	using Vector4u32 = Vector<4, u32>;
	using Vector4u64 = Vector<4, u64>;
	using Vector4i8 = Vector<4, i8>;
	using Vector4i16 = Vector<4, i16>;
	using Vector4i32 = Vector<4, i32>;
	using Vector4i64 = Vector<4, i64>;
	using Vector4f = Vector<4, fp32>;
	using Vector4d = Vector<4, fp64>;
	using Vector4 = Vector4f;


	//--- Vector construction/destruction ---

	template<typename T>
	constexpr Vector<4, T>::Vector() :
		x(static_cast<T>(0)),
		y(static_cast<T>(0)),
		z(static_cast<T>(0)),
		w(static_cast<T>(0))
	{}

	template<typename T>
	constexpr Vector<4, T>::Vector(T a_scalar) :
		x(a_scalar),
		y(a_scalar),
		z(a_scalar),
		w(a_scalar)
	{}

	template<typename T>
	constexpr Vector<4, T>::Vector(T a_x, T a_y, T a_z, T a_w) :
		x(a_x),
		y(a_y),
		z(a_z),
		w(a_w)
	{}

	template<typename T>
	constexpr Vector<4, T>::Vector(T* a_pData)
	{
		memcpy(Data, a_pData, GetLength() * sizeof(T));
	}

	template<typename T>
	constexpr Vector<4, T>::Vector(const Vector<3, T>& a_rhs) :
		x(a_rhs.x),
		y(a_rhs.y),
		z(a_rhs.z),
		w(1.0f)
	{}


	//--- Vector getters/setters ---

	template<typename T>
	T& Vector<4, T>::operator[](usize i)
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	const T& Vector<4, T>::operator[](usize i) const
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	constexpr usize Vector<4, T>::GetLength() const
	{
		return 4;
	}


	//--- Vector class operators ---

	template<typename T>
	constexpr Vector<4, T>& Vector<4, T>::operator*=(T a_scalar)
	{
		x *= a_scalar;
		y *= a_scalar;
		z *= a_scalar;
		w *= a_scalar;
		return (*this);
	}

	template<typename T>
	constexpr Vector<4, T>& Vector<4, T>::operator/=(T a_scalar)
	{
		T divider = 1.0f / a_scalar;
		x *= divider;
		y *= divider;
		z *= divider;
		w *= divider;
		return (*this);
	}

	template<typename T>
	constexpr Vector<4, T>& Vector<4, T>::operator+=(T a_scalar)
	{
		x += a_scalar;
		y += a_scalar;
		z += a_scalar;
		w += a_scalar;
		return *this;
	}

	template<typename T>
	constexpr Vector<4, T>& Vector<4, T>::operator-=(T a_scalar)
	{
		x -= a_scalar;
		y -= a_scalar;
		z -= a_scalar;
		w -= a_scalar;
		return *this;
	}


	//--- Vector global operators ---

	template<typename T>
	constexpr Vector<4, T> operator+(const Vector<4, T>& a_lhs, const Vector<4, T>& a_rhs)
	{
		return Vector<4, T>(a_lhs[0] + a_rhs[0], a_lhs[1] + a_rhs[1], a_lhs[2] + a_rhs[2], a_lhs[3] + a_rhs[3]);
	}

	template<typename T>
	constexpr Vector<4, T> operator+(const Vector<4, T>& a_lhs, const T a_scalar)
	{
		return Vector<4, T>(a_lhs[0] + a_scalar, a_lhs[1] + a_scalar, a_lhs[2] + a_scalar, a_lhs[3] + a_scalar);
	}

	template<typename T>
	constexpr Vector<4, T> operator+(const T a_scalar, const Vector<4, T>& a_rhs)
	{
		return a_rhs + a_scalar;
	}

	template<typename T>
	constexpr Vector<4, T> operator-(const Vector<4, T>& a_lhs, const Vector<4, T>& a_rhs)
	{
		return Vector<4, T>(a_lhs[0] - a_rhs[0], a_lhs[1] - a_rhs[1], a_lhs[2] - a_rhs[2], a_lhs[3] - a_rhs[3]);
	}

	template<typename T>
	constexpr Vector<4, T> operator-(const Vector<4, T>& a_lhs, const T a_scalar)
	{
		return Vector<4, T>(a_lhs[0] - a_scalar, a_lhs[1] - a_scalar, a_lhs[2] - a_scalar, a_lhs[3] - a_scalar);
	}

	template<typename T>
	constexpr Vector<4, T> operator-(const T a_scalar, const Vector<4, T>& a_rhs)
	{
		return a_rhs - a_scalar;
	}

	template<typename T>
	constexpr Vector<4, T> operator*(const Vector<4, T>& a_vector, const T a_scalar)
	{
		return Vector<4, T>(a_vector[0] * a_scalar, a_vector[1] * a_scalar, a_vector[2] * a_scalar, a_vector[3] * a_scalar);
	}

	template<typename T>
	constexpr Vector<4, T> operator*(const T a_scalar, const Vector<4, T>& a_vector)
	{
		return Vector<4, T>(a_vector[0] * a_scalar, a_vector[1] * a_scalar, a_vector[2] * a_scalar, a_vector[3] * a_scalar);
	}

	template<typename T>
	constexpr Vector<4, T> operator/(const Vector<4, T>& a_vector, const T a_scalar)
	{
		T divider = 1.0f / a_scalar;
		return Vector<4, T>(a_vector[0] * divider, a_vector[1] * divider, a_vector[2] * divider, a_vector[3] * divider);
	}

	template<typename T>
	constexpr Vector<4, T> operator-(const Vector<4, T>& a_vector)
	{
		return Vector<4, T>(-a_vector[0], -a_vector[1], -a_vector[2], -a_vector[3]);
	}


	//--- Vector comparison operators ---

	template<typename T>
	constexpr bool operator==(const Vector<4, T>& a_lhs, const Vector<4, T>& a_rhs)
	{
		return a_lhs[0] == a_rhs[0] && a_lhs[1] == a_rhs[1] && a_lhs[2] == a_rhs[2] && a_lhs[3] == a_rhs[3];
	}

	template<typename T>
	constexpr bool operator!=(const Vector<4, T>& a_lhs, const Vector<4, T>& a_rhs)
	{
		return !(a_lhs == a_rhs);
	}


	//--- Vector applicable functionality ---

	template<typename T>
	constexpr T SqrMagnitude(const Vector<4, T>& a_vector)
	{
		return a_vector[0] * a_vector[0] + a_vector[1] * a_vector[1] + a_vector[2] * a_vector[2] + a_vector[3] * a_vector[3];
	}

	template<typename T>
	constexpr T Magnitude(const Vector<4, T>& a_vector)
	{
		return std::sqrt(a_vector[0] * a_vector[0] + a_vector[1] * a_vector[1] + a_vector[2] * a_vector[2] + a_vector[3] * a_vector[3]);
	}

	template<typename T>
	constexpr Vector<4, T> Normalize(const Vector<4, T>& a_vector)
	{
		return a_vector / Magnitude(a_vector);
	}

	template<typename T>
	constexpr T Dot(const Vector<4, T>& a_lhs, const Vector<4, T>& a_rhs)
	{
		return (a_lhs[0] * a_rhs[0] + a_lhs[1] * a_rhs[1] + a_lhs[2] * a_rhs[2] + a_lhs[3] * a_rhs[3]);
	}

}

#pragma warning(pop)