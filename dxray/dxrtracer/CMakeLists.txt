cmake_minimum_required(VERSION 3.27)

set(HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/dxrtracer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/window.h"
)

set(SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dxrtracer.cpp"
)

set(DEPS
	"stb"
	"core"

	"d3d12.lib"
	"dxgi.lib"
	"dxcompiler.lib"
	"d3dcompiler.lib"
)

project_add_target(
	NAME dxrtracer
	TYPE EXECUTABLE
	HEADERS ${HEADERS}
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
	PCH_ON
)