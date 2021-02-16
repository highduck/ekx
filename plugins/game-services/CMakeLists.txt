cmake_minimum_required(VERSION 3.13)
project(plugin-game-services)

add_library(${PROJECT_NAME} STATIC
        src/GameServices.hpp
        src/GameServices.cpp
        null/NullGameService.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME}
        PUBLIC ek)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
