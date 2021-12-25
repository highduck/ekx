cmake_minimum_required(VERSION 3.19)
include(npm.cmake)

project(std C)

add_library(${PROJECT_NAME} STATIC
        src/ek_std.c
        src/ek_bitmap.c
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        )

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_compile_options(${PROJECT_NAME}
        PRIVATE -g
#        PRIVATE -flto
        PRIVATE -Os
        PRIVATE -Wall
        PRIVATE -Wextra
        PRIVATE -std=c18
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
#        PRIVATE -mstackrealign
#        PRIVATE -fshort-enums
        )

target_link_options(${PROJECT_NAME}
        PRIVATE -g
#        PRIVATE -flto
        PRIVATE -Os
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC stb
        PUBLIC sokol
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_link_libraries(${PROJECT_NAME}
            PUBLIC log
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
else ()
    error("Platform: UNKNOWN")
endif ()


#if (EKX_BUILD_TESTS)
    project(std-tests)

    add_executable(${PROJECT_NAME}
            test/main.cpp
            )

    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

    set_target_properties(${PROJECT_NAME} PROPERTIES
            C_STANDARD 11
            CXX_STANDARD 17
            )

    target_compile_options(${PROJECT_NAME}
            PUBLIC -g
            #PUBLIC -flto
            PUBLIC -Os
            PUBLIC -fno-strict-aliasing
            PUBLIC -fno-exceptions
            PUBLIC -fno-rtti
            PUBLIC -ffast-math
            PUBLIC -Wstrict-aliasing=2
#            PUBLIC -fshort-enums
            )

    target_link_libraries(${PROJECT_NAME}
            PUBLIC ekx::std
            PUBLIC doctest
            )
#endif ()