#include "core/containers/string.h"

namespace dxray
{
	String StringEncoder::UnicodeToUtf8(const WString& a_wideString)
	{
		DXRAY_ASSERT(!a_wideString.empty());
		const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, a_wideString.data(), static_cast<int>(a_wideString.length()), NULL, 0, NULL, NULL);
		String utf8String(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, a_wideString.data(), static_cast<int>(a_wideString.length()), utf8String.data(), sizeNeeded, NULL, NULL);
		return utf8String;
	}

	WString StringEncoder::Utf8ToUnicode(const String& a_narrowString)
	{
		DXRAY_ASSERT(!a_narrowString.empty());
		const int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, a_narrowString.data(), static_cast<int>(a_narrowString.length()), NULL, 0);
		WString mbString(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, a_narrowString.data(), static_cast<int>(a_narrowString.length()), mbString.data(), sizeNeeded);
		return mbString;
	}
}