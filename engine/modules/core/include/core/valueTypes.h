#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include <concepts>

namespace dxray
{
	// Platform arithmetic types.
	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using i8 = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	using fp32 = float;
	using fp64 = double;

	using wchar = wchar_t;
	using char8 = char8_t;
	using char16 = char16_t;
	using char32 = char32_t;

	using usize = std::size_t; // Not sure if this is cross platform, stl contains it so it's assumed to be.
	
	// Limits
	inline constexpr usize umin = std::numeric_limits<usize>::min();
	inline constexpr usize umax = std::numeric_limits<usize>::max();

	inline constexpr u8 u8max = std::numeric_limits<u8>::max();
	inline constexpr u16 u16max = std::numeric_limits<u16>::max();
	inline constexpr u32 u32max = std::numeric_limits<u32>::max();
	inline constexpr u64 u64max = std::numeric_limits<u64>::max();

	inline constexpr i8 i8min = std::numeric_limits<i8>::min();
	inline constexpr i8 i8max = std::numeric_limits<i8>::max();
	inline constexpr i16 i16min = std::numeric_limits<i16>::min();
	inline constexpr i16 i16max = std::numeric_limits<i16>::max();
	inline constexpr i32 i32min = std::numeric_limits<i32>::min();
	inline constexpr i32 i32max = std::numeric_limits<i32>::max();
	inline constexpr i64 i64min = std::numeric_limits<i64>::min();
	inline constexpr i64 i64max = std::numeric_limits<i64>::max();

	inline constexpr fp32 fp32min = std::numeric_limits<fp32>::lowest();
	inline constexpr fp32 fp32max = std::numeric_limits<fp32>::max();
	inline constexpr fp64 fp64min = std::numeric_limits<fp64>::lowest();
	inline constexpr fp64 fp64max = std::numeric_limits<fp64>::max();

	// Arithmetic concepts.
	template<typename T>
	concept Arithmetic = std::is_arithmetic_v<T>;

	template<typename T>
	concept UnsignedArithmatic = std::floating_point<T> || std::unsigned_integral<T>;

	template<typename T>
	concept FloatingPoint = std::floating_point<T>;

	template<typename T>
	concept Integral = std::integral<T>;

	template<typename T>
	concept UnsignedIntegral = std::unsigned_integral<T>;
}