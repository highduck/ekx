cmake_minimum_required(VERSION 3.13)
project(ek-tests)

add_executable(${PROJECT_NAME}
        tests.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti)
target_link_libraries(${PROJECT_NAME}
        PUBLIC doctest
        PUBLIC ek)

add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${PROJECT_NAME})