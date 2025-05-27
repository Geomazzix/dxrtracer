cmake_minimum_required(VERSION 3.27)

# Maps provided files to a filter within the supported IDEs.
function(map_files_to_filter)
    cmake_parse_arguments(
        SOURCE
        ""
        "STRIP_PREFIX;FILTER_ROOT"
        "FILES"
        ${ARGN}
    )

    if (CMAKE_GENERATOR MATCHES "Visual Studio")
        foreach(FILE_PATH IN ITEMS ${SOURCE_FILES})
            if (IS_ABSOLUTE "${FILE_PATH}")
                file(RELATIVE_PATH RELATIVE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_STRIP_PREFIX}" "${FILE_PATH}")
            else()
                set(RELATIVE_FILE_PATH "${FILE_PATH}")
            endif()

            get_filename_component(FILTER_PATH "${RELATIVE_FILE_PATH}" DIRECTORY)           
            string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_FILTER_ROOT}/${FILTER_PATH}")
            source_group("${SOURCE_PATH_MSVC}" FILES "${FILE_PATH}")
        endforeach()
    else()
        message(FATAL_ERROR "The currently defined IDE does not have an implementation for source mappings. Ensure to define behaviour for this IDE.")
    endif()
endfunction(map_files_to_filter)


# Maps target files into folders within supported IDEs.
function(map_target_to_filter)
    cmake_parse_arguments(
        TARGET
        ""
        "NAME;FILTER"
        "HEADERS;SOURCE"
        ${ARGN}
    )

    # Setup of the msvc filters.
    if (CMAKE_GENERATOR MATCHES "Visual Studio")
        map_files_to_filter(
            STRIP_PREFIX "include/${TARGET_NAME}"
            FILTER_ROOT "Header Files"
            FILES "${TARGET_HEADERS}"
        )
        map_files_to_filter(
            STRIP_PREFIX "src" 
            FILTER_ROOT "Source Files"
            FILES "${TARGET_SOURCE}"
        )

        # Append the target to the modules filter if its not a runnable instance.
        if("${TARGET_FILTER}" STREQUAL "")
            set(TARGET_FILTER "modules")
        endif()

        if(NOT "${TARGET_TYPE}" STREQUAL "EXECUTABLE")
            set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${TARGET_FILTER})
        endif()
    endif()
endfunction(map_target_to_filter)


# Adds a non-compilable target to the project.
function(project_add_interface)
    cmake_parse_arguments(
        TARGET
        "NO_INSTALL"
        "NAME;FILTER"
        "HEADERS;FILES;LINK_DEPS"
        ${ARGN}
    )
  
    add_library(
        ${TARGET_NAME} 
        INTERFACE
        ${TARGET_HEADERS}
    )

    target_include_directories(${TARGET_NAME} 
    INTERFACE
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
	    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    )

    target_link_libraries(${TARGET_NAME} 
    INTERFACE 
        ${TARGET_LINK_DEPS}
    )

    # Todo: Add source file mapping as this is currently hardcoded for Header files and Source files.

    message(STATUS "[module] ${TARGET_NAME}")
endfunction(project_add_interface)


# Adds a compilable target to the project.
function(project_add_target)
    cmake_parse_arguments(
        TARGET
        "PCH_ON;NO_INSTALL"
        "NAME;TYPE;FILTER"
        "HEADERS;SOURCE;LINK_DEPS"
        ${ARGN}
    )

    # When the module is forced to be build as shared make sure to export the windows symbols.
    if("${TARGET_TYPE}" STREQUAL "SHARED")
        add_library(
            ${TARGET_NAME} 
            ${TARGET_TYPE} 
            ${TARGET_HEADERS} 
            ${TARGET_SOURCE}
        )

        # When using MSVC the cmake build type cannot be traced with CMAKE_BUILD_TYPE. - #Todo set this in the cmakepreset.
        if (MSVC AND WIN32 AND ${CMAKE_CONFIGURATION_TYPES} STREQUAL "Debug")
            set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
        endif()
    elseif("${TARGET_TYPE}" STREQUAL "EXECUTABLE")
        add_executable(
            ${TARGET_NAME}
            ${TARGET_HEADERS}
            ${TARGET_SOURCE}
        )
    else()
        # If none of the above is selected it means the target is either a static/module target. These can both be added in a similar manner.
        add_library(
            ${TARGET_NAME} 
            ${TARGET_TYPE} 
            ${TARGET_HEADERS} 
            ${TARGET_SOURCE}
        )
    endif()
    
    map_target_to_filter(
        NAME ${TARGET_NAME}
        FILTER ${TARGET_FILTER}
        HEADERS ${TARGET_HEADERS}
        SOURCE ${TARGET_SOURCE}
    )

    if(NOT "${TARGET_TYPE}" STREQUAL "EXECUTABLE")
        message(STATUS "[module] ${TARGET_NAME}")
    else()
        message(STATUS "[executable] ${TARGET_NAME}")
    endif()

    # Target setup.
    target_include_directories(${TARGET_NAME} 
    PUBLIC
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
	    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    )

    target_link_libraries(${TARGET_NAME} 
    PUBLIC 
        ${TARGET_LINK_DEPS}
    )
    
    target_compile_definitions(${TARGET_NAME}
    PRIVATE
        # Target needs to export any API functionality if shared/module.
        SHARED_LIB_EXPORT

    	# Configs
        $<$<CONFIG:Debug>:CONFIG_DEBUG>
	    $<$<CONFIG:RelWithDebInfo>:CONFIG_DEVELOPMENT>
        $<$<CONFIG:Release>:CONFIG_RELEASE>
        
        # Platforms.
        $<$<PLATFORM_ID:Windows>:PLATFORM_WINDOWS>
        # Note: Add new platform macros here!
    )

    if(${TARGET_PCH_ON})
        target_precompile_headers(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/${TARGET_NAME}/pch.h")
    endif()
endfunction(project_add_target)