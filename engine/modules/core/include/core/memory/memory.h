#pragma once
#include <core/valueTypes.h>

namespace dxray
{
	template<Arithmetic T>
	inline T Align(const T a_value, const T a_alignment)
	{
		return (a_value + (a_alignment - static_cast<T>(1))) & ~(a_alignment - static_cast<T>(1));
	}
}