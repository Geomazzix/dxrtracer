cmake_minimum_required(VERSION 3.23)

# Note: Difference between shared and source is shared containing bridging files/almost always wished to be included, while source is strictly internal to the shader
# files not to be touched outside that directory, apart from being treated as assets.
set(ENGINE_SHADER_SHARED_DIRECTORY "${ENGINE_DIRECTORY}/shaders/include" CACHE STRING "The source directory for the engine shaders" FORCE)
message(STATUS "[Config] Engine shared shader directory pointing to: ${ENGINE_SHADER_DIRECTORY}")
set(ENGINE_SHADER_DIRECTORY "${ENGINE_DIRECTORY}/shaders/src" CACHE STRING "The source directory for the engine shaders" FORCE)
message(STATUS "[Config] Engine shader directory pointing to: ${ENGINE_SHADER_DIRECTORY}")

set(SHARED_FILES
	""
)

set(SHADER_FILES
	"${SHARED_FILES}"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/raytracer.rt.hlsl"
)

project_add_interface(
	NAME "shaders"
	# add filter here once configured
	HEADERS "${SHADER_FILES}"
)

target_include_directories("shaders"
INTERFACE
	"${ENGINE_SHADER_SHARED_DIRECTORY}"
)