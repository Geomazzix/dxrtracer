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

	//File system paths.
	using Path = std::filesystem::path;

	//UNICODE (16-bit) support for WinApi/D3D12 specific functionality.
#if PLATFORM_WINDOWS
	using WString = std::wstring;
	using WOfStream = std::wofstream;
	using WIfstream = std::wifstream;
	using WStringView = std::wstring_view;
	using WStringStream = std::wstringstream;
#endif
}