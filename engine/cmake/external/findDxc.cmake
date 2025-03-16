cmake_minimum_required(VERSION 3.27)
include_guard()

message(STATUS "[Thirdparty] DirectX Shader Compiler")

#1. Download the specified release - update version here if required.
set(DXC_VERSION v1.8.2502)
set(DXC_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${DXC_VERSION}/dxc_2025_02_20.zip")
set(DXC_DIRECTORY "${THIRD_PARTY_DIRECTORY}/dxc/${DXC_VERSION}")
set(DXC_ARCHIVE "${DXC_DIRECTORY}/dxc.zip")

if(EXISTS ${DXC_DIRECTORY})
    file(DOWNLOAD "${DXC_URL}" "${DXC_ARCHIVE}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf "${DXC_ARCHIVE}" 
        WORKING_DIRECTORY "${THIRD_PARTY_DIRECTORY}/dxc/${DXC_VERSION}"
    )
    file(REMOVE "${DXC_ARCHIVE}")
endif()

#2. Internal project setup - #Todo: add imported target support in to the target utility, for now hardcoded.
add_library(dxShaderCompiler::dxc SHARED IMPORTED GLOBAL)
set_target_properties(dxShaderCompiler::dxc PROPERTIES
    IMPORTED_LOCATION "${DXC_DIRECTORY}/bin/x64/dxcompiler.dll"
    IMPORTED_IMPLIB "${DXC_DIRECTORY}/lib/x64/dxcompiler.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${DXC_DIRECTORY}/inc"
)

add_library(dxShaderCompiler::dxil SHARED IMPORTED GLOBAL)
set_target_properties(dxShaderCompiler::dxil PROPERTIES
    IMPORTED_LOCATION "${DXC_DIRECTORY}/bin/x64/dxil.dll"
    IMPORTED_IMPLIB "${DXC_DIRECTORY}/lib/x64/dxil.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${DXC_DIRECTORY}/inc"
)

add_library(dxShaderCompiler INTERFACE IMPORTED GLOBAL)
set_property(TARGET dxShaderCompiler
    PROPERTY INTERFACE_LINK_LIBRARIES 
        dxShaderCompiler::dxc 
        dxShaderCompiler::dxil
)

#3. Ensure the dynamic libraries are copied over to the application directory.
add_custom_target(dxc_copy ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${DXC_DIRECTORY}/bin/x64/dxil.dll"
    "${DXC_DIRECTORY}/bin/x64/dxcompiler.dll"
    $<$<CONFIG:debug>:"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/debug">
    $<$<CONFIG:development>:"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/development">
    $<$<CONFIG:release>:"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/release">
)
set_target_properties(dxc_copy PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/directx")
add_dependencies(dxShaderCompiler dxc_copy)