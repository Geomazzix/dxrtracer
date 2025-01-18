#pragma once

//C
#include <cstdlib>

//Global accessible defines, used throughout the whole framework.
#include "core/string.h"		//Takes care of <string>
#include "core/valueTypes.h"	//Defines mathematical values/
#include "core/debug.h"			//Debug logging/asserts.

//Memory and containers.
#include <memory>
#include <format>
#include <filesystem>
#include <iterator>
#include <unordered_map>
#include <array>
#include <vector>

//Undefine any overlapping functionality from 3rd party dependencies/platforms - colliding with winapi definitions, future-proof.
#if defined(max)
#undef max
#endif

#if defined(min)
#undef min
#endif