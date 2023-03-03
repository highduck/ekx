cmake_minimum_required(VERSION 3.19)
project(ekc)

add_executable(${PROJECT_NAME}
        src/main.c
        src/sprite_packer/sprpk_image.c
        src/sprite_packer/binpack.c
        src/sprite_packer/sprite_packer.c
        src/bmfont_export/bmfont_export.c
        src/obj-export/obj_export.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type -Woverloaded-virtual -Wcast-align
            -Wno-deprecated-declarations
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            -fno-strict-aliasing
            -Wstrict-aliasing=2
            )
endif ()

target_link_libraries(${PROJECT_NAME}
        PUBLIC miniz
        PUBLIC freetype
        PUBLIC ekx::std
        PUBLIC ekx::sg-file
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        )


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g")
    else ()
        set(EKC_COMPILE_FLAGS "-Os -flto -DNDEBUG -fno-exceptions -fno-rtti -march=native")
        set(EKC_LINK_FLAGS "-Os -flto -Wl,-dead_strip -Wl,-dead_strip_dylibs")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EKC_COMPILE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EKC_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EKC_LINK_FLAGS}")

    endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE shlwapi
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME}
            PUBLIC pthread)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Os)
    endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../cache/bin"
        RUNTIME_OUTPUT_NAME "ekc"
        RUNTIME_OUTPUT_NAME_DEBUG "ekc-debug"
        )