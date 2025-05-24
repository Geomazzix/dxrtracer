cmake_minimum_required(VERSION 3.27)

#Cmake utility setup and third party retrieval.
message("Retrieving ${PROJECT_NAME} external dependencies")
set(ENGINE_DIRECTORY "${PROJECT_SOURCE_DIR}/engine")
set(ENGINE_MODULE_DIRECTORY "${ENGINE_DIRECTORY}/modules")
list(APPEND CMAKE_MODULE_PATH "${ENGINE_DIRECTORY}/cmake")
include(utility/targetUtility)
include(external/findExternals)

#Define global directories - Note: Not sure about having these global, they might only be needed on the executable target.
message("Configuring the engine")
set(ENGINE_ASSET_DIRECTORY "${CMAKE_SOURCE_DIR}/samples/assets/")
set(ENGINE_CACHE_DIRECTORY "${CMAKE_BINARY_DIR}/cache")
message(STATUS "[Config] Engine Asset directory pointing to: ${ENGINE_ASSET_DIRECTORY}")
message(STATUS "[Config] Engine Cache directory pointing to: ${ENGINE_CACHE_DIRECTORY}")

#Now include subdirectories.
add_subdirectory(shaders)
add_subdirectory(modules)

#Testing framework setup (only if enabled)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
	message("Building ${PROJECT_NAME} unit tests")
	include(CTest)	
	enable_testing()
	add_subdirectory(tests)
endif()
