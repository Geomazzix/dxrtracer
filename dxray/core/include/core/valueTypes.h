#pragma once
#include <cstdint>
#include <cmath>
#include <limits>

namespace dxray
{
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

	using usize = std::size_t;

	constexpr u8 u8max = std::numeric_limits<u8>::max();
	constexpr u16 u16max = std::numeric_limits<u16>::max();
	constexpr u32 u32max = std::numeric_limits<u32>::max();
	constexpr u64 u64max = std::numeric_limits<u64>::max();

	constexpr i8 i8min = std::numeric_limits<i8>::min();
	constexpr i8 i8max = std::numeric_limits<i8>::max();
	constexpr i16 i16min = std::numeric_limits<i16>::min();
	constexpr i16 i16max = std::numeric_limits<i16>::max();
	constexpr i32 i32min = std::numeric_limits<i32>::min();
	constexpr i32 i32max = std::numeric_limits<i32>::max();
	constexpr i64 i64min = std::numeric_limits<i64>::min();
	constexpr i64 i64max = std::numeric_limits<i64>::max();

	constexpr fp32 fp32min = std::numeric_limits<fp32>::lowest();
	constexpr fp32 fp32max = std::numeric_limits<fp32>::max();
	constexpr fp64 fp64min = std::numeric_limits<fp64>::lowest();
	constexpr fp64 fp64max = std::numeric_limits<fp64>::max();
}