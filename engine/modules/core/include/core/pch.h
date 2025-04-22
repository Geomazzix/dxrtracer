#pragma once

//C
#include <cstdlib>

//Global accessible defines, used throughout the whole framework.
#include "core/containers/string.h"		//Takes care of <string>
#include "core/valueTypes.h"			//Defines mathematical values/
#include "core/debug.h"					//Debug logging/asserts.

//Memory and containers.
#include <memory>
#include <format>
#include <filesystem>
#include <iterator>
#include <unordered_map>
#include <array>
#include <vector>

#if PLATFORM_WINDOWS

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#endif //_MSC_VER

#define NOMINMAX
#include <windows.h>
#include <wrl.h>
#include <shellapi.h>

#endif //PLATFORM_WINDOWS