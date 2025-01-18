#pragma once
#include <string>
#include <fstream>
#include <ostream>
#include <sstream>

//#Todo: When starting with WINAPI UTF16 support will be needed.

namespace dxray
{
	//8-byte fixed width (ANSI).
	using String = std::basic_string<char>;
	using OfStream = std::ofstream;
	using IfStream = std::fstream;
	using StringView = std::basic_string_view<char>;
	using StringStream = std::basic_stringstream<char>;
}