#pragma once
#include "core/string.h"

//#Todo: Implement a proper filereader/writer class, these functions are quite messy on the inside.

namespace dxray
{
	/*!
	 * @brief Pointer for a blob of memory. Currently only supports u8 encodings - i.e. Unicode or Binary.
	 * #Todo: Support multiple encodings e.g. UTF16.
	 */
	struct DataBlob
	{
		void* Data = nullptr;
		usize SizeInBytes = 0;
	};

	/*!
	 * @brief Opens a file input stream to read the file contents.
	 * @param a_filePath the file to be read
	 * @param a_bIsBinary optional flag for binary contents. Binary contents are treated as u8 within the context of a string.
	 * @return 
	 */
	String Read(const Path& a_filePath, bool a_bIsBinary = false);

	/*!
	 * @brief Opens a filestream to write non-binary content to.
	 * @param a_filePath filepath to save the contents.
	 * @param a_content the non-binary contents.
	 * @return 
	 */
	bool Write(const Path& a_filePath, const String& a_content);

	/*!
	 * @brief Opens a filestream to write the binary to.
	 * @param a_filePath filepath to save the blob.
	 * @param a_dataBlob the binary data to be stored.
	 * @return 
	 */
	bool WriteBinary(const Path& a_filePath, const DataBlob& a_dataBlob);
}