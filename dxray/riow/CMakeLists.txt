cmake_minimum_required(VERSION 3.27)

set(HEADERS
	#Raytraceables
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/traceable/raytraceable.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/traceable/sphere.h"

	#Application
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/color.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/material.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/scene.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/camera.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/ray.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/renderer.h"
)

set(SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/traceable/sphere.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/src/scene.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/camera.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/renderer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/riow.cpp"
)

set(DEPS
	"core"
)

project_add_target(
	NAME riow
	TYPE EXECUTABLE
	HEADERS ${HEADERS}
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
	PCH_ON
)