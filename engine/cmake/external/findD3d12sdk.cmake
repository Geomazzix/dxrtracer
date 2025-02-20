cmake_minimum_required(VERSION 3.27)

include_guard()
MESSAGE(STATUS "[Thirdparty] D3d12Sdk")

# Find the Win10 SDK path. #Note: Only works when generating for visual studio - e.g. Ninja doesn't set the ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}.
if ("$ENV{WIN10_SDK_PATH}$ENV{WIN10_SDK_VERSION}" STREQUAL "")
    get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
    set (WIN10_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
else()
    set (WIN10_SDK_PATH $ENV{WIN10_SDK_PATH})
    set (WIN10_SDK_VERSION $ENV{WIN10_SDK_VERSION})
endif()

#Note: Reason for DXIL to be explicitly copied over is because it doesn't have an import library - so either copy it over or find a way to point to dll location to load.
add_custom_target(copy_dxil ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${WIN10_SDK_PATH}/bin/${WIN10_SDK_VERSION}/x64/dxil.dll"
    $<$<CONFIG:debug>:"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/debug">
    $<$<CONFIG:development>:"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/development">
    $<$<CONFIG:release>:"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/release">
)
set_target_properties(copy_dxil PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/directx")