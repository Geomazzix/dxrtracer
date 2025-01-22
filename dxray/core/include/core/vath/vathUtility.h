#pragma once
#include "core/valueTypes.h"
#include <random>

//#Note: Attached to std for consistency in cpp type usage.
namespace std
{
    template<typename T>
    concept arithmetic = std::is_arithmetic_v<T>;
}

namespace dxray::vath
{
	//--- General Utility ---

	template<typename T> requires std::arithmetic<T>
	constexpr T Clamp(const T a_value, const T a_min, const T a_max)
	{
		return std::clamp<T>(a_value, a_min, a_max);
	}

	template <typename T> requires std::arithmetic<T>
	constexpr T Min(const T a_value, const T a_min)
	{
        return std::min<T>(a_value, a_min);
	}

	template <typename T> requires std::arithmetic<T>
	constexpr T Max(const T a_value, const T a_max)
	{
		return std::max<T>(a_value, a_max);
	}

	template <typename T> requires std::arithmetic<T> && std::signed_integral<T>
	constexpr T Abs(const T a_value)
	{
		return std::abs<T>(a_value);
	}

	template <typename T> requires std::floating_point<T>
	constexpr T Floor(const T a_fp)
	{
		return std::floor(a_fp);
	}

	template <typename T> requires std::floating_point<T>
	constexpr T Ceil(const T a_fp)
	{
		return std::ceil<T>(a_fp);
	}

	template <typename T> requires std::floating_point<T>
	constexpr T Round(const T a_fp)
	{
		return std::round(a_fp);
	}

	template <typename T> requires std::floating_point<T>
	constexpr T Lerp(const T a_min, const T a_max, const T a_coefficient)
	{
		return std::lerp(a_min, a_max, a_coefficient);
	}

	template<typename T> requires std::floating_point<T>
	constexpr T Infinity()
	{
		return std::numeric_limits<T>::infinity();
	}


	//--- cmath automatic type deductions ---

	template <typename T> requires std::floating_point<T>
	constexpr T Epsilon()
	{
		return std::numeric_limits<T>::epsilon();
	}

	template<typename T> requires std::floating_point<T>
	constexpr T Pi()
	{
		return (sizeof(T) > sizeof(float)) ? 3.141592653589793 : 3.1415926f;
	}

	template <typename T> requires std::floating_point<T>
	constexpr T DegToRad(const T a_valueInDeg)
	{
		return a_valueInDeg * (Pi<T>() / static_cast<T>(180));
	}

	template <typename T> requires std::floating_point<T>
	constexpr T RadToDeg(const T a_valueInRad)
	{
		return a_valueInRad * (static_cast<T>(180) / Pi<T>());
	}


	//--- random number generator ---

	template <typename T> requires std::arithmetic<T>
	inline T RandomNumber()
	{
		static std::mt19937 generator;
		static std::uniform_real_distribution<T> distribution(static_cast<T>(0.0), static_cast<T>(1.0));
		return distribution(generator);
	}

	template <typename T> requires std::arithmetic<T>
	inline T RandomNumber(const T a_min, const T a_max)
	{
		return a_min + (a_max - a_min) * RandomNumber<T>();
	}
}