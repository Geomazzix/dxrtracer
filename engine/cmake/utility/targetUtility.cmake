cmake_minimum_required(VERSION 3.27)

#Maps the given sources to a filter in MSVC
function(map_source_to_filter)
    if(MSVC)
        foreach(SOURCE_ITEM IN ITEMS ${ARGN})
            if (IS_ABSOLUTE "${SOURCE_ITEM}")
                file(RELATIVE_PATH RELATIVE_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${SOURCE_ITEM}")
            else()
                set(RELATIVE_SOURCE_PATH "${SOURCE_ITEM}")
            endif()

            get_filename_component(SOURCE_PATH "${RELATIVE_SOURCE_PATH}" PATH)
            string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
            source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE_ITEM}")
        endforeach()
    endif()
endfunction(map_source_to_filter)


#Setup of virtual folder structure within IDE
function(target_set_ide_folders)
    cmake_parse_arguments(
        TARGET
        ""
        "NAME;FILTER"
        "HEADERS;SOURCE"
        ${ARGN}
    )

    #Setup of the msvc filters.
    if(MSVC)
        map_source_to_filter(${TARGET_HEADERS})
        map_source_to_filter(${TARGET_SOURCE})

        #Append the target to the modules filter if its not an runnable instance.
        if("${TARGET_FILTER}" STREQUAL "")
            set(TARGET_FILTER "modules")
        endif()

        if(NOT "${TARGET_TYPE}" STREQUAL "EXECUTABLE")
            set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${TARGET_FILTER})
        endif()
    endif()
endfunction(target_set_ide_folders)


#Adds a non-compilable target to the project.
function(project_add_interface)
    cmake_parse_arguments(
        TARGET
        "NO_INSTALL"
        "NAME;FILTER"
        "HEADERS;LINK_DEPS"
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

    target_link_libraries(${TARGET_NAME} INTERFACE ${TARGET_LINK_DEPS})
    
    target_set_ide_folders(
        NAME ${TARGET_NAME}
        FILTER ${TARGET_FILTER}
        HEADERS ${HEADERS}
        SOURCE ${SOURCE}
    )

    MESSAGE(STATUS "[module] ${TARGET_NAME}")
endfunction(project_add_interface)


#Adds a compilable target to the project.
function(project_add_target)
    cmake_parse_arguments(
        TARGET
        "PCH_ON;NO_INSTALL"
        "NAME;TYPE;FILTER"
        "HEADERS;SOURCE;LINK_DEPS"
        ${ARGN}
    )

    #When the module is forced to be build as shared make sure to export the windows symbols.
    if("${TARGET_TYPE}" STREQUAL "SHARED")
        add_library(
            ${TARGET_NAME} 
            ${TARGET_TYPE} 
            ${TARGET_HEADERS} 
            ${TARGET_SOURCE}
        )

        #When using MSVC the cmake build type cannot be traced with CMAKE_BUILD_TYPE. - #Todo set this in the cmakepreset.
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
        add_library(
            ${TARGET_NAME} 
            ${TARGET_TYPE} 
            ${TARGET_HEADERS} 
            ${TARGET_SOURCE}
        )
    endif()
    
    target_set_ide_folders(
        NAME ${TARGET_NAME}
        FILTER ${TARGET_FILTER}
        HEADERS ${HEADERS}
        SOURCE ${SOURCE}
    )

    if(NOT "${TARGET_TYPE}" STREQUAL "EXECUTABLE")
        MESSAGE(STATUS "[module] ${TARGET_NAME}")
    else()
        MESSAGE(STATUS "[executable] ${TARGET_NAME}")
    endif()

    #Target setup.
    target_include_directories(${TARGET_NAME} 
    PUBLIC
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
	    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    )

    target_link_libraries(${TARGET_NAME} PUBLIC ${TARGET_LINK_DEPS})
    
    target_compile_definitions(${TARGET_NAME}
    PRIVATE
        #Target needs to export any API functionality if shared/module.
        SHARED_LIB_EXPORT

    	#Configs
        $<$<CONFIG:Debug>:CONFIG_DEBUG>
	    $<$<CONFIG:RelWithDebInfo>:CONFIG_DEVELOPMENT>
        $<$<CONFIG:Release>:CONFIG_RELEASE>
        
        #Platforms.
        $<$<PLATFORM_ID:Windows>:PLATFORM_WINDOWS>
        #Note: Add new platform macros here!
    )

    if(${TARGET_PCH_ON})
        target_precompile_headers(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/${TARGET_NAME}/pch.h")
    endif()
endfunction(project_add_target)