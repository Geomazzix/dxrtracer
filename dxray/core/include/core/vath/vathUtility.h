#pragma once
#include "core/valueTypes.h"
#include <random>

namespace dxray::vath
{
	//--- General Utility ---

	template<typename T>
	constexpr T Clamp(T a_value, T a_min, T a_max)
	{
		return a_value < a_min ? a_min : a_value > a_max ? a_max : a_value;
	}

	template <typename T>
	constexpr T Min(T a_value, T a_min)
	{
		return a_value < a_min ? a_min : a_value;
	}

	template <typename T>
	constexpr T Max(T a_value, T a_max)
	{
		return a_value > a_max ? a_max : a_value;
	}

	template <typename T>
	constexpr T Abs(T a_value)
	{
		return a_value < 0 ? -a_value : a_value;
	}

	template <typename S, typename T>
	constexpr S Floor(T a_fp)
	{
		return static_cast<S>(a_fp) - (a_fp < 0 && a_fp % 1 != 0);
	}

	template <typename S, typename T>
	constexpr S Ceil(T a_fp)
	{
		return static_cast<S>(a_fp) + (a_fp > 0 && a_fp % 1 != 0);
	}

	template <typename S, typename T>
	constexpr S Round(T a_fp)
	{
		return static_cast<S>(floor(a_fp) + (a_fp > 0 && a_fp % 1 >= 0.5) + (a_fp < 0 && (1 + a_fp % 1) % 1 >= 0.5));
	}

	template <typename T>
	constexpr T Lerp(T a_min, T a_max, T a_coefficient)
	{
		return (static_cast<T>(1) - a_coefficient) * a_min + a_coefficient * a_max;
	}

	template<typename T>
	constexpr T Infinity()
	{
		static_assert(std::numeric_limits<T>::has_infinity);
		return std::numeric_limits<T>::infinity();
	}

	//--- cmath automatic type deductions ---

	template <typename T>
	constexpr T Epsilon()
	{
		return std::numeric_limits<T>::epsilon();
	}

	template<typename T>
	constexpr T Pi()
	{
		return (sizeof(T) > 4) ? 3.141592653589793 : 3.1415926f;
	}

	template <typename T>
	constexpr T DegToRad(T a_valueInDeg)
	{
		return a_valueInDeg * (Pi<T>() / static_cast<T>(180));
	}

	template <typename T>
	constexpr T RadToDeg(T a_valueInRad)
	{
		return a_valueInRad * (static_cast<T>(180) / Pi<T>());
	}

	//--- random number generator ---

	template <typename T>
	inline T RandomNumber()
	{
		static std::mt19937 generator;
		static std::uniform_real_distribution<T> distribution(static_cast<T>(0.0), static_cast<T>(1.0));
		return distribution(generator);
	}

	template <typename T>
	inline T RandomNumber(const T a_min, const T a_max)
	{
		return a_min + (a_max - a_min) * RandomNumber<T>();
	}
}