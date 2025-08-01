#pragma once
#include "core/vath/vathTemplate.h"
#include "core/debug.h"

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
	/**
	 * @brief Vector 2 specialization, represented by value type T.
	 * @tparam T Value type of elements.
	 */
	template<typename T>
	class Vector<2, T> final
	{
	public:
		using Type = Vector<2, T>;
		using ValueType = T;

		constexpr Vector();
		constexpr explicit Vector(T a_scalar);
		constexpr Vector(T a_x, T a_y);
		constexpr Vector(T* a_pData);
		~Vector() = default;

		constexpr Vector<2, T>(const Vector<2, T>& a_rhs) = default;
		constexpr Vector<2, T>& operator=(const Vector<2, T>& a_rhs) = default;

		T& operator[](usize i);
		const T& operator[](usize i) const;
		constexpr usize GetLength() const;

		constexpr Vector& operator +=(T a_scalar);
		constexpr Vector& operator -=(T a_scalar);
		constexpr Vector& operator *=(T a_scalar);
		constexpr Vector& operator /=(T a_scalar);

		union
		{
			T Data[2];
			struct { T x, y; };
			struct { T r, g; };
			struct { T u, v; };
		};
	};


	//--- Vector definition ---

	using Vector2u8 = Vector<2, u8>;
	using Vector2u16 = Vector<2, u16>;
	using Vector2u32 = Vector<2, u32>;
	using Vector2u64 = Vector<2, u64>;
	using Vector2i8 = Vector<2, i8>;
	using Vector2i16 = Vector<2, i16>;
	using Vector2i32 = Vector<2, i32>;
	using Vector2i64 = Vector<2, i64>;
	using Vector2f = Vector<2, fp32>;
	using Vector2d = Vector<2, fp64>;
	using Vector2 = Vector2f;


	//--- Vector construction/destruction ---

	template<typename T>
	constexpr Vector<2, T>::Vector() :
		x(static_cast<T>(0)),
		y(static_cast<T>(0))
	{}

	template<typename T>
	constexpr Vector<2, T>::Vector(T a_scalar) :
		x(a_scalar),
		y(a_scalar)
	{}

	template<typename T>
	constexpr Vector<2, T>::Vector(T a_x, T a_y) :
		x(a_x),
		y(a_y)
	{}

	template<typename T>
	constexpr Vector<2, T>::Vector(T* a_pData) :
		x(static_cast<T>(0)),
		y(static_cast<T>(0))
	{
		memcpy(Data, a_pData, 2 * sizeof(T));
	}


	//--- Vector getters/setters ---

	template<typename T>
	T& Vector<2, T>::operator[](usize i)
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	const T& Vector<2, T>::operator[](usize i) const
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	constexpr usize Vector<2, T>::GetLength() const
	{
		return 2;
	}


	//--- Vector class operators ---

	template<typename T>
	constexpr Vector<2, T>& Vector<2, T>::operator+=(T a_scalar)
	{
		x += a_scalar;
		y += a_scalar;
		return *this;
	}

	template<typename T>
	constexpr Vector<2, T>& Vector<2, T>::operator-=(T a_scalar)
	{
		x -= a_scalar;
		y -= a_scalar;
		return *this;
	}

	template<typename T>
	constexpr Vector<2, T>& Vector<2, T>::operator*=(T a_scalar)
	{
		x *= a_scalar;
		y *= a_scalar;
		return *this;
	}

	template<typename T>
	constexpr Vector<2, T>& Vector<2, T>::operator/=(T a_scalar)
	{
		T divider = 1.0f / a_scalar;
		x *= a_scalar;
		y *= a_scalar;
		return *this;
	}


	//--- Vector global operators ---

	template<typename T>
	constexpr Vector<2, T> operator+(const Vector<2, T>& a_lhs, const Vector<2, T>& a_rhs)
	{
		return Vector<2, T>(a_lhs[0] + a_rhs[0], a_lhs[1] + a_rhs[1]);
	}

	template<typename T>
	constexpr Vector<2, T> operator+(const Vector<2, T>& a_lhs, const T a_scalar)
	{
		return Vector<2, T>(a_lhs[0] + a_scalar, a_lhs[1] + a_scalar);
	}

	template<typename T>
	constexpr Vector<2, T> operator+(const T a_scalar, const Vector<2, T>& a_rhs)
	{
		return a_rhs + a_scalar;
	}

	template<typename T>
	constexpr Vector<2, T> operator-(const Vector<2, T>& a_lhs, const Vector<2, T>& a_rhs)
	{
		return Vector<2, T>(a_lhs[0] - a_rhs[0], a_lhs[1] - a_rhs[1]);
	}

	template<typename T>
	constexpr Vector<2, T> operator-(const Vector<2, T>& a_lhs, const T a_scalar)
	{
		return Vector<2, T>(a_lhs[0] - a_scalar, a_lhs[1] - a_scalar);
	}

	template<typename T>
	constexpr Vector<2, T> operator-(const T a_scalar, const Vector<2, T>& a_rhs)
	{
		return a_rhs - a_scalar;
	}

	template<typename T>
	constexpr Vector<2, T> operator*(const Vector<2, T>& a_lhs, const T a_scalar)
	{
		return Vector<2, T>(a_lhs[0] * a_scalar, a_lhs[1] * a_scalar);
	}

	template<typename T>
	constexpr Vector<2, T> operator*(const T a_scalar, const Vector<2, T>& a_rhs)
	{
		return Vector<2, T>(a_rhs[0] * a_scalar, a_rhs[1] * a_scalar);
	}

	template<typename T>
	constexpr Vector<2, T> operator/(const Vector<2, T>& a_lhs, const T a_scalar)
	{
		T divider = 1.0f / a_scalar;
		return Vector<2, T>(a_lhs[0] * divider, a_lhs[1] * divider);
	}

	template<typename T>
	constexpr Vector<2, T> operator-(const Vector<2, T>& a_rhs)
	{
		return Vector<2, T>(-a_rhs[0], -a_rhs[1]);
	}


	//--- Vector comparison operators ---

	template<typename T>
	constexpr bool operator==(const Vector<2, T>& a_lhs, const Vector<2, T>& a_rhs)
	{
		return abs(a_lhs[0] - a_rhs[0]) <= vath::Epsilon<T>() && abs(a_lhs[1] - a_rhs[1]) <= vath::Epsilon<T>();
	}

	template<typename T>
	constexpr bool operator!=(const Vector<2, T>& a_lhs, const Vector<2, T>& a_rhs)
	{
		return !(a_lhs == a_rhs);
	}


	//--- Vector applicable functionality ---

	template<typename T>
	constexpr T SqrMagnitude(const Vector<2, T>& a_vector)
	{
		return a_vector[0] * a_vector[0] + a_vector[1] * a_vector[1];
	}

	template<typename T>
	constexpr T Magnitude(const Vector<2, T>& a_vector)
	{
		return std::sqrt(a_vector[0] * a_vector[0] + a_vector[1] * a_vector[1]);
	}

	template<typename T>
	constexpr Vector<2, T> Normalize(const Vector<2, T>& a_vector)
	{
		return a_vector / Magnitude(a_vector);
	}

	template<typename T>
	constexpr T Dot(const Vector<2, T>& a_lhs, const Vector<2, T>& a_rhs)
	{
		return a_lhs[0] * a_rhs[0] + a_lhs[1] * a_rhs[1];
	}

	template<typename T>
	constexpr Vector<2, T> Project(const Vector<2, T>& a_lhs, const Vector<2, T>& a_rhs)
	{
		return (a_rhs * Dot(a_lhs, a_rhs) / Dot(a_lhs, a_rhs));
	}

	template<typename T>
	constexpr Vector<2, T> Reject(const Vector<2, T>& a_lhs, const Vector<2, T>& a_rhs)
	{
		return (a_lhs - a_rhs * (Dot(a_lhs, a_rhs) / Dot(a_rhs, a_rhs)));
	}
}

#pragma warning(pop)