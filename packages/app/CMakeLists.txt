cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(app)

add_library(${PROJECT_NAME} STATIC "")
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_subdirectory(src)

#target_link_libraries(${PROJECT_NAME}
#        PUBLIC ekx::core
#        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

target_compile_options(${PROJECT_NAME} PRIVATE
        -g
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_subdirectory(platforms/web)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    add_subdirectory(platforms/apple)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_subdirectory(platforms/apple)
    file(GLOB_RECURSE ek_app_c_file ${CMAKE_SOURCE_DIR}/**/ek_app.c)
    message(STATUS ${ek_app_c_file})
    set_source_files_properties(${ek_app_c_file} PROPERTIES
            LANGUAGE CXX
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_subdirectory(platforms/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_subdirectory(platforms/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_subdirectory(platforms/linux)
else ()
    error("Platform: UNKNOWN")
endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()
