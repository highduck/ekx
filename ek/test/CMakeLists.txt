cmake_minimum_required(VERSION 3.12)
project(ek-test)

add_executable(${PROJECT_NAME}
        dummy_test.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions)
target_link_libraries(${PROJECT_NAME} gtest gtest_main ek)

add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${PROJECT_NAME})