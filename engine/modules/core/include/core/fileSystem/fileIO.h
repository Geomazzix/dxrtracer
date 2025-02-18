#pragma once
#include "core/string.h"

//#Todo: Implement a proper filereader/writer class, these functions are quite messy on the inside.

namespace dxray
{
	String Read(const Path& a_filePath, bool a_bIsBinary = false);

	bool Write(const Path& a_filePath, const String& a_content);

	bool WriteBinary(const Path& a_filePath, const void* a_pSource, usize a_sizeInBytes);
}