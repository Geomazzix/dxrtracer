cmake_minimum_required(VERSION 3.27)

set(HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/window.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/shaderCompiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/modelLoader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/uploadBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/renderPass.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/accelerationStructure.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/scene.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/dxrtracer/renderer.h"
)

set(SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dxShaderCompiler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/modelLoader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/uploadBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/renderPass.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/accelerationStructure.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/scene.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/renderer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dxrtracer.cpp"
)

set(DEPS
	"assimp"
	"stb"
	"core"
	"d3dx12::headers"
	"d3dx12::guids"
	"d3d12"
	"dxgi"
	"dxShaderCompiler"
)

project_add_target(
	NAME dxrtracer
	TYPE EXECUTABLE
	HEADERS ${HEADERS}
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
	PCH_ON
)