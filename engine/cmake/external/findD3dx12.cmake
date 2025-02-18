cmake_minimum_required(VERSION 3.27)

include_guard()
MESSAGE(STATUS "[Thirdparty] DirectXHeaders")

FetchContent_Declare(
	d3dx12_external
	GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
	GIT_TAG "27d63115c1037d0e4e119085f34345bd9ecf7ada"
	GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(d3dx12_external)

set_target_properties(DirectX-Guids PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/directx")
set_target_properties(DirectX-Headers PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/directx")

#Alias the targets.
add_library(d3dx12::headers ALIAS DirectX-Headers)
add_library(d3dx12::guids ALIAS DirectX-Guids)