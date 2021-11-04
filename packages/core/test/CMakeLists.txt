cmake_minimum_required(VERSION 3.19)
project(ek-core-tests)

add_executable(${PROJECT_NAME}
        tests.cpp
        )

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 17
        )

target_compile_options(${PROJECT_NAME} PRIVATE
#        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::core
        PUBLIC doctest
        )