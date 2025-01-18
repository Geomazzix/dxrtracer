#pragma once

namespace dxray
{
	void GetFilesInDirectory(const std::filesystem::path& a_directory, std::vector<String>& a_fileBuffer);

	String Read(const std::filesystem::path& a_filePath, bool a_bIsBinary = false);

	bool Write(const std::filesystem::path& a_filePath, const String& a_content);

	bool WriteBinary(const std::filesystem::path& a_filePath, const void* a_pSource, usize a_sizeInBytes);
}