cmake_minimum_required(VERSION 3.19)
enable_testing()
project(ek-core-tests)
add_executable(${PROJECT_NAME}
        tests.cpp
        )
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 11
        )

target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING)
target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::core
        PUBLIC ekx::unit
        )