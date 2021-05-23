cmake_minimum_required(VERSION 3.13)
project(ek-core-tests)

add_executable(${PROJECT_NAME}
        tests.cpp
        ecxx/common/identity_generator_test_1.cpp
        ecxx/common/identity_generator_test_2.cpp
        )

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
        PUBLIC ek-core
        PUBLIC doctest
        )