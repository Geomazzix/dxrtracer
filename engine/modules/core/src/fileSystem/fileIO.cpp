#include "core/fileSystem/fileIO.h"

namespace dxray
{
	String ReadFile(const Path& a_filePath, bool a_bIsBinary /*= false*/)
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

	bool WriteFile(const Path& a_filePath, const String& a_content)
	{
		if (a_filePath.has_parent_path())
		{
			const Path directoryPath = a_filePath.parent_path();
			if (!std::filesystem::exists(directoryPath))
			{
				DXRAY_ASSERT(std::filesystem::create_directories(directoryPath));
				DXRAY_WARN("Writing binary to newly existing directory: {}", directoryPath.string());
			}
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

	bool WriteBinaryFile(const Path& a_filePath, const DataBlob& a_dataBlob)
	{
		if (a_filePath.has_parent_path())
		{
			const Path directoryPath = a_filePath.parent_path();
			if (!std::filesystem::exists(directoryPath))
			{
				DXRAY_ASSERT(std::filesystem::create_directories(directoryPath));
				DXRAY_WARN("Writing binary to newly existing directory: {}", directoryPath.string());
			}
		}

		OfStream fileStream(a_filePath, std::ios::out | std::ios::binary);
		if (!fileStream.is_open())
		{
			DXRAY_ERROR("Failed to open file for write: {}.", a_filePath.string());
			return false;
		}

		fileStream.write(static_cast<const char*>(a_dataBlob.Data), static_cast<std::streamsize>(a_dataBlob.SizeInBytes));
		fileStream.flush();
		fileStream.close();
		return true;
	}
}