#pragma once
#include "core/valueTypes.h"
#include <random>
#include <numbers>

namespace dxray::vath
{
	//--- Arithmetic Utility ---

	template<Arithmetic T>
	inline constexpr T Clamp(const T a_value, const T a_min, const T a_max)
	{
		return std::clamp<T>(a_value, a_min, a_max);
	}

	template <Arithmetic T>
	inline constexpr T Min(const T a_value, const T a_min)
	{
        return std::min<T>(a_value, a_min);
	}

	template <Arithmetic T>
	inline constexpr T Max(const T a_value, const T a_max)
	{
		return std::max<T>(a_value, a_max);
	}

	template <Arithmetic T, Arithmetic S = T>
	inline constexpr S Sign(const T a_value)
	{
		return std::signbit(a_value) ? static_cast<S>(-1) : static_cast<S>(1);
	}

	template <UnsignedArithmatic T>
	inline constexpr T Abs(const T a_value)
	{
		return std::abs(a_value);
	}

    template <FloatingPoint T>
	inline constexpr T Floor(const T a_fp)
    {
		return std::floor(a_fp);
    }

    template <FloatingPoint T>
	inline constexpr T Ceil(const T a_fp)
    {
		return std::ceil(a_fp);
    }

	template <FloatingPoint T>
	inline constexpr T Round(const T a_fp)
	{
		return std::round(a_fp);
	}

	template <FloatingPoint T>
	inline constexpr T Lerp(const T a_min, const T a_max, const T a_coefficient)
	{
		return std::lerp(a_min, a_max, a_coefficient);
	}

	template <FloatingPoint T>
	inline constexpr T Infinity()
	{
		return std::numeric_limits<T>::infinity();
	}

	template <FloatingPoint T>
	inline constexpr T Epsilon()
	{
		return std::numeric_limits<T>::epsilon();
	}

	template<FloatingPoint T>
	inline constexpr T Pi()
	{
		return std::numbers::pi_v<fp32>;
	}

	template <FloatingPoint T>
	inline constexpr T DegToRad(const T a_valueInDeg)
	{
		return a_valueInDeg * (Pi<T>() / static_cast<T>(180));
	}

	template <FloatingPoint T>
	inline constexpr T RadToDeg(const T a_valueInRad)
	{
		return a_valueInRad * (static_cast<T>(180) / Pi<T>());
	}


	//--- random number generation ---

	template <FloatingPoint T>
	inline T RandomNumber(const T a_min = 0.0, const T a_max = 1.0)
	{
		std::random_device dev;
		static std::mt19937 generator(dev());
		static std::uniform_real_distribution<T> distribution(static_cast<T>(0.0), static_cast<T>(1.0));
		return a_min + (a_max - a_min) * distribution(generator);
	}

    template <Integral T>
    inline T RandomNumber(const T a_min, const T a_max)
    {
        std::random_device dev;
        static std::mt19937 generator(dev());
        static std::uniform_int_distribution<T> distribution(static_cast<T>(0.0), static_cast<T>(1.0));
        return distribution(generator);
    }
}