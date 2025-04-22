cmake_minimum_required(VERSION 3.23)

set(HEADERS
	#Root includes
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/debug.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/valueTypes.h"

	#Time
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/time/stopwatch.h"

	#Threading
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/thread/taskScheduler.h"

	#IO
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/fileSystem/fileIO.h"

	#Containers
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/containers/array.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/containers/sparseSet.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/containers/sparseArray.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/containers/string.h"

	#Math
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/matrix2x2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/matrix3x3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/matrix4x4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vector2.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vector3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vector4.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/quaternion.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vathUtility.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/rect.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vathTemplate.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/core/vath/vath.h"
)

set(SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fileSystem/fileIO.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/thread/taskScheduler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/winApiString.cpp"
)

project_add_target(
	NAME "core"
	TYPE STATIC
	FILTER "${MODULES_FILTER}"
	HEADERS ${HEADERS}
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
	PCH_ON
)

target_compile_definitions("core"
PUBLIC
	ENGINE_ROOT_DIRECTORY="${CMAKE_SOURCE_DIR}"
	PROJECT_NAME="${CMAKE_PROJECT_NAME}"
	ENGINE_ASSET_DIRECTORY="${ENGINE_ASSET_DIRECTORY}"
	ENGINE_SHADER_DIRECTORY="${ENGINE_SHADER_DIRECTORY}"
	ENGINE_CACHE_DIRECTORY="${ENGINE_CACHE_DIRECTORY}"
)