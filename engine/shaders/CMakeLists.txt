cmake_minimum_required(VERSION 3.23)

set(ENGINE_SHADER_DIRECTORY "${ENGINE_DIRECTORY}/shaders" CACHE STRING "The source directory for the engine shaders" FORCE)

set(SHADER_FILES
	#Raytrace shaders
	"${ENGINE_SHADER_DIRECTORY}/raytracer.rt.hlsl"
)

project_add_interface(
	NAME "shaders"
	FILTER "${MODULES_FILTER}"
	HEADERS "${SHADER_FILES}"
)

target_include_directories("shaders"
INTERFACE
	"${ENGINE_SHADER_DIRECTORY}"
)