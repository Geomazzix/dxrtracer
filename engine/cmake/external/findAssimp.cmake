cmake_minimum_required(VERSION 3.27)

include_guard()
MESSAGE(STATUS "[Thirdparty] Assimp")

FetchContent_Declare(
	assimp_external
	GIT_REPOSITORY https://github.com/assimp/assimp.git
	GIT_TAG "213d73ebc6a0ab64b880bf144aefa2c2f6092cb6"
	GIT_PROGRESS TRUE
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp_external)

set_target_properties(assimp PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/assimp")
set_target_properties(zlibstatic PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/assimp")
set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/assimp")