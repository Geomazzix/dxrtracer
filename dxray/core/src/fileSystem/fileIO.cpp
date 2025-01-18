#include "core/fileSystem/fileIO.h"

namespace dxray
{
	void GetFilesInDirectory(const std::filesystem::path& a_directory, std::vector<String>& a_fileBuffer)
	{
		if (!std::filesystem::exists(a_directory))
		{
			DXRAY_ERROR("Could not find: {}", a_directory.string());
			return;
		}

		std::vector<std::filesystem::path> filePaths;
		for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(a_directory))
		{
			filePaths.push_back(file);
		}

		for (const std::filesystem::path& file : filePaths)
		{
			a_fileBuffer.push_back(file.filename().string());
		}
	}

	String Read(const std::filesystem::path& a_filePath, bool a_bIsBinary /*= false*/)
	{
		String content("");
		IfStream fileStream(a_filePath, a_bIsBinary
			? IfStream::in | IfStream::binary
			: IfStream::in);

		if (!fileStream)
		{
			return content;
		}

		StringStream contentStream;
		contentStream << fileStream.rdbuf();
		content = contentStream.str();
		fileStream.close();
		return content;
	}

	bool Write(const std::filesystem::path& a_filePath, const String& a_content)
	{
		if (std::filesystem::exists(a_filePath))
		{
			DXRAY_INFO("Overwriting existing file: {}", a_filePath.string());
		}

		OfStream fileStream(a_filePath);
		if (!fileStream.is_open())
		{
			DXRAY_WARN("Failed to open file for write: {}", a_filePath.string());
			return false;
		}

		fileStream.write(a_content.c_str(), a_content.size());
		fileStream.close();
		return true;
	}

	bool WriteBinary(const std::filesystem::path& a_file, const void* a_pSource, usize a_sizeInBytes)
	{
		OfStream fileStream(a_file, std::ios::out | std::ios::binary);
		if (!fileStream.is_open())
		{
			DXRAY_ERROR("Failed to open file for write: {}.", a_file.string());
			return false;
		}

		fileStream.write(static_cast<const char*>(a_pSource), a_sizeInBytes);
		fileStream.flush();
		fileStream.close();
		return true;
	}
}