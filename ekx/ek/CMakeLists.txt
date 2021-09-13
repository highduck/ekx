cmake_minimum_required(VERSION 3.19)
project(ek)

add_library(${PROJECT_NAME} STATIC "")

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ek-core
        PUBLIC ek-app
        PUBLIC auph
        PUBLIC sokol
        )

if (EKX_BUILD_DEV_TOOLS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SOKOL_TRACE_HOOKS)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

target_compile_options(${PROJECT_NAME} PRIVATE
        -g
        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

else ()

endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()
