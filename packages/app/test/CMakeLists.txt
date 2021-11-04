cmake_minimum_required(VERSION 3.13)
project(ek-app-test)

add_executable(${PROJECT_NAME}
        test.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti -fno-exceptions)
target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_NO_EXCEPTIONS)
target_link_libraries(${PROJECT_NAME}
        PUBLIC doctest
        PUBLIC ekx::app)

add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${PROJECT_NAME})