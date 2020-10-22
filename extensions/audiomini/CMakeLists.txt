cmake_minimum_required(VERSION 3.13)
project(audiomini)

add_library(${PROJECT_NAME} STATIC "")

add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} ek)

#target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_subdirectory(web)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    add_subdirectory(apple)
    add_subdirectory(ios)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_subdirectory(apple)
    add_subdirectory(macos)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_subdirectory(platforms/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_subdirectory(null)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_subdirectory(null)
else ()
    error("Platform: UNKNOWN")

endif ()