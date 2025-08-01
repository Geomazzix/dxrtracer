#pragma once
#include "core/vath/vathTemplate.h"
#include "core/vath/vector4.h"
#include "core/debug.h"

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
	/**
	 * @brief Vector 3 specialization, represented by value type T.
	 * @tparam T Value type of elements.
	 */
	template<typename T>
	class Vector<3, T> final
	{
	public:
		using Type = Vector<3, T>;
		using ValueType = T;

		constexpr Vector();
		constexpr explicit Vector(T a_scalar);
		constexpr Vector(T a_x, T a_y, T a_z);
		constexpr Vector(T* a_pData);
		~Vector() = default;

		template<typename U>
		constexpr Vector(const Vector<4, U>& a_vector4);

		constexpr Vector<3, T>(const Vector<3, T>& a_rhs) = default;
		constexpr Vector<3, T>& operator=(const Vector<3, T>& a_rhs) = default;
		
		T& operator[](usize i);
		const T& operator[](usize i) const;
		constexpr usize GetLength() const;

		constexpr Vector<3, T>& operator +=(T a_scalar);
		constexpr Vector<3, T>& operator +=(const Vector<3, T>& a_rhs);
		constexpr Vector<3, T>& operator -=(T a_scalar);
		constexpr Vector<3, T>& operator -=(const Vector<3, T>& a_rhs);
		constexpr Vector<3, T>& operator *=(T a_scalar);
		constexpr Vector<3, T>& operator /=(T a_scalar);

		union
		{
			T Data[3];
			struct { T x, y, z; };
			struct { T r, g, b; };
			struct { T u, v, w; };
		};
	};


	//--- Vector definitions ---

	using Vector3u8 = Vector<3, u8>;
	using Vector3u16 = Vector<3, u16>;
	using Vector3u32 = Vector<3, u32>;
	using Vector3u64 = Vector<3, u64>;
	using Vector3i8 = Vector<3, i8>;
	using Vector3i16 = Vector<3, i16>;
	using Vector3i32 = Vector<3, i32>;
	using Vector3i64 = Vector<3, i64>;
	using Vector3f = Vector<3, fp32>;
	using Vector3d = Vector<3, fp64>;
	using Vector3 = Vector3f;


	//--- Vector construction/destruction ---

	template<typename T>
	constexpr Vector<3, T>::Vector() :
		x(static_cast<T>(0)),
		y(static_cast<T>(0)),
		z(static_cast<T>(0))
	{}

	template<typename T>
	constexpr Vector<3, T>::Vector(T* a_pData)
	{
		memcpy(Data, a_pData, GetLength() * sizeof(T));
	}

	template<typename T>
	constexpr Vector<3, T>::Vector(T a_x, T a_y, T a_z) :
		x(a_x),
		y(a_y),
		z(a_z)
	{}

	template<typename T>
	constexpr Vector<3, T>::Vector(T a_scalar) :
		x(a_scalar),
		y(a_scalar),
		z(a_scalar)
	{}

	template<typename T>
	template<typename U>
	constexpr Vector<3, T>::Vector(const Vector<4, U>& a_vector4) :
		x(static_cast<T>(a_vector4[0])),
		y(static_cast<T>(a_vector4[1])),
		z(static_cast<T>(a_vector4[2]))
	{}


	//--- Vector getters/setters ---

	template<typename T>
	T& Vector<3, T>::operator[](usize i)
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	const T& Vector<3, T>::operator[](usize i) const
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	constexpr usize Vector<3, T>::GetLength() const
	{
		return 3;
	}


	//--- Vector class operators ---

	template<typename T>
	constexpr Vector<3, T>& Vector<3, T>::operator+=(T a_scalar)
	{
		x += a_scalar;
		y += a_scalar;
		z += a_scalar;
		return *this;
	}

	template<typename T>
	constexpr Vector<3, T>& Vector<3, T>::operator+=(const Vector<3, T>& a_rhs)
	{
		x += a_rhs.x;
		y += a_rhs.y;
		z += a_rhs.z;
		return (*this);
	}

	template<typename T>
	constexpr Vector<3, T>& Vector<3, T>::operator-=(T a_scalar)
	{
		x -= a_scalar;
		y -= a_scalar;
		z -= a_scalar;
		return *this;
	}

	template<typename T>
	constexpr Vector<3, T>& Vector<3, T>::operator-=(const Vector<3, T>& a_rhs)
	{
		x -= a_rhs.x;
		y -= a_rhs.y;
		z -= a_rhs.z;
		return (*this);
	}

	template<typename T>
	constexpr Vector<3, T>& Vector<3, T>::operator*=(T a_scalar)
	{
		x *= a_scalar;
		y *= a_scalar;
		z *= a_scalar;
		return (*this);
	}

	template<typename T>
	constexpr Vector<3, T>& Vector<3, T>::operator/=(T a_scalar)
	{
		T divider = 1.0f / a_scalar;
		x *= divider;
		y *= divider;
		z *= divider;
		return (*this);
	}


	//--- Vector global operators ---

	template<typename T>
	constexpr Vector<3, T> operator +(const Vector<3, T>& a_vector, const Vector<3, T>& a_rhs)
	{
		return Vector<3, T>(a_vector[0] + a_rhs[0], a_vector[1] + a_rhs[1], a_vector[2] + a_rhs[2]);
	}

	template<typename T>
	constexpr Vector<3, T> operator +(const Vector<3, T>& a_vector, const T a_scalar)
	{
		return Vector<3, T>(a_vector[0] + a_scalar, a_vector[1] + a_scalar, a_vector[2] + a_scalar);
	}

	template<typename T>
	constexpr Vector<3, T> operator +(const T a_scalar, const Vector<3, T>& a_vector)
	{
		return Vector<3, T>(a_vector + a_scalar);
	}

	template<typename T>
	constexpr Vector<3, T> operator -(const Vector<3, T>& a_vector, const Vector<3, T>& a_rhs)
	{
		return Vector<3, T>(a_vector[0] - a_rhs[0], a_vector[1] - a_rhs[1], a_vector[2] - a_rhs[2]);
	}

	template<typename T>
	constexpr Vector<3, T> operator -(const Vector<3, T>& a_vector, const T a_scalar)
	{
		return Vector<3, T>(a_vector[0] - a_scalar, a_vector[1] - a_scalar, a_vector[2] - a_scalar);
	}

	template<typename T>
	constexpr Vector<3, T> operator -(const T a_scalar, const Vector<3, T>& a_vector)
	{
		return Vector<3, T>(a_vector - a_scalar);
	}

    template<typename T>
    constexpr Vector<3, T> operator *(const Vector<3, T>& a_lhs, const Vector<3, T>& a_rhs)
    {
        return Vector<3, T>(a_lhs[0] * a_rhs[0], a_lhs[1] * a_rhs[1], a_lhs[2] * a_rhs[2]);
    }

	template<typename T>
	constexpr Vector<3, T> operator *(const Vector<3, T>& a_vector, const T a_scalar)
	{
		return Vector<3, T>(a_vector[0] * a_scalar, a_vector[1] * a_scalar, a_vector[2] * a_scalar);
	}

	template<typename T>
	constexpr Vector<3, T> operator *(const T a_scalar, const Vector<3, T>& a_vector)
	{
		return a_vector * a_scalar;
	}

	template<typename T>
	constexpr Vector<3, T> operator /(const Vector<3, T>& a_vector, const T a_scalar)
	{
		const T divider = static_cast<T>(1.0) / a_scalar;
		return Vector<3, T>(a_vector[0] * divider, a_vector[1] * divider, a_vector[2] * divider);
	}

	template<typename T>
	constexpr Vector<3, T> operator -(const Vector<3, T>& a_vector)
	{
		return Vector<3, T>(-a_vector[0], -a_vector[1], -a_vector[2]);
	}


	//--- Vector comparison operators ---

	template<typename T>
	constexpr bool operator==(const Vector<3, T>& a_lhs, const Vector<3, T>& a_rhs)
	{
		return abs(a_lhs[0] - a_rhs[0]) <= vath::Epsilon<T>() && abs(a_lhs[1] - a_rhs[1]) <= vath::Epsilon<T>() && abs(a_lhs[2] - a_rhs[2]) <= vath::Epsilon<T>();
	}

	template<typename T>
	constexpr bool operator!=(const Vector<3, T>& a_lhs, const Vector<3, T>& a_rhs)
	{
		return !(a_lhs == a_rhs);
	}

	
	//--- Vector applicable functionality ---

	template<typename T>
	constexpr T SqrMagnitude(const Vector<3, T>& a_vector)
	{
		return a_vector[0] * a_vector[0] + a_vector[1] * a_vector[1] + a_vector[2] * a_vector[2];
	}

	template<typename T>
	constexpr T Magnitude(const Vector<3, T>& a_vector)
	{
		return std::sqrt(a_vector[0] * a_vector[0] + a_vector[1] * a_vector[1] + a_vector[2] * a_vector[2]);
	}

	template<typename T>
	constexpr Vector<3, T> Normalize(const Vector<3, T>& a_vector)
	{
		return a_vector / Magnitude(a_vector);
	}

	template<typename T>
	constexpr T Dot(const Vector<3, T>& a_lhs, const Vector<3, T>& a_rhs)
	{
		return (a_lhs[0] * a_rhs[0] + a_lhs[1] * a_rhs[1] + a_lhs[2] * a_rhs[2]);
	}

	template<typename T>
	constexpr Vector<3, T> Cross(const Vector<3, T>& a_lhs, const Vector<3, T>& a_rhs)
	{
		return Vector<3, T>(
			a_lhs[1] * a_rhs[2] - a_lhs[2] * a_rhs[1],
			a_lhs[2] * a_rhs[0] - a_lhs[0] * a_rhs[2],
			a_lhs[0] * a_rhs[1] - a_lhs[1] * a_rhs[0]
		);
	}

	template<typename T>
	constexpr Vector<3, T> Project(const Vector<3, T>& a_lhs, const Vector<3, T>& a_rhs)
	{
		return (a_rhs * Dot(a_lhs, a_rhs) / Dot(a_lhs, a_rhs));
	}

	template<typename T>
	constexpr Vector<3, T> Reject(const Vector<3, T>& a_lhs, const Vector<3, T>& a_rhs)
	{
		return (a_lhs - a_rhs * (Dot(a_lhs, a_rhs) / Dot(a_rhs, a_rhs)));
	}

}

#pragma warning(pop)