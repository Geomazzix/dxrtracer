cmake_minimum_required(VERSION 3.23)

set(ENGINE_SHADER_DIRECTORY "${ENGINE_DIRECTORY}/shaders" CACHE STRING "The source directory for the engine shaders" FORCE)
message(STATUS "[Config] Engine shader directory pointing to: ${ENGINE_SHADER_DIRECTORY}")

set(SHADER_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/global.hlsli"
	"${CMAKE_CURRENT_SOURCE_DIR}/math.hlsli"
	"${CMAKE_CURRENT_SOURCE_DIR}/random.hlsli"
)

set(SHADER_SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/raytracer.rt.hlsl"
)

set(SHADER_FILES
	"${SHADER_HEADERS}"
	"${SHADER_SOURCE}"
)

add_custom_target("shaders" 
	ALL
	SOURCES "${SHADER_FILES}"
)

MESSAGE(STATUS "[module] shaders")