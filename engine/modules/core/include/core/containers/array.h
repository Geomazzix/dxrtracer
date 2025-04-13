#pragma once
#include "core/valueTypes.h"
#include <array>
#include <vector>

namespace dxray
{
	template<typename T>
	using Array = std::vector<T>;

	template<typename T, size_t Size>
	using FixedArray = std::array<T, Size>;
}