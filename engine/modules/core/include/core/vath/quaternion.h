#pragma once
#include "core/vath/vathTemplate.h"
#include "core/vath/vathUtility.h"
#include "core/debug.h"

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
	/**
	 * @brief Quat implementation, used to rotate objects preventing them from suffering from Gimbal lock.
	 * @tparam T Value type of elements.
	 */
	template<typename T>
	class Quat final
	{
		static_assert(std::is_floating_point_v<T>); //Quaternions should only be used with decimal numbers.
		using ValueType = T;

	public:
		Quat();
		explicit Quat(T a_scalar);
		Quat(T a_x, T a_y, T a_z, T a_scalar);
		Quat(const Vector<3, T>& a_vector, T a_scalar);
		Quat(T* a_pData);
		Quat(const Matrix<3, 3, T>& a_rotationMatrix);
		~Quat<T>() = default;

		constexpr Quat<T>(const Quat<T>& a_rhs) = default;
		constexpr Quat<T>& operator=(const Quat<T>& a_rhs) = default;

		T& operator[](usize i);
		const T& operator[](usize i) const;
		constexpr Vector<3, T> GetComplex() const;
		constexpr T GetReal() const;
		constexpr usize GetLength() const;

		constexpr Quat& operator +=(T a_scalar);
		constexpr Quat& operator -=(T a_scalar);
		constexpr Quat& operator *=(T a_scalar);
		constexpr Quat& operator /=(T a_scalar);

	private:
		union
		{
			T Data[4];
			struct { T x, y, z, w; };
			struct { T r, g, b, a; };
		};
	};


	//--- Quaternion definitions ---

	using Quaternionf = Quat<fp32>;
	using Quaterniond = Quat<fp64>;
	using Quaternion = Quaternionf;


	//--- Quat construction/destruction ---

	template<typename T>
	Quat<T>::Quat() :
		x(0.0),
		y(0.0),
		z(0.0),
		w(1.0)
	{}

	template<typename T>
	Quat<T>::Quat(T a_scalar) :
		x(a_scalar),
		y(a_scalar),
		z(a_scalar),
		w(a_scalar)
	{}

	template<typename T>
	Quat<T>::Quat(T a_x, T a_y, T a_z, T a_scalar) :
		x(a_x),
		y(a_y),
		z(a_z),
		w(a_scalar)
	{}

	template<typename T>
	Quat<T>::Quat(const Vector<3, T>& a_vector, T a_scalar) :
		x(a_vector[0]),
		y(a_vector[1]),
		z(a_vector[2]),
		w(a_scalar)
	{}

	template<typename T>
	Quat<T>::Quat(T* a_pData)
	{
		memcpy(Data, a_pData, GetLength() * sizeof(T));
	}

	template<typename T>
	Quat<T>::Quat(const Matrix<3, 3, T>& a_rotationMatrix)
	{
		Quat<T> quat;

		const T m00 = a_rotationMatrix[0][0];
		const T m11 = a_rotationMatrix[1][1];
		const T m22 = a_rotationMatrix[2][2];
		const T sum = m00 + m11 + m22;

		if (sum > 0.0)
		{
			quat[3] = Sqrt<T>(sum + 1.0) * 0.5;
			const T f = 0.25 / quat[3];
			quat[0] = (a_rotationMatrix[2][1] + a_rotationMatrix[1][2]) * f;
			quat[1] = (a_rotationMatrix[0][2] + a_rotationMatrix[2][0]) * f;
			quat[2] = (a_rotationMatrix[1][0] + a_rotationMatrix[0][1]) * f;

		}
		else if ((m00 > m11) && (m00 > m22))
		{
			quat[0] = Sqrt<T>(m00 - m11 - m22 + 1.0) * 0.5;
			const T f = 0.25 / quat[0];
			quat[1] = (a_rotationMatrix[1][0] + a_rotationMatrix[0][1]) * f;
			quat[2] = (a_rotationMatrix[0][2] + a_rotationMatrix[2][0]) * f;
			quat[3] = (a_rotationMatrix[2][1] + a_rotationMatrix[1][2]) * f;
		}
		else if (m11 > m22)
		{
			quat[1] = Sqrt<T>(m11 - m00 - m22 + 1.0) * 0.5;
			const T f = 0.25 / quat[1];
			quat[0] = (a_rotationMatrix[1][0] + a_rotationMatrix[0][1]) * f;
			quat[2] = (a_rotationMatrix[2][1] + a_rotationMatrix[1][2]) * f;
			quat[3] = (a_rotationMatrix[0][2] + a_rotationMatrix[2][0]) * f;
		}
		else
		{
			quat[2] = Sqrt<T>(m22 - m00 - m11 + 1.0) * 0.5;
			const T f = 0.25 / quat[2];
			quat[0] = (a_rotationMatrix[0][2] + a_rotationMatrix[2][0]) * f;
			quat[1] = (a_rotationMatrix[2][1] + a_rotationMatrix[1][2]) * f;
			quat[3] = (a_rotationMatrix[1][0] + a_rotationMatrix[0][1]) * f;
		}
	}


	//--- Quat getters/setters ---

	template<typename T>
	T& Quat<T>::operator[](usize i)
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	const T& Quat<T>::operator[](usize i) const
	{
		DXRAY_ASSERT(i < GetLength());
		return Data[i];
	}

	template<typename T>
	constexpr Vector<3, T> Quat<T>::GetComplex() const
	{
		return Vector<3, T>(x, y, z);
	}

    template<typename T>
    constexpr T Quat<T>::GetReal() const 
    {
        return w;
    }

	template<typename T>
	constexpr usize Quat<T>::GetLength() const
	{
		return 4;
	}


	//--- Quat class operators ---

	template<typename T>
	constexpr Quat<T>& Quat<T>::operator+=(T a_scalar)
	{
		x += a_scalar;
		y += a_scalar;
		z += a_scalar;
		w += a_scalar;
		return (*this);
	}

	template<typename T>
	constexpr Quat<T>& Quat<T>::operator-=(T a_scalar)
	{
		x -= a_scalar;
		y -= a_scalar;
		z -= a_scalar;
		w -= a_scalar;
		return (*this);
	}

	template<typename T>
	constexpr Quat<T>& Quat<T>::operator*=(T a_scalar)
	{
		x *= a_scalar;
		y *= a_scalar;
		z *= a_scalar;
		w *= a_scalar;
		return (*this);
	}

	template<typename T>
	constexpr Quat<T>& Quat<T>::operator/=(T a_scalar)
	{
		x /= a_scalar;
		y /= a_scalar;
		z /= a_scalar;
		w /= a_scalar;
		return (*this);
	}


	//--- Quat global operations.

	template<typename T>
	constexpr Quat<T> operator+(const Quat<T>& a_lhs, const Quat<T>& a_rhs)
	{
		return Quat<T>(
			a_lhs[0] + a_rhs[0],
			a_lhs[1] + a_rhs[1],
			a_lhs[2] + a_rhs[2],
			a_lhs[3] + a_rhs[3]
		);
	}

	template<typename T>
	constexpr Quat<T> operator-(const Quat<T>& a_lhs, const Quat<T>& a_rhs)
	{
		return Quat<T>(
			a_lhs[0] - a_rhs[0],
			a_lhs[1] - a_rhs[1],
			a_lhs[2] - a_rhs[2],
			a_lhs[3] - a_rhs[3]
		);
	}

	template<typename T>
	constexpr Quat<T> operator*(const Quat<T>& a_lhs, const Quat<T>& a_rhs)
	{
		return Quat<T>(
			a_lhs[3] * a_rhs[0] + a_lhs[0] * a_rhs[3] + a_lhs[1] * a_rhs[2] - a_lhs[2] * a_rhs[1],
			a_lhs[3] * a_rhs[1] - a_lhs[0] * a_rhs[2] + a_lhs[1] * a_rhs[3] + a_lhs[2] * a_rhs[0],
			a_lhs[3] * a_rhs[2] + a_lhs[0] * a_rhs[1] - a_lhs[1] * a_rhs[0] + a_lhs[2] * a_rhs[3],
			a_lhs[3] * a_rhs[3] - a_lhs[0] * a_rhs[0] - a_lhs[1] * a_rhs[1] - a_lhs[2] * a_rhs[2]
		);
	}

	template<typename T>
	constexpr Quat<T> operator*(const Quat<T>& a_lhs, T a_scalar)
	{
		return Quat<T>(
			a_lhs[0] * a_scalar,
			a_lhs[1] * a_scalar,
			a_lhs[2] * a_scalar,
			a_lhs[3] * a_scalar
		);
	}

	template<typename T>
	constexpr Quat<T> operator/(const Quat<T>& a_lhs, T a_scalar)
	{
		return Quat<T>(
			a_lhs[0] / a_scalar,
			a_lhs[1] / a_scalar,
			a_lhs[2] / a_scalar,
			a_lhs[3] / a_scalar
		);
	}

	//--- Quat comparison operators ---

	template<typename T>
	constexpr bool operator==(const Quat<T>& a_lhs, const Quat<T>& a_rhs)
	{
		return a_lhs[0] == a_rhs[0] && a_lhs[1] == a_rhs[1] && a_lhs[2] == a_rhs[2] && a_lhs[3] == a_rhs[3];
	}

	template<typename T>
	constexpr bool operator!=(const Quat<T>& a_lhs, const Quat<T>& a_rhs)
	{
		return !(a_lhs == a_rhs);
	}


	//--- Quat applicable functionality ---

	template<typename T>
	constexpr T SqrMagnitude(const Quat<T>& a_quat)
	{
		return a_quat[0] * a_quat[0] + a_quat[1] * a_quat[1] + a_quat[2] * a_quat[2] + a_quat[3] * a_quat[3];
	}

	template<typename T>
	constexpr T Magnitude(const Quat<T>& a_quat)
	{
		return std::sqrt(a_quat[0] * a_quat[0] + a_quat[1] * a_quat[1] + a_quat[2] * a_quat[2] + a_quat[3] * a_quat[3]);
	}

	template<typename T>
	constexpr T Dot(const Quat<T>& a_lhs, const Quat<T>& a_rhs)
	{
		return (a_lhs[0] * a_rhs[0] + a_lhs[1] * a_rhs[1] + a_lhs[2] * a_rhs[2] + a_lhs[3] * a_rhs[3]);
	}

	template<typename T>
	constexpr Quat<T> Normalize(const Quat<T>& a_quat)
	{
		return a_quat / Magnitude(a_quat);
	}

    template<typename T>
    constexpr Quat<T> Conjugate(const Quat<T>& a_quat)
    {
        return Quat(
            -a_quat[0],
            -a_quat[1],
            -a_quat[2],
            a_quat[3]
        );
    }

	template<typename T>
	constexpr Quat<T> Inverse(const Quat<T>& a_quat)
	{
		const T denom = static_cast<T>(1.0) / SqrMagnitude(a_quat);
		return Quat<T>(
			-a_quat[0] * denom,
			-a_quat[1] * denom,
			-a_quat[2] * denom,
			a_quat[3] * denom
		);
    }

    template<typename T>
    constexpr Quat<T> ToQuat(const Vector<3, T>& a_eulerAngles)
    {
		const T cosx = std::cos(a_eulerAngles[0] * static_cast<T>(0.5));
		const T cosy = std::cos(a_eulerAngles[1] * static_cast<T>(0.5));
		const T cosz = std::cos(a_eulerAngles[2] * static_cast<T>(0.5));

		const T sinx = std::sin(a_eulerAngles[0] * static_cast<T>(0.5));
		const T siny = std::sin(a_eulerAngles[1] * static_cast<T>(0.5));
		const T sinz = std::sin(a_eulerAngles[2] * static_cast<T>(0.5));

        //returned in order: roll(x)-pitch(y)-yaw(z) angles in radians.
		return Quat<T>(
			sinx * cosy * cosz - cosx * siny * sinz,
			cosx * siny * cosz + sinx * cosy * sinz,
			cosx * cosy * sinz - sinx * siny * cosz,
			cosx * cosy * cosz + sinx * siny * sinz
		);
    }

    template<typename T>
    constexpr Vector<3, T> ToEuler(const Quat<T>& a_quat)
    {
		Vector<3, T> euler;

		const T sqx = a_quat[0] * a_quat[0];
		const T sqy = a_quat[1] * a_quat[1];
		const T sqz = a_quat[2] * a_quat[2];
		const T sqw = a_quat[3] * a_quat[3];

		//Compute the euler angles with 90 degrees kept in mind as this would normally result in NaN values. This is prevented through the flipping of the values.
		euler[1] = std::asin(static_cast<T>(2.0) * (a_quat[3] * a_quat[1] - a_quat[0] * a_quat[2]));
		if ((Pi<T>() * static_cast<T>(0.5)) - Abs<T>(euler[1]) > 1e-5)
        {
			euler[2] = std::atan2(static_cast<T>(2.0) * (a_quat[0] * a_quat[1] + a_quat[3] * a_quat[2]), sqx - sqy - sqz + sqw);
			euler[0] = std::atan2(static_cast<T>(2.0) * (a_quat[3] * a_quat[0] + a_quat[1] * a_quat[2]), sqw - sqx - sqy + sqz);
		}
		else 
        {
			euler[2] = std::atan2(
				static_cast<T>(2.0) * a_quat[1] * a_quat[2] - static_cast<T>(2.0) * a_quat[0] * a_quat[3], 
				static_cast<T>(2.0) * a_quat[0] * a_quat[2] + static_cast<T>(2.0) * a_quat[1] * a_quat[3]
			);
			euler[0] = static_cast<T>(0.0);

			if (euler[1] < static_cast<T>(0.0))
            {
				euler[2] = Pi<T>() - euler[2];
            }
		}
		return euler;
    }

	template<typename T>
	constexpr Quat<T> slerp(const Quat<T>& a_from, const Quat<T>& a_to, T a_time) 
	{
		const T clamped = std::acos(Clamp<T>(Dot(a_from, a_to), static_cast<T>(-1.0), static_cast<T>(1.0)));
		if (Abs<T>(clamped) < Epsilon<T>())
		{
			clamped = Epsilon<T>();
		}

		const T s = std::sin(clamped);
		const T st0 = std::sin((1.0 - a_time) * clamped) / s;
		const T st1 = std::sin(a_time * clamped) / s;

		return a_from * st0 + a_to * st1;
	}

	template<typename T>
	constexpr Quat<T> AngleAxisRoll(T a_angleInRad)
	{
		const T halfAngle = a_angleInRad * static_cast<T>(0.5);
		return Quat<T>(
			std::sin(halfAngle), 
			0, 
			0, 
			std::cos(halfAngle)
		);
	}

	template<typename T>
	constexpr Quat<T> AngleAxisPitch(T a_angleInRad)
	{
		const T halfAngle = a_angleInRad * static_cast<T>(0.5);
		return Quat<T>(
			0, 
			std::sin(halfAngle), 
			0, 
			std::cos(halfAngle)
		);
	}

	template<typename T>
	constexpr Quat<T> AngleAxisYaw(T a_angleInRad)
	{
		const T halfAngle = a_angleInRad * static_cast<T>(0.5);
		return Quat<T>(
			0, 
			0, 
			std::sin(halfAngle), 
			std::cos(halfAngle)
		);
	}

	template<typename T>
	constexpr Quat<T> AngleAxis(const Quat<T>& a_axis, T a_angleInRad)
	{
		const T halfAngle = a_angleInRad * static_cast<T>(0.5);
		const T halfSin = std::sin(halfAngle);

		return Quat<T>(
			halfSin,
			halfSin,
			halfSin,
			std::cos(halfAngle)
		);
	}

	template<typename T>
	constexpr Matrix<3, 3, T> Rotation(const Quat<T>& a_quat)
	{
		const T x2 = a_quat[0] * a_quat[0];
		const T y2 = a_quat[1] * a_quat[1];
		const T z2 = a_quat[2] * a_quat[2];
		const T xy = a_quat[0] * a_quat[1];
		const T xz = a_quat[0] * a_quat[2];
		const T yz = a_quat[1] * a_quat[2];
		const T wx = a_quat[3] * a_quat[0];
		const T wy = a_quat[3] * a_quat[1];
		const T wz = a_quat[3] * a_quat[2];

		return Matrix<3, 3, T>(
			static_cast<T>(1.0) - static_cast<T>(2.0) * (y2 + z2), static_cast<T>(2.0) * (xy - wz), static_cast<T>(2.0) * (xz + wy),
			static_cast<T>(2.0) * (xy + wz), static_cast<T>(1.0) - static_cast<T>(2.0) * (x2 + z2), static_cast<T>(2.0) * (yz - wx),
			static_cast<T>(2.0) * (xz - wy), static_cast<T>(2.0) * (yz + wx), static_cast<T>(1.0) - static_cast<T>(2.0) * (x2 - y2)
		);
	}

	template<typename T>
	constexpr Vector<3, T> operator*(const Vector<3, T>& a_vector, const Quat<T>& a_quat)
	{
		return (a_quat * Quat<T>(a_vector, static_cast<T>(0.0)) * Conjugate(a_quat)).GetComplex();
	}

}

#pragma warning(pop)