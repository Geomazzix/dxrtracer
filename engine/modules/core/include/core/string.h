#pragma once
#include <string>
#include <fstream>
#include <ostream>
#include <sstream>
#include <format>

namespace dxray
{
	//8-byte fixed width (ANSI).
	using String = std::string;
	using OfStream = std::ofstream;
	using IfStream = std::fstream;
	using StringView = std::string_view;
	using StringStream = std::stringstream;
	using OStringStream = std::ostringstream;
	using IStringStream = std::istringstream;

	//File system paths.
	using Path = std::filesystem::path;

	//UNICODE (16-bit) support for WinApi/D3D12 specific functionality.
#if PLATFORM_WINDOWS
	using WString = std::wstring;
	using WOfStream = std::wofstream;
	using WIfstream = std::wifstream;
	using WStringView = std::wstring_view;
	using WStringStream = std::wstringstream;
	using WOStringStream = std::wostringstream;
	using WIStringStream = std::wistringstream;

	/*!
	 * @brief StringEncoder class holds encoding conversions specific to the Windows platform.
	 * Serves as an API, does not have a constructor nor destructor.
	 */
	class StringEncoder final
	{
	public:
		StringEncoder() = delete;
		~StringEncoder() = delete;

		static String UnicodeToUtf8(const WString& a_wideString);
		static WString Utf8ToUnicode(const String& a_narrowString);
	};
#endif
}