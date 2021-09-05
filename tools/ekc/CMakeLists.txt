cmake_minimum_required(VERSION 3.13)
include(npm.cmake)
project(ekc)

add_executable(${PROJECT_NAME}
        src/cli_main.cpp

        src/ek/system/system.cpp
        src/ek/system/system.hpp
        src/ek/system/working_dir.cpp
        src/ek/system/working_dir.hpp
        src/ek/bmfont.hpp
        src/ek/flash.hpp
        src/ek/atlas.hpp
        src/ek/obj.hpp
        src/fast_obj.h)

target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_compile_definitions(${PROJECT_NAME} PUBLIC EK_TOOLS)
target_compile_definitions(ek-core PUBLIC EK_TOOLS)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g -Os)
endif ()

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::flash-export
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EKC_OS_PREFIX osx)

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
    set(EKC_OS_PREFIX win32)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EKC_OS_PREFIX linux)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${EKC_OS_PREFIX}"
        RUNTIME_OUTPUT_NAME "ekc"
        RUNTIME_OUTPUT_NAME_DEBUG "ekc-debug"
        )