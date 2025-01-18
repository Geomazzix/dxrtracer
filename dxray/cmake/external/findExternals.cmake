cmake_minimum_required(VERSION 3.27)

include(GNUInstallDirs)
include(FetchContent)

#Setup the fetch content.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/dxray/cmake/external)
set(THIRD_PARTY_FOLDER "thirdParty")
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/dxray/thirdparty)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/dxray/thirdparty CACHE PATH "Third party dependency directory" FORCE)

#Find global cmake thirdparty dependencies - platforms should be checked in this file.
find_package(Threads REQUIRED)
include(findStb)

if(BUILD_TESTS)
	include(findGoogleTest)
endif()