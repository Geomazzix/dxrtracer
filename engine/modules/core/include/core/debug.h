#pragma once
#include <cstdio>
#include <cassert>

#include "core/containers/string.h"

#ifndef CONFIG_RELEASE
#define DXRAY_TRACE(message, ...)		printf("trace: %s\n", std::format(message, __VA_ARGS__).c_str())
#define DXRAY_INFO(message, ...)		printf("info: %s\n", std::format(message, __VA_ARGS__).c_str())
#define DXRAY_WARN(message, ...)		printf("waring: %s\n", std::format(message, __VA_ARGS__).c_str())
#define DXRAY_ERROR(message, ...)		printf("error: %s\n", std::format(message, __VA_ARGS__).c_str())
#define DXRAY_CRITICAL(message, ...)	printf("critical: %s\n", std::format(message, __VA_ARGS__).c_str())

#define DXRAY_ASSERT(a_expr)																															\
					if(a_expr){}																														\
					else																																\
					{																																	\
						printf("%s\n", std::format("Assertion failed: {} in {} in {} at line {}.", #a_expr, __FUNCTION__, __FILE__, __LINE__).c_str());	\
						__debugbreak();																													\
					}
#define DXRAY_ASSERT_WITH_MSG(a_expr, ...)																																	\
					if(a_expr){}																																			\
					else																																					\
					{																																						\
						printf("%s\n", std::format("Assertion failed: {} in {} in {} at line {}: {}.", #a_expr, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__).c_str());	\
						__debugbreak();																																		\
					}
#else
#define DXRAY_TRACE(...)
#define DXRAY_INFO(...)
#define DXRAY_WARN(...)
#define DXRAY_ERROR(...)
#define DXRAY_CRITICAL(...)

#define DXRAY_ASSERT(a_expr) ((void)(a_expr))
#define DXRAY_ASSERT_WITH_MSG(a_expr, ...) ((void)(a_expr))
#endif //!definded CONFIG_RELEASE