cmake_minimum_required(VERSION 3.27)

include(GNUInstallDirs)
include(FetchContent)

#Setup the fetch content.
set(THIRD_PARTY_FOLDER "thirdParty") #Defines the thirdparty filter root.
set(THIRD_PARTY_DIRECTORY "${ENGINE_DIRECTORY}/${THIRD_PARTY_FOLDER}")
list(APPEND CMAKE_MODULE_PATH "${ENGINE_DIRECTORY}/cmake/external")
file(MAKE_DIRECTORY "${THIRD_PARTY_DIRECTORY}")
set(FETCHCONTENT_BASE_DIR "${THIRD_PARTY_DIRECTORY}" CACHE PATH "Third party dependency directory" FORCE)

#Find global cmake thirdparty dependencies - platforms should be checked in this file.
find_package(Threads REQUIRED)
include(findStb)
include(findD3dx12)
include(findDxc)

if(BUILD_TESTS)
	include(findGoogleTest)
endif()