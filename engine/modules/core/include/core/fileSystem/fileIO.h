#pragma once
#include "core/containers/string.h"

//#Todo: Implement a wrapper class for file system handling. The current approach breaks the decoupling of shaders from the core...

namespace dxray
{
	/*!
	 * @brief Pointer for a blob of memory. Currently only supports u8 encodings - i.e. Unicode or Binary.
	 * #Todo: Support multiple encodings e.g. UTF16.
	 */
	struct DataBlob
	{
		const void* Data = nullptr;
		usize SizeInBytes = 0;
	};

	/**
	 * @brief Wildcards come in the form of [<type>:<module>] - i.e. [source:core].
	 * @param a_wildCardFilePath [<type>:<module>] - i.e. [source:core]
	 * @return A raw valid path pointing to the provided wild card location.
	 */
	Path ResolveWildCard(const String& a_wildCardFilePath);

	/*!
	 * @brief Opens a file input stream to read the file contents.
	 * @param a_filePath the file to be read
	 * @param a_bIsBinary optional flag for binary contents. Binary contents are treated as u8 within the context of a string.
	 * @return 
	 */
	String ReadFile(const Path& a_filePath, bool a_bIsBinary = false);

	/*!
	 * @brief Opens a filestream to write non-binary content to.
	 * @param a_filePath filepath to save the contents.
	 * @param a_content the non-binary contents.
	 * @return 
	 */
	bool WriteFile(const Path& a_filePath, const String& a_content);

	/*!
	 * @brief Opens a filestream to write the binary to.
	 * @param a_filePath filepath to save the blob.
	 * @param a_dataBlob the binary data to be stored.
	 * @return 
	 */
	bool WriteBinaryFile(const Path& a_filePath, const DataBlob& a_dataBlob);
}