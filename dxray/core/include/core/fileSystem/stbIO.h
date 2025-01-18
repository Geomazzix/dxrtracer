#pragma once
#include "core/vath/vath.h"

namespace dxray
{
	void StorePNG(String a_fileName, i32 a_width, i32 a_height, i32 a_numChannels, vath::Vector3f* a_pColorData, bool a_bNormalizedData);
}