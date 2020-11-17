cmake_minimum_required(VERSION 3.13)
project(scenex)

add_library(${PROJECT_NAME} STATIC "")

add_subdirectory(ek/graphics)
add_subdirectory(ek/draw2d)
add_subdirectory(ek/scenex)
add_subdirectory(ek/goodies)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        #UNITY_BUILD ON
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

if()
    target_compile_definitions(${PROJECT_NAME} PUBLIC EK_EDITOR)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wno-dollar-in-identifier-extension
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR
            ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -flto)
    endif ()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_link_libraries(${PROJECT_NAME} -Wl,--whole-archive ek -Wl,--no-whole-archive)
else ()
    target_link_libraries(${PROJECT_NAME} ek)
endif ()