cmake_minimum_required(VERSION 3.27)

#Cmake utility setup.
message("Retrieving ${PROJECT_NAME} external dependencies")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/dxray/cmake)
include(utility/targetUtility)
include(external/findExternals)

#Now include subdirectories.
add_subdirectory("core")
add_subdirectory("riow")

#Todo: add shader directory.
#add_subdirectory(shaders)

#Testing framework setup (only if enabled)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
	message("Building ${PROJECT_NAME} unit tests")
	include(CTest)	
	enable_testing()
	add_subdirectory(tests)
endif()
