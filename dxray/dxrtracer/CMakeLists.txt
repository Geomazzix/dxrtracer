cmake_minimum_required(VERSION 3.27)

set(HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/dxrtracer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/window.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/dx12/d3d12Device.h"
)

set(SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dx12/d3d12Device.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dxrtracer.cpp"
)

set(DEPS
	"stb"
	"core"
	"d3dx12::headers"
	"d3dx12::guids"
	"d3d12"
	"dxgi"
	"dxcompiler"
	"d3dcompiler"
)

project_add_target(
	NAME dxrtracer
	TYPE EXECUTABLE
	HEADERS ${HEADERS}
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
	PCH_ON
)