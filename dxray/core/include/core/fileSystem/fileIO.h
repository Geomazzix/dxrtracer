#pragma once
#include "core/string.h"

namespace dxray
{
	void GetFilesInDirectory(const Path& a_directory, std::vector<String>& a_fileBuffer);

	String Read(const Path& a_filePath, bool a_bIsBinary = false);

	bool Write(const Path& a_filePath, const String& a_content);

	bool WriteBinary(const Path& a_filePath, const void* a_pSource, usize a_sizeInBytes);
}