cmake_minimum_required(VERSION 3.23)

set(HEADERS
	#Root includes
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/debug.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/valueTypes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/string.h"

	#Time
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/time/stopwatch.h"

	#IO
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/fileSystem/fileIO.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/fileSystem/stbIO.h"

	#Math
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/matrix2x2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/matrix3x3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/matrix4x4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vector2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vector3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vector4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/quaternion.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vathUtility.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vathTemplate.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vath.h"
)

set(SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fileSystem/fileIO.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fileSystem/stbIO.cpp"
)

set(DEPS
	"stb"
)

project_add_target(
	NAME "core"
	TYPE STATIC
	FILTER "framework"
	HEADERS ${HEADERS}
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
	PCH_ON
)

target_compile_definitions("core"
PUBLIC
	ENGINE_ROOT_DIRECTORY="${CMAKE_SOURCE_DIR}"
	PROJECT_NAME="${CMAKE_PROJECT_NAME}"
)