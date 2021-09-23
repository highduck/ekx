cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(bmfont-export)

add_executable(${PROJECT_NAME}
        # Common general builders:
        src/bmfont_export/bmfont_export.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type -Woverloaded-virtual -Wcast-align
            -Wno-deprecated-declarations
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            -fstrict-aliasing
            -Wstrict-aliasing
            )
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC
        -DPUGIXML_NO_XPATH)

target_link_libraries(${PROJECT_NAME}
        PUBLIC pugixml
        PUBLIC freetype
        PUBLIC stb
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(BMFONT_OS_PREFIX osx)

    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        set(EKC_COMPILE_FLAGS "-Oz -flto -DNDEBUG -fno-exceptions -fno-rtti")
        set(EKC_LINK_FLAGS "-Oz -flto -Wl,-dead_strip -Wl,-dead_strip_dylibs")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EKC_COMPILE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EKC_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EKC_LINK_FLAGS}")

    endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE shlwapi
            )
    set(BMFONT_OS_PREFIX win32)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(BMFONT_OS_PREFIX linux)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${BMFONT_OS_PREFIX}"
        RUNTIME_OUTPUT_NAME "bmfont-export"
        RUNTIME_OUTPUT_NAME_DEBUG "bmfont-export-debug"
        )