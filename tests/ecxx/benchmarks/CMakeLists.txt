cmake_minimum_required(VERSION 3.13)

project(ecxx-benchmarks)

add_executable(${PROJECT_NAME}
        ecxx_benchmark.cpp
        entt_benchmark.cpp
        common.hpp
        main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} ek-core entt)
target_compile_options(${PROJECT_NAME} PRIVATE
        -std=c++17 -Os -g
        -fno-exceptions -fno-rtti)

target_compile_definitions(${PROJECT_NAME} PUBLIC NDEBUG)
target_include_directories(${PROJECT_NAME} PRIVATE
        . # benchmark code
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
        )
