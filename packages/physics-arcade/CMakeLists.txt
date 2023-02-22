cmake_minimum_required(VERSION 3.19)
project(physics-arcade)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} ekx::ecs)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
