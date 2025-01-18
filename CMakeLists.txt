cmake_minimum_required(VERSION 3.27)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#Project options - setting default cmake options as well.
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	option(BUILD_SHARED_LIBS "Builds Eruption as a shared library" OFF)
	option(BUILD_TESTS "Enable unit testing" ON)
	#option(BUILD_DOCUMENTATION "Build documentation" OFF)
endif()

#Project setup.
set(PROJECT_NAME dxrRaytracer)
message("Creating project [${PROJECT_NAME}] working environment with toolchain: ${CMAKE_TOOLCHAIN_FILE}")

#Hardcoded version for now - not really important to keept track of as this is not a commercially released product.
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

project(
	${PROJECT_NAME}
	VERSION ${PROJECT_VERSION}
	DESCRIPTION "DXR raytracer is a self-study primarily focused on building a solid foundation on the state-of-the-art ray-tracing techniques."
	HOMEPAGE_URL https://github.com/Geomazzix/DXR-Raytracer.git
	LANGUAGES C CXX
)

#Set the build directories for all output to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

#Global includes.
add_subdirectory(dxray)